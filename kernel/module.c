#include "module.h"
#include "memory.h"

valtype * parse_valtype(struct buffer *buf) {
    valtype *valTy = malloc(sizeof(valtype));
    *valTy = readbyte(buf);
    return valTy;
}

resulttype * parse_resulttype(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);

    resulttype *retTy = malloc(sizeof(resulttype) + sizeof(valtype *) * n);

    retTy->n = n;

    for(int i = 0; i < retTy->n; i++)
        retTy->x[i] = parse_valtype(buf);
    return retTy;
}

functype * parse_functype(struct buffer *buf) {
    functype * funcTy = malloc(sizeof(functype));

    // todo: assert == 0x60
    readbyte(buf);
    
    // rt1
    funcTy->rt1 = parse_resulttype(buf);
    // rt2
    funcTy->rt2 = parse_resulttype(buf);

    return funcTy;
}

typeidx * parse_typeidx(struct buffer *buf) {
    typeidx *typeidx = malloc(sizeof(typeidx));
    *typeidx = readu32_LEB128(buf);
    return typeidx;
}

instr * parse_instr(struct buffer *buf) {
    instr *i = malloc(sizeof(instr));

    i->op = readbyte(buf);

    switch(i->op) {
        case Unreachable:
        case Nop:
            break;
        
        case Block:
        case Loop: {
            i->block.bt = readbyte(buf);
            LIST_INIT(&i->block.in);
            instr *j;
            do {
                j = parse_instr(buf);
                list_push_back(&i->block.in, &j->link);
            } while(j->op != End);
            break;
        }

        case If: {
            i->If.bt = readbyte(buf);
            LIST_INIT(&i->If.in1);
            instr *j;
            do {
                j = parse_instr(buf);
                list_push_back(&i->If.in1, &j->link);
            } while(j->op != End && j->op != Else);

            if(i->op == Else) {
                LIST_INIT(&i->If.in2);
                do {
                    j =  parse_instr(buf);
                    list_push_back(&i->If.in2, &j->link);
                } while(j->op != End);
            }
            break;
        }

        case End:
            break;
        
        case Br:
        case BrIf:
            i->br.l = readu32_LEB128(buf);
            break;
        
        case BrTable: {
            int n = readu32_LEB128(buf);
            
            i->br_table.n = n;
            i->br_table.labels = malloc(sizeof(uint32_t) * n);
            for(int j = 0; j < n; j++)
                i->br_table.labels[j] = readu32_LEB128(buf);
            
            i->br_table.default_l = readu32_LEB128(buf);
            break;
        }

        case Return:
            break;
        
        case Call:
            i->call.idx = readu32_LEB128(buf);
            break;
        
        case Drop:
            break;
        
        case LocalGet:
            i->local_get = (local_get_instr) {
                .idx = readu32_LEB128(buf)
            };
            break;
        
        case LocalSet:
            i->local_set = (local_set_instr) {
                .idx = readu32_LEB128(buf)
            };
            break;
        
        case GlobalGet:
            i->global_get = (global_get_instr) {
                .idx        = readu32_LEB128(buf)
            };
            break;

        case GlobalSet:
            i->global_set = (global_set_instr) {
                .idx    = readu32_LEB128(buf)
            };
            break;
        
        case I32Load:
        case I64Load:
        case I32Load8_s:
        case I32Load8_u:
        case I32Store:
        case I64Store:
        case I32Store8:
            i->memarg = (memarg) {
                .align  = readu32_LEB128(buf),
                .offset = readu32_LEB128(buf)
            };
            break;
        
        case I32Const:
            i->i32_const = (i32_const_instr) {
                .n = readi32_LEB128(buf)
            };
            break;
        
        case I32Eqz:
        case I32Eq:
        case I32Ne:
        case I32Lt_s:
        case I32Gt_s:
        case I32Gt_u:
        case I32Ge_s:
        case I32Add:
        case I32Sub:
        case I32Mul:
        case I32Div_s:
        case I32Rem_s:
        case I32And:
        case I32Or:
        case I32Shl:
        case I32Shr_s:
            break;
        
        // unsupported instruction
        default:
            printf("[-] unsupported instruction op = %x\n", i->op);
            task_exit(0);
        
    }

    return i;
}

