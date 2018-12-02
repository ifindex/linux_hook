/*
 *
 *  hook function for x86 cpu
 *
 *  author  : sunjianping    (ifindex@163.com)
 *  date    : 2018/11/19
 *  
 *  Copyright All Reserved 2018-2020 by sunjianping (ifindex@163.com)
 *  
 *
 */

/*
 *  usage: sjp_hook_func(XXX, func1, func2);
 *
 *  code part example:
 *  {
 *      ...
 *      ret = XXX(a, b);
 *      ...
 *  }
 *
 *  Run step:
 *      1. run func1, func1(a, b)
 *      2. run XXX function, XXX(a, b)
 *      3. run func2, func2(ret, a, b)
 *      4. return ret as XXX return value
 *
 *       function XXX symbol BEGIN:     ----> Hook Before
 *          instruction 1;
 *          instruction 2;
 *          instruction 3;
 *          ......
 *          instruction END;            ----> Hook After
 *
 */
#include "import.h"

static int g_hook_inited    = 0;

#define X86_COPY_SIZE   0x07

#define __AND__         "and    "
#define __PUSH__        "push   "
#define __POP__         "pop    "
#define __ADD__         "add    "
#define __SUB__         "sub    "
#define __MOV__         "mov    "
#define __CALL__        "call   "

#define __RG_EAX__      "%eax"
#define __RG_EBX__      "%ebx"
#define __RG_ESP__      "%esp"
#define __RG_ESP_A__    "(%esp)"
#define __RG_EBP__      "%ebp"
#define __RG_EBP_A__    "(%ebp)"

/*
 *  control for new stack size
 */
#define __STACK_SIZE__  "$0x168"

#define __MOV_STACK__(x, y)                                     \
    HKASM(__MOV__       ""#x""__RG_EBP_A__","__RG_EAX__);       \
    HKASM(__MOV__       ""__RG_EAX__","#y""__RG_ESP_A__);

#define __STACK_IN__                                            \
    HKASM(__AND__       "$0xff, "__RG_EAX__);                   \
    HKASM(__PUSH__      ""__RG_EAX__);                          \
    HKASM(__PUSH__      ""__RG_EBX__);                          \
    HKASM(__SUB__       __STACK_SIZE__", "__RG_ESP__);

#define __ENTRY_IN__                                            \
    HKASM(__MOV__       ""__RG_EAX__", -12"__RG_EBP_A__);       \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_ESP_A__);          \
    HKASM(__CALL__      "__entry_callback__");                  \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_EBX__);

#define __MK_STACK0__                                           \
    __MOV_STACK__(0x08,     0x00);                              \
    __MOV_STACK__(0x0c,     0x04);                              \
    __MOV_STACK__(0x10,     0x08);                              \
    __MOV_STACK__(0x14,     0x0c);                              \
    __MOV_STACK__(0x18,     0x10);                              \
    __MOV_STACK__(0x1c,     0x14);                              \
    __MOV_STACK__(0x20,     0x18);                              \
    __MOV_STACK__(0x24,     0x1c);                              \
    __MOV_STACK__(0x28,     0x20);                              \
    __MOV_STACK__(0x2c,     0x24);                              \
    __MOV_STACK__(0x30,     0x28);                              \
    __MOV_STACK__(0x34,     0x2c);                              \
    __MOV_STACK__(0x38,     0x30);                              \
    __MOV_STACK__(0x3c,     0x34);                              \
    __MOV_STACK__(0x40,     0x38);                              \
    __MOV_STACK__(0x44,     0x3c);                              \
    __MOV_STACK__(0x48,     0x40);                              \
    __MOV_STACK__(0x4c,     0x44);                              \
    __MOV_STACK__(0x50,     0x48);                              \
    __MOV_STACK__(0x54,     0x4c);                              \
    __MOV_STACK__(0x58,     0x50);                              \
    __MOV_STACK__(0x5c,     0x54);                              \
    __MOV_STACK__(0x60,     0x58);                              \
    __MOV_STACK__(0x64,     0x5c);                              \
    __MOV_STACK__(0x68,     0x60);                              \
    __MOV_STACK__(0x6c,     0x64);                              \
    __MOV_STACK__(0x70,     0x68);                              \
    __MOV_STACK__(0x74,     0x6c);                              \
    __MOV_STACK__(0x78,     0x70);                              \
    __MOV_STACK__(0x7c,     0x74);                              \
    __MOV_STACK__(0x80,     0x78);                              \
    __MOV_STACK__(0x84,     0x7c);                              \
    __MOV_STACK__(0x88,     0x80);                              \
    __MOV_STACK__(0x8c,     0x84);                              \
    __MOV_STACK__(0x90,     0x88);                              \
    __MOV_STACK__(0x94,     0x8c);                              \
    __MOV_STACK__(0x98,     0x90);                              \
    __MOV_STACK__(0x9c,     0x94);                              \
    __MOV_STACK__(0xa0,     0x98);                              \
    __MOV_STACK__(0xa4,     0x9c);                              \
    __MOV_STACK__(0xa8,     0xa0);                              \
    __MOV_STACK__(0xac,     0xa4);                              \
    __MOV_STACK__(0xb0,     0xa8);                              \
    __MOV_STACK__(0xb4,     0xac);                              \
    __MOV_STACK__(0xb8,     0xb0);                              \
    __MOV_STACK__(0xbc,     0xb4);                              \
    __MOV_STACK__(0xc0,     0xb8);                              \
    __MOV_STACK__(0xc4,     0xbc);                              \
    __MOV_STACK__(0xc8,     0xc0);                              \
    __MOV_STACK__(0xcc,     0xc4);                              \
    __MOV_STACK__(0xd0,     0xc8);                              \
    __MOV_STACK__(0xd4,     0xcc);                              \
    __MOV_STACK__(0xd8,     0xd0);                              \
    __MOV_STACK__(0xdc,     0xd4);                              \
    __MOV_STACK__(0xe0,     0xd8);                              \
    __MOV_STACK__(0xe4,     0xdc);                              \
    __MOV_STACK__(0xe8,     0xe0);                              \
    __MOV_STACK__(0xec,     0xe4);                              \
    __MOV_STACK__(0xf0,     0xe8);                              \
    __MOV_STACK__(0xf4,     0xec);                              \
    __MOV_STACK__(0xf8,     0xf0);                              \
    __MOV_STACK__(0xfc,     0xf4);                              \
    __MOV_STACK__(0x100,    0xf8);                              \
    __MOV_STACK__(0x104,    0xfc);                              \
    __MOV_STACK__(0x108,    0x100);                             \
    __MOV_STACK__(0x10c,    0x104);                             \
    __MOV_STACK__(0x110,    0x108);                             \
    __MOV_STACK__(0x114,    0x10c);                             \
    __MOV_STACK__(0x118,    0x110);                             \
    __MOV_STACK__(0x11c,    0x114);                             \
    __MOV_STACK__(0x120,    0x118);                             \
    __MOV_STACK__(0x124,    0x11c);                             \
    __MOV_STACK__(0x128,    0x120);                             \
    __MOV_STACK__(0x12c,    0x124);                             \
    __MOV_STACK__(0x130,    0x128);                             \
    __MOV_STACK__(0x134,    0x12c);                             \
    __MOV_STACK__(0x138,    0x130);                             \
    __MOV_STACK__(0x13c,    0x134);                             \
    __MOV_STACK__(0x140,    0x138);

    
