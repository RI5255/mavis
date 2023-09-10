#pragma once

#include "buffer.h"
#include "list.h"
#include <stdint.h>

#define DEFINE_VECTOR(elem_ty)      \
typedef struct {                    \
    int n;                          \
    elem_ty * x[];                  \
} elem_ty##_v;

// num type
#define I32         0x7f
#define I64         0x7e
#define F32         0x7d
#define F64         0x7c
// ref type
#define FuncRef     0x70
#define ExternRef   0x6f

typedef uint8_t valtype;
DEFINE_VECTOR(valtype);

typedef valtype_v resulttype;

typedef struct {
    resulttype  *rt1;
    resulttype  *rt2;
} functype;
DEFINE_VECTOR(functype);

typedef uint32_t typeidx;
DEFINE_VECTOR(typeidx);

typedef struct {
    uint32_t num;
    uint8_t ty;
} locals;
DEFINE_VECTOR(locals);

typedef enum {
    Unreachable = 0x00,
    Nop         = 0x01,
    Block       = 0x02,
    Loop        = 0x03,
    If          = 0x04,
    Else        = 0x05,
    End         = 0x0b,
    Br          = 0x0c,
    BrIf        = 0x0d,
    BrTable     = 0x0e,
    Return      = 0x0f,
    Call        = 0x10,
    Drop        = 0x1a,
    LocalGet    = 0x20,
    LocalSet    = 0x21,
    GlobalGet   = 0x23,
    GlobalSet   = 0x24,
    I32Load     = 0x28,
    I64Load     = 0x29,
    I32Load8_s  = 0x2c,
    I32Load8_u  = 0x2d,
    I32Store    = 0x36,
    I64Store    = 0x37,
    I32Store8   = 0x3a,
    I32Const    = 0x41,
    I32Eqz      = 0x45,
    I32Eq       = 0x46,
    I32Ne       = 0x47,
    I32Lt_s     = 0x48,
    I32Gt_s     = 0x4a,
    I32Gt_u     = 0x4b,
    I32Ge_s     = 0x4e,
    I32Add      = 0x6a,
    I32Sub      = 0x6b,
    I32Mul      = 0x6c,
    I32Div_s    = 0x6d,
    I32Rem_s    = 0x6f,
    I32And      = 0x71,
    I32Shl      = 0x74,
    I32Shr_s    = 0x75
} op;

typedef struct {
    uint32_t    n;
} i32_const_instr;

typedef struct {
    uint32_t    idx;
} local_get_instr;

typedef struct {
    uint32_t    idx;
} local_set_instr;

typedef uint32_t globalidx;
typedef struct {
    globalidx idx;
} global_get_instr;

typedef struct {
    globalidx idx;
} global_set_instr;

// do not support s33 for now
typedef valtype blocktype;
typedef struct {
    blocktype   bt;
    list_t      in1;
    list_t      in2;
} if_instr;

typedef struct {
    blocktype   bt;
    list_t      in;
} block_instr;

typedef uint32_t labelidx;
typedef struct {
    labelidx    l;
} br_instr;

typedef struct {
    int n;
    labelidx    *labels;
    labelidx    default_l;
} br_table_instr;

typedef uint32_t funcidx;
typedef struct {
    funcidx    idx;
} call_instr;

typedef struct {
    uint32_t    align;
    uint32_t    offset;
} memarg;

typedef struct {
    list_elem_t link;
    list_elem_t link_block; //used to link block instructions
    uint8_t     op;
    union {
        i32_const_instr     i32_const;
        memarg              memarg;
        local_get_instr     local_get;
        local_set_instr     local_set;
        global_get_instr    global_get;
        global_set_instr    global_set;
        if_instr            If;
        block_instr         block;
        br_instr            br;
        br_table_instr      br_table;
        call_instr          call;
    };
} instr;

typedef struct {
    list_t      expr;
    locals_v    locals;
} func;

typedef struct {
    uint32_t    size;
    func        *code;
} code;
DEFINE_VECTOR(code);

typedef struct {
    uint8_t     kind;
    uint32_t    idx;   
} exportdesc;

typedef struct {
    char        *name;
    exportdesc  *d; 
} exp0rt;

DEFINE_VECTOR(exp0rt);

typedef struct {
    uint8_t     kind;
    union {
        // todo: add types
        typeidx idx;
    };
} importdesc;

typedef struct {
    char        *mod;
    char        *nm;
    importdesc  *d;
} import;
DEFINE_VECTOR(import);

typedef struct {
    uint8_t     kind;
    uint32_t    min;
    uint32_t    max;
} limits;

typedef limits memtype;
typedef struct {
    memtype     mt;
} mem;
DEFINE_VECTOR(mem);

typedef uint8_t byte;
typedef uint32_t memidx;

typedef struct {
    uint32_t    kind;
    memidx      x;
    list_t      expr;
    // todo: fix this. add readByteVec to "buffer.c"?
    uint32_t    n;
    byte        *data;
} data;
DEFINE_VECTOR(data);

typedef uint8_t mut;
typedef struct {
    valtype     ty;
    mut         m;
} globaltype;

typedef struct {
    globaltype  gt;
    list_t      expr;
} global;
DEFINE_VECTOR(global);

#define TYPE_SECTION_ID         1
#define IMPORT_SECTION_ID       2
#define FUNC_SECTION_ID         3
#define MEM_SECTION_ID          5
#define GLOBAL_SECTION_ID       6
#define EXPORT_SECTION_ID       7
#define CODE_SECTION_ID         10
#define DATA_SECTION_ID         11

typedef struct {
    list_elem_t link;
    int id;
    union {
        functype_v  functypes;      // typesec
        import_v    imports;        // importsec
        typeidx_v   typeidxes;      // funcsec
        mem_v       mems;           // memsec
        global_v    globals;        // globalsec
        exp0rt_v    exports;        // exportsec
        code_v      codes;          // codesec
        data_v      datas;          // datasec
    };
} section;

typedef struct {
    uint32_t    magic;
    uint32_t    version;

    // known sections
    section     *typesec;
    section     *importsec;
    section     *funcsec;
    section     *memsec;
    section     *globalsec;
    section     *exportsec;
    section     *codesec;
    section     *datasec;

    // todo: support custom sections?
} module;

module * new_module(struct buffer *buf);


