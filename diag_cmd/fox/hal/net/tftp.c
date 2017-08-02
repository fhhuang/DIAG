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
***    FILE NAME :
***      tftp.c
***
***    DESCRIPTION :
***      for TFTP component
***
***    HISTORY :
***       - 2015/01/15, 16:30:52, Chungmin Lai
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <dirent.h>

#include "porting.h"
#include "net.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define HASHES_PER_LINE     65      /* Number of "loading" hashes per line  */

/*
 *  TFTP operations.
 */
#define TFTP_RRQ            1
#define TFTP_WRQ            2
#define TFTP_DATA           3
#define TFTP_ACK            4
#define TFTP_ERROR          5
#define TFTP_OACK           6

#define STATE_RRQ           1
#define STATE_DATA          2
#define STATE_TOO_LARGE     3
#define STATE_BAD_MAGIC     4
#define STATE_OACK          5

#define TFTP_BLOCK_SIZE     512                 /* default TFTP block size  */
#define TFTP_SEQUENCE_SIZE  ((ulong)(1<<16))    /* sequence number is 16 bit */

#define WELL_KNOWN_PORT     69                  /* Well known TFTP port #       */
#define TIMEOUT             10                  /* Seconds to timeout for a lost pkt    */

#define MTD0_SIZE           2*1024*1024
#define MTD1_SIZE           16*1024*1024
#define MTD2_SIZE           16*1024*1024
#define MTD3_SIZE           16*1024*1024
#define MTD4_SIZE           16*1024*1024

#define BLOCK_SIZE          1024*64
#define POLL_TIME           1000000

#define UBOOTMTD            "/dev/mtd0"
#define UBOOTENVMTD         "/dev/mtd1"
#define CONFIGMTD           "/dev/mtd2"
#define ROOTFSMTD           "/dev/mtd2"
#define SMALLMTD            "/dev/mtd4"

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Static Variable segment
 *
 *==========================================================================
 */
static int      TftpServerPort;       /* The UDP port at their end        */
static int      TftpOurPort;          /* The UDP port at our end      */
static ulong    TftpBlock;            /* packet sequence number       */
static ulong    TftpLastBlock;        /* last packet sequence number received */
static ulong    TftpBlockWrap;        /* count of sequence number wraparounds */
static ulong    TftpBlockWrapOffset;  /* memory offset due to wrapping    */
static int      TftpState;

extern IPaddr_t NetOurIP;             /* Our IP addr (0 = unknown)        */
extern IPaddr_t NetServerIP;          /* Our IP addr (0 = unknown)        */
extern uchar    NetServEther[6];

uchar           NetTxPacket[1518+64];
static char     tftp_filename[256]={0};
static char     tftp_savefilename[256]={0};

int             customer_flash=0, diag_image=0;
int             check_finish=0;
int             tftp_time_out=0;
int             tftp_alloc_buffer(void);
int             tftp_free_buffer(void);

ulong           tftp_file_size=0;
uchar           *load_addr=0;

char            flashname[64], srcMtdName[64];
int             isMtd=1;
int             isFW=0;


/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
int write_to_mtd(char *dstname,char *srcbuf,int upgradesize);

/*==========================================================================
 *
 *      Static Funtion Body segment
 *
 *==========================================================================
 */
static __inline__ void
store_block (unsigned block, uchar * src, unsigned len)
{
    ulong offset = block * TFTP_BLOCK_SIZE + TftpBlockWrapOffset;
    ulong newsize = offset + len;
#ifdef CFG_DIRECT_FLASH_TFTP
    int i, rc = 0;

    for (i=0; i<CFG_MAX_FLASH_BANKS; i++) {
        /* start address in flash? */
        if (load_addr + offset >= flash_info[i].start[0]) {
            rc = 1;
            break;
        }
    }

    if (rc) { /* Flash is destination for this packet */
        rc = flash_write ((char *)src, (ulong)(load_addr+offset), len);
        if (rc) {
            flash_perror (rc);
            NetState = NETLOOP_FAIL;
            return;
        }
    }
    else
#endif /* CFG_DIRECT_FLASH_TFTP */
    if(load_addr)
    {
        (void)memcpy((void *)(load_addr + offset), src, len);
    }

    if (tftp_file_size < newsize)
    {
        tftp_file_size = newsize;
    }
}