#define __MK_STACK1__                                           \
    __MK_STACK0__;                                              \
    __MOV_STACK__(-4,       0x13c);

#define __MK_STACK2__                                           \
    __MK_STACK0__;                                              \
    __MOV_STACK__(0x144,    0x13c);

#define __MK_STACK3__                                           \
    __MOV_STACK__(-16,      0x00);                              \
    __MOV_STACK__(0x08,     0x04);                              \
    __MOV_STACK__(0x0c,     0x08);                              \
    __MOV_STACK__(0x10,     0x0c);                              \
    __MOV_STACK__(0x14,     0x10);                              \
    __MOV_STACK__(0x18,     0x14);                              \
    __MOV_STACK__(0x1c,     0x18);                              \
    __MOV_STACK__(0x20,     0x1c);                              \
    __MOV_STACK__(0x24,     0x20);                              \
    __MOV_STACK__(0x28,     0x24);                              \
    __MOV_STACK__(0x2c,     0x28);                              \
    __MOV_STACK__(0x30,     0x2c);                              \
    __MOV_STACK__(0x34,     0x30);                              \
    __MOV_STACK__(0x38,     0x34);                              \
    __MOV_STACK__(0x3c,     0x38);                              \
    __MOV_STACK__(0x40,     0x3c);                              \
    __MOV_STACK__(0x44,     0x40);                              \
    __MOV_STACK__(0x48,     0x44);                              \
    __MOV_STACK__(0x4c,     0x48);                              \
    __MOV_STACK__(0x50,     0x4c);                              \
    __MOV_STACK__(0x54,     0x50);                              \
    __MOV_STACK__(0x58,     0x54);                              \
    __MOV_STACK__(0x5c,     0x58);                              \
    __MOV_STACK__(0x60,     0x5c);                              \
    __MOV_STACK__(0x64,     0x60);                              \
    __MOV_STACK__(0x68,     0x64);                              \
    __MOV_STACK__(0x6c,     0x68);                              \
    __MOV_STACK__(0x70,     0x6c);                              \
    __MOV_STACK__(0x74,     0x70);                              \
    __MOV_STACK__(0x78,     0x74);                              \
    __MOV_STACK__(0x7c,     0x78);                              \
    __MOV_STACK__(0x80,     0x7c);                              \
    __MOV_STACK__(0x84,     0x80);                              \
    __MOV_STACK__(0x88,     0x84);                              \
    __MOV_STACK__(0x8c,     0x88);                              \
    __MOV_STACK__(0x90,     0x8c);                              \
    __MOV_STACK__(0x94,     0x90);                              \
    __MOV_STACK__(0x98,     0x94);                              \
    __MOV_STACK__(0x9c,     0x98);                              \
    __MOV_STACK__(0xa0,     0x9c);                              \
    __MOV_STACK__(0xa4,     0xa0);                              \
    __MOV_STACK__(0xa8,     0xa4);                              \
    __MOV_STACK__(0xac,     0xa8);                              \
    __MOV_STACK__(0xb0,     0xac);                              \
    __MOV_STACK__(0xb4,     0xb0);                              \
    __MOV_STACK__(0xb8,     0xb4);                              \
    __MOV_STACK__(0xbc,     0xb8);                              \
    __MOV_STACK__(0xc0,     0xbc);                              \
    __MOV_STACK__(0xc4,     0xc0);                              \
    __MOV_STACK__(0xc8,     0xc4);                              \
    __MOV_STACK__(0xcc,     0xc8);                              \
    __MOV_STACK__(0xd0,     0xcc);                              \
    __MOV_STACK__(0xd4,     0xd0);                              \
    __MOV_STACK__(0xd8,     0xd4);                              \
    __MOV_STACK__(0xdc,     0xd8);                              \
    __MOV_STACK__(0xe0,     0xdc);                              \
    __MOV_STACK__(0xe4,     0xe0);                              \
    __MOV_STACK__(0xe8,     0xe4);                              \
    __MOV_STACK__(0xec,     0xe8);                              \
    __MOV_STACK__(0xf0,     0xec);                              \
    __MOV_STACK__(0xf4,     0xf0);                              \
    __MOV_STACK__(0xf8,     0xf4);                              \
    __MOV_STACK__(0xfc,     0xf8);                              \
    __MOV_STACK__(0x100,    0xfc);                              \
    __MOV_STACK__(0x104,    0x100);                             \
    __MOV_STACK__(0x108,    0x104);                             \
    __MOV_STACK__(0x10c,    0x108);                             \
    __MOV_STACK__(0x110,    0x10c);                             \
    __MOV_STACK__(0x114,    0x110);                             \
    __MOV_STACK__(0x118,    0x114);                             \
    __MOV_STACK__(0x11c,    0x118);                             \
    __MOV_STACK__(0x120,    0x11c);                             \
    __MOV_STACK__(0x124,    0x120);                             \
    __MOV_STACK__(0x128,    0x124);                             \
    __MOV_STACK__(0x12c,    0x128);                             \
    __MOV_STACK__(0x130,    0x12c);                             \
    __MOV_STACK__(0x134,    0x130);                             \
    __MOV_STACK__(0x138,    0x134);                             \
    __MOV_STACK__(0x13c,    0x138);                             \
    __MOV_STACK__(0x140,    0x13c);

