#include "vm.h"
#include "arch.h"
#include "ipc.h"
#include "memory.h"
#include "common.h"
#include "message.h"
#include "module.h"
#include "task.h"
#include <stdint.h>

struct local_variable *create_local_variable(valtype ty) {
    struct local_variable *val = malloc(sizeof(struct local_variable));
    *val = (struct local_variable) {
        .ty         = ty,
        .val        = 0
    };

    return val;
}

struct wasm_func * create_imported_func(module *m, int idx) {
    import *info = m->importsec->imports.x[idx];
    functype *ty = m->typesec->functypes.x[info->d->idx];

    int num_params = ty->rt1->n;
    struct wasm_func *wasmf = malloc(sizeof(struct wasm_func) + sizeof(struct local_variable *) * num_params);
    *wasmf = (struct wasm_func) {
        .ty         = ty,
        .imported   = true,
        .modName    = info->mod,
        .name       = info->nm
    };

    // create local variables
    for(int i = 0; i < num_params; i++) {
        wasmf->locals[i] = create_local_variable(*(ty->rt1->x[i]));
    }

    return wasmf;
}

struct wasm_func *create_defined_func(module *m, int idx) {
    int typeIdx = *m->funcsec->typeidxes.x[idx];

    func *f = m->codesec->codes.x[idx]->code;
    functype *ty = m->typesec->functypes.x[typeIdx];
 
    // count local variables
    int num_params = ty->rt1->n;

    int num_local = num_params;
    for(int i = 0; i < f->locals.n; i++) {
        num_local += f->locals.x[i]->num;
    }

    struct wasm_func *wasmf = malloc(sizeof(struct wasm_func) + sizeof(struct local_variable *) * num_local);
    wasmf->ty = ty;
    wasmf->codes = &f->expr;

    // create local variables
    for(int i = 0; i < num_params; i++) {
        wasmf->locals[i] = create_local_variable(*(ty->rt1->x[i]));
    }

    int localIdx = num_params;
    locals *locals;
    for(int i = 0; i < f->locals.n; i++) {
        locals = f->locals.x[i];
        for(uint32_t j = 0; j < locals->num; j++) {
            wasmf->locals[localIdx++] = create_local_variable(locals->ty);
        }
    }

    return wasmf;
}

void enter_block(struct context *ctx, instr *instr) {
    //puts("[+] enter block");
    list_push_back(&ctx->blocks, &instr->link_block);
}

void exit_block(struct context *ctx) {
    //puts("[+] exit block");
    list_pop_tail(&ctx->blocks);
}

void enter_frame(struct context *ctx, struct wasm_func *f) {
    //puts("[+] enter frame");
    list_push_back(&ctx->call_stack, &f->link);
}

void exit_frame(struct context *ctx) {
    //puts("[+] exit frame");
    list_pop_tail(&ctx->call_stack);
}

instr * end(struct context *ctx) {
    // taget block
    list_elem_t *block = list_tail(&ctx->blocks);

    if(!block)
        return NULL;

    instr *i = LIST_CONTAINER(block, instr, link_block);

    switch(i->op) {
        case Call:
            exit_block(ctx);
            exit_frame(ctx);
            return LIST_CONTAINER(
                i->link.next,
                instr,
                link
            );
        
        case If:
            exit_block(ctx);
            return LIST_CONTAINER(
                i->link.next,
                instr,
                link
            );
        
        // If the end instruction of the loop block is executed, the next instruction is returned.
        case Block:
        case Loop:
            exit_block(ctx);
            return LIST_CONTAINER(
                i->link.next,
                instr,
                link
            );

        // unexpected block
        default:
            return NULL;
    }
}

instr * branch_in(struct context *ctx, int idx) {
    if(list_tail(&ctx->blocks) == NULL) {
        return NULL;
    }

    // todo: check block`s existence

    // exit blocks
    while(idx--) {
        exit_block(ctx);
    }

    // taget block
    list_elem_t *block = list_tail(&ctx->blocks);
    instr *i = LIST_CONTAINER(block, instr, link_block);

    switch(i->op) {

        // todo: support if block?

        // If the loop block is jumped to by a br or br_if instruction, the first instruction is returned.
        case Loop:
            return LIST_CONTAINER(
                list_head(&i->block.in),
                instr,
                link
            );
        
        case Block: {
            exit_block(ctx);

            return LIST_CONTAINER(
                i->link.next,
                instr,
                link
            );
        }

        // unexpected block
        default:
            return NULL;
    }
}