static void
TftpSend (void)
{
    volatile uchar *    pkt;
    volatile uchar *    xp;
    int         len = 0, i;
    volatile ushort *s;

    /*  We will always be sending some sort of packet, so
     *  cobble together the packet headers now.
     */
    pkt = NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE;

    switch (TftpState) {
    case STATE_RRQ:
        xp = pkt;
        s = (ushort *)pkt;
        *s++ = htons(TFTP_RRQ);
        pkt = (uchar *)s;
        strcpy ((char *)pkt, tftp_filename);
        pkt += strlen(tftp_filename) + 1;
        strcpy ((char *)pkt, "octet");
        pkt += 5 /*strlen("octet")*/ + 1;
        strcpy ((char *)pkt, "timeout");
        pkt += 7 /*strlen("timeout")*/ + 1;
        sprintf((char *)pkt, "%d", TIMEOUT);
        log_printf("send option \"timeout %s\"\n", (char *)pkt);
        pkt += strlen((char *)pkt) + 1;
        len = pkt - xp;
        break;

    case STATE_DATA:
    case STATE_OACK:
        xp = pkt;
        s = (ushort *)pkt;
        *s++ = htons(TFTP_ACK);
        *s++ = htons(TftpBlock);
        pkt = (uchar *)s;
        len = pkt - xp;
        break;

    case STATE_TOO_LARGE:
        xp = pkt;
        s = (ushort *)pkt;
        *s++ = htons(TFTP_ERROR);
        *s++ = htons(3);
        pkt = (uchar *)s;
        strcpy ((char *)pkt, "File too large");
        pkt += 14 /*strlen("File too large")*/ + 1;
        len = pkt - xp;
        break;

    case STATE_BAD_MAGIC:
        xp = pkt;
        s = (ushort *)pkt;
        *s++ = htons(TFTP_ERROR);
        *s++ = htons(2);
        pkt = (uchar *)s;
        strcpy ((char *)pkt, "File has bad magic");
        pkt += 18 /*strlen("File has bad magic")*/ + 1;
        len = pkt - xp;
        break;
    }
    NetSendUDPPacket(NetTxPacket,NetServEther, NetServerIP, TftpServerPort, TftpOurPort, len);
}

/*==========================================================================
 *
 *      Local Function Body segment
 *
 *==========================================================================
 */
void TftpHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
    ushort proto;
    ushort *s;

    if (dest != TftpOurPort) {
        return;
    }

    if (TftpState != STATE_RRQ && src != TftpServerPort) {
        return;
    }

    if (len < 2) {
        return;
    }
    len -= 2;
    /* warning: don't use increment (++) in ntohs() macros!! */
    s = (ushort *)pkt;
    proto = *s++;
    pkt = (uchar *)s;
    switch (ntohs(proto)) {
    case TFTP_RRQ:
    case TFTP_WRQ:
    case TFTP_ACK:
        break;
    default:
        break;

    case TFTP_OACK:
        TftpState = STATE_OACK;
        TftpServerPort = src;
        TftpSend (); /* Send ACK */
        break;
    case TFTP_DATA:
        if (len < 2)
            return;
        len -= 2;
        TftpBlock = ntohs(*(ushort *)pkt);

        /* RFC1350 specifies that the first data packet will
         * have sequence number 1. If we receive a sequence
         * number of 0 this means that there was a wrap
         * around of the (16 bit) counter.
         */
        if (TftpBlock == 0) {
            TftpBlockWrap++;
            TftpBlockWrapOffset += TFTP_BLOCK_SIZE * TFTP_SEQUENCE_SIZE;
            printf ("\n\t %lu MB reveived\n\t ", TftpBlockWrapOffset>>20);
        } else {
            if (((TftpBlock - 1) % 10) == 0) {
                putc ('#',stdout);
            } else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0) {
                fputs ("\n",stdout);
            }
        }

#ifdef ET_DEBUG
        if (TftpState == STATE_RRQ) {
            fputs ("Server did not acknowledge timeout option!\n",stdout);
        }
