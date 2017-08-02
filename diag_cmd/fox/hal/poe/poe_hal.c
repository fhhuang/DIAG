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
***      poe_hal.c
***
***    DESCRIPTION :
***      Hardware Abstraction Layer for PoE
***
***    HISTORY :
***       - 2010/01/05, Jungle Chen
***             File Creation
***
***************************************************************************/
/*==========================================================================
 *                                                                          
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
/* System Library */

/* User-defined Library */
#include "cmn_type.h"
#include "porting.h"
#include "poe_hal.h"
#include "log.h"
#include "sys_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
/*==========================================================================
 *
 *      Constant Definition Segment
 *
 *==========================================================================
 */
#define DBG_PRINTF log_dbgPrintf

/*==========================================================================
 *
 *      Structrue Definition segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Static Variable segment
 *
 *==========================================================================
 */
static UINT8 scp_seqNum = 0;
static UINT8 scp_echoNum;
static BOOL  POE_FW_loaded = FALSE;
static UINT32 g_poeEnable = 0;

UINT8  poeSwVersion[64] = "PoE Software Version: N/A";
int fd = -1;

const POE_BUDGE_T poeConfig8P[] = 
{   /*power bank,  power limit,  max voltage,  min voltage*/
    {0x0,          0x43,        0x22b,        0x1c7},
    {0x1,          0x43,        0x22b,        0x1c7},
    {0x2,          0x43,        0x22b,        0x1c7},
    {0x3,          0x43,        0x22b,        0x1c7},
    {0x4,          0x43,        0x22b,        0x1c7},
    {0x5,          0x43,        0x22b,        0x1c7},
    {0x6,          0x43,        0x22b,        0x1c7},
    {0x7,          0x43,        0x22b,        0x1c7},
    {0x8,          0x43,        0x22b,        0x1c7},
    {0x9,          0x43,        0x22b,        0x1c7},
    {0xa,          0x43,        0x22b,        0x1c7},
    {0xb,          0x43,        0x22b,        0x1c7},
    {0xc,          0x43,        0x22b,        0x1c7},
    {0xd,          0x43,        0x22b,        0x1c7},
    {0xe,          0x43,        0x22b,        0x1c7},
    {0xf,          0x43,        0x22b,        0x1c7},
};

const POE_BUDGE_T poeConfig16P[] = 
{   /*power bank,  power limit,  max voltage,  min voltage*/
    {0x0,          0x78,        0x22b,        0x1c7},
    {0x1,          0x78,        0x22b,        0x1c7},
    {0x2,          0x78,        0x22b,        0x1c7},
    {0x3,          0x78,        0x22b,        0x1c7},
    {0x4,          0x78,        0x22b,        0x1c7},
    {0x5,          0x78,        0x22b,        0x1c7},
    {0x6,          0x78,        0x22b,        0x1c7},
    {0x7,          0x78,        0x22b,        0x1c7},
    {0x8,          0x78,        0x22b,        0x1c7},
    {0x9,          0x78,        0x22b,        0x1c7},
    {0xa,          0x78,        0x22b,        0x1c7},
    {0xb,          0x78,        0x22b,        0x1c7},
    {0xc,          0x78,        0x22b,        0x1c7},
    {0xd,          0x78,        0x22b,        0x1c7},
    {0xe,          0x78,        0x22b,        0x1c7},
    {0xf,          0x78,        0x22b,        0x1c7},
};

const POE_BUDGE_T poeConfig24P[] = 
{   /*power bank,  power limit,  max voltage,  min voltage*/
    {0x0,          0xc3,        0x22b,        0x1c7},
    {0x1,          0xc3,        0x22b,        0x1c7},
    {0x2,          0xc3,        0x22b,        0x1c7},
    {0x3,          0xc3,        0x22b,        0x1c7},
    {0x4,          0xc3,        0x22b,        0x1c7},
    {0x5,          0xc3,        0x22b,        0x1c7},
    {0x6,          0xc3,        0x22b,        0x1c7},
    {0x7,          0xc3,        0x22b,        0x1c7},
    {0x8,          0xc3,        0x22b,        0x1c7},
    {0x9,          0xc3,        0x22b,        0x1c7},
    {0xa,          0xc3,        0x22b,        0x1c7},
    {0xb,          0xc3,        0x22b,        0x1c7},
    {0xc,          0xc3,        0x22b,        0x1c7},
    {0xd,          0xc3,        0x22b,        0x1c7},
    {0xe,          0xc3,        0x22b,        0x1c7},
    {0xf,          0xc3,        0x22b,        0x1c7},
};

const POE_BUDGE_T poeConfig48P[] = 
{   /*power bank,  power limit,  max voltage,  min voltage*/
    {0x0,          0x172,        0x22b,        0x1c7},
    {0x1,          0x172,        0x22b,        0x1c7},
    {0x2,          0x172,        0x22b,        0x1c7},
    {0x3,          0x172,        0x22b,        0x1c7},
    {0x4,          0x172,        0x22b,        0x1c7},
    {0x5,          0x172,        0x22b,        0x1c7},
    {0x6,          0x172,        0x22b,        0x1c7},
    {0x7,          0x172,        0x22b,        0x1c7},
    {0x8,          0x172,        0x22b,        0x1c7},
    {0x9,          0x172,        0x22b,        0x1c7},
    {0xa,          0x172,        0x22b,        0x1c7},
    {0xb,          0x172,        0x22b,        0x1c7},
    {0xc,          0x172,        0x22b,        0x1c7},
    {0xd,          0x172,        0x22b,        0x1c7},
    {0xe,          0x172,        0x22b,        0x1c7},
    {0xf,          0x172,        0x22b,        0x1c7},
};

/*==========================================================================
 *                                                                          
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */


/*==========================================================================
 *
 *      Local Function segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      External Funtion segment
 *
 *==========================================================================
 */

#define POE_MODULE_RESET_REG     0xD0200000
#define POE_MODULE_RESET_DEF     0xBF
#define POE_MODULE_RESET_MASK    0x10