static void print_instr(instr *i) {
    switch(i->op) {
        case Unreachable:
            puts("unreachable");
            break;
        
        case Nop:
            puts("nop");
            break;
        
        case Block:
        case Loop:
            printf("%s\n", i->op == Block? "block" : "loop");
            break;
        
        case If:
            printf("if %x\n", i->If.bt);
            break;

        case End:
            puts("end");
            break;
        
        case Br:
        case BrIf:
            printf(
                "%s %x\n", 
                i->op == Br? "br" : "br_if", 
                i->br.l
            );
            break;

        case Return:
            puts("return");
            break;
        
        case Call:
            printf("call %x\n", i->call.idx);
            break;
        
        case Drop:
            puts("drop");
            break;
        
        case LocalGet:
            printf("local.get %x\n", i->local_get.idx);
            break;
        
        case LocalSet:
            printf("local.set %x\n", i->local_set.idx);
            break;
        
        case GlobalGet:
            printf("global.get %x\n", i->global_get.idx);
            break;
        
        case GlobalSet:
            printf("global.set %x\n", i->global_set.idx);
            break;

        case I32Load:
        case I64Load:
        case I32Load8_s:
        case I32Load8_u:
        case I32Store:
        case I64Store:
        case I32Store8: {
            char *op;
            switch(i->op) {
                case I32Load:
                    op = "i32.load";
                    break;
                
                case I64Load:
                    op = "i64.load";
                    break;
                
                case I32Load8_s:
                    op = "i32.load8_s";
                    break;
                
                case I32Load8_u:
                    op = "i32.load8_u";
                    break;
                
                case I32Store:
                    op = "i32.store";
                    break;
                
                case I64Store:
                    op = "i64.store";
                    break;
                
                case I32Store8:
                    op = "i32.store8";
                    break;
            }
            printf(
                "%s %x %x\n", 
                op,
                i->memarg.align,
                i->memarg.offset
            );
            break;
        }

        case I32Const:
            printf("i32.const %x\n", i->i32_const.n);
            break;
        
        case I32Eqz:
            puts("i32.eqz");
            break;
        
        case I32Eq:
            puts("i32.eq");
            break;
        
        case I32Ne:
            puts("i32.ne");
            break;

        case I32Lt_s:
            puts("i32.lt_s");
            break;
       
        case I32Gt_s:
            puts("i32.gt_s");
            break;
        
        case I32Gt_u:
            puts("i32.gt_u");
            break;
        
        case I32Ge_s:
            puts("i32.ge_s");
            break;
        
        case I32Add:
            puts("i32.add");
            break;
        
        case I32Sub:
            puts("i32.sub");
            break;

        case I32Mul:
            puts("i32.mul");
            break;

        case I32Div_s:
            puts("i32.div_s");
            break;
        
        case I32Rem_s:
            puts("i32.rem_s");
            break;
                
        case I32And:
            puts("i32.and");
            break;

        case I32Shl:
            puts("i32.shl");
            break;
        
        case I32Shr_s:
            puts("i32.shr_s");
            break;
    }   
}

instr *invoke_f(struct context *ctx, struct wasm_func *f);