#endif

        if (TftpState == STATE_RRQ || TftpState == STATE_OACK) {
            /* first block received */
            TftpState = STATE_DATA;
            TftpServerPort = src;
            TftpLastBlock = 0;
            TftpBlockWrap = 0;
            TftpBlockWrapOffset = 0;

            if (TftpBlock != 1) {   /* Assertion */
                printf ("\nTFTP error: "
                    "First block is not block 1 (%ld)\n"
                    "Starting again\n\n",
                    TftpBlock);
                break;
            }
        }

        if (TftpBlock == TftpLastBlock) {
            /*
             *  Same block again; ignore it.
             */
            break;
        }

        TftpLastBlock = TftpBlock;

        store_block (TftpBlock - 1, pkt + 2, len);

        /*  Acknoledge the block just received, which will prompt
         *  the server for the next one.
         */
        TftpSend ();

        if (len < TFTP_BLOCK_SIZE) {
            /*  We received the whole thing.  Try to
             *  run it.
             */
            fputs ("\ndone\n",stdout);
                    log_printf("Recv size %d \n",tftp_file_size);
                    check_finish=1;
        }
        break;

    case TFTP_ERROR:
        printf ("\nTFTP error: '%s' (%d)\n",
                    pkt + 2, ntohs(*(ushort *)pkt));
        fputs ("Starting again\n\n",stdout);
        break;
    }
}

void
TftpStart (void)
{
    if(!strlen(tftp_filename))
    {
        log_printf("set get file name first\n");
        return;
    }

    tftp_alloc_buffer();

    puts ("TFTP from server "); print_IPaddr (NetServerIP);
    puts ("our IP address is ");  print_IPaddr (NetOurIP);

    log_printf("Filename '%s'.", tftp_filename);
    log_printf("\r\nPress '1' if need to terminate TFTP transmission\r\n.");
    TftpServerPort = WELL_KNOWN_PORT;
    TftpState = STATE_RRQ;
    /* Use a pseudo-random port unless a specific port is set */
    TftpOurPort = 37234;

    TftpBlock = 0;
    tftp_file_size=0;
    check_finish=0;
    TftpSend ();
}

void tftpsetfilename(char *filestr)
{
    if(!filestr)
        return;

    strcpy(tftp_filename,filestr);
    return;

}

void tftpsetsavefilename(char *filestr)
{
    if(!filestr)
        return;

    strcpy(tftp_savefilename,filestr);
    return;
}

int tftp_alloc_buffer(void)
{
    if(!load_addr)
    {
        load_addr=malloc(1024*1024*30); /* 30M */
    }

    return 0;
}

int tftp_free_buffer(void)
{
    if(load_addr)
    {
        free(load_addr);
        load_addr=0;
    }

    return 0;
}

int tftp_set_flashname(int func, int mtd_index)
{
    memset(flashname,0,64);
    memset(srcMtdName,0,64);

    switch(func){
    case 0:
        sprintf(flashname,"/dev/mtd%s\0", mtd_index);
        sprintf(srcMtdName,"/dev/mtdblock%s\0", mtd_index);
        customer_flash=0;
        diag_image=1;
        isMtd=1;
        break;
    case 1:
        strcpy(flashname,SMALLMTD);
        customer_flash=0;
        isMtd=1;
        break;
    case 2:
        sprintf(flashname,"/dev/mtd%s\0", mtd_index);
        customer_flash=0;
        diag_image=0;
        isMtd=1;
        break;
    case 4:
        sprintf(flashname,"/dev/mtd%s\0", mtd_index);
        customer_flash=1;
        diag_image=0;
        isMtd=1;
        break;
    case 3:
        memset(flashname,64,0);
        if(strlen(tftp_savefilename))
            sprintf(flashname,"/tmp/%s\0",tftp_savefilename);
        else
            sprintf(flashname,"/tmp/%s\0",tftp_filename);
        customer_flash=0;
        isFW=1;
        break;
    default:
        customer_flash=0;
        strcpy(flashname,ROOTFSMTD);
        break;
    }

    return 0;
}

int cmp_data(char *srcdat,char *dstdat,int size)
{
    int i;

    for(i=0;i<size;i++){
        if(srcdat[i] !=dstdat[i]){
            return -1;
        }
    }
    return 0;
}