#define __ENTRY_HOOK__                                          \
    __MK_STACK1__                                               \
    HKASM(__CALL__      "*"__RG_EBX__);                         \
    HKASM(__MOV__       ""__RG_EAX__", -16"__RG_EBP_A__);

#define __ENTRY_CHG_PARA__                                      \
    HKASM(__MOV__       __RG_EBP__", "__RG_EAX__);              \
    HKASM(__ADD__       ""__RG_EAX__", 8");                     \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_ESP_A__);          \
    HKASM(__CALL__      "*"__RG_EBX__);

#define __GET_SRC_INSTR__                                       \
    HKASM(__MOV__       "-12"__RG_EBP_A__", "__RG_EAX__);       \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_ESP_A__);          \
    HKASM(__CALL__      "__execute__");                         \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_EBX__);

#define __BACK_SRC_INSTR__                                      \
    __MK_STACK2__                                               \
    HKASM(__CALL__      "*"__RG_EBX__);                         \
    HKASM(__MOV__       ""__RG_EAX__", -16"__RG_EBP_A__);

#define __EXIT_ADDR__                                           \
    HKASM(__MOV__       "-12"__RG_EBP_A__", "__RG_EAX__);       \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_ESP_A__);          \
    HKASM(__CALL__      "__exit_callback__");                   \
    HKASM(__MOV__       ""__RG_EAX__", "__RG_EBX__);

#define __EXIT_HOOK__                                           \
    __MK_STACK3__                                               \
    HKASM(__CALL__      "*"__RG_EBX__);

#define __STACK_OUT__                                           \
    HKASM(__ADD__       __STACK_SIZE__", "__RG_ESP__);          \
    HKASM(__POP__       ""__RG_EBX__);                          \
    HKASM(__POP__       ""__RG_EAX__);                          \
    HKASM(__MOV__       "-16"__RG_EBP_A__", "__RG_EAX__);

#define DECLARE_HOOKI(x)                                        \
    static __XPF__ __hk_x86_x##x##i__() {                       \
        __STACK_IN__;   __ENTRY_IN__;   __ENTRY_HOOK__;         \
        __GET_SRC_INSTR__;  __BACK_SRC_INSTR__;                 \
        __EXIT_ADDR__;  __EXIT_HOOK__;  __STACK_OUT__;          \
    }

#define DECLARE_HOOKR(x)                                        \
    static __XPF__ __hk_x86_x##x##r__() {                       \
        __STACK_IN__;   __ENTRY_IN__;   __ENTRY_HOOK__;         \
        __STACK_OUT__;                                          \
    }

#define DECLARE_HOOK(x) \
    DECLARE_HOOKI(x)    \
    DECLARE_HOOKR(x)

DECLARE_HOOK(hook)