instr *invoke_i(struct context *ctx, instr *ip) {
    instr *next_ip = LIST_CONTAINER(ip->link.next, instr , link);
    
    // get current func
    struct wasm_func *func = LIST_CONTAINER(list_tail(&ctx->call_stack), struct wasm_func, link);
   
    //printf("[+] ip = ");
    //print_instr(ip);

    switch(ip->op) {
        case Unreachable:
            // exit current task
            puts("[!] unreachable!");
            task_exit(0);
            break;
        
        case Nop:
            break;
        
        case Block:
        case Loop:
            enter_block(ctx, ip);
            next_ip = LIST_CONTAINER(
                list_head(&ip->block.in),
                instr,
                link
            );
            break;
        
        case If: {
            enter_block(ctx, ip);
            int32_t cond = readi32(ctx->stack);
            if(cond) {
                next_ip = LIST_CONTAINER(
                    list_head(&ip->If.in1),
                    instr,
                    link
                );
            } else {
                next_ip = LIST_CONTAINER(
                    list_head(&ip->If.in2),
                    instr,
                    link
                );
            }
            break;
        }

        case Else:
        case End: 
            next_ip =  end(ctx);
            break;

        case Br:
            next_ip = branch_in(ctx, ip->br.l);
            break;
        
        case BrIf: {
            int32_t cond = readi32(ctx->stack);
            if(cond)
                next_ip = branch_in(ctx, ip->br.l);
            break;
        }

        case Return:
            next_ip =  end(ctx);
            break;
        
        case Call:
            enter_block(ctx, ip);
            next_ip = invoke_f(ctx, ctx->funcs[ip->call.idx]);
            break;

        case Drop:
            readi32(ctx->stack);
            break;
        
        case LocalGet: {
            writei32(
                ctx->stack, 
                func->locals[ip->local_get.idx]->val
            );
            break;
        }

        case LocalSet: {
            int32_t val = readi32(ctx->stack);
            func->locals[ip->local_set.idx]->val = val;
            break;
        }

        case GlobalGet: {
            writei32(
                ctx->stack, 
                ctx->globals[ip->global_get.idx]->val
            );
            break;
        }

        case GlobalSet: {
            int32_t val = readi32(ctx->stack);
            ctx->globals[ip->global_set.idx]->val = val;
            break;
        }

        case I32Load: {
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            int32_t c = loadi32(ctx->mem, ea);
            writei32(ctx->stack, c);
            break;
        }

        case I64Load: {
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            int64_t c = loadi64(ctx->mem, ea);
            writei64(ctx->stack, c);
            break;
        }

        case I32Load8_s: {
            // memarg.align is ignored
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            int8_t c = loadbyte(ctx->mem, ea);
            writei32(ctx->stack, c);
            break;
        }

        case I32Load8_u: {
            // memarg.align is ignored
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            uint8_t c = loadbyte(ctx->mem, ea);
            writei32(ctx->stack, c);
            break;
        }

        case I32Store: {
            // memarg.align is ignored
            int32_t c = readi32(ctx->stack);
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            storei32(ctx->mem, ea, c);
            break;
        }

        case I64Store: {
            // memarg.align is ignored
            int64_t c = readi64(ctx->stack);
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            storei64(ctx->mem, ea, c);
            break;
        }

        case I32Store8: {
            // memarg.align is ignored
            int32_t c = readi32(ctx->stack);
            int32_t i = readi32(ctx->stack);
            int32_t ea = i + ip->memarg.offset;
            storebyte(ctx->mem, ea, (uint8_t)c);
            break;
        }

        case I32Const:
            writei32(ctx->stack, ip->i32_const.n);
            break;

        case I32Eqz: {
            int32_t c = readi32(ctx->stack);
            writei32(ctx->stack, c == 0);
            break;
        }

        case I32Eq: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs == rhs);
            break;
        }

        case I32Ne: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs != rhs);
            break;
        }

        case I32Lt_s: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs < rhs);
            break;
        }

        case I32Gt_s: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs > rhs);
            break;
        }

        case I32Gt_u: {
            uint32_t rhs = readu32(ctx->stack);
            uint32_t lhs = readu32(ctx->stack);
            writei32(ctx->stack, lhs > rhs);
            break;
        }

        case I32Ge_s: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs >= rhs);
            break;
        }

        case I32Add: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs + rhs);
            break;
        }

        case I32Sub: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs - rhs);
            break;
        }

        case I32Mul: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs * rhs);
            break;
        }

        case I32Div_s: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs / rhs);
            break;
        }

        case I32Rem_s: {
            // todo: assert rhs != 0
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs % rhs);
            break;
        }

        case I32And: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs & rhs);
            break;
        }

        case I32Shl: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs << rhs);
            break;
        }

        case I32Shr_s: {
            int32_t rhs = readi32(ctx->stack);
            int32_t lhs = readi32(ctx->stack);
            writei32(ctx->stack, lhs >> rhs);
            break;
        }
        
        default:
            next_ip = NULL;
            break;
    }

    return next_ip;
}

void run_vm(struct context *ctx) {
    // todo: add entry to task struct
    instr *ip = ctx->entry;

    while(ip)
        ip = invoke_i(ctx, ip);
}

 // The WASM page size is defined by the specification as 16 pages (65536 bytes).
#define WASM_PAGE_SIZE  (PAGE_SIZE * 16)

struct context *create_context(module *m) {
    // funsec required
    if(!m->funcsec) {
        puts("[-] no funcsec");
        return NULL;
    }

    // _start function required
    section *exportsec = m->exportsec;
    exp0rt *export = NULL;

    for(int i = 0; i < exportsec->exports.n; i++) {
        exp0rt *e = exportsec->exports.x[i];
        if(e->d->kind == 0 && strcmp(e->name, "_start") == 0) {
            export = e;
            break;
        }
    }
    if(!export) {
        puts("[-] no _start function");
        return NULL;
    }

    // count functions(including imported functions)
    int num_imports = 0;
    if(m->importsec)
        num_imports = m->importsec->imports.n;
    
    int num_defined =m->funcsec->typeidxes.n;

    int num_funcs =  num_imports + num_defined;
    