locals * parse_locals(struct buffer *buf) {
    locals * l = malloc(sizeof(locals));

    *l = (locals) {
        .num    = readu32_LEB128(buf),
        .ty     = readbyte(buf)
    };
    return l;
}

func * parse_func(struct buffer *buf) {
    uint32_t n =  readu32_LEB128(buf);

    func *f = malloc(sizeof(func) + sizeof(locals *) * n);

    f->locals.n = n;

    for(int i = 0; i < f->locals.n; i++)
        f->locals.x[i] = parse_locals(buf);

    LIST_INIT(&f->expr);

    while(!eof(buf)) {
        list_push_back(
                &f->expr, 
                &parse_instr(buf)->link
        );
    }

    return f;
}

code * parse_code(struct buffer *buf) {
    code *c = malloc(sizeof(code));

    c->size = readu32_LEB128(buf);

    struct buffer *buffer = readbuffer(buf, c->size);

    c->code = parse_func(buffer);

    return c;   
}

exportdesc * parse_exportdesc(struct buffer *buf) {
    exportdesc *d = malloc(sizeof(exportdesc));
    *d = (exportdesc) {
        .kind   = readbyte(buf),
        .idx    = readu32_LEB128(buf)
    };
    return d;
}

exp0rt * parse_export(struct buffer *buf) {
    exp0rt *e = malloc(sizeof(exp0rt));

    e->name = readname(buf);
    e->d = parse_exportdesc(buf);

    return e;
}

importdesc * parse_importdesc(struct buffer *buf) {
    importdesc *d = malloc(sizeof(importdesc));

    *d = (importdesc) {
        .kind       = readbyte(buf),
        .idx        = readu32_LEB128(buf)
    };

    return d;
}

import * parse_import(struct buffer *buf) {
    import *i = malloc(sizeof(import));

    *i = (import) {
        .mod    = readname(buf),
        .nm     = readname(buf),
        .d      = parse_importdesc(buf)
    };

    return i;
}

mem * parse_mem(struct buffer *buf) {
    mem *m = malloc(sizeof(mem));
    *m = (mem) {
        .mt = (memtype) {
            .kind   = readbyte(buf),
            .min    = readu32_LEB128(buf)
        }
    };
    
    if(m->mt.kind)
        m->mt.max = readu32_LEB128(buf);

    return m;
}

data * parse_data(struct buffer *buf) {
    data *d = malloc(sizeof(data));
    d->kind = readu32_LEB128(buf);

    switch(d->kind) {
        case 0: {
            LIST_INIT(&d->expr);
            instr *i;
            do {
                i = parse_instr(buf);
                list_push_back(&d->expr, &i->link);
            } while(i->op != End);
            
            d->n = readu32_LEB128(buf);
            d->data = malloc(sizeof(uint8_t) * d->n);
            for(uint32_t i = 0; i < d->n; i++) {
                d->data[i] = readbyte(buf);
            }
            break;
        }
        case 1:
        case 2:
            // todo: support this
            break;
    }

    return d;
}

section * parse_typesec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);

    section *sec = malloc(sizeof(section) + sizeof(functype *) * n);
    
    sec->id = TYPE_SECTION_ID;
    sec->functypes.n = n;

    for(int i = 0; i < sec->functypes.n; i++) {
        sec->functypes.x[i] = parse_functype(buf);
    }

    return sec;
}

section * parse_funcsec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);

    section *sec = malloc(sizeof(section) + sizeof(typeidx *) * n);

    sec->id = FUNC_SECTION_ID;
    sec->typeidxes.n = n;

    for(int i = 0; i < sec->typeidxes.n; i++) {
        sec->typeidxes.x[i] = parse_typeidx(buf);
    }

    return sec;
}