#define __XSET__(src, dst, out_len)                             \
    do {                                                        \
        uint d_intv, d_loop;                                    \
        uchar *d_dst = (uchar *)dst + X86_COPY_SIZE;            \
        mprotect((void *)ROUND_DOWN(dst, LINUX_MM_ALIGN_SIZE),  \
            LINUX_MM_PROTECT_SIZE, LINUX_MM_WRITABLE);          \
        d_intv = close_interrupt();                             \
        memcpy(dst, src, X86_COPY_SIZE);                        \
        for (d_loop = 0; d_loop < out_len; d_loop++) {          \
            *d_dst++ = X86_INSTR_NOP;                           \
        }                                                       \
        open_interrupt(d_intv);                                 \
        mprotect((void *)ROUND_DOWN(dst, LINUX_MM_ALIGN_SIZE),  \
            LINUX_MM_PROTECT_SIZE, LINUX_MM_UNWRITABLE);        \
    } while (0)

#define X86_OP_REL32            0x01
#define X86_OP_MOVREG           0x02
#define X86_OP_LEA              0x03

#define X86_INSTR_REGMODE       0xC0
#define X86_INSTR_MOVAL         0xB0
#define X86_INSTR_MOV_TO_EBX    0xBB
#define X86_INSTR_JMPFUNC       0xE9
#define X86_INSTR_PUSHEBX       0x53
#define X86_INSTR_PUSHEBP       0x55
#define X86_INSTR_PUSHESI       0x56
#define X86_INSTR_PUSHEDI       0x57
#define X86_INSTR_SUBREGUN      0x81
#define X86_INSTR_SUBREG        0x83
#define X86_INSTR_MOVEBP        0x89
#define X86_INSTR_MOVEBP_OFF    0x8B
#define X86_INSTR_MOVEAX        0xB8
#define X86_INSTR_MOVECX        0xB9
#define X86_INSTR_CALLNEAR      0xE8
#define X86_INSTR_EAX_ADDNUM    0x05
#define X86_INSTR_NOP           0x90
#define X86_INSTR_MAXOPCODE     0x100
#define X86_INSTR_PC_TO_EBX     0xC3241C80
#define X86_INSTR_MOV_NUM_TO_EBPOF  0xC7
#define X86_INSTR_CMPL_GS       0x65
#define X86_INSTR_LEA_EBP_TO_XXX    0x8D

/*
 *  lea sub instruction
 */
#define X86_LEA_EBP_TO_EDX      0x95
#define X86_LEA_EBP_TO_EAX      0x85
#define X86_LEA_EBP_TO_ESI      0x75



struct hook_struct          g_hook_buf[MAX_HOOK_NUM]  = {0};
#define __HK_BUF__          g_hook_buf
#define __HK_IT__(x)        __HK_BUF__[x]
#define __HK_IT_ADDR__(x)   (&__HK_BUF__[x])
#define __HK_IT_VALID__(x)  __HK_IT__(x).valid_flag
#define __HK_IT_SRC__(x)    __HK_IT__(x).hsrc
#define __HK_IT_X0__(x)     __HK_IT__(x).hx0
#define __HK_IT_X1__(x)     __HK_IT__(x).hx1
#define __HK_IT_ACTION__(x) __HK_IT__(x).action
#define __HK_IT_SINSTR__(x) __HK_IT__(x).src_instr
#define __HK_IT_EINSTR__(x) __HK_IT__(x).exe_instr

struct decoder_struct       g_instr_decoder[X86_INSTR_MAXOPCODE]    = {0};
#define __XDER__            g_instr_decoder
struct decoder_struct       g_instr_decoder_lea[X86_INSTR_MAXOPCODE]    = {0};
#define __XDER_LEA__        g_instr_decoder_lea
static void __NF__() {}


/*
 *  instruction decoder function
 */
uint x86_decoder(uchar **dst, uchar **src)
{
    uint tmp_value, inst, length;
    uchar * func    = *src;
    uchar * asm_instr   = *dst;
    uchar op_code   = *func;

    length = __XDER__[op_code].length;

    if (X86_OP_REL32 & __XDER__[op_code].flag) {
        func++;
        tmp_value = *func;
        func++;
        tmp_value += *func << 8;
        func++;
        tmp_value += *func << 16;
        func++;
        tmp_value += *func << 24;
        func++;
        inst = *(unsigned int *)(tmp_value + func);
        if (X86_INSTR_PC_TO_EBX == inst) {
            op_code     = X86_INSTR_MOV_TO_EBX;
            tmp_value   = (uint)func;
        } else {
            tmp_value += (uint)func;
            tmp_value -= (uint)asm_instr + 5;
        }

        *asm_instr++  = op_code;
        *asm_instr++  = GET_BYTE1(tmp_value);
        *asm_instr++  = GET_BYTE2(tmp_value);
        *asm_instr++  = GET_BYTE3(tmp_value);
        *asm_instr++  = GET_BYTE4(tmp_value);
    } else {
        if ((X86_OP_MOVREG & __XDER__[op_code].flag)
            && (X86_INSTR_REGMODE != (*(func + 1) & X86_INSTR_REGMODE))
            && X86_INSTR_MOVEBP_OFF != op_code) {
            length++;
        }

        for (inst = 0; inst < length; inst++) {
            *asm_instr++  = *func++;
        }
    }

    *src    = func;
    *dst    = asm_instr;

    return length;
}