    // create struct context
    struct context *ctx = malloc(sizeof(struct context) + sizeof(struct wasm_func *) * num_funcs);

    // create stack: 1 page for mow
    uint8_t *buf = pmalloc(1);
    ctx->stack = newstack(buf, 4096);

    // inti call stack
    LIST_INIT(&ctx->call_stack);

    // init block
    LIST_INIT(&ctx->blocks);

    // init global variables if globalsec is defined
    if(m->globalsec) {
        int num_globals = m->globalsec->globals.n;
        struct global_variable **globals = malloc(sizeof(struct global_variable *) * num_globals);
        for(int i = 0; i < num_globals; i++) {
            global *g = m->globalsec->globals.x[i];
            struct global_variable *v = malloc(sizeof(struct global_variable));
            v->ty = g->gt;
            
            // calculate the initial value(constant expression expected)
            instr *ip = LIST_CONTAINER(list_head(&g->expr), instr, link);
            while(ip)
                ip = invoke_i(ctx, ip);
                
            v->val = readi32(ctx->stack);
            globals[i] = v;
            //printf("[+] globals %d = %x\n", i, v->val);
        }
        ctx->globals = globals;
    }

    // create mem if memsec is defined
    if(m->memsec) {
        mem *mem = m->memsec->mems.x[0];
        uint8_t *page = pmalloc(WASM_PAGE_SIZE * mem->mt.min / PAGE_SIZE);
        ctx->mem = newbuffer(page, WASM_PAGE_SIZE * mem->mt.min);
        
        // init mem if datasec is defined
        if(m->datasec) {
            data *data;
            for(int i = 0; i < m->datasec->datas.n; i++) {
                data = m->datasec->datas.x[i];

                if(data->kind == 0) {
                    // get offs(constant expr expected)
                    instr *ip = LIST_CONTAINER(list_head(&data->expr), instr, link);
                    while(ip)
                        ip = invoke_i(ctx, ip);

                    int32_t offs = readi32(ctx->stack);

                    // write data
                    for(uint32_t i = 0; i < data->n; i++) {
                        storebyte(ctx->mem, offs + i, data->data[i]);
                    }
                }
            }
        }
    }

    // create functions(including imported functions)
    int funcIdx = 0;
    for(int i = 0; i < num_imports; i++) {
        ctx->funcs[funcIdx++] = create_imported_func(m, i);
    }
    for(int i = 0; i < num_defined; i++) {
        ctx->funcs[funcIdx++] = create_defined_func(m, i);
    }

    // set entry & ip
    struct wasm_func *start = ctx->funcs[export->d->idx];
    enter_frame(ctx, start);
    ctx->entry = LIST_CONTAINER(list_head(start->codes), instr, link);

    return ctx;
}

int32_t invoke_external(struct context *ctx, struct wasm_func *f) {
    // import from another wasm binary is not supported yet
    if(strcmp(f->modName, "env") == 0) {
        if(strcmp(f->name, "task_exit") == 0) {
            task_exit(f->locals[0]->val);
        }
        if(strcmp(f->name, "vm_create") == 0) {
            return vm_create(ctx->mem->p + f->locals[0]->val, f->locals[1]->val);
        }
        if(strcmp(f->name, "arch_serial_write") == 0) {
            arch_serial_write(f->locals[0]->val);
        }
        if(strcmp(f->name, "arch_serial_read") == 0) {
            return arch_serial_read();
        }
        if(strcmp(f->name, "ipc_send") == 0) {
            struct message *msg = (struct message *)(ctx->mem->p + f->locals[1]->val);
            return ipc_send(f->locals[0]->val, msg);
        }
        if(strcmp(f->name, "ipc_receive") == 0) {
            struct message *msg = (struct message *)(ctx->mem->p + f->locals[1]->val);
            return ipc_receive(f->locals[0]->val, msg);
        }
        if(strcmp(f->name, "ipc_call") == 0) {
            struct message *msg = (struct message *)(ctx->mem->p + f->locals[1]->val);
            return ipc_call(f->locals[0]->val, msg);
        }
    }

    return 0;
}

instr *invoke_f(struct context *ctx, struct wasm_func *f) {
    enter_frame(ctx, f);
    
    // set args
    for(int i = f->ty->rt1->n - 1; i >= 0; i--) {
        // todo: validate type
        f->locals[i]->val = readi32(ctx->stack);
    }

    if(f->imported) {
        int32_t ret = invoke_external(ctx, f);
        if(f->ty->rt2->n)
            writei32(ctx->stack, ret);
        return end(ctx);
    }
    else {
        return LIST_CONTAINER(list_head(f->codes), instr, link);
    }
}