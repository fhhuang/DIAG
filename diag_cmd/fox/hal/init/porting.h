/***************************************************************************
***
***    Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without
***    the written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.
***    Co. Ltd. company private, proprietary, and trade secret property and
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************
***
***  Filename :
***     porting.h
***
***  Description:
***     the header file defines the common porting function
***
***  History:
***       - 2009/05/21, 14:30:52, Eden Weng
***             File Creation
***
*************************************************************************/

#ifndef _PORTING_H
#define _PORTING_H

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 *      Library Inclusion Segment
 *
 *========================================================================
 */
/* Swap tool */

/* 16bit nibble swap. For example 0x1234 -> 0x2143                          */
#define FOX_NIBBLE_SWAP_16BIT(X)      (((X & 0xf) << 4) |     \
                                       ((X & 0xf0) >> 4) |    \
                                       ((X & 0xf00) << 4) |   \
                                       ((X & 0xf000) >> 4))

/* 32bit nibble swap. For example 0x12345678 -> 0x21436587                  */
#define FOX_NIBBLE_SWAP_32BIT(X)      (((X & 0xf) << 4) |       \
                                       ((X & 0xf0) >> 4) |      \
                                       ((X & 0xf00) << 4) |     \
                                       ((X & 0xf000) >> 4) |    \
                                       ((X & 0xf0000) << 4) |   \
                                       ((X & 0xf00000) >> 4) |  \
                                       ((X & 0xf000000) << 4) | \
                                       ((X & 0xf0000000) >> 4))

/* 16bit byte swap. For example 0x1122 -> 0x2211                            */
#define FOX_BYTE_SWAP_16BIT(X)        ((((X) & 0xff) << 8) | (((X) & 0xff00) >> 8))

/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
#define FOX_BYTE_SWAP_32BIT(X)        ((((X) & 0xff) << 24) |                       \
                                       (((X) & 0xff00) << 8) |                      \
                                       (((X) & 0xff0000) >> 8) |                    \
                                       (((X) & 0xff000000) >> 24))

/* 64bit byte swap. For example 0x11223344.55667788 -> 0x88776655.44332211  */
#define FOX_BYTE_SWAP_64BIT(X)        ((l64) ((((X) & 0xffULL) << 56) |             \
                                       (((X) & 0xff00ULL) << 40) |           \
                                       (((X) & 0xff0000ULL) << 24) |         \
                                       (((X) & 0xff000000ULL) << 8) |        \
                                       (((X) & 0xff00000000ULL) >> 8) |      \
                                       (((X) & 0xff0000000000ULL) >> 24) |   \
                                       (((X) & 0xff000000000000ULL) >> 40) | \
                                       (((X) & 0xff00000000000000ULL) >> 56)))


/* Endianess macros.                                                        */
/* default Big Endian */
#if !defined(FOX_CPU_LE) && !defined(FOX_CPU_BE)
#define FOX_CPU_BE
#endif

#if defined(FOX_CPU_LE)
    #define FOX_16BIT_LE(X)  (X)
    #define FOX_32BIT_LE(X)  (X)
    #define FOX_64BIT_LE(X)  (X)
    #define FOX_16BIT_BE(X)  FOX_BYTE_SWAP_16BIT (X)
    #define FOX_32BIT_BE(X)  FOX_BYTE_SWAP_32BIT (X)
    #define FOX_64BIT_BE(X)  FOX_BYTE_SWAP_64BIT (X)
#elif defined(FOX_CPU_BE)
    #define FOX_16BIT_LE(X)  FOX_BYTE_SWAP_16BIT (X)
    #define FOX_32BIT_LE(X)  FOX_BYTE_SWAP_32BIT (X)
    #define FOX_64BIT_LE(X)  FOX_BYTE_SWAP_64BIT (X)
    #define FOX_16BIT_BE(X)  (X)
    #define FOX_32BIT_BE(X)  (X)
    #define FOX_64BIT_BE(X)  (X)
#else
    #error "CPU endianess isn't defined!\n"
#endif 
/*========================================================================
 *
 *      Constant Definition Segment
 *
 *========================================================================
 */
#ifdef CFG_MDK
#include "cdk_printf.h"
#include "cdk_stdlib.h"

#include "cfe.h"
#endif
/*==========================================================================
 *
 *      MIB Definition segment
 *
 *==========================================================================
 */

#if !(defined(WIN32) || defined (_PC_SIMULATOR_) || defined(FOX_KERNEL))
#include <stdarg.h>

int    sprintf(char * buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

void    putc(const char c);
void    puts(const char *s);
/* stdin */
int    getc(void);
int    tstc(void);


#ifndef CFG_MDK
/* stdout */
//void    printf(const char *fmt, ...);

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

/* move standard lib definition here. In MDK use their macro */
void * memset(void * s,int c,unsigned int count);
void * memcpy(void * dest,const void *src,unsigned int count);
int memcmp(const void * cs,const void * ct, size_t count);

int strcmp ( const char * str1, const char * str2 );
char * strcpy(char * dest,const char *src);
char * strncpy(char * dest,const char *src,unsigned int count);

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);

#define MEMORY_ALLOC(size)        ((UINT32*)0)
#define MEMORY_FREE(addr)       
int    ctrlc (void);
void udelay(unsigned long usec);
#define GET_CHAR    getc()

