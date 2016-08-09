//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELASTOS_ASM_H__
#define __ELASTOS_ASM_H__

#if defined(_mips) || (__GNUC__ >= 4)
#define C_SYMBOL(symbol)    symbol
#define C_PREFIX
#else
#define C_SYMBOL(symbol)    _##symbol
#define C_PREFIX            "_"
#endif

#define CODEINIT    .text.init
#define DATAINIT    .data.init

#endif //__ELASTOS_ASM_H__