section * parse_codesec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);

    section *sec = malloc(sizeof(section) + sizeof(code *) * n);

    sec->id = CODE_SECTION_ID;
    sec->codes.n = n;

    for(int i = 0; i < sec->typeidxes.n; i++) {
        sec->codes.x[i] = parse_code(buf);
    }

    return sec;
}

section * parse_exportsec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);

    section *sec = malloc(sizeof(section) + sizeof(exp0rt *) * n);

    sec->id = EXPORT_SECTION_ID;
    sec->exports.n = n;

    for(int i = 0; i < sec->exports.n; i++) {
        sec->exports.x[i] = parse_export(buf);
    }

    return sec;
}

section *parse_importsec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);

    section *sec = malloc(sizeof(section) + sizeof(import *) * n);

    sec->id = IMPORT_SECTION_ID;
    sec->imports.n = n;

    for(int i = 0; i < sec->imports.n; i++) {
        sec->imports.x[i] = parse_import(buf);
    }

    return sec;
}

section * parse_memsec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);
    section *sec = malloc(sizeof(section) + sizeof(mem *) * n);

    sec->id = MEM_SECTION_ID;
    sec->mems.n = n;
    for(int i = 0; i < sec->mems.n; i++) {
        sec->mems.x[i] = parse_mem(buf);
    }
    return sec;
}

section * parse_datasec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);
    section *sec = malloc(sizeof(section) + sizeof(data *) * n);

    sec->id = DATA_SECTION_ID;
    sec->datas.n = n;

    for(int i = 0; i < sec->datas.n; i++) {
        sec->datas.x[i] = parse_data(buf);
    }

    return sec;
}

global *parse_global(struct buffer *buf) {
    global *g = malloc(sizeof(global));
    g->gt = (globaltype) {
        .ty = readbyte(buf),
        .m  = readbyte(buf)
    };

    LIST_INIT(&g->expr);
    instr *i;
    do {
        i = parse_instr(buf);
        list_push_back(&g->expr, &i->link);
    } while(i->op != End);

    return g;
}

section * parse_globalsec(struct buffer *buf) {
    uint32_t n = readu32_LEB128(buf);
    section *sec = malloc(sizeof(section) + sizeof(global *) * n);
    sec->id = GLOBAL_SECTION_ID;
    sec->globals.n = n;

    for(int i = 0; i < sec->globals.n; i++) {
        sec->globals.x[i] = parse_global(buf);
    }

    return sec;
}

module * new_module(struct buffer *buf) {
    module *m = malloc(sizeof(module));
    *m = (module) {
        .magic = readu32(buf),
        .version = readu32(buf)
    };

    section **sec = &m->typesec;
    for(int i = 0; i < 8; i++)
        sec[i] = NULL;
    
    // parse known sections
    while(!eof(buf)) {
        uint8_t id  = readbyte(buf);
        uint32_t size = readu32_LEB128(buf);
        struct buffer *sec = readbuffer(buf, size);

        switch(id) {
            case TYPE_SECTION_ID:
                m->typesec = parse_typesec(sec);
                break;
            
            case IMPORT_SECTION_ID:
                m->importsec = parse_importsec(sec);
                break;

            case FUNC_SECTION_ID:
                m->funcsec = parse_funcsec(sec);
                break;

            case MEM_SECTION_ID:
                m->memsec = parse_memsec(sec);
                break;

            case GLOBAL_SECTION_ID:
                m->globalsec = parse_globalsec(sec);
                break;
            
            case EXPORT_SECTION_ID:
                m->exportsec = parse_exportsec(sec);
                break;

            case CODE_SECTION_ID:
                m->codesec = parse_codesec(sec);
                break;

            case DATA_SECTION_ID:
                m->datasec = parse_datasec(sec);
                break;

            // ignore other sections
        }
    }

    return m;
}