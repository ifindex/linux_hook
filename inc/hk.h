/*
 *
 *  hook function header
 *
 *  author  : sunjianping    (ifindex@163.com)
 *  date    : 2018/11/19
 *  
 *  Copyright All Reserved 2018-2020 by sunjianping (ifindex@163.com)
 *  
 *
 */
#ifndef __HK_H__
#define __HK_H__

#define HKASM                   __asm__
#define ROUND_DOWN(x, align)    ((uint)(x) & ~(align - 1))

#define MAX_HOOK_NUM            512

typedef void (* __XPF__)(void);
typedef uint (* __XDECODER__)(uchar **, uchar **);
typedef void *  __HK_HND__;

#define LINUX_MM_ALIGN_SIZE     0x1000
#define LINUX_MM_PROTECT_SIZE   0x2000

#define LINUX_MM_UNWRITABLE     5
#define LINUX_MM_WRITABLE       7

#define SRC_INSTR_SIZE          5
#define EXE_INSTR_SIZE          14

#define GET_BYTE1(x)            ((uint)(x) & 0x000000FF)
#define GET_BYTE2(x)            (((uint)(x) & 0x0000FF00) >> 8)
#define GET_BYTE3(x)            (((uint)(x) & 0x00FF0000) >> 16)
#define GET_BYTE4(x)            (((uint)(x) & 0xFF000000) >> 24)

#define ACTION_HOOK             1
#define ACTION_STUB             2
#define ACTION_MAX              255

#define INVALID_IDX             0xFFFF


/*
 *  hook item struct in global array
 */
struct hook_struct {
    int     valid_flag;         /* item valid flag */
    __XPF__ hsrc;               /* source function */
    __XPF__ hx0;                /* x0 address */
    __XPF__ hx1;                /* x1 address */
    int     action;             /* hook action type */
    uint src_instr[SRC_INSTR_SIZE]; /* backup source instrution */
    uint exe_instr[EXE_INSTR_SIZE]; /* replaced instruction */
};

/*
 *  instruction decoder
 */
struct decoder_struct {
    uchar   op_code;            /* operation code */
    uchar   length;             /* instruction length */
    ushort  flag;               /* instruction flag */
    __XDECODER__    decoder;    /* instruction decoder */
};

#endif  /* __HK_H__ */