/*
 *  decoder for lea instruction
 */
uint x86_decoder_lea(uchar **dst, uchar **src)
{
    uint inst, length;
    uchar *func     = *src;
    uchar *asm_instr      = *dst;
    uchar op_code   = *(func + 1);

    length = __XDER_LEA__[op_code].length;
    for (inst = 0; inst < length; inst++) {
        *asm_instr++  = *func++;
    }

    *src = func;
    *dst = asm_instr;

    return length;
}

/*
 *  init known instruction
 */
void x86_init_decoder()
{
    __XDER__[X86_INSTR_PUSHEBX].op_code     = X86_INSTR_PUSHEBX;
    __XDER__[X86_INSTR_PUSHEBX].length      = 1;
    __XDER__[X86_INSTR_PUSHEBX].decoder     = x86_decoder;
    
    __XDER__[X86_INSTR_PUSHEBP].op_code     = X86_INSTR_PUSHEBP;
    __XDER__[X86_INSTR_PUSHEBP].length      = 1;
    __XDER__[X86_INSTR_PUSHEBP].decoder     = x86_decoder;
    
    __XDER__[X86_INSTR_PUSHESI].op_code     = X86_INSTR_PUSHESI;
    __XDER__[X86_INSTR_PUSHESI].length      = 1;
    __XDER__[X86_INSTR_PUSHESI].decoder     = x86_decoder;
    
    __XDER__[X86_INSTR_PUSHEDI].op_code     = X86_INSTR_PUSHEDI;
    __XDER__[X86_INSTR_PUSHEDI].length      = 1;
    __XDER__[X86_INSTR_PUSHEDI].decoder     = x86_decoder;
    
    __XDER__[X86_INSTR_SUBREGUN].op_code    = X86_INSTR_SUBREGUN;
    __XDER__[X86_INSTR_SUBREGUN].length     = 6;
    __XDER__[X86_INSTR_SUBREGUN].decoder    = x86_decoder;
    
    __XDER__[X86_INSTR_SUBREG].op_code      = X86_INSTR_SUBREG;
    __XDER__[X86_INSTR_SUBREG].length       = 3;
    __XDER__[X86_INSTR_SUBREG].decoder      = x86_decoder;
    
    __XDER__[X86_INSTR_MOVEBP].op_code      = X86_INSTR_MOVEBP;
    __XDER__[X86_INSTR_MOVEBP].length       = 2;
    __XDER__[X86_INSTR_MOVEBP].decoder      = x86_decoder;
    __XDER__[X86_INSTR_MOVEBP].flag         = X86_OP_MOVREG;
    
    __XDER__[X86_INSTR_MOVEBP_OFF].op_code  = X86_INSTR_MOVEBP_OFF;
    __XDER__[X86_INSTR_MOVEBP_OFF].length   = 3;
    __XDER__[X86_INSTR_MOVEBP_OFF].decoder  = x86_decoder;
    __XDER__[X86_INSTR_MOVEBP_OFF].flag     = X86_OP_MOVREG;
    
    __XDER__[X86_INSTR_MOVEAX].op_code      = X86_INSTR_MOVEAX;
    __XDER__[X86_INSTR_MOVEAX].length       = 5;
    __XDER__[X86_INSTR_MOVEAX].decoder      = x86_decoder;
    
    __XDER__[X86_INSTR_MOVECX].op_code      = X86_INSTR_MOVECX;
    __XDER__[X86_INSTR_MOVECX].length       = 5;
    __XDER__[X86_INSTR_MOVECX].decoder      = x86_decoder;
    
    __XDER__[X86_INSTR_CALLNEAR].op_code    = X86_INSTR_CALLNEAR;
    __XDER__[X86_INSTR_CALLNEAR].length     = 5;
    __XDER__[X86_INSTR_CALLNEAR].decoder    = x86_decoder;
    __XDER__[X86_INSTR_CALLNEAR].flag       = X86_OP_REL32;
    
    __XDER__[X86_INSTR_EAX_ADDNUM].op_code  = X86_INSTR_EAX_ADDNUM;
    __XDER__[X86_INSTR_EAX_ADDNUM].length   = 5;
    __XDER__[X86_INSTR_EAX_ADDNUM].decoder  = x86_decoder;
    
    __XDER__[X86_INSTR_MOV_NUM_TO_EBPOF].op_code    = X86_INSTR_MOV_NUM_TO_EBPOF;
    __XDER__[X86_INSTR_MOV_NUM_TO_EBPOF].length     = 7;
    __XDER__[X86_INSTR_MOV_NUM_TO_EBPOF].decoder    = x86_decoder;
    
    __XDER__[X86_INSTR_CMPL_GS].op_code     = X86_INSTR_CMPL_GS;
    __XDER__[X86_INSTR_CMPL_GS].length      = 8;
    __XDER__[X86_INSTR_CMPL_GS].decoder     = x86_decoder;
    
    __XDER__[X86_INSTR_LEA_EBP_TO_XXX].op_code  = X86_INSTR_LEA_EBP_TO_XXX;
    __XDER__[X86_INSTR_LEA_EBP_TO_XXX].decoder  = x86_decoder_lea;
    __XDER__[X86_INSTR_LEA_EBP_TO_XXX].flag     = X86_OP_LEA;
}