int mtd_erase(char *dstname)
{
    int dstfd;
    struct mtd_info_user meminfo;
    struct erase_info_user er;

    dstfd=open(dstname,O_RDWR|O_SYNC);

    if(dstfd<0){
        log_printf("open dst  %s fail\n",dstname);
        return -1;
    }

    if (ioctl(dstfd, MEMGETINFO, &meminfo))
    {
        log_printf("get mtd info fail\n");
        close(dstfd);
        return -1;
    }

    er.start = 0;
    er.length = meminfo.size;
    if (ioctl(dstfd, MEMERASE, &er)){
        log_printf("\nMTD Erase failure: %d\n");
        close(dstfd);
        return -1;
    }

    log_printf("Flash erasing %s \n",dstname);
    close(dstfd);
    return 0;

}

/* 20160630 - download tftp file and write back to tmp folder */
int write_tftpfileToTmp(char *dstname,char *srcbuf,int upgradesize)
{
    int dstfd;
    int i=0;

    log_dbgPrintf("%s filename=%s size=0x%08x\n",__FUNCTION__, dstname, upgradesize);

    dstfd=open(dstname,O_RDWR|O_TRUNC | O_CREAT);

    if(dstfd<0)
    {
        log_printf("can't open or create dst %s fail\n",dstname);
        return -1;
    }

    lseek(dstfd, 0, SEEK_SET);

    /* Write file */
    for (i=0; i < upgradesize ; i++)
    {
        if(write(dstfd,&srcbuf[i], 1) != 1)
        {
            log_printf("write error!\n");
            close(dstfd);
            return -1;
        }
    }

    sync();
    chmod(dstname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IWOTH);
    close(dstfd);

    return 0;
}