#else
#define simple_strtoul CDK_STRTOUL

#define MEMORY_ALLOC(size)  KMALLOC(size, 0)
#define MEMORY_FREE(addr)   KFREE((void *)addr)
#define udelay(usec)  cfe_usleep(usec)

#if 0
#define GET_CHAR(x)     while (console_read(&(x),1) != 1) { POLL(); }
#define ctrlc()  0
#else
static inline unsigned char cdk_getc(void)
{
    unsigned char getChar;
    while (console_read(&(getChar),1) != 1)
    {
        POLL();
    }
    return getChar;
}
#define GET_CHAR    cdk_getc()
#define ctrlc()    ((console_status()==1)?((cdk_getc()==0x18)?1:0):0)
#endif

#endif

unsigned long	get_timer(unsigned long base);
//char	*getenv(char *);
int getenv_r (char *name, char *buf, unsigned len);

void	pci_init(void);


unsigned long long get_ticks(void);

#define FFLUSH_STD_OUT
#define time(x)        x
#define srand(x)    if(x)
#define RAND        i //get_ticks() 

#define uartAttrSet(a)
#define uartAttrRestore()

#else

#if defined(FOX_KERNEL)

#ifdef __KERNEL__ /* ko */
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/types.h>
#include <linux/module.h>
#define printf printk
#define malloc(size) kmalloc(size, GFP_KERNEL)
#define free(size) kfree(size)
#else
#include <stdio.h>
#include <stdlib.h>

#define simple_strtoul    strtoul

#endif
#include <errno.h>
#include <string.h>

//#include <linux/compiler.h>
extern int is_nowaitting_input;
#define GET_CHAR    getc(stdin) 
//#define GET_CHAR xGET_CHAR()
#define FFLUSH_STD_OUT    fflush(stdout)

#define RAND    rand()

#include <termios.h>
#include <fcntl.h> 


#define uartAttrSet(blocking) \
{ \
    struct termios tio; \
    int   iflg; \
    \
    /* no need input "enter" */  \
    tcgetattr( fileno(stdin), &tio ); \
    tio.c_lflag &= ~ICANON; \
    tio.c_lflag &= ~ECHO; \
    tcsetattr( fileno(stdin), TCSANOW, &tio ); \
    \
    /* non blocking for getc */ \
    if( blocking == 0 ) \
    { \
        iflg   =   fcntl(fileno(stdin),F_GETFL,0); \
        fcntl(fileno(stdin),F_SETFL,iflg|O_NONBLOCK); \
    } \
}

#define uartAttrRestore() \
{ \
    struct termios tio; \
    int   iflg; \
    \
    /* no need input "enter" */  \
    tcgetattr( fileno(stdin), &tio ); \
    tio.c_lflag |= ICANON; \
    tio.c_lflag |= ECHO; \
    tcsetattr( fileno(stdin), TCSANOW, &tio ); \
    \
    /* non blocking for getc */ \
    iflg   =   fcntl(fileno(stdin),F_GETFL,0); \
    fcntl(fileno(stdin),F_SETFL,iflg&(~O_NONBLOCK)); \
}

#define ctrlc()    ((getc(stdin)==0x18)?1:0)

#define MEMORY_ALLOC(size)       malloc(size)
#define MEMORY_FREE(addr)        if( addr ) free((void*)addr)

#ifndef CPU_ARM
#include <unistd.h>

#ifndef __KERNEL__ /* process */


#ifndef setf
#define setf(flags,mask) (flags |= mask)
#endif

#ifndef clrf
#define clrf(flags,mask) (flags &= ~mask)
#endif

#ifndef testf
#define testf(flags,mask) (flags & mask)
#endif


#define c_udelay(usec) { \
        struct timeval tv; \
        tv.tv_sec = 0; \
        tv.tv_usec = usec; \
        select(0, NULL, NULL,  NULL, &tv); \
}

#define c_delay(sec) { \
        struct timeval tv; \
        tv.tv_sec = sec; \
        tv.tv_usec = 0; \
        select(0, NULL, NULL,  NULL, &tv); \
}


#define udelay(usec)        c_udelay(usec)
#define delay(sec)  c_delay(sec)
#else
void	udelay	      (unsigned long usec);
#endif

#include <time.h>
#endif /* temparily modified for G2424 used because in G2424 can't find usleep defition */

#define get_timer(x)        (clock()/1000 - x)

#ifdef INDEPENDENCE_PROC
#define U_BOOT_CMD(...)  
#endif

/* added macro for memory barrier synchronize   */
#ifdef PPC_CPU
#ifdef __GNUC__
#ifdef _FreeBSD
#define CPU_SYNC __asm __volatile (" eieio")
#else
#define CPU_SYNC __asm__("   eieio")
#endif
#else
#ifdef _DIAB_TOOL
#define CPU_SYNC asm(" eieio ")
#else
#error
#endif
#endif
#endif

#ifdef MIPS_CPU
#define CPU_SYNC __asm__ __volatile__ ("sync")
#endif

#ifdef INTEL_CPU
#define CPU_SYNC
#endif

#ifdef CPU_ARM
#define CPU_SYNC
#endif


#endif

#endif

#ifdef __cplusplus
}
#endif

#endif    /* _PORTING_H */

