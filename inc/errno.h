/*
 *
 *  error number header
 *
 *  author  : sunjianping    (ifindex@163.com)
 *  date    : 2018/12/01
 *  
 *  Copyright All Reserved 2018-2020 by sunjianping (ifindex@163.com)
 *  
 *
 */
#ifndef __ERRNO_H__
#define __ERRNO_H__


/*
 *  error number define
 */
enum errno {
    ERR_FROM_USER       = 0x10000000,   /* generate error from user         */
    ERR_INPUT_PARA,                     /* input para not correct           */
    ERR_IDX_USED_FULL,                  /* used full index                  */
    ERR_UNHOOK_NOT_FOUND,               /* not found unhook function        */
    ERR_UNHOOK_ACTION_NOT_MACTH,        /* unhook action not matched        */
    ERR_UNSTUB_NOT_FOUND,               /* not found unstub function        */
    ERR_UNSTUB_ACTION_NOT_MACTH,        /* unstub action not matched        */
    
    ERR_FROM_INNER      = 0x20000000,   /* generate error from inner proc   */
    ERR_NOT_SUPPORT_INSTR,              /* not support instruction          */
    ERR_DECODE_INSTR_LEN_ZERO,          /* decode instruction length zero   */
};


#endif  /* __ERRNO_H__ */