int write_to_mtd(char *dstname,char *srcbuf,int upgradesize)
{
    int dstfd;
    struct mtd_info_user meminfo;
    struct erase_info_user er;
    ulong totalBlockNum=0, sectorSize=0, writeLen=0, readLen=0;
    uchar *databuf, *readBuf, *writeBuf;
    int finishFlag=0;
    loff_t offset=0;
    int i=0, j=0, result=0;
    char file_name[64];
    char buf[256];
    int   *trgt = "/mnt/flash";
    int   *type = "yaffs2";
    unsigned long mntflags = 0;
    fd_set readfs;    /* file descriptor set */
    struct timeval Timeout;
    DIR    *d;
    struct dirent *dir;

    if( diag_image == 0x1)
    {
        /* Upgrade initramfs for YAFFS2 */

        /* mount yaffs2 file system */
        result = mount(srcMtdName, trgt, type, mntflags, NULL);

        if( result != 0 )
        {
            log_printf("fail to mount yaffs2.\n");
            return -1;
        }

        /* List DIR */
        d = opendir("/mnt/flash");
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                memset(buf, 0x0, 256);
                sprintf(buf,"/mnt/flash/%s\0", dir->d_name);
                remove(buf);
            }
            closedir(d);
        }

        sprintf(file_name,"/mnt/flash/%s\0",tftp_filename);

        if ((dstfd = open (file_name, O_CREAT | O_RDWR)) < 0)
        {
            perror(file_name);
            return -1;
        }

        lseek(dstfd, 0, SEEK_SET);

        /* Write file */
        for (i=0; i < upgradesize ; i++)
        {
            if(write(dstfd,&srcbuf[i], 1) != 1)
            {
                log_printf("write error!\n");
                close(dstfd);
                return -1;
            }
        }

        sync();

        chmod(file_name, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IWOTH);
    }
    else
    {
        if(isMtd)
        {
            dstfd=open(dstname,O_RDWR|O_SYNC);
        }
        else
        {
            dstfd=open(dstname,O_RDWR|O_TRUNC | O_CREAT);
        }

        if(dstfd<0)
        {
            log_printf("open dst %s fail\n",dstname);
            return -1;
        }

        if(isMtd)
        {
            if (ioctl(dstfd, MEMGETINFO, &meminfo))
            {
                log_printf("get mtd info fail\n");
                close(dstfd);
                return -1;
            }
        }

        if(meminfo.size < upgradesize)
        {
            log_printf("File size over MTD size\r\n");
            return -1;
        }

        databuf=srcbuf;

        /* Calculate total block number */
        totalBlockNum= meminfo.size/meminfo.erasesize;

        /* Get sector size */
        sectorSize = meminfo.erasesize;

        readBuf = malloc(sectorSize);
        writeBuf = malloc(sectorSize);

        memset(readBuf, 0xff, sizeof(readBuf));
        memset(writeBuf, 0xff, sizeof(writeBuf));

        while( finishFlag == 0x0 )
        {
            offset = i*sectorSize;

            if( upgradesize <  sectorSize)
            {
                sectorSize = upgradesize;
                finishFlag=0x1;
            }
            else
            {
                upgradesize -= sectorSize;
            }

            memcpy(writeBuf, &databuf[offset], sectorSize);

            /* Check Bad sector */
            if (ioctl(dstfd, MEMGETBADBLOCK, &offset))
            {
                /* Skip bad block */
                log_printf("Detect BAD block, skip it.\r\n");
                continue;
            }

            /* Erase */
            er.start = offset;
            er.length = meminfo.erasesize;
            if (ioctl(dstfd, MEMERASE, &er))
            {
                log_printf("\nMTD Erase failure: %d\n");
                free(databuf);
                free(writeBuf);
                free(readBuf);
                close(dstfd);
                return -1;
            }

            /* Write */
            fflush(stdout);
            writeLen = pwrite(dstfd, writeBuf, meminfo.erasesize, offset);
            if (writeLen < 0)
            {
                log_printf ("NAND write to offset %x failed %d\n",offset, writeLen);
                free(writeBuf);
                free(databuf);
                free(readBuf);
                close(dstfd);
                return -1;
            }

            if (writeLen < sectorSize)
            {
                log_printf ("NAND write to offset %x failed %d\n",offset, writeLen);
                free(databuf);
                free(writeBuf);
                free(readBuf);
                close(dstfd);
                return -1;
            }

            /* Read back and compare */
            fflush(stdout);
            readLen = pread(dstfd, readBuf, sectorSize, offset);
            if (readLen < sectorSize)
            {
                log_printf ("NAND read to offset %x failed %d\n", offset, readLen);
                free(databuf);
                free(writeBuf);
                free(readBuf);
                close(dstfd);
                return -1;
            }

            for(j=0;j<sectorSize;j++)
            {
                if( *(writeBuf+j) != *(readBuf+j))
                {
                    log_printf("NAND comapre fail at %lx \n",offset+j);
                    free(databuf);
                    free(writeBuf);
                    free(readBuf);
                    close(dstfd);
                    return -1;
                }
            }
            i++;
        }
        free(writeBuf);
        free(readBuf);
    }

    close(dstfd);
    if( diag_image == 0x1 )
    {
        umount(trgt);
    }
    return 0;
}