/*
 *  init decoder lea sub instruction
 */
void x86_init_decoder_lea()
{
    __XDER_LEA__[X86_LEA_EBP_TO_EDX].op_code  = X86_LEA_EBP_TO_EDX;
    __XDER_LEA__[X86_LEA_EBP_TO_EDX].length   = 6;
    __XDER_LEA__[X86_LEA_EBP_TO_EDX].decoder  = x86_decoder_lea;
    
    __XDER_LEA__[X86_LEA_EBP_TO_EAX].op_code  = X86_LEA_EBP_TO_EAX;
    __XDER_LEA__[X86_LEA_EBP_TO_EAX].length   = 6;
    __XDER_LEA__[X86_LEA_EBP_TO_EAX].decoder  = x86_decoder_lea;
    
    __XDER_LEA__[X86_LEA_EBP_TO_ESI].op_code  = X86_LEA_EBP_TO_ESI;
    __XDER_LEA__[X86_LEA_EBP_TO_ESI].length   = 3;
    __XDER_LEA__[X86_LEA_EBP_TO_ESI].decoder  = x86_decoder_lea;
}

static void x86_init_page_attr()
{
    mprotect((void *)ROUND_DOWN(g_hook_buf, LINUX_MM_ALIGN_SIZE), 0xC000, LINUX_MM_WRITABLE);
}

static void x86_init_hook_item(struct hook_struct *item)
{
    item->valid_flag    = FALSE;
    item->hsrc          = NULL;
    item->hx0           = NULL;
    item->hx1           = NULL;
    item->action        = ACTION_MAX;
    memset(item->src_instr, 0, sizeof(item->src_instr));
    memset(item->exe_instr, 0, sizeof(item->exe_instr));
}

static void x86_init_hook_array()
{
    int loop;
    for (loop = 0; loop < MAX_HOOK_NUM; loop++) {
        x86_init_hook_item(__HK_IT_ADDR__(loop));
    }
}

static bool is_hook_exist(__HK_HND__ hnd)
{
    int loop = 0;

    for (; loop < MAX_HOOK_NUM; loop++) {
        if (TRUE == __HK_IT_VALID__(loop)
            && (void *)__HK_IT_SRC__(loop) == hnd) {
            return TRUE;
        }
    }

    return FALSE;
}

static ushort get_free_idx()
{
    int loop = 0;

    for (; loop < MAX_HOOK_NUM; loop++) {
        if (FALSE == __HK_IT_VALID__(loop)) {
            return loop;
        }
    }

    return INVALID_IDX;
}

static __XPF__ __entry_callback__(ushort idx)
{
    return __HK_IT_X0__(idx);
}

static __XPF__ __exit_callback__(ushort idx)
{
    return __HK_IT_X1__(idx);
}

static uint __execute__(ushort idx)
{
    return (uint)&(__HK_IT_EINSTR__(idx));
}

static uint x86_get_hook(int action)
{
    uint hook = (uint)__hk_x86_xhooki__;

    switch (action) {
        case ACTION_HOOK:
            hook = (uint)__hk_x86_xhooki__;
            break;
        case ACTION_STUB:
            hook = (uint)__hk_x86_xhookr__;
            break;
        default:
            break;
    }

    return hook;
}

static bool x86_is_adapt_position_func(uchar *src)
{
    uint    call_addr;
    uchar   *   real_addr;
    uchar   *   pos = (uchar *)&call_addr;

    if (*src != X86_INSTR_CALLNEAR) {
        return FALSE;
    }

    if (*(src + 5) != X86_INSTR_EAX_ADDNUM) {
        return FALSE;
    }

    *(pos + 0)  = *(src + 1);
    *(pos + 1)  = *(src + 2);
    *(pos + 2)  = *(src + 3);
    *(pos + 3)  = *(src + 4);

    real_addr = src + 5 + call_addr;

    if (*(real_addr + 0) == 0x8B
        && *(real_addr + 1) == 0x04
        && *(real_addr + 2) == 0x24         /* mov (%esp), %eax */
        && *(real_addr + 3) == 0xC3) {      /* ret              */
        return TRUE;
    }

    return FALSE;
}

static uint x86_adapt_position_func(uchar **dst_addr, uchar **src_addr)
{
    int     offset;
    uint    basic_so_addr;
    uchar   *   pos = (uchar *)&offset;
    uchar   *   src = *src_addr;
    uchar   *   asm_instr = *dst_addr;

    *(pos + 0)  = *(src + 6);
    *(pos + 1)  = *(src + 7);
    *(pos + 2)  = *(src + 8);
    *(pos + 3)  = *(src + 9);

    basic_so_addr = (uint)(src + 5 + offset);
    *asm_instr++  = X86_INSTR_MOVEAX;     /* mov $0x12345678, %eax */
    *asm_instr++  = GET_BYTE1(basic_so_addr);
    *asm_instr++  = GET_BYTE2(basic_so_addr);
    *asm_instr++  = GET_BYTE3(basic_so_addr);
    *asm_instr++  = GET_BYTE4(basic_so_addr);
    *asm_instr++  = X86_INSTR_NOP;
    *asm_instr++  = X86_INSTR_NOP;
    *asm_instr++  = X86_INSTR_NOP;
    *asm_instr++  = X86_INSTR_NOP;
    *asm_instr++  = X86_INSTR_NOP;

    *src_addr   = src + 10;
    *dst_addr   = asm_instr;

    return 10;
}