#define POE_MODULE_RESET_EN      *((UINT8 *)POE_MODULE_RESET_REG) = (POE_MODULE_RESET_DEF & ~POE_MODULE_RESET_MASK)
#define POE_MODULE_RESET_DIS     *((UINT8 *)POE_MODULE_RESET_REG) = (POE_MODULE_RESET_DEF & POE_MODULE_RESET_MASK)

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      halSCPPktChecksum
 *
 *  DESCRIPTION :
 *      a local API to generate the check sum of a SCP packet to be transmitted
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *     the calculated checksum
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
static INT32 halSCPPktChecksum
(
    IN S_SCP_PKT *pkt
)
{
    INT32 i,sum = 0;
    UINT8 *item = (UINT8 *)pkt;

    for (i=0;i<SCP_PKT_CHKSUM_LEN;i++)
    {
        sum += *(item+i);
    }

    return sum;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      halSCPChecksumInit
 *
 *  DESCRIPTION :
 *      a local API to init the check sum of a SCP packet to be transmitted
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      *pkt - points to a SCP packet with a calculated checksum
 *
 *  RETURN :
 *     none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
static void halSCPChecksumInit
(
    IN S_SCP_PKT *pkt
)
{
    int Csum;
   
    Csum = halSCPPktChecksum(pkt);
   
    pkt->csumh= (UINT8)(Csum>>8);
    pkt->csuml= (UINT8)(Csum&0xff);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      halSCPChecksumVerify
 *
 *  DESCRIPTION :
 *      a local API to verify the check sum of a received SCP packet 
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      1 means OK; 0 means Error
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
static INT32 halSCPChecksumVerify
( 
    IN S_SCP_PKT *pkt
)
{
    INT32 Csum;
    UINT8 CsumH,CsumL, i;

    Csum = halSCPPktChecksum(pkt);
   
    CsumH = (UINT8)(Csum>>8);
    CsumL = (UINT8)(Csum&0xff);

    if (CsumH == pkt->csumh && CsumL == pkt->csuml)
    {
        return 1;
    }
   
    return 0; 
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      halSCPEchoNumInit
 *
 *  DESCRIPTION :
 *      a local API to generate a echo number for a SCP packet
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      *pkt - points to a SCP packet with an assigned echo number
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
static void halSCPEchoNumInit
(    
    IN S_SCP_PKT *pkt
)
{
  
    scp_echoNum = scp_seqNum;
    scp_seqNum++;
  
    if (scp_seqNum > 0xFE)
    {
        scp_seqNum = 0;
    }

    pkt->echo= scp_echoNum;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      halSCPPktPrint
 *
 *  DESCRIPTION :
 *      a local API to print the content of a SCP packet
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
static void halSCPPktPrint
(
    IN S_SCP_PKT *pkt
)
{
    char *pkt_string[]=
    {
        "Command",
        "Program",
        "Request",
        "Telemetry",
        "Report"
    };
       
    if (pkt->key < 0x03)
    {
        log_printf("** %s Packet Transmitted **\n", pkt_string[pkt->key]);
        log_printf("+--------+--------+--------+--------+--------+--------+--------+--------+\n");
        log_printf("|  KEY   |  ECHO  |Subject |Subject1|Subject2|  DATA  |  DATA  |  DATA  |\n");
        log_printf("|  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |\n",
        pkt->key,pkt->echo,pkt->subject,pkt->subject1,pkt->subject2,pkt->data[0],pkt->data[1],pkt->data[2]);
    }
    else
    {
        if (pkt->key == 0x03)
            log_printf("** %s Packet Received **\n",pkt_string[3]);
        else
            log_printf("** %s Packet Received **\n",pkt_string[4]);
           
        log_printf("+--------+--------+--------+--------+--------+--------+--------+--------+\n");
        log_printf("|  KEY   |  ECHO  |  DATA  |  DATA  |  DATA  |  DATA  |  DATA  |  DATA  |\n");
        log_printf("|  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |\n",
        pkt->key,pkt->echo,pkt->subject,pkt->subject1,pkt->subject2,pkt->data[0],pkt->data[1],pkt->data[2]);
    }
    log_printf("+--------+--------+--------+--------+--------+--------+--------+--------+\n");
    log_printf("|  DATA  |  DATA  |  DATA  |  DATA  |  DATA  | CSum H | CSum L |\n");
    log_printf("|  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |  0x%02x  |\n",
    pkt->data[3],pkt->data[4],pkt->data[5],pkt->data[6],pkt->data[7],pkt->csumh,pkt->csuml);
    log_printf("+--------+--------+--------+--------+--------+--------+--------+\n");
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktPrint
 *
 *  DESCRIPTION :
 *      a global API to print the content of a SCP packet, solely for debugging
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void poe_halSCPPktPrint
(
    IN S_SCP_PKT *pkt
)
{
    halSCPPktPrint(pkt);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktSyncLossHandler
 *
 *  DESCRIPTION :
 *      a API to do synchronization during communication loss
 *
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code 
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPPktSyncLossHandler
(  
    void
)
{
    S_SCP_PKT req_pkt1,res_pkt1;
    E_SCP_RET_CODE result;
  
    /* Host assumes abnormal event (such as RESET) */
    if ((result = poe_halSCPPktReceive(&res_pkt1, SCP_MAX_RESET_WAIT)) == E_SCP_SUCCESS)
    {
        /* System Status received */
        return E_SCP_SUCCESS; 
    }
    else if (result == E_SCP_TIMEOUT_ERROR) /* If no System Status received */
    {
        memset(&req_pkt1,0x4e,sizeof(S_SCP_PKT));
        memset(&res_pkt1,0,sizeof(S_SCP_PKT));
  
        req_pkt1.key      = SCP_REQUEST;
        req_pkt1.subject  = SCP_GLOBAL;
        req_pkt1.subject1 = SCP_SYSTEMSTATUS;

        POE_DBG_CHECK(log_printf("host requests a SCP packet for system status\n"));
     
        /* host sends a "Get System Status" command */
        if (poe_halSCPPktTransmit(&req_pkt1) == E_SCP_SUCCESS) 
        {
            /* Host waits for "System Status" telemetry */
            if (poe_halSCPPktReceive(&res_pkt1,SCP_MAX_PKT_WAIT) == E_SCP_SUCCESS)
            {
                /* System Status received */
                return E_SCP_SUCCESS; 
            }
            else /* Assumes - loss of sync. */
            {
                /* Host initiates hardware reset */
                POE_MODULE_RESET_EN;
                udelay(500);
                POE_MODULE_RESET_DIS;

                POE_DBG_CHECK(log_printf("host initiates hardware reset\n"));

                /* Host waits for "System Status" telemetry */
                if ((result = poe_halSCPPktReceive(&res_pkt1,SCP_MAX_RESET_WAIT)) == E_SCP_SUCCESS)
                {
                    /* System Status received */
                    return E_SCP_SUCCESS; 
                }
                else if (result == E_SCP_TIMEOUT_ERROR)/* re-sync error fail */
                {            
                    return E_SCP_RE_SYNC_ERROR;
                }
            }
        }
    }
    return result;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktReceive
 *
 *  DESCRIPTION :
 *      receive a SCP packet
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *      max_wait - before receiving a SCP packet via i2c, how long will it hold on.
 *
 *  OUTPUT :
 *      *pkt - points to the received packet
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code 
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPPktReceive
(
    OUT S_SCP_PKT *pkt, 
    IN UINT32 max_wait
)
{
    INT32 ret=E_TYPE_SUCCESS, retry_timer = 0;
    UINT8 *data = (UINT8 *)pkt, i;

    UINT32 regVal;
    UINT32 buf[POE_DRAGONITE_DATA_MSG_LEN];

    for(i=0; i<POE_TIME_OUT;i++)
    {
        if (poe_halDragoniteRegRead(POE_RX_MO_ADDR, &regVal) < 0)
        {
            log_printf("prvPoeRegRead fail\n");
            return E_TYPE_IO_ERROR;
        }

        if (regVal == POE_RX_MO_HOST_OWNERSHIP_CODE)
        {
            break;
        }
        else
        {
            udelay(1);
        }

        if (i == (POE_TIME_OUT - 1) )
        {
            log_printf("RX_MO time out: %x\n", regVal);
            return E_TYPE_IO_ERROR;
        }
    }

    if (lseek(fd, POE_RX_BUF_ADDR, SEEK_SET) < 0)
    {
        log_printf("lseek fail\n");
        return E_TYPE_IO_ERROR;
    }

    do {
        if ( retry_timer > max_wait)
        {
            /* check the time */
            return E_SCP_TIMEOUT_ERROR;
        }

        retry_timer += 10;
        udelay(10*1000);

        if (read(fd, buf, sizeof(buf)) < 0)
        {
            log_printf("read fail\n");
            return E_TYPE_IO_ERROR;
        }
    }while(buf[13] == 0 && buf[14] == 0);

    /* Wordaround for align issue */
    pkt->key = buf[0];
    pkt->echo = buf[1];
    pkt->subject = buf[2];
    pkt->subject1 = buf[3];
    pkt->subject2 = buf[4];
    pkt->data[0] = buf[5];
    pkt->data[1] = buf[6];
    pkt->data[2] = buf[7];
    pkt->data[3] = buf[8];
    pkt->data[4] = buf[9];
    pkt->data[5] = buf[10];
    pkt->data[6] = buf[11];
    pkt->data[7] = buf[12];
    pkt->csumh = buf[13];
    pkt->csuml = buf[14];

    POE_DBG_CHECK(halSCPPktPrint(pkt));

    if (halSCPChecksumVerify(pkt))
    {
        if (pkt->echo == scp_echoNum)
        {
            return poe_halDragoniteRegWrite(POE_RX_MO_ADDR, POE_RX_MO_POE_OWNERSHIP_CODE);
        }
        else
        {
            /* receive system status telemetry */
            if ( pkt->echo == 0xff )
            {
                return poe_halDragoniteRegWrite(POE_RX_MO_ADDR, POE_RX_MO_POE_OWNERSHIP_CODE);
            }
            else
            {
                POE_DBG_CHECK(log_printf("SKIP Echo number mismatch. TX ECHO (%d) != RX ECHO (%d)\n", scp_echoNum, pkt->echo));
                return E_SCP_ECHO_ERROR;
            }
        } 
    }
    else
    {
        POE_DBG_CHECK(log_printf("Checksum error: %x %x!!\n", pkt->csumh, pkt->csuml));
        return E_SCP_CHECKSUM_ERROR;
    }

    return poe_halDragoniteRegWrite(POE_RX_MO_ADDR, POE_RX_MO_POE_OWNERSHIP_CODE);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktTransmit
 *
 *  DESCRIPTION :
 *      transmit a SCP packet
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPPktTransmit
(
    IN S_SCP_PKT *pkt
)
{
    INT32  ret=E_TYPE_SUCCESS;
    UINT8  *data = (UINT8 *)pkt, i=0;
    UINT32 buf[POE_DRAGONITE_DATA_MSG_LEN], regVal=0;
  
    halSCPEchoNumInit(pkt);
    halSCPChecksumInit(pkt);
    
    POE_DBG_CHECK(halSCPPktPrint(pkt));

    if (poe_halDragoniteRegRead(POE_TX_MO_ADDR, &regVal) < 0)
    {
        log_printf("TX_MO_ADDR: 0x%x\n", regVal);
        return E_TYPE_IO_ERROR;
    }

    if (regVal != POE_TX_MO_HOST_OWNERSHIP_CODE) {
        log_printf("TX_MO_REG: 0x%x\n", regVal);
        return E_TYPE_IO_ERROR;
    }

    if (lseek(fd, POE_TX_BUF_ADDR, SEEK_SET) < 0)
    {
        log_printf("lseek failed: 0x%x\n", POE_TX_BUF_ADDR);
        return E_TYPE_IO_ERROR;
    }

    for (i=0; i<POE_DRAGONITE_DATA_MSG_LEN; i++)
    {
        buf[i] = *(data+i);
    }

    if ( write(fd, &buf, sizeof(buf)) < 0) {
        log_printf("Cannot write to Dragonite\n");
        return E_TYPE_IO_ERROR;
    }

    return poe_halDragoniteRegWrite(POE_TX_MO_ADDR, POE_TX_MO_POE_OWNERSHIP_CODE);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSCPDebugCmd
 *
 *  DESCRIPTION :
 *      a API for communicating with PoE system, which transmits a command/request SCP packet
 *      and then receives a report/telemetry SCP packet.
 *
 *
 *  INPUT :
 *
 *     pkt - points to a SCP packet, which will be transmitted
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPDebugCmd 
(
    IN S_SCP_PKT *req_pkt
)
{
    E_SCP_RET_CODE result;
    S_SCP_PKT res_pkt;
  
    
    memset(&res_pkt,0,sizeof(S_SCP_PKT));
  
    if ((result = poe_halSCPPktTransmit(req_pkt)) == E_SCP_SUCCESS)
    {
        /* For Save System Setting command, it takes longer time to complete */
        if ( req_pkt->subject == 0x06 && req_pkt->subject1 == 0x0f )
        {
            log_printf("Add more delay time 100ms for save system setting\n");
            udelay(100*1000);
        }       

        /* For Restore Factory Default command, it takes longer time to complete */
        if ( req_pkt->subject == 0x2d )
            udelay(1000*1000);  

        udelay(100*1000);   
        if ((result = poe_halSCPPktReceive(&res_pkt, SCP_MAX_PKT_WAIT)) == E_SCP_SUCCESS)
        {
            halSCPPktPrint(req_pkt);
            halSCPPktPrint(&res_pkt);   
            
            /* For RESET command, delay 3s before reading report */
            if ( req_pkt->subject == 0x07 && req_pkt->subject1 == 0x55 )
            {
                udelay(3*1000*1000);
                memset(&res_pkt, 0, sizeof(S_SCP_PKT));
                poe_halSCPPktReceive(&res_pkt, SCP_MAX_PKT_WAIT);
                halSCPPktPrint(&res_pkt);
            }
        }
        else if (result == E_SCP_TIMEOUT_ERROR)/* Host assumes ¡V packet loss */
        {
            log_printf("No echo packet from POE module, Re-transmit again.\n");
                
            /* Host transmitted Command or Request 2nd try */
            if ((result = poe_halSCPPktTransmit(req_pkt)) == E_SCP_SUCCESS)
            {
                if ((result = poe_halSCPPktReceive(&res_pkt,SCP_MAX_PKT_WAIT)) == E_SCP_SUCCESS)
                {
                    halSCPPktPrint(req_pkt);
                    halSCPPktPrint(&res_pkt);     
                }
                else /* Host assumes abnormal event (such as RESET) */
                {
                    log_printf("Host assumes abnormal event (such as RESET)\n");
                    result = poe_halSCPPktSyncLossHandler(); 
                }        
            }
        }      
    }
  
    return result;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoECtrlStateGet
 *
 *  DESCRIPTION :
 *      a API to get the state of PoE module ctrl in I/O expander
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      state - disable/enable
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoECtrlStateGet
(
    OUT BOOL *state
)
{
    INT32  ret      = E_TYPE_SUCCESS;
#if 0
    UINT8  Data     = 0;

    if(Data & 0x08 == 0x08)
    {
        *state = TRUE;
    }
    else
    {
        *state = FALSE;
    }
#endif
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoECtrlStateSet
 *
 *  DESCRIPTION :
 *      a API to set the state of PoE module ctrl in I/O Expander
 *
 *  INPUT :
 *      state - disable/enable
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoECtrlStateSet
(
    IN  BOOL    state
)
{
    INT32  ret      = E_TYPE_SUCCESS;
#if 0
    UINT8  Data     = 0;

    if (state)
    {
        Data |= 0x08;
    }
    else
    {
        Data &= 0x08;
    }
#endif
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPHandshake
 *
 *  DESCRIPTION :
 *      a API to do SCP handshaking
 *
 *
 *  INPUT :
 *      *pReqPkt - a request/command/program SCP packet
 *
 *
 *  OUTPUT :
 *      *pRspPkt - a report/telemetry SCP packet
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSCPHandshake
(
    IN  S_SCP_PKT  *pReqPkt, 
    OUT S_SCP_PKT  *pRspPkt
)
{
    UINT32 i;
    E_SCP_RET_CODE result;
    
    for (i = 0; i < POE_SCP_MAX_RETRY; i ++)
    {
        if ((result = poe_halSCPPktTransmit(pReqPkt)) == E_SCP_SUCCESS)
        {
            /* For Power bank command, it takes longer time to complete */
            if ( pReqPkt->subject == 0x07 && pReqPkt->subject1 == 0x0b && pReqPkt->subject2 == 0x57 )
                POE_DELAY(50*1000);

            /* For Save System Setting command, it takes longer time to complete */
            if ( pReqPkt->subject == 0x06 && pReqPkt->subject1 == 0x0f )
                POE_DELAY(100*1000);    

            /* For Restore Factory Default command, it takes longer time to complete */
            if ( pReqPkt->subject == 0x2d )
                POE_DELAY(100*1000);    

            if ((result = poe_halSCPPktReceive(pRspPkt,SCP_MAX_PKT_WAIT)) == E_SCP_SUCCESS)
            {
                return E_SCP_SUCCESS;
            }
            else if (result == E_SCP_TIMEOUT_ERROR)/* Host assumes ¡V packet loss */
            {
                log_printf("No echo packet from POE module\n");                
                memset(pRspPkt,0,sizeof(S_SCP_PKT));
            }
        }
    }

    return result;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDragoniteRegRead
 *
 *  DESCRIPTION :
 *      Read value from Dragonite register
 *
 *  INPUT :
 *      addr - address of Dragonite
 *
 *  OUTPUT :
 *      val  - value
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDragoniteRegRead
(
    UINT32 addr, 
    UINT32 *val
)
{
    if (lseek(fd, addr, SEEK_SET) < 0)
    {
        return E_TYPE_IO_ERROR;
    }

    return read(fd, val, sizeof(val) ) < 0 ?  E_TYPE_IO_ERROR: E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDragoniteRegWrite
 *
 *  DESCRIPTION :
 *      Write Dragonite register
 *
 *  INPUT :
 *      addr - address of Dragonite
 *      val  - value
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDragoniteRegWrite
(
    UINT32 addr, 
    UINT32 val
)
{
    if (lseek(fd, addr, SEEK_SET) < 0)
    {
        return E_TYPE_IO_ERROR;
    }

    return write(fd, &val, sizeof(val) ) < 0 ? E_TYPE_IO_ERROR: E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDownloadFirmware
 *
 *  DESCRIPTION :
 *      Download dragonite fw from filesystem
 *
 *  INPUT :
 *      f_name
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDownloadFirmware
(
    char *f_name, 
    UINT32 size
)
{
    int rfd;
    int ret = E_TYPE_IO_ERROR;
    char *buf = NULL;
    ssize_t nr, nw;
    BOOL msg;

    POE_CHK_INIT(fd);

    /* open file with firmware */
    rfd = open(f_name, O_RDONLY);
    if (rfd <= 0) 
    {
        /* fixme - ros error fprintf(stderr, "Cannot open %s file.\n", f_name); */
        return E_TYPE_IO_ERROR;
    }

    buf = malloc(size); /* fixme - ros malloc */
    if (buf == NULL)
    {
        /* fixme - ros error fprintf(stderr, "Cannot malloc buf.\n"); */
        goto fail_write_close;
    }

    nr = read(rfd, buf, size);
    if (nr < 0)
    {
        /* fixme - ros error fprintf(stderr, "Cannot read from %s.\n", f_name);*/
        goto fail_write_free;
    }

    msg = POE_ITCM_DIR;

    if (ioctl(fd, POE_DRAGONITE_IOC_SETMEM_TYPE, &msg) < 0)
    {
        goto fail_write_free;
    }

    if (lseek(fd, POE_FW_LOAD_ADDR, SEEK_SET) < 0)
    {
        goto fail_write_free;
    }

    nw = write(fd, buf, (size_t)nr);
    if (nw < 0)
    {
        /* fixme ROS error fprintf(stderr, "Cannot write to %s.\n", f_name); */
        goto fail_write_free;
    }

    ret = E_TYPE_SUCCESS;
    POE_FW_loaded = TRUE;

fail_write_free:
    free(buf);
fail_write_close:
    msg = POE_DTCM_DIR;
    ioctl(fd, POE_DRAGONITE_IOC_SETMEM_TYPE, &msg); /* restore mem type to DTCM */

    /* MTL add for config below from Microsmi
        ./mscc_tool -w 0 1 1
        ./mscc_tool -w 30 8e3d 1
    */
    poe_halDragoniteRegWrite(0x0, 1);
    poe_halDragoniteRegWrite(0x30, 0x8e3d);

    close(rfd);
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halLoadFirmware
 *
 *  DESCRIPTION :
 *      Load dragonite fw
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halLoadFirmware
(
    void
)
{
    /* int ret; */
    BOOL msg;

    POE_CHK_INIT(fd);

    if (!POE_FW_loaded)
    {
        /* put Dragonite into reset */
        msg = FALSE;
        if (ioctl(fd, POE_DRAGONITE_IOC_UNRESET, &msg) < 0)
        {
            return E_TYPE_IO_ERROR;
        }

        if (poe_halDownloadFirmware(POE_FW_FILE, POE_FW_SIZE) != E_TYPE_SUCCESS)
        {
            printf("Error poeDownloadFirmware\n");
        }
        else
        {
            /* Init TX_MO_ADDR reg */
            poe_halDragoniteRegWrite(POE_TX_MO_ADDR, POE_INIT_VAL);

            msg = TRUE;
            if (ioctl(fd, POE_DRAGONITE_IOC_UNRESET, &msg) < 0)
            {
                return E_TYPE_IO_ERROR;
            }
        }
    }
    return poe_halDragoniteRegWrite(POE_RX_MO_ADDR, POE_RX_MO_HOST_OWNERSHIP_CODE);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halInitDragonite
 *
 *  DESCRIPTION :
 *      Open dragonite fd
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void poe_halInitDragonite
(
    void
)
{
    S_SCP_PKT   res_pkt, req_pkt;

    /* Open Dragonite */
    if(fd>0)
    {
        close(fd);
        fd=-1;
    }

    fd = open("/dev/dragonite", O_RDWR);
    if (fd < 0)
    {
        log_printf("failed to open /dev/dragonite\n");
    }

    poe_halLoadFirmware();

    udelay(150000);

    if(poe_halSCPPktReceive(&res_pkt,SCP_MAX_PKT_WAIT) == E_TYPE_SUCCESS)
    {
        /* Nothing to do here. */
    }
    else
    {
        log_printf("%s(%d): poeRunFirmware read failed\n", __FUNCTION__, __LINE__);
    }
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halGetSoftwareVersion
 *
 *  DESCRIPTION :
 *      Get software version
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halGetSoftwareVersion
(
    OUT UINT8 *softwareVersion
)
{
    S_SCP_PKT req_pkt, res_pkt;
    E_SCP_RET_CODE  retval;
    UINT8  st_prodNum[8]={0}, st_paramCode[8]={0}, st_version[8]={0};
    UINT32 ret = E_SCP_SUCCESS, swVersion=0;
    /* show PoE software version */
    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_GLOBAL;
    req_pkt.subject1 = SCP_VERSIONZ;
    req_pkt.subject2 = SCP_SOFTWAREVERSION;

    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }

    swVersion = (res_pkt.data[0] << 8) | (res_pkt.data[1]);

    if(res_pkt.subject2 >= 16)
    {
        sprintf((char *)&st_prodNum, "%x", res_pkt.subject2);
    }
    else
    {
        sprintf((char *)&st_prodNum, "0%x", res_pkt.subject2);
    }

    if (swVersion >= 1000)
    {
        sprintf((char *)&st_version, "%d", swVersion);
    }
    else
    {
        sprintf((char *)&st_version, "0%d", swVersion);
    }
        
    if(res_pkt.data[2] >= 16)
    {
        sprintf((char *)&st_paramCode, "%x", res_pkt.data[2]);
    }
    else
    {
        sprintf((char *)&st_paramCode, "0%x", res_pkt.data[2]);    
    }

    memset(poeSwVersion, (char)0, 64);
    sprintf((char *)&poeSwVersion, "%s.%s.%s",  st_prodNum, st_version, st_paramCode);
    memcpy(softwareVersion,poeSwVersion,strlen(poeSwVersion));

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halPowerShow
 *
 *  DESCRIPTION :
 *      Show the power info of a PoE port
 *
 *
 *  INPUT :
 *      lPort - the specify port to show
 *
 *  OUTPUT :
 *      powerInfo - power information
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPowerShow
(
    IN  UINT32  lPort,
    OUT POE_POWERINFO_T *powerInfo
)
{
    S_SCP_PKT req_pkt, res_pkt;
    E_SCP_RET_CODE  retval;

    POE_DELAY(500000); /* delay 500ms */

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));

    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_PARAMZ;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    log_printf("Port %d ", lPort);

    if (retval != E_SCP_SUCCESS)
    {
        log_printf("Data request error\n");          
        return E_SCP_STATUS_TRANSMIT_ERROR;
    }

    powerInfo->vmainVoltage = (res_pkt.subject << 8) + res_pkt.subject1;
    powerInfo->caculatedCurrent = (res_pkt.subject2 << 8) + res_pkt.data[0];
    powerInfo->powerConsumption = (res_pkt.data[1] << 8) + res_pkt.data[2];
    powerInfo->vportVoltage = (res_pkt.data[4] << 8) + res_pkt.data[5];

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSetPortForcePowerOn
 *
 *  DESCRIPTION :
 *      Force enable or disable a PoE port.
 *
 *  INPUT :
 *      lPort - PoE port
 *      state - enable or disable
 *
 *  OUTPUT :
 *       none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetPortForcePowerOn
(
    IN  UINT32  lPort,
    IN  UINT8   forceState
)
{
    S_SCP_PKT req_pkt, res_pkt;
    E_SCP_RET_CODE  retval;
    UINT32 testRetval = E_SCP_SUCCESS;

    POE_DELAY(1500000); /* delay 1500ms */

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));
     
    /* write user data */
    req_pkt.key = SCP_COMMAND;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_FORCEPOWER;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    req_pkt.data[0] = (UINT8)forceState;

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    if (retval != E_SCP_SUCCESS)
    {
        log_printf("Data request error\n");
        return E_SCP_STATUS_TRANSMIT_ERROR;  
    }

    return testRetval;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSetPortMode
 *
 *  DESCRIPTION :
 *      Configure mode of a PoE port.
 *
 *  INPUT :
 *      lPort - PoE port
 *      af_mode - at or af
 *      state - enable or disable
 *
 *  OUTPUT :
 *       none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetPortMode
(
    UINT32 lPort,
    BOOL   af_mode,
    UINT8  state
)
{
    S_SCP_PKT res_pkt, req_pkt;
    UINT32 ret = E_SCP_SUCCESS;
    
    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));
     
    /* write user data */
    req_pkt.key      = SCP_COMMAND;
    req_pkt.subject  = SCP_CHANNEL;
    req_pkt.subject1 = SCP_ENDIS;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    req_pkt.data[0]  = state;

    if(af_mode)
    {
        req_pkt.data[1] = 0;
    }
    else
    {
        /* 20160825, set AT mode with port_type AF/AT = 1,
           Microsemi P69200 4.3.5 , fixed bug PoE 30w for new v1.8.5 FW problem */
        req_pkt.data[1] = 1;
    }

    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }
    return ret;        
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSaveSystemSetting
 *
 *  DESCRIPTION :
 *      a API to save system settings.
 *
 *  INPUT :
 *      none.
 *
 *  OUTPUT :
 *      none.
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSaveSystemSetting
(
    void
)
{
    S_SCP_PKT res_pkt, req_pkt;
    INT32 ret = E_SCP_SUCCESS;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));

    req_pkt.key = SCP_PROGRAM;
    req_pkt.subject = SCP_E2;
    req_pkt.subject1 = SCP_SAVECONFIG;

    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }    

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSetDeratingData
 *
 *  DESCRIPTION :
 *      a API to set the power derating parameters.
 *
 *  INPUT :
 *      POE_BUDGE_T - power derating parameters.
 *
 *
 *  OUTPUT :
 *      GT_OK; GT_FAIL
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetDeratingData
(
    IN  POE_BUDGE_T deratingData
)
{
    S_SCP_PKT res_pkt, req_pkt;
    UINT32 ret = E_SCP_SUCCESS;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));

    req_pkt.key = SCP_COMMAND;
    req_pkt.subject = SCP_GLOBAL;
    req_pkt.subject1 = SCP_SUPPLY;
    req_pkt.subject2 = SCP_POWERBUDGET;
    req_pkt.data[0] =  deratingData.powerBank;
    req_pkt.data[1] =  (deratingData.powerLimit & 0xff00) >> 8;       /* power limit */
    req_pkt.data[2] =  deratingData.powerLimit & 0xff;
    req_pkt.data[3] =  (deratingData.powerMaxVoltage & 0xff00) >> 8;  /* max voltage, 57.5V */
    req_pkt.data[4] =  deratingData.powerMaxVoltage & 0xff;
    req_pkt.data[5] =  (deratingData.powerMinVoltage & 0xff00) >> 8;  /* min voltage, 45.5 */
    req_pkt.data[6] =  deratingData.powerMinVoltage & 0xff;
    req_pkt.data[7] =  0x1;  /* guard band enable */

    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoESetPmMethod
 *
 *  DESCRIPTION :
 *      a API to set the power management mode of operation
 *
 *  INPUT :
 *      pm1, method to calculate system power
 *      pm2, port power limit
 *      pm3, start up condition
 *
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoESetPmMethod
(
    IN  UINT8 pm1,
    IN  UINT8 pm2,
    IN  UINT8 pm3
)
{
    S_SCP_PKT res_pkt, req_pkt;
    INT32 ret = E_SCP_SUCCESS;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));

    req_pkt.key = SCP_COMMAND;
    req_pkt.subject = SCP_GLOBAL;
    req_pkt.subject1 = SCP_SUPPLY;
    req_pkt.subject2 = SCP_POWERMANAGEMODE;
    req_pkt.data[0] =  pm1;
    req_pkt.data[1] =  pm2;
    req_pkt.data[2] =  pm2;;
    
    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoESetSystemMask
 *
 *  DESCRIPTION :
 *      a API to set the system mask for power management disconnect method 
 *      and capacitor detect
 *
 *  INPUT :
 *      maskBits
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoESetSystemMask
(
    IN  UINT32  maskBits
)
{
    S_SCP_PKT res_pkt, req_pkt;
    UINT32 ret = E_SCP_SUCCESS;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));

    req_pkt.key = SCP_COMMAND;
    req_pkt.subject = SCP_GLOBAL;
    req_pkt.subject1 = SCP_MASKZ;
    req_pkt.subject2 = maskBits;

    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSetIndividualMask
 *
 *  DESCRIPTION :
 *      a API to set the individual mask
 *
 *  INPUT :
 *      maskKeyNum
 *      state - disable/enable
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetIndividualMask
(
    IN  UINT32  maskKeyNum,
    IN  UINT8   state
)
{
    S_SCP_PKT res_pkt, req_pkt;
    INT32 ret = E_SCP_SUCCESS;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));

    req_pkt.key = SCP_COMMAND;
    req_pkt.subject = SCP_GLOBAL;
    req_pkt.subject1 = SCP_INDIVIDUAL_MASK;
    req_pkt.subject2 = maskKeyNum;
    req_pkt.data[0] =  state;

    if(poe_halSCPHandshake(&req_pkt, &res_pkt) != E_SCP_SUCCESS)
    {
        log_printf("%d, handshake fail \n", __LINE__);
        ret = E_SCP_STATUS_TRANSMIT_ERROR;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halInit
 *
 *  DESCRIPTION :
 *      Init procedure of PoE
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halInit
(
    void
)
{
    S_SCP_PKT   res_pkt, req_pkt;
    POE_BUDGE_T poeConfig[MAX_POE_BANK];
    char        *poeEnableEnv = NULL; 
    INT32       ret = E_TYPE_SUCCESS;
    UINT32      initNum=0, i, maskKeyNum=0, state=0, maskBits=0, max_port=0, lPort=0;
    UINT32      swVersion=0;
    UINT8       st_prodNum[8]={0}, st_paramCode[8]={0}, st_version[8]={0};
    UINT8       regData, pm1=0, pm2=0, pm3=0, poeState=0;
    E_BOARD_ID  boardId = sys_utilsDevBoardIdGet();

    /* Release reset of PoE output during init */
    if((ret = poe_halPoECtrlStateSet(0x1)) != E_TYPE_SUCCESS)
    {
        goto __FUN_RET;
    }

    /* call to init Dragonite function */
    poe_halInitDragonite();

    /* Initialize the i2c module system every 10sec of inactivity. */
    maskKeyNum=0x1b;
    state=0x1;
    poe_halSetIndividualMask(maskKeyNum, state);

    /* configure DC-Disconnection */    
    maskKeyNum=0x9;
    state=0x0;
    poe_halSetIndividualMask(maskKeyNum, state);
    
    log_printf("Init PoE ... DC-Disconnect Mode\n");

    /* set the system mask to default value */
    maskBits = 0x4 | (0x1 << E_BIT0_POWER_DISCONNECT) | (0x1 << E_BIT1_CAPACITOR_DETECT);
    poe_halPoESetSystemMask(maskBits);

    /* Set PM to default value */
    pm1=0x0;
    pm2=0x0;
    pm3=0x0;
    poe_halPoESetPmMethod(pm1, pm2, pm3);

    /* show PoE software version */
    poe_halGetSoftwareVersion(poeSwVersion);
    log_printf("PoE Software Version: %s\n", poeSwVersion);

    /* Set Power Bank and Power limit*/
    switch(boardId)
    {
        case E_BOARD_ID_HAYWARDS_48G4G_P:
            initNum = sizeof(poeConfig48P) / sizeof(POE_BUDGE_T);
            memcpy(poeConfig, poeConfig48P, initNum*(sizeof(POE_BUDGE_T)));
            break;
        case E_BOARD_ID_HAYWARDS_24G4G_P:
            initNum = sizeof(poeConfig24P) / sizeof(POE_BUDGE_T);
            memcpy(poeConfig, poeConfig24P, initNum*(sizeof(POE_BUDGE_T)));
            break;
        case E_BOARD_ID_HAYWARDS_16G2G_P:
            initNum = sizeof(poeConfig16P) / sizeof(POE_BUDGE_T);
            memcpy(poeConfig, poeConfig16P, initNum*(sizeof(POE_BUDGE_T)));
            break;
        case E_BOARD_ID_HAYWARDS_8G2G_P:
            initNum = sizeof(poeConfig8P) / sizeof(POE_BUDGE_T);
            memcpy(poeConfig, poeConfig8P, initNum*(sizeof(POE_BUDGE_T)));
            break;
        default:
            goto __FUN_RET;
    }

    /* Set power budget and derating parameters */
    for(i=0; i < initNum; i++)
    {
        poe_halSetDeratingData(poeConfig[i]);
    }

    POE_DELAY(5000000); /*delay 5s */

    /* save system setting */
    poe_halSaveSystemSetting();
    log_printf("Save system setting\n");

    /*delay 1s */
    POE_DELAY(1000000);

    /* Disable capacitor detection (legacy mode) for sifos's verification
     * for more detail, please refer to 4.5.10 in microsemi's user guide
     */
    maskBits = 0x4;
    poe_halPoESetSystemMask(maskBits);

    /* by Ken Hsu, 20101224, Set Power Mode, If class error occurred, this error port will auto-recover every 5s */
    pm1=0x00;
    pm2=0x02;
    pm3=0x00;
    poe_halPoESetPmMethod(pm1, pm2, pm3);

    g_poeEnable = 1;

    max_port = sys_utilsPoePortNumGet();

    /* Disalbe the poe port at default, enable before poetest */
    if(g_poeEnable != 1)
    {
        poeState=0x0;
    }
    else
    {
        poeState=0x1;
    }

    for (lPort = 1; lPort <= max_port; lPort++)
    {
        log_printf("PoE port %d enabled\n", lPort);
        poe_halSetPortMode(lPort, FALSE, 0x1);
        POE_DELAY(100000); /*delay 100ms */
    }

    if((ret = poe_halPoECtrlStateSet(0x1)) != E_TYPE_SUCCESS)
    {
        goto __FUN_RET;
    }

__FUN_RET:
    if(ret != E_TYPE_SUCCESS)
    {
        log_printf("Fail\n");
    }
    else
    {
        log_printf("Done\n");
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDetectTest
 *
 *  DESCRIPTION :
 *      a API to test detection of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *      pdType - which type the PD under test belongs to
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDetectTest
(
    IN  UINT32  lPort, 
    IN  E_POE_PD_TYPE pdType
)
{
    S_SCP_PKT req_pkt,res_pkt;
    E_SCP_RET_CODE retval;
    E_ERROR_TYPE testRetval = E_TYPE_SUCCESS;

    if(pdType != E_POE_PD_LEGACY && pdType != E_POE_PD_STANDARD)
    {
        pdType = POE_PD_DEFAULT_TYPE;
    }

    log_printf("\nEnable PoE port %d\n", lPort);

    poe_halSetPortMode(lPort, FALSE, 0x1);
    POE_DELAY(3000000); /* delay 3s to bring up the port */
    
    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));
     
    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_PORTSTATUS;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);
    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);
    
    log_printf("Port %d ", lPort);

    if (retval != E_SCP_SUCCESS)
    {
        return E_TYPE_MCU_REQUEST_ERROR;
    }

    switch (res_pkt.subject1)
    {
        case 0:
            log_printf("Legacy PD detected\r\n");
            break;
        case 1:                      
            log_printf("802.3at/af-compliant PD detected\r\n");
            break;
        default:
            log_printf("Not detected\r\n");
            break;
    }

    if (res_pkt.subject1 != pdType)
    {
        log_printf("PD Type not match %d != %d\r\n", res_pkt.subject1, pdType);

        POE_DELAY(5000000); /* delay 5s */

        memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
        memset(&res_pkt,0,sizeof(S_SCP_PKT));

        /* write user data */
        req_pkt.key = SCP_REQUEST;
        req_pkt.subject = SCP_CHANNEL;
        req_pkt.subject1 = SCP_PORTSTATUS;
        req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);
        retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

        if(res_pkt.subject1 != pdType)
        {
            poe_halSCPPktPrint(&res_pkt);

            memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
            memset(&res_pkt,0,sizeof(S_SCP_PKT));

            /* write user data */
            req_pkt.key = SCP_REQUEST;
            req_pkt.subject = SCP_CHANNEL;
            req_pkt.subject1 = SCP_PARAMZ;
            req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

            retval = poe_halSCPHandshake(&req_pkt, &res_pkt); 
            poe_halSCPPktPrint(&res_pkt);
            return E_TYPE_POE_TEST_NO_PD_DETECTED;
        }
        else
        {
            log_printf("Auto-retry Detection of %s - Pass\n", pdType?"AF/AT":"Legacy");
            testRetval = E_TYPE_SUCCESS;
        }
    }                                

    return testRetval;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halClassifyTest
 *
 *  DESCRIPTION :
 *      a API to test classification of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halClassifyTest
(
    IN  UINT32  lPort,
    IN  UINT32  pdClass
)
{
    S_SCP_PKT req_pkt,res_pkt;
    E_SCP_RET_CODE  retval;
    E_ERROR_TYPE  testRetval = E_TYPE_SUCCESS;

    if(pdClass > 4 || pdClass < 0)
    {
        pdClass = POE_PD_DEFAULT_CLASS;
    }
      
    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));
     
    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_PORTSTATUS;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    log_printf("Port %d\r\n", lPort); 

    if (retval != E_SCP_SUCCESS)
    {
        return E_TYPE_MCU_REQUEST_ERROR;
    }

    /* PD undetected */
    if(res_pkt.subject1 > 1)
    {
        return E_TYPE_POE_TEST_NO_PD_DETECTED;
    }
    /*  PD class mismatch */
    else if(res_pkt.data[1] != pdClass)
    {
        POE_DBG_CHECK(log_printf("Class %d, ", res_pkt.data[1]));
        POE_DBG_CHECK(log_printf("Not Class %d, ", pdClass));

        POE_DELAY(5000000); /* delay 5s, re-read classification results */

        memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
        memset(&res_pkt,0,sizeof(S_SCP_PKT));

        /* write user data */
        req_pkt.key = SCP_REQUEST;
        req_pkt.subject = SCP_CHANNEL;
        req_pkt.subject1 = SCP_PORTSTATUS;
        req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);
        retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

        if(res_pkt.data[1] != pdClass)
        {
            poe_halSCPPktPrint(&res_pkt);

            memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
            memset(&res_pkt,0,sizeof(S_SCP_PKT));

            /* write user data */
            req_pkt.key = SCP_REQUEST;
            req_pkt.subject = SCP_CHANNEL;
            req_pkt.subject1 = SCP_PARAMZ;
            req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

            retval = poe_halSCPHandshake(&req_pkt, &res_pkt); 
            poe_halSCPPktPrint(&res_pkt);
            return E_TYPE_POE_TEST_CLASS_MISMATCH;
        }
        else
        {
            log_printf("Retry Class %d - Pass\n", pdClass);
        }
    }

    return testRetval;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halPowerUpTest
 *
 *  DESCRIPTION :
 *      a API to test powering of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPowerUpTest
(
    IN  UINT32  lPort
)
{
    S_SCP_PKT req_pkt, res_pkt;
    E_SCP_RET_CODE  retval;
    E_ERROR_TYPE  testRetval = E_TYPE_SUCCESS;
    INT32 vmainVoltage;
    INT32 caculatedCurrent;
    INT32 powerConsumption;
    INT32 vportVoltage=0;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));

    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_PARAMZ;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    log_printf("Port %d \r\n", lPort);

    if (retval != E_SCP_SUCCESS)
    {
        return E_TYPE_MCU_REQUEST_ERROR;
    }

    vmainVoltage = (res_pkt.subject << 8) + res_pkt.subject1;
    caculatedCurrent = (res_pkt.subject2 << 8) + res_pkt.data[0];
    powerConsumption = (res_pkt.data[1] << 8) + res_pkt.data[2];
    vportVoltage = (res_pkt.data[4] << 8) + res_pkt.data[5];
    
    POE_DBG_CHECK(log_printf("[vmain]%d.%dV %dmA %dmW [vport]%d.%dV \r\n", vmainVoltage/10, vmainVoltage%10, \
                    caculatedCurrent, powerConsumption, vportVoltage/10, vportVoltage%10));

    if(vmainVoltage < (POE_PD_POWER_LOWER_LIMIT * 10) || vmainVoltage > (POE_PD_POWER_UPPER_LIMIT * 10))
    {
        POE_DBG_CHECK(log_printf("Out of range %d - %d v \r\n", POE_PD_POWER_LOWER_LIMIT, POE_PD_POWER_UPPER_LIMIT));
        testRetval = E_TYPE_DATA_MISMATCH;
        log_printf("FAIL\n");
        poe_halSCPPktPrint(&res_pkt);
    }
    else if(powerConsumption==0)
    {
        POE_DBG_CHECK(log_printf("Out of range %d - %d v \r\n", POE_PD_POWER_LOWER_LIMIT, POE_PD_POWER_UPPER_LIMIT));
        testRetval = E_TYPE_DATA_MISMATCH;
        log_printf("FAIL\n");
        poe_halSCPPktPrint(&res_pkt);
        return E_TYPE_POE_TEST_POWER_DOWN;
    }

    return testRetval;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDisconnetTest
 *
 *  DESCRIPTION :
 *      a API to test disconnection functionality of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDisconnetTest 
(
    IN  UINT32  lPort
)
{
    S_SCP_PKT req_pkt, res_pkt;
    E_SCP_RET_CODE  retval;
    E_ERROR_TYPE  testRetval = E_TYPE_SUCCESS;

    /* Disabe test PoE port */
    poe_halSetPortMode(lPort, FALSE, 0x0);
    POE_DELAY(10000); /*delay 10ms */

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));

    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_PARAMZ;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    log_printf("Port %d \r\n", lPort);

    if (retval != E_SCP_SUCCESS)
    {
        return E_TYPE_MCU_REQUEST_ERROR;
    }

    if(res_pkt.subject2 || res_pkt.data[0] || res_pkt.data[1] || res_pkt.data[2])
    {
        return E_TYPE_POE_TEST_DISCONNECT_FAILED;
    }

    /* Enable test PoE port */
    poe_halSetPortMode(lPort, FALSE, 0x1);
    POE_DELAY(10000); /*delay 10ms */

    return testRetval;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halVopCheckTest
 *
 *  DESCRIPTION :
 *      a API to test vopcheck functionality of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halVopCheckTest 
(
    IN  UINT32  lPort,
    IN  UINT32  vop_margin
)
{
    UINT32 margin=0;
    POE_POWERINFO_T portPowerInfo;
    E_ERROR_TYPE  testRetval = E_TYPE_SUCCESS;

    memset(&portPowerInfo, 0x0, sizeof(POE_POWERINFO_T));
    
    /* Get the port status data */
    testRetval = poe_halPowerShow(lPort, &portPowerInfo);
    if (testRetval != E_TYPE_SUCCESS)
    {
        log_printf("[Err]: fail to get the port status data.\n(%d)",testRetval);
        return testRetval;
    }
    
    /* check the margin*/
    if (portPowerInfo.vmainVoltage > portPowerInfo.vportVoltage)
    {
        margin = (portPowerInfo.vmainVoltage - portPowerInfo.vportVoltage) * 100;
    }
    else
    {
        margin = (portPowerInfo.vportVoltage - portPowerInfo.vmainVoltage) * 100;
    }

    log_printf("vmain: %dmv vport: %dmv margin: %dmv user_define_margin: %dmv\n", \
                    portPowerInfo.vmainVoltage*100, portPowerInfo.vportVoltage*100, margin, vop_margin);    
    if (margin > vop_margin)
    {        
        testRetval = E_TYPE_POE_TEST_VOPCHECK_FAILED; 
    }  

    return testRetval;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halGetPdClass
 *
 *  DESCRIPTION :
 *      a API to get the classified pd class
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *     pd class
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halGetPdClass 
(
    IN     UINT32  lPort,
    OUT  UINT8   *pdclass
)
{
    S_SCP_PKT req_pkt,res_pkt;
    E_SCP_RET_CODE  retval;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));
     
    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_CHANNEL;
    req_pkt.subject1 = SCP_PORTSTATUS;
    req_pkt.subject2 = port_utilsLportToPoeChNum(lPort);

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    if (retval != E_SCP_SUCCESS)
    {
        *pdclass = 255;
        return E_TYPE_MCU_REQUEST_ERROR;
    }

    /* PD undetected */
    if(res_pkt.subject1 > 2)
    {
        *pdclass = 255;
        return E_TYPE_POE_TEST_NO_PD_DETECTED;
    }

    if( res_pkt.data[1] >= 4 )
        *pdclass = 4;
    else
        *pdclass = res_pkt.data[1];
    
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halGetTotalPower
 *
 *  DESCRIPTION :
 *      a API to get total power consumption in watts
 *
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *     total power consumption in watts
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halGetTotalPower
(
    OUT  UINT32   *total_power
)
{
    S_SCP_PKT req_pkt,res_pkt;
    E_SCP_RET_CODE  retval;

    memset(&req_pkt,0x4e,sizeof(S_SCP_PKT));
    memset(&res_pkt,0,sizeof(S_SCP_PKT));

    /* write user data */
    req_pkt.key = SCP_REQUEST;
    req_pkt.subject = SCP_GLOBAL;
    req_pkt.subject1 = SCP_SUPPLY;
    req_pkt.subject2 = SCP_EXPENDEDPOWERINFO;

    retval = poe_halSCPHandshake(&req_pkt, &res_pkt);

    if (retval != E_SCP_SUCCESS)
    {
        return E_TYPE_MCU_REQUEST_ERROR;
    }

    *total_power = (res_pkt.subject << 8) + res_pkt.subject1;

    return E_TYPE_SUCCESS;
}