int write_to_all(void)
{
    uchar *write_addr;
    ulong writesize;

    if(tftp_file_size <= MTD0_SIZE){
        write_addr=load_addr;
        writesize=tftp_file_size;
        if(write_to_mtd(UBOOTMTD,write_addr,writesize))
            goto fail;

        mtd_erase(UBOOTENVMTD);
        mtd_erase(CONFIGMTD);
        mtd_erase(ROOTFSMTD);
        mtd_erase(SMALLMTD);
    }
    else if (tftp_file_size <= (MTD0_SIZE+MTD1_SIZE)){
        write_addr=load_addr;
        writesize=MTD0_SIZE;

        if(write_to_mtd(UBOOTMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE;
        writesize=tftp_file_size-MTD0_SIZE;

        if(write_to_mtd(UBOOTENVMTD,write_addr,writesize))
            goto fail;

        mtd_erase(CONFIGMTD);
        mtd_erase(ROOTFSMTD);
        mtd_erase(SMALLMTD);

    }
    else if (tftp_file_size <= (MTD0_SIZE+MTD1_SIZE+MTD2_SIZE)){
        write_addr=load_addr;
        writesize=MTD0_SIZE;
        if(write_to_mtd(UBOOTMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE;
        writesize=MTD1_SIZE;
        if(write_to_mtd(UBOOTENVMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE+MTD1_SIZE;
        writesize=tftp_file_size-(MTD0_SIZE+MTD1_SIZE);
        if(write_to_mtd(CONFIGMTD,write_addr,writesize))
            goto fail;

        mtd_erase(ROOTFSMTD);
        mtd_erase(SMALLMTD);
    }
    else if (tftp_file_size <= (MTD0_SIZE+MTD1_SIZE+MTD2_SIZE+MTD3_SIZE)){

        write_addr=load_addr;
        writesize=MTD0_SIZE;
        if(write_to_mtd(UBOOTMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE;
        writesize=MTD1_SIZE;
        if(write_to_mtd(UBOOTENVMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE+MTD1_SIZE;
        writesize=MTD2_SIZE;
        if(write_to_mtd(CONFIGMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE+MTD1_SIZE+MTD2_SIZE;
        writesize=tftp_file_size-(MTD0_SIZE+MTD1_SIZE+MTD2_SIZE);;
        if(write_to_mtd(ROOTFSMTD,write_addr,writesize))
            goto fail;

        mtd_erase(SMALLMTD);

    }else{

        write_addr=load_addr;
        writesize=MTD0_SIZE;
        if(write_to_mtd(UBOOTMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE;
        writesize=MTD1_SIZE;
        if(write_to_mtd(UBOOTENVMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE+MTD1_SIZE;
        writesize=MTD2_SIZE;
        if(write_to_mtd(CONFIGMTD,write_addr,writesize))
            goto fail;

        write_addr=load_addr+MTD0_SIZE+MTD1_SIZE+MTD2_SIZE;
        writesize=MTD3_SIZE;
        if(write_to_mtd(ROOTFSMTD,write_addr,writesize))
            goto fail;


        write_addr=load_addr+MTD0_SIZE+MTD1_SIZE+MTD2_SIZE+MTD3_SIZE;
        writesize=tftp_file_size-(MTD0_SIZE+MTD1_SIZE+MTD2_SIZE+MTD3_SIZE);
        if(write_to_mtd(SMALLMTD,write_addr,writesize))
            goto fail;


    }
    return 0;
fail:
    return -1;
}

void checkTFTPLoop(void)
{
    fd_set allset,rset;     // the receive fd set
    int i = 0;
    int maxfd = 0;
    struct timeval tv;
    unsigned char chardata[2];
    int n;

    FD_ZERO(&allset);

    FD_SET(0,&allset);

    /* start trigger */
    tftp_time_out=0;
    udelay(500000);

    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = POLL_TIME;

        memcpy(&rset,&allset,sizeof(fd_set));

        if(select(maxfd+1, &rset, NULL, NULL, &tv)>0)
        {
            if(FD_ISSET(0, &rset))
            {
                n=read(0,chardata,1);
                if(chardata[0]=='1')
                {
                    break;
                }
            }
        }

        if(TftpBlock == TftpLastBlock)
        {
            tftp_time_out++;
        }

        if(tftp_time_out==3)
        {
            putc('T',stdout);
            TftpSend();
            tftp_time_out=0;
        }

        /* 20160630 -copy FW file to /tmp only, ignore write back to flash */
        if(isFW==1) {
            /* Add finish check 20160721*/
            if(check_finish)
            {
                if(!write_tftpfileToTmp(flashname,load_addr,tftp_file_size))
                {
                    log_printf("Download firmware file success to /tmp !!\n");
                }
                else
                {
                    log_printf("Download firmware file fail!! , Please retry \n");
                }

                check_finish=0;
                break;
            }
        }
        else
        {
            if(check_finish)
            {
                if(customer_flash)
                {
                    if(!write_to_all())
                    {
                        log_printf("Burn in firmware success!!\n");
                    }
                    else
                    {
                        log_printf("Burn in firmware fail!! , Please retry \n");
                    }
                }
                else
                {
                    if(!write_to_mtd(flashname,load_addr,tftp_file_size))
                    {
                        log_printf("Burn in firmware success!!\n");
                    }
                    else
                    {
                        log_printf("Burn in firmware fail!! , Please retry \n");
                    }
                }
                check_finish=0;
                break ;
            }
        }
    }
    net_clean();
    log_printf("end LoopCheck\n");
}