static uint x86_copy_src_instruction(struct hook_struct *hk, __XPF__ hsrc, uint size)
{
    uint    loop, length, target_value;
    uchar   *exe_asm    = (uchar *)hk->exe_instr;
    uchar   *src        = (uchar *)hsrc;

    memcpy(hk->src_instr, hsrc, size);
    for (loop = 0; loop < size; loop += length) {
        if (NULL == __XDER__[*src].decoder) {
            PRINTF("not support instruction, 0x%02x\n", *src);
            return ERR_NOT_SUPPORT_INSTR;
        }

        if (TRUE == x86_is_adapt_position_func(src)) {
            length = x86_adapt_position_func(&exe_asm, &src);
        } else {
            length = __XDER__[*src].decoder(&exe_asm, &src);
        }

        if (0 == length) {
            PRINTF("decoder instruction length zero.\n");
            return ERR_DECODE_INSTR_LEN_ZERO;
        }
    }

    target_value    = (uint)src - ((uint)exe_asm + 5);
    *exe_asm++  = X86_INSTR_JMPFUNC;
    *exe_asm++  = GET_BYTE1(target_value);
    *exe_asm++  = GET_BYTE2(target_value);
    *exe_asm++  = GET_BYTE3(target_value);
    *exe_asm++  = GET_BYTE4(target_value);

    return OK;
}

static uint x86_stub_instruction(struct hook_struct *hk, ushort idx)
{
    uint    src_addr    = (uint)hk->hsrc;
    uchar   rep_instr[X86_COPY_SIZE]    = {0};
    uint    self    = x86_get_hook(hk->action);
    uint    target_value;
    uchar   *   exe_asm = (uchar *)hk->exe_instr;
    uchar   *   src_asm = (uchar *)hk->src_instr;

    self    = self - (src_addr + X86_COPY_SIZE);
    rep_instr[0]    = X86_INSTR_MOVAL;
    rep_instr[1]    = GET_BYTE1(idx);
    rep_instr[2]    = X86_INSTR_JMPFUNC;
    rep_instr[3]    = GET_BYTE1(self);
    rep_instr[4]    = GET_BYTE2(self);
    rep_instr[5]    = GET_BYTE3(self);
    rep_instr[6]    = GET_BYTE4(self);

    memcpy(hk->src_instr, hk->hsrc, X86_COPY_SIZE);
    target_value    = (uint)src_asm - ((uint)exe_asm + 5);
    *exe_asm++      = X86_INSTR_JMPFUNC;
    *exe_asm++      = GET_BYTE1(target_value);
    *exe_asm++      = GET_BYTE2(target_value);
    *exe_asm++      = GET_BYTE3(target_value);
    *exe_asm++      = GET_BYTE4(target_value);

    __XSET__(rep_instr, hk->hsrc, 0);
    return OK;
}

static uint x86_hook_instruction(struct hook_struct *hk, ushort idx)
{
    uint    ret;
    uint    src     = (uint)hk->hsrc;
    uchar   rep_instr[X86_COPY_SIZE]    = {0};
    uint    self    = x86_get_hook(hk->action);

    self = self - (src + X86_COPY_SIZE);
    rep_instr[0]    = X86_INSTR_MOVAL;
    rep_instr[1]    = GET_BYTE1(idx);
    rep_instr[2]    = X86_INSTR_JMPFUNC;
    rep_instr[3]    = GET_BYTE1(self);
    rep_instr[4]    = GET_BYTE2(self);
    rep_instr[5]    = GET_BYTE3(self);
    rep_instr[6]    = GET_BYTE4(self);

    ret = x86_copy_src_instruction(hk, hk->hsrc, X86_COPY_SIZE);
    if (OK != ret) {
        return ret;
    }

    __XSET__(rep_instr, hk->hsrc, 0);
    return OK;
}

/*
 *  stub function
 *  return value: 
 *      OK  : stub function success
 *      other value: stub failed
 */
uint sjp_stub_func(__HK_HND__ hsrc, __HK_HND__ hdst)
{
    ushort free_idx;

    if (NULL == hsrc || NULL == hdst) {
        PRINTF("stub function input para null\n");
        return ERR_INPUT_PARA;
    }
    
    if (is_hook_exist(hsrc)) {
        PRINTF("stub has exist, not have any sense\n");
        return OK;
    }

    free_idx = get_free_idx();
    if (INVALID_IDX == free_idx) {
        PRINTF("get free index failed\n");
        return ERR_IDX_USED_FULL;
    }
    
    __HK_IT_VALID__(free_idx)   = TRUE;
    __HK_IT_SRC__(free_idx)     = hsrc;
    __HK_IT_X0__(free_idx)      = hdst;
    __HK_IT_ACTION__(free_idx)  = ACTION_STUB;

    if (x86_stub_instruction(__HK_IT_ADDR__(free_idx), free_idx)) {
        x86_init_hook_item(__HK_IT_ADDR__(free_idx));
        return -1;
    }

    return 0;
}

