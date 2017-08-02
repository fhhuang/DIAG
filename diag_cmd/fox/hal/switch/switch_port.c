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
***      switch_port.c
***
***    DESCRIPTION :
***      for switch port
***
***    HISTORY :
***       - 2009/05/21, 14:30:52, Eden Weng
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include "cmn_type.h"
#include "porting.h"
#include "switch_hal.h"
#include "switch_port.h"
#include "port_utils.h"
#include "err_type.h"
#include "log.h"
#include "i2c_hal.h"
#include "sys_utils.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

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
/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Local Function Body segment
 *
 *==========================================================================
 */
static INT32 switchPortStatusGet
(
    IN  UINT32                  portNum,
    OUT S_SWITCH_PORT_STATUS *  portStatus
)
{
    INT32       ret = E_TYPE_SUCCESS;
    UINT32      link_status=0, speed = 0, duplex = 0, lb_status=0, autoneg=0;
    UINT8       admin=0;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(portNum);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail", __FUNCTION__, portNum);
        return E_TYPE_DATA_GET;
    }

    /* Get port link status */
    if ( (ret = switch_portLinkGet(portNum, &link_status)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if( link_status == TRUE )
    {
        portStatus->link = E_SWITCH_PORT_LINK_UP;
    }
    else if( link_status == FALSE )
    {
        portStatus->link = E_SWITCH_PORT_LINK_DOWN;
    }

    /* Get port speed */
    if ( (ret = switch_portSpeedGet(portNum, &speed)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if ( speed == 10 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_10M;
    }
    else if ( speed == 100 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_100M;
    }
    else if ( speed == 1000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_1G;
    }
    else if ( speed == 2500 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_2G5;
    }
    else if ( speed == 10000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_10G;
    }
    else if ( speed == 16000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_16G;
    }
    else if ( speed == 20000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_20G;
    }
    else if ( speed == 40000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_40G;
    }
    else
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_100G;
    }

    /* Get port duplex */
    if ( (ret = switch_portDuplexGet(portNum, &duplex)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    portStatus->duplex = duplex;

    /* Get port autonego */
    if ( (ret = switch_portAutoNegGet(portNum, &autoneg)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if ( autoneg == 1 )
    {
        portStatus->autoneg = E_SWITCH_PORT_AUTONEG_ON;
    }
    else
    {
        portStatus->autoneg = E_SWITCH_PORT_AUTONEG_OFF;
    }

    if ( (ret = switch_portLoopbackGet(portNum, &lb_status)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    portStatus->loopback = lb_status;

    /* Get port admin status */
    if ( (ret = switch_portForceLinkDownEnableGet(portNum, &admin)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if ( admin == 0 )
    {
        portStatus->admin = E_SWITCH_PORT_ADMIN_ON;
    }
    else
    {
        portStatus->admin = E_SWITCH_PORT_ADMIN_OFF;
    }

    switch(portInfo->sfpPortId){
    case 1:
        i2c_halSfpClkSel(1);
        i2c_halSfpInfoGet(portNum,&portStatus->sfpInfo);
        break;
    case 2:
        i2c_halSfpClkSel(2);
        i2c_halSfpInfoGet(portNum,&portStatus->sfpInfo);
        break;
    case 3:
        i2c_halSfpClkSel(3);
        i2c_halSfpInfoGet(portNum,&portStatus->sfpInfo);
        break;
    case 4:
        i2c_halSfpClkSel(4);
        i2c_halSfpInfoGet(portNum,&portStatus->sfpInfo);
        break;
    default:
        break;
    }

    return ret;
}

static INT32 switchCascadePortStatusGet
(
    IN  UINT32                  portNum,
    OUT S_SWITCH_PORT_STATUS *  portStatus
)
{
    INT32       ret = E_TYPE_SUCCESS;
    UINT32      link_status=0, speed = 0, duplex = 0, autoneg=0, devNum=0, phyPort=0;
    UINT8       admin=0;

    /* Get port link status */
    if(portNum==1)
    {
        devNum=1;
        phyPort=24;
    }
    else if(portNum==2)
    {
        devNum=1;
        phyPort=26;
    }
    else if(portNum==3)
    {
        devNum=0;
        phyPort=24;
    }
    else if(portNum==4)
    {
        devNum=0;
        phyPort=26;
    }

    if ( (ret = switch_halCascadePortMACLinkStatusGet(devNum, phyPort, &link_status)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if( link_status == TRUE )
    {
        portStatus->link = E_SWITCH_PORT_LINK_UP;
    }
    else if( link_status == FALSE )
    {
        portStatus->link = E_SWITCH_PORT_LINK_DOWN;
    }

    /* Get port speed */
    if ( (ret = switch_halCascadePortSpeedGet(devNum, phyPort, &speed)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if ( speed == 10 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_10M;
    }
    else if ( speed == 100 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_100M;
    }
    else if ( speed == 1000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_1G;
    }
    else if ( speed == 2500 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_2G5;
    }
    else if ( speed == 10000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_10G;
    }
    else if ( speed == 16000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_16G;
    }
    else if ( speed == 20000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_20G;
    }
    else if ( speed == 40000 )
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_40G;
    }
    else
    {
        portStatus->speed = E_SWITCH_PORT_SPEED_100G;
    }

    /* Get port duplex */
    if ( (ret = switch_halCascadePortDuplexGet(devNum, phyPort, &duplex)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    portStatus->duplex = duplex;

    /* Get port autonego */
    if ( (ret = switch_halCascadePortAutoNegGet(devNum, phyPort, &autoneg)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if ( autoneg == 1 )
    {
        portStatus->autoneg = E_SWITCH_PORT_AUTONEG_ON;
    }
    else
    {
        portStatus->autoneg = E_SWITCH_PORT_AUTONEG_OFF;
    }

    /* Get port admin status */
    if ( (ret = switch_halCascadePortForceLinkDownEnableGet(devNum, phyPort, &admin)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    if ( admin == 0 )
    {
        portStatus->admin = E_SWITCH_PORT_ADMIN_ON;
    }
    else
    {
        portStatus->admin = E_SWITCH_PORT_ADMIN_OFF;
    }
    return ret;
}

static INT32 switchPortCounterGet
(
    IN  UINT32          portNum,
    OUT S_PORT_CNT *    portCounter
)
{
    INT32   ret = 0;

    if ( (ret = switch_halPortAllCntGet(portNum, &portCounter->txCnt, &portCounter->rxCnt, &portCounter->errCnt)) != E_TYPE_SUCCESS )
    {
        log_printf("Get port TX counter fail\n");
        return ret;
    }

    return ret;
}

static INT32 switchCascadePortCounterGet
(
    IN  UINT32          portNum,
    OUT S_PORT_CNT *    portCounter
)
{
    INT32   ret = 0;

    if ( (ret = switch_halCascadePortAllCntGet(portNum, &portCounter->txCnt, &portCounter->rxCnt, &portCounter->errCnt)) != E_TYPE_SUCCESS )
    {
        log_printf("Get port TX counter fail\n");
        return ret;
    }

    return ret;
}

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portStatusDump
 *
 *  DESCRIPTION :
 *      dump port staus
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port status
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portStatusDump
(
    IN UINT32 lPort,
    OUT S_SWITCH_PORT_STATUS *portStatus
)
{
    INT32 ret = E_TYPE_SUCCESS;

    memset((void*)portStatus, 0, sizeof(S_SWITCH_PORT_STATUS));
    ret = switchPortStatusGet(lPort, portStatus);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortStatusDump
 *
 *  DESCRIPTION :
 *      dump cascade port staus
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port status
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortStatusDump
(
    IN UINT32 lPort,
    OUT S_SWITCH_PORT_STATUS *portStatus
)
{
    INT32 ret = E_TYPE_SUCCESS;

    memset((void*)portStatus, 0, sizeof(S_SWITCH_PORT_STATUS));
    ret = switchCascadePortStatusGet(lPort, portStatus);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSFPInfoDump
 *
 *  DESCRIPTION :
 *      dump port SFP information
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port sfp info
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSFPInfoDump
(
    IN  UINT32                  lPort,
    OUT S_SWITCH_PORT_STATUS *  portStatus
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 log_dbg_st = FALSE;
    
    memset(portStatus, 0, sizeof(S_SWITCH_PORT_STATUS));

    ret = switchPortStatusGet(lPort, portStatus);

    /* 07272016, The sfp port dump may causes C2800 terminal console buffer queue stop 
      * we've changed the dump SFP raw data only for debug LOG enabled 
      */
    log_dbg_st =log_dbgPrintFlagGet();

    if(log_dbg_st == TRUE) {
        log_printf("Address 0xA0 raw data dump:\n");
        log_rawDataDump(portStatus->sfpInfo.raw_data.sfpInfoA0, 256);

        log_printf("Address 0xA2 raw data dump:\n");
        log_rawDataDump(portStatus->sfpInfo.sfpInfoA2, 256);
    }
    
    portStatus->sfpInfo.parsed_data.vendor_name[15] = '\0';
    portStatus->sfpInfo.parsed_data.vendor_PN[15] = '\0';
    portStatus->sfpInfo.parsed_data.serial_no[15] = '\0';
    portStatus->sfpInfo.parsed_data.date_code[7] = '\0';

    log_printf("identifier: %d\n", portStatus->sfpInfo.parsed_data.identifier);
    log_printf("extend id: %d\n", portStatus->sfpInfo.parsed_data.ext_id);
    log_printf("connector: %d\n", portStatus->sfpInfo.parsed_data.connector);
    log_printf("transceiver: %s\n", portStatus->sfpInfo.parsed_data.transceiver);
    log_printf("encoding: %d\n", portStatus->sfpInfo.parsed_data.encoding);
    log_printf("baudrate: %d\n", portStatus->sfpInfo.parsed_data.baudrate);
    log_printf("length_9u_km: %d\n", portStatus->sfpInfo.parsed_data.length_9u_km);
    log_printf("length_9u: %d\n", portStatus->sfpInfo.parsed_data.length_9u);
    log_printf("length_50u: %d\n", portStatus->sfpInfo.parsed_data.length_50u);
    log_printf("length_62_5u: %d\n", portStatus->sfpInfo.parsed_data.length_62_5u);
    log_printf("length_Cu: %d\n", portStatus->sfpInfo.parsed_data.length_Cu);
    log_printf("vendor_name: %s\n", portStatus->sfpInfo.parsed_data.vendor_name);
    log_printf("vendor_OUI: %02x %02x %02x\n", portStatus->sfpInfo.parsed_data.vendor_OUI[0], portStatus->sfpInfo.parsed_data.vendor_OUI[1], portStatus->sfpInfo.parsed_data.vendor_OUI[2]);
    log_printf("vendor_PN: %s\n", portStatus->sfpInfo.parsed_data.vendor_PN);
    log_printf("vendor_Rev: %02x%02x%02x%02x\n", portStatus->sfpInfo.parsed_data.vendor_Rev[0],portStatus->sfpInfo.parsed_data.vendor_Rev[1],portStatus->sfpInfo.parsed_data.vendor_Rev[2],portStatus->sfpInfo.parsed_data.vendor_Rev[3]);
    log_printf("wavelength: %02x%02x\n", portStatus->sfpInfo.parsed_data.wavelength[0], portStatus->sfpInfo.parsed_data.wavelength[1]);
    log_printf("cc_base: %d\n", portStatus->sfpInfo.parsed_data.cc_base);
    log_printf("cc_ext: %d\n", portStatus->sfpInfo.parsed_data.cc_ext);

    log_printf("options: %02x%02x\n", portStatus->sfpInfo.parsed_data.options[0], portStatus->sfpInfo.parsed_data.options[1]);
    log_printf("br_max: %d\n", portStatus->sfpInfo.parsed_data.br_max);
    log_printf("br_min: %d\n", portStatus->sfpInfo.parsed_data.br_min);
    log_printf("serial_no: %s\n", portStatus->sfpInfo.parsed_data.serial_no);
    log_printf("date_code: %s\n", portStatus->sfpInfo.parsed_data.date_code);
    
    return ret ;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSFPSetRXSerdesToDAC
 *
 *  DESCRIPTION :
 *      read port SFP information if DAC7m and DAC10m and set new RX serdes parameters
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port sfp info
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSFPSetRXSerdesToDAC
(
    IN  UINT32                  lPort,
    OUT S_SWITCH_PORT_STATUS *  portStatus
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32      link_status=0;
    S_PORT_INFO *portInfo;
    S_BOARD_INFO boardInfo;
    
    portInfo = port_utilsLPortInfoGet(lPort);
    sys_utilsBoardInfoGet(&boardInfo);
    
    memset(portStatus, 0, sizeof(S_SWITCH_PORT_STATUS));

    switch(portInfo->sfpPortId){
    case 1:
        i2c_halSfpClkSel(1);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    case 2:
        i2c_halSfpClkSel(2);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    case 3:
        i2c_halSfpClkSel(3);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    case 4:
        i2c_halSfpClkSel(4);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    default:
        break;
    }
    
    portStatus->sfpInfo.parsed_data.vendor_name[15] = '\0';
    portStatus->sfpInfo.parsed_data.vendor_PN[15] = '\0';
    portStatus->sfpInfo.parsed_data.serial_no[15] = '\0';
    portStatus->sfpInfo.parsed_data.date_code[7] = '\0';

    log_dbgPrintf("length_Cu: %d\n", portStatus->sfpInfo.parsed_data.length_Cu);
    log_dbgPrintf("vendor_name: %s\n", portStatus->sfpInfo.parsed_data.vendor_name);
    log_dbgPrintf("vendor_PN: %s\n", portStatus->sfpInfo.parsed_data.vendor_PN);
    log_dbgPrintf("vendor_Rev: %02x%02x%02x%02x\n", portStatus->sfpInfo.parsed_data.vendor_Rev[0],portStatus->sfpInfo.parsed_data.vendor_Rev[1],portStatus->sfpInfo.parsed_data.vendor_Rev[2],portStatus->sfpInfo.parsed_data.vendor_Rev[3]);
    log_dbgPrintf("serial_no: %s\n", portStatus->sfpInfo.parsed_data.serial_no);
    log_dbgPrintf("date_code: %s\n", portStatus->sfpInfo.parsed_data.date_code);

    if ((portStatus->sfpInfo.parsed_data.length_Cu == 7) || (portStatus->sfpInfo.parsed_data.length_Cu == 10))
    {
        /* 05042017 -- Set RX Serdes for DAC active 7m, 10 cable */
        /* DAC7m, 10m -- vendor LOROM and TYCO 15, 7, 10 */
        if ((strncmp(portStatus->sfpInfo.parsed_data.vendor_name,"CISCO-TYCO", 10) == 0) ||
            (strncmp(portStatus->sfpInfo.parsed_data.vendor_name,"CISCO-LOROM", 11) == 0)){
            switch_tuneRxManualSet(lPort-boardInfo.firstfiberNum, 15, 7, 10, 95);

            /* Get port link status */
            switch_portLinkGet(lPort, &link_status);

            /* For MFG burn in high temperature CRC issue, we set rx autotune */
            if( link_status == TRUE ){
                log_dbgPrintf("lPort %d is link up and doing RX serdes auto\n", lPort);
                switch_tuneRxAutoSet(lPort-boardInfo.firstfiberNum);
            }
        } else if (strncmp(portStatus->sfpInfo.parsed_data.vendor_name,"CISCO-JPC", 9) == 0) {
            /* JESSE LINK JPC RX serdex, 15, 7, 14 - this fixed high temperature CRC */
            switch_tuneRxManualSet(lPort-boardInfo.firstfiberNum, 15, 7, 14, 95);

            /* Get port link status */
            switch_portLinkGet(lPort, &link_status);

            if( link_status == TRUE ){
                log_dbgPrintf("lPort %d is link up and doing RX serdes auto\n", lPort);
                switch_tuneRxAutoSet(lPort-boardInfo.firstfiberNum);
            }
       } else {
           /* If vendor name isn't specify, doing RX serdes auto tune when it's link up */
           /* To be define: switch_tuneRxAutoSet(lPort-boardInfo.firstfiberNum); */
       }
       
    } else if ((portStatus->sfpInfo.parsed_data.length_Cu == 1) &&
               (strncmp(portStatus->sfpInfo.parsed_data.vendor_name,"CISCO-AVAGO", 11) == 0)){
         
            /* To be fixed AOC1mCRC issue, doing RX maual tune and auto tune */
            switch_tuneRxManualSet(lPort-boardInfo.firstfiberNum, 15, 7, 10, 95);

            /* Get port link status */
            switch_portLinkGet(lPort, &link_status);

            /* For MFG burn in high temperature CRC issue, we set rx autotune */
            if( link_status == TRUE ){
                log_dbgPrintf("lPort %d is link up and doing RX serdes auto\n", lPort);
                switch_tuneRxAutoSet(lPort-boardInfo.firstfiberNum);
            }
         
    }
                 
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSFPDiagMonitorTemp
 *
 *  DESCRIPTION :
 *      Show port SFP/SFP+ Diagnostic Monioring data of temperature
 *
 *  INPUT :
 *      lPort      - logical port
 *     portStatus - port sfp info
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSFPDiagMonitorTemp
(
    IN  UINT32                  lPort,
    OUT S_SWITCH_PORT_STATUS *  portStatus
)
{
    INT32 ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    UINT8 buf[256]={0};
    UINT32 temp_data=0, temp2_data=0, twoComp=0, minus_temp=0; 
    UINT8 *raw_data;

    portInfo = port_utilsLPortInfoGet(lPort);

    switch(portInfo->sfpPortId){
    case 1:
        i2c_halSfpClkSel(1);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    case 2:
        i2c_halSfpClkSel(2);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    case 3:
        i2c_halSfpClkSel(3);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    case 4:
        i2c_halSfpClkSel(4);
        i2c_halSfpInfoGet(lPort,&portStatus->sfpInfo);
        break;
    default:
        break;
    }

    /* read Diagnositc Monioring data is implemented, reg. address [92] */
    raw_data =  portStatus->sfpInfo.raw_data.sfpInfoA0;

    if (*(raw_data+SFP_DIAG_MON_TYPE)> 0){
         log_dbgPrintf("P%d Diagnositc Monioring data is implemented: 0x%02x\n", \
         lPort, *(raw_data+SFP_DIAG_MON_TYPE));
    }else{
         log_printf("P%d\t  Diagnostic Monitoring data is not available ", lPort);
         log_dbgPrintf( "diag_mon_reg[%d] = 0x%02x", SFP_DIAG_MON_TYPE, *(raw_data+SFP_DIAG_MON_TYPE));
         log_printf("\n");
         return ret;
    }

    /* Read SFP EEPROM Diagnostic data from A2 address */
    ret = i2c_halRegGet(SFP_EEPROM_I2C_ADDR_A2, 0, 1, buf, 256);
    if (ret != E_TYPE_SUCCESS)
    {
        log_printf("Get the SFP Port %d address A2 EEPROM content failed.\n", lPort);
        return E_TYPE_REG_READ;
    }

    /* Diagnostic Monitoring Data Temperature defined TEMP_MSB buf[96], TEMP_LSB buf[97] 
     * Check MSB_TEMP D7 is SIGN bit, then D6-D0 temperature value.
     * Check LSB_TEMP is formar .xxx/256
     * Temperature data range: -128..000 +128
     * For example MSB=0x7F, LSB=0xFF calculated +127, 255/256 (+127.996)
     */
    temp_data = buf[SFP_DIAG_TEMP_MSB] & 0xFF;
    temp2_data =  buf[SPF_DIAG_TEMP_LSB] & 0xFF;

    log_dbgPrintf("msb_temp buf[%d]=0x%02x, lsb_temp buf[%d]=0x%02x\n", \ 
    SFP_DIAG_TEMP_MSB, temp_data, SPF_DIAG_TEMP_LSB, temp2_data);
     
    /* check sign bit 7 +/-*/
    if(((temp_data >>7) & 0x1) == 0x1){
        temp_data &= 0x7F;

        /* when sign bit is minus, need to two complement calcuate temperature */
        for(twoComp =0; twoComp<7; twoComp++)
        {
            if (((temp_data >> twoComp) & 0x1) == 0x1){
              minus_temp +=0;
            }else{
              minus_temp += (1 << twoComp) ; 
            }  
        }

        /* calcuate minus temperature data, when temp_lsb isn't 0 */
        if (((temp2_data*1000)/256) > 0)
        {
            log_printf("P%2d\t  -%2d.%03d\n", lPort, minus_temp-1, (temp2_data*1000)/256);
        }else{
            log_printf("P%2d\t  -%2d.%03d\n", lPort, minus_temp, (temp2_data*1000)/256);
        }

    }else{
        log_printf("P%2d\t  +%2d.%03d\n", lPort, temp_data, (temp2_data*1000)/256);
    }
    
    return ret ;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portCounterDump
 *
 *  DESCRIPTION :
 *      dump port counter
 *
 *  INPUT :
 *      lPort       - logical port
 *      portCounter - port statistic counter
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portCounterDump
(
    IN  UINT32          lPort,
    OUT S_PORT_CNT *    portCounter
)
{
    INT32 ret = E_TYPE_SUCCESS;

    memset(portCounter, 0, sizeof(S_PORT_CNT));
    if ( (ret = switchPortCounterGet(lPort, portCounter)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    log_printf("Port %d\n", lPort);

    log_printf("TX UNICAST             : %llu \nRX UNICAST             : %llu\n" \
               "TX MCAST               : %llu \nRX MCAST               : %llu\n" \
               "TX BCAST               : %llu \nRX BCAST               : %llu\n",
               portCounter->txCnt.txUnicastNum,  portCounter->rxCnt.rxUnicastNum,
               portCounter->txCnt.txMulticastNum,portCounter->rxCnt.rxMulticastNum,
               portCounter->txCnt.txBroadcastNum,portCounter->rxCnt.rxBroadcastNum);

    log_printf("TX Deferred            : %llu \nTX Excessive Collision : %llu\n" \
               "TX Sent Multiple       : %llu \nTX Collision           : %llu\n",
               portCounter->errCnt.txDeferred,      portCounter->errCnt.txExcessiveCollision,
               portCounter->errCnt.txSentMultiple,  portCounter->errCnt.txCollisionNum);

    log_printf("TX CRC error           : %llu\n", portCounter->errCnt.txCrcNum);
    log_printf("RX CRC error           : %llu\n", portCounter->errCnt.rxCrcNum);
#if 0
    log_printf("RX bad bytes           : %llu\n", portCounter->errCnt.rxBadBytes);
#endif
    log_printf("RX Jabber              : %llu \nRX Error               : %llu\n" \
               "RX Overrun             : %llu \nRX Undersize           : %llu\n" \
               "RX Fragments           : %llu \nRX Oversize            : %llu\n" \
               "==============================================================\n",
               portCounter->errCnt.rxJabbberNum, portCounter->errCnt.rxErrorNum,
               portCounter->errCnt.rxOverRunNum, portCounter->errCnt.rxUnderSizeNum,
               portCounter->errCnt.rxFragmentsNum, portCounter->errCnt.rxOverSizeNum);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortCounterDump
 *
 *  DESCRIPTION :
 *      dump port counter
 *
 *  INPUT :
 *      lPort       - logical port
 *      portCounter - port statistic counter
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortCounterDump
(
    IN  UINT32          lPort,
    OUT S_PORT_CNT *    portCounter
)
{
    INT32 ret = E_TYPE_SUCCESS;

    memset(portCounter, 0, sizeof(S_PORT_CNT));
    if ( (ret = switchCascadePortCounterGet(lPort, portCounter)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    log_printf("Cascade Port %d\n", lPort);

    log_printf("TX UNICAST             : %llu \nRX UNICAST             : %llu\n" \
               "TX MCAST               : %llu \nRX MCAST               : %llu\n" \
               "TX BCAST               : %llu \nRX BCAST               : %llu\n",
               portCounter->txCnt.txUnicastNum,  portCounter->rxCnt.rxUnicastNum,
               portCounter->txCnt.txMulticastNum,portCounter->rxCnt.rxMulticastNum,
               portCounter->txCnt.txBroadcastNum,portCounter->rxCnt.rxBroadcastNum);

    log_printf("TX Deferred            : %llu \nTX Excessive Collision : %llu\n" \
               "TX Sent Multiple       : %llu \nTX Collision           : %llu\n",
               portCounter->errCnt.txDeferred,      portCounter->errCnt.txExcessiveCollision,
               portCounter->errCnt.txSentMultiple,  portCounter->errCnt.txCollisionNum);

    log_printf("TX CRC error           : %llu\n", portCounter->errCnt.txCrcNum);
    log_printf("RX CRC error           : %llu\n", portCounter->errCnt.rxCrcNum);
#if 0
    log_printf("RX bad bytes           : %llu\n", portCounter->errCnt.rxBadBytes);
#endif
    log_printf("RX Jabber              : %llu \nRX Error               : %llu\n" \
               "RX Overrun             : %llu \nRX Undersize           : %llu\n" \
               "RX Fragments           : %llu \nRX Oversize            : %llu\n" \
               "==============================================================\n",
               portCounter->errCnt.rxJabbberNum, portCounter->errCnt.rxErrorNum,
               portCounter->errCnt.rxOverRunNum, portCounter->errCnt.rxUnderSizeNum,
               portCounter->errCnt.rxFragmentsNum, portCounter->errCnt.rxOverSizeNum);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portLinkGet
 *
 *  DESCRIPTION :
 *      get port link status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      linkStatus - UP/DOWN
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portLinkGet
(
    IN UINT32 lPort,
    OUT UINT32 *linkStatus
)
{
    UINT32 macLink=0, phyLink=0;
    switch_halPortMACLinkStatusGet(lPort, &macLink);
    switch_halPortLinkStatusGet(lPort, &phyLink);
    if( (macLink==0x1) && (phyLink==0x1) )
    {
        *linkStatus = 0x1;
    }
    else
    {
        *linkStatus = 0x0;
    }
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSpeedGet
 *
 *  DESCRIPTION :
 *      get port speed
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      portSpeed - 10M/100M/1G
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSpeedGet
(
    IN UINT32 lPort,
    OUT UINT32 *portSpeed
)
{
    return switch_halPortSpeedGet(lPort, portSpeed);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSpeedSet
 *
 *  DESCRIPTION :
 *      set port speed
 *
 *  INPUT :
 *      lPort - logical port
 *      portSpeed - 10M/100M/1G
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSpeedSet
(
    IN UINT32 lPort,
    IN UINT32 portSpeed
)
{
    return switch_halPortSpeedSet(lPort, portSpeed, E_LB_TEST_TYPE_NONE);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portDuplexGet
 *
 *  DESCRIPTION :
 *      get port duplex
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      portDuplex - Half/Full
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portDuplexGet
(
    IN UINT32 lPort,
    OUT UINT32 *portDuplex
)
{
    return switch_halPortDuplexGet(lPort, portDuplex);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portDuplexSet
 *
 *  DESCRIPTION :
 *      set port duplex
 *
 *  INPUT :
 *      lPort - logical port
 *      portDuplex - Half/Full
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portDuplexSet
(
    IN UINT32 lPort,
    IN UINT32 portDuplex
)
{
#if 0
    return switch_halPortDuplexSet(lPort, portDuplex);
#endif
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portAutoNegGet
 *
 *  DESCRIPTION :
 *      get port autonego status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      portAutoneg - ON/OFF
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portAutoNegGet
(
    IN UINT32 lPort,
    OUT UINT32 *portAutoneg
)
{
    return switch_halPortAutoNegGet(lPort, portAutoneg);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portAutoNegSet
 *
 *  DESCRIPTION :
 *      set port autonego status
 *
 *  INPUT :
 *      lPort - logical port
 *      portAutoneg - ON/OFF
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portAutoNegSet
(
    IN UINT32 lPort,
    IN UINT32 portAutoneg
)
{
    return switch_halPortAutoNegSet(lPort, portAutoneg);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portForceLinkDownEnableGet
 *
 *  DESCRIPTION :
 *      get port admin status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      status - enable/disable
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portForceLinkDownEnableGet
(
    IN UINT32 lPort,
    OUT UINT8 *status
)
{
    return switch_halPortForceLinkDownEnableGet(lPort, status);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portForceLinkDownEnableSet
 *
 *  DESCRIPTION :
 *      set port admin status
 *
 *  INPUT :
 *      lPort - logical port
 *      portEnable - enable/disable
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portForceLinkDownEnableSet
(
    IN UINT32 lPort,
    IN UINT8 portEnable
)
{
    return switch_halPortForceLinkDownEnableSet(lPort, portEnable);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portLoopbackGet
 *
 *  DESCRIPTION :
 *      get port loopback status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      loopbackStatus - MAC/PHY/NONE
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portLoopbackGet
(
    IN UINT32 lPort,
    OUT UINT32 *loopbackStatus
)
{
    return switch_halPortLoopbackGet(lPort, loopbackStatus);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portLoopbackSet
 *
 *  DESCRIPTION :
 *      set port loopback status
 *
 *  INPUT :
 *      lPort - logical port
 *      loopbackStatus - MAC/PHY/NONE
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portLoopbackSet
(
    IN UINT32 lPort,
    IN UINT32 loopbackStatus
)
{
    UINT32 speed=0;
    if( (loopbackStatus == E_LB_TEST_TYPE_PHY) ||(loopbackStatus == E_LB_TEST_TYPE_NONE) || (loopbackStatus == E_LB_TEST_TYPE_MAC) )
    {
        switch_halPortSpeedGet(lPort, &speed);
        switch_halPortSpeedSet(lPort, speed, loopbackStatus);
    }
    return switch_halPortLoopbackSet(lPort, loopbackStatus);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFrameSizeGet
 *
 *  DESCRIPTION :
 *      Get port max frame size
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      frameSize - 1518 bytes ~ 16360 bytes
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portFrameSizeGet
(
    IN  UINT32  lPort,
    OUT  UINT32  *frameSize
)
{
    return switch_halPortFrameSizeGet(lPort, frameSize);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFrameSizeSet
 *
 *  DESCRIPTION :
 *      Get port max frame size
 *
 *  INPUT :
 *      lPort - logical port
 *      frameSize - 1518 bytes ~ 16360 bytes
 *
 *  OUTPUT :
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portFrameSizeSet
(
    IN  UINT32  lPort,
    IN  UINT32  frameSize
)
{
    return switch_halPortFrameSizeSet(lPort, frameSize);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFrameSizeGet
 *
 *  DESCRIPTION :
 *      Get cascade port max frame size
 *
 *  INPUT :
 *      lPort - logical cascade port
 *
 *  OUTPUT :
 *      frameSize - 1518 bytes ~ 16360 bytes
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortFrameSizeGet
(
    IN   UINT32  lPort,
    OUT  UINT32  *frameSize
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 devNum=0, phyPort=0;

    /* Get port link status */
    if(lPort==1)
    {
        devNum=1;
        phyPort=24;
    }
    else if(lPort==2)
    {
        devNum=1;
        phyPort=26;
    }
    else if(lPort==3)
    {
        devNum=0;
        phyPort=24;
    }
    else if(lPort==4)
    {
        devNum=0;
        phyPort=26;
    }

    if ( (ret = switch_halCascadePortFrameSizeGet(devNum, phyPort, frameSize)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFrameSizeSet
 *
 *  DESCRIPTION :
 *      Get cascade port max frame size
 *
 *  INPUT :
 *      lPort - cascade port
 *      frameSize - 1518 bytes ~ 16360 bytes
 *
 *  OUTPUT :
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortFrameSizeSet
(
    IN  UINT32  lPort,
    IN  UINT32  frameSize
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 devNum=0, phyPort=0;

    /* Get port link status */
    if(lPort==1)
    {
        devNum=1;
        phyPort=24;
    }
    else if(lPort==2)
    {
        devNum=1;
        phyPort=26;
    }
    else if(lPort==3)
    {
        devNum=0;
        phyPort=24;
    }
    else if(lPort==4)
    {
        devNum=0;
        phyPort=26;
    }

    if ( (ret = switch_halCascadePortFrameSizeSet(devNum, phyPort, frameSize)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFlowControlGet
 *
 *  DESCRIPTION :
 *      get port flow control status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      status - enabled/disabled
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portFlowControlGet
(
    IN  UINT32  lPort,
    OUT  UINT8  *status
)
{
    return switch_halPortFlowControlGet(lPort, status);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFlowControlSet
 *
 *  DESCRIPTION :
 *      set port flow control status
 *
 *  INPUT :
 *      lPort - logical port
 *      status - enabled/disabled
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portFlowControlSet
(
    IN  UINT32  lPort,
    IN  UINT8  status
)
{
    return switch_halPortFlowControlSet(lPort, status);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFlowControlGet
 *
 *  DESCRIPTION :
 *      Get cascade port flow control status
 *
 *  INPUT :
 *      lPort - logical cascade port
 *
 *  OUTPUT :
 *      status - enabled/disabled
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortFlowControlGet
(
    IN   UINT32  lPort,
    OUT  UINT8   *status
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 devNum=0, phyPort=0;

    /* Get port link status */
    if(lPort==1)
    {
        devNum=1;
        phyPort=24;
    }
    else if(lPort==2)
    {
        devNum=1;
        phyPort=26;
    }
    else if(lPort==3)
    {
        devNum=0;
        phyPort=24;
    }
    else if(lPort==4)
    {
        devNum=0;
        phyPort=26;
    }

    if ( (ret = switch_halCascadePortFlowControlGet(devNum, phyPort, status)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFlowControlSet
 *
 *  DESCRIPTION :
 *      Get cascade port flow control status
 *
 *  INPUT :
 *      lPort - cascade port
 *      status - enabled/disabled
 *
 *  OUTPUT :
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortFlowControlSet
(
    IN  UINT32  lPort,
    IN  UINT8   status
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 devNum=0, phyPort=0;

    /* Get port link status */
    if(lPort==1)
    {
        devNum=1;
        phyPort=24;
    }
    else if(lPort==2)
    {
        devNum=1;
        phyPort=26;
    }
    else if(lPort==3)
    {
        devNum=0;
        phyPort=24;
    }
    else if(lPort==4)
    {
        devNum=0;
        phyPort=26;
    }

    if ( (ret = switch_halCascadePortFlowControlSet(devNum, phyPort, status)) != E_TYPE_SUCCESS )
    {
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portClearCounter
 *
 *  DESCRIPTION :
 *      clear statistics of the specific port
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portClearCounter
(
    IN UINT32 lPort
)
{
    return switch_halClearPortCounter(lPort);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortClearCounter
 *
 *  DESCRIPTION :
 *      clear statistics of the cascade port
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_cascadePortClearCounter
(
    IN UINT32 lPort
)
{
    return switch_halClearCascadePortCounter(lPort);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSfpRateSet
 *
 *  DESCRIPTION :
 *      port sfp+ rate select high or low
 *
 *  INPUT :
 *      lPort       - logical port
 *      rateLevel - sfp port rate select
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSfpRateSet
(
    IN UINT32 lPort,
    IN UINT32 rateLevel
)
{
    return switch_halPortSfpRateSet(lPort, rateLevel);    
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSfpRateTest
 *
 *  DESCRIPTION :
 *      Test sfp port rate select 
 *
 *  INPUT :
 *      lPort      - logical port
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portSfpRateTest
(
    IN  UINT32                  lPort
)
{
    return switch_halPortSfpRateTest(lPort);    
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portCableDiag
 *
 *  DESCRIPTION :
 *      switch cable diagnostic test
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portCableDiag
(
    IN UINT32 lPort,
    OUT PHY_CABLE_INFO *cableStatus
)
{
    return switch_halCableDiag(lPort, cableStatus);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portEEESet
 *
 *  DESCRIPTION :
 *      Set the LPI mode
 *
 *  INPUT :
 *      lPort - logical port
 *      mode - enable/disable
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portEEESet
(
    IN UINT32 lPort,
    IN E_PHY_LPI_MODE mode
)
{
    return switch_halEEESet(lPort, mode);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portEEEGet
 *
 *  DESCRIPTION :
 *      Get the LPI mode
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      mode - EEE mode status
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_portEEEGet
(
    IN UINT32 lPort,
    OUT E_PHY_LPI_MODE *mode
)
{
    return switch_halEEEGet(lPort, mode);
}