/*
 *  hook function
 *  return value: 
 *      OK  : stub function success
 *      other value: hook failed
 */
uint sjp_hook_func(__HK_HND__ hsrc, __HK_HND__ hx0, __HK_HND__ hx1)
{
    ushort free_idx;

    if (NULL == hsrc || (NULL == hx0 && NULL == hx1)) {
        PRINTF("hook function input para null\n");
        return ERR_INPUT_PARA;
    }

    if (is_hook_exist(hsrc)) {
        PRINTF("hook has exist, not have any sense\n");
        return OK;
    }
    
    free_idx = get_free_idx();
    if (INVALID_IDX == free_idx) {
        PRINTF("get free index failed\n");
        return ERR_IDX_USED_FULL;
    }
    
    __HK_IT_VALID__(free_idx)   = TRUE;
    __HK_IT_SRC__(free_idx)     = hsrc;
    __HK_IT_X0__(free_idx)      = (NULL == hx0) ? __NF__ : hx0;
    __HK_IT_X1__(free_idx)      = (NULL == hx1) ? __NF__ : hx1;
    __HK_IT_ACTION__(free_idx)  = ACTION_HOOK;

    if (x86_hook_instruction(__HK_IT_ADDR__(free_idx), free_idx)) {
        x86_init_hook_item(__HK_IT_ADDR__(free_idx));
        return -1;
    }

    return OK;
}

/*
 *  unhook function
 */
uint sjp_unhook_func(__HK_HND__ hsrc)
{
    int loop;

    if (NULL == hsrc) {
        PRINTF("unhook function input para null\n");
        return ERR_INPUT_PARA;
    }

    for (loop = 0; loop < MAX_HOOK_NUM; loop++) {
        if (hsrc == __HK_IT_SRC__(loop)
            && TRUE == __HK_IT_VALID__(loop)) {
            break;
        }
    }

    if (loop >= MAX_HOOK_NUM) {
        PRINTF("not found unhook function\n");
        return ERR_UNHOOK_NOT_FOUND;
    }

    if (__HK_IT_ACTION__(loop) != ACTION_HOOK) {
        PRINTF("unhook function action not matched\n");
        return ERR_UNHOOK_ACTION_NOT_MACTH;
    }

    __XSET__(__HK_IT_SINSTR__(loop), __HK_IT_SRC__(loop), 0);
    x86_init_hook_item(__HK_IT_ADDR__(loop));

    return OK;
}

/*
 *  unstub function
 */
uint sjp_unstub_func(__HK_HND__ hsrc)
{
    int loop;

    if (NULL == hsrc) {
        PRINTF("unstub function input para null\n");
        return ERR_INPUT_PARA;
    }

    for (loop = 0; loop < MAX_HOOK_NUM; loop++) {
        if (hsrc == __HK_IT_SRC__(loop)
            && TRUE == __HK_IT_VALID__(loop)) {
            break;
        }
    }

    if (loop >= MAX_HOOK_NUM) {
        PRINTF("not found unstub function\n");
        return ERR_UNSTUB_NOT_FOUND;
    }

    if (__HK_IT_ACTION__(loop) != ACTION_STUB) {
        PRINTF("unstub function action not matched\n");
        return ERR_UNSTUB_ACTION_NOT_MACTH;
    }

    __XSET__(__HK_IT_SINSTR__(loop), __HK_IT_SRC__(loop), 0);
    x86_init_hook_item(__HK_IT_ADDR__(loop));

    return OK;

}

/*
 *  unhook all function
 */
void sjp_unhook_all()
{
    int loop;

    for (loop = 0; loop < MAX_HOOK_NUM; loop++) {
        if (TRUE == __HK_IT_VALID__(loop)
            && __HK_IT_ACTION__(loop) == ACTION_HOOK) {
            __XSET__(__HK_IT_SINSTR__(loop), __HK_IT_SRC__(loop), 0);
            x86_init_hook_item(__HK_IT_ADDR__(loop));
        }
    }
}

/*
 *  unstub all function
 */
void sjp_unstub_all()
{
    int loop;

    for (loop = 0; loop < MAX_HOOK_NUM; loop++) {
        if (TRUE == __HK_IT_VALID__(loop)
            && __HK_IT_ACTION__(loop) == ACTION_STUB) {
            __XSET__(__HK_IT_SINSTR__(loop), __HK_IT_SRC__(loop), 0);
            x86_init_hook_item(__HK_IT_ADDR__(loop));
        }
    }
}

/*
 *  hk init function
 */
void sjp_hk_init()
{
    if (g_hook_inited) {
        return;
    }

    x86_init_decoder();
    x86_init_decoder_lea();
    x86_init_hook_array();
    x86_init_page_attr();
    g_hook_inited = 1;
}


