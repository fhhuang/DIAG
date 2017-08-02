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
***      cmd_smi.c
***
***    DESCRIPTION :
***      for switch smi interface
***
***    HISTORY :
***       - 2009/05/20, 16:30:52, Eden Weng
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
#include "port_defs.h"
#include "switch_hal.h"
#include "err_type.h"
#include "log.h"
#include "foxCommand.h"
#include "port_utils.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define DBG_PRINTF log_dbgPrintf

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

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
INT32 do_smirw
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32               ret = 0;
    UINT16              data = 0;
    UINT32              offset = 0;
    UINT32              paraCnt = 0, startPort = 0, endPort = 0, lPortBase = 0, lPort, port;
    E_LINER_PORT_TYPE   lPortType;
    INT8                lPortTypeStr[20];
	
    if ( strstr(argv[0], "smiread") )
    {
        if ( argc < 3 || argc > 4 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;
        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr)<E_TYPE_SUCCESS) 
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        offset = simple_strtoul(argv[paraCnt], NULL, 16);

        lPortBase = 0;
        port = ( startPort == 0 ) ?  1 : startPort;
        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            if ( (ret=switch_halSMIRegGet(lPort, offset, &data)) == E_TYPE_SUCCESS )
                log_printf("Read SMI: %s %02d, Register 0x%x = 0x%x\r\n", lPortTypeStr, lPort, offset, data);
            else
                log_printf("Read SMI: %s %02d, Register 0x%x = N/A\r\n", lPortTypeStr, lPort, offset);
        }
    }
    else if ( strstr(argv[0], "smiwrite") )
    {
        if ( argc < 4 || argc > 5 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        
        paraCnt = 1;
        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr)<E_TYPE_SUCCESS ) 
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        offset = simple_strtoul(argv[paraCnt], NULL, 16);
        paraCnt++;
        data = simple_strtoul(argv[paraCnt], NULL, 16);

        lPortBase = 0;
        port = ( startPort == 0 ) ?  1 : startPort;
        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            if ( (ret=switch_halSMIRegSet(lPort, offset, data)) == E_TYPE_SUCCESS )
                log_printf("Write SMI: %s %02d, Register 0x%x = 0x%x\r\n", lPortTypeStr, lPort, offset, data);
            else
                log_printf("Write SMI: %s %02d, Register 0x%x = N/A\r\n", lPortTypeStr, lPort, offset);
        }
    }	
    
    return ret;
    
__CMD_ERROR:
    return ret;
}

INT32 do_IEEEtest
(
	IN cmd_tbl_t *cmdtp,
	IN INT32 flag,
	IN INT32 argc,
	IN INT8 *argv[]
)
{
	INT32 ret=E_TYPE_SUCCESS, testmode;
	UINT32 lport=0;
    S_PORT_INFO *portInfo;
	UINT16 MSCtrlMode,IEEEtestCtrlmode;
    UINT16 orgReg9Value,orgReg26Value,orgReg27Value;
	

	if( argc != 3 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

	testmode = simple_strtoul(argv[1], NULL, 10);
	lport = simple_strtoul(argv[2], NULL, 10);
	
	portInfo = port_utilsLPortInfoGet(lport);
	
	if( portInfo->portSpeed>=PORT_SPEED_CAP_10G ) 
	{
        ret = E_TYPE_INVALID_PARA;
        goto __CMD_ERROR;
	}

	switch(testmode)
	{
		case 1:
			MSCtrlMode = 0x1F00;
			IEEEtestCtrlmode = 0x3F00;
			break;
		case 2:
			MSCtrlMode = 0x1F00;
			IEEEtestCtrlmode = 0x5F00;
			break;
		case 3:
			MSCtrlMode = 0x1700;
			IEEEtestCtrlmode = 0x7700;
			break;
		case 4:
			MSCtrlMode = 0x1F00;
			IEEEtestCtrlmode = 0x9F00;
			break;
		default:
			ret = E_TYPE_INVALID_PARA;
			goto __CMD_ERROR;
			break;
	}



	//Set PHY to Master/Slave mode
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 0);
	switch_halSMIRegGet(lport, 9, &orgReg9Value);
	switch_halSMIRegSet(lport, 9, MSCtrlMode);
	//Soft reset
	switch_halSMIRegSet(lport, PHY_CNTL_REG_OFFSET, 0x9140);

	//Disable Colck on the HSDACP/N (bit 8 = 0)
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 4);
	switch_halSMIRegGet(lport, 27, &orgReg27Value);
	switch_halSMIRegSet(lport, 27, 0x3E80);

	//Enable TX_TCLK
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 6);
	switch_halSMIRegGet(lport, 26, &orgReg26Value);
	switch_halSMIRegSet(lport, 26, 0x8000);

	//Start test IEEE mode
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 0);
	switch_halSMIRegSet(lport, 9, IEEEtestCtrlmode);

#if 1
	log_printf("Port%d IEEE Test %d Start , press Any key to break ... \r\n",lport,testmode);
	getchar();
	printf("Stop test\n");
#else
	log_printf("Port%d IEEE Test %d Start , press Ctrl+x to break ... \r\n",lport,testmode);
	do
	{
		if( ctrlc() )
		{
			printf("Stop test\n");
			break;
		}
		printf("Keep test\n");
		delay(1);
	}while(1);
#endif
	//Set back to OriginValue
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 4);
	switch_halSMIRegSet(lport, 27, orgReg27Value);
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 6);
	switch_halSMIRegSet(lport, 26, orgReg26Value);
	switch_halSMIRegSet(lport, PHY_PAGE_REG_OFFSET, 0);
	switch_halSMIRegSet(lport, 9, orgReg9Value);
	switch_halSMIRegSet(lport, PHY_CNTL_REG_OFFSET, 0x9140);


__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
     smiread,    CONFIG_SYS_MAXARGS,    1,    do_smirw,
     "smiread \t- Read PHY register.\n",
     "<port> <offset>\n"
     "  - port : Specify the port number to read. Valid range is 1~24 (or 48).\n"
     "  - offset : Specify the register offset.\n"
);

U_BOOT_CMD(
     smiwrite,    CONFIG_SYS_MAXARGS,    1,    do_smirw,
     "smiwrite \t- Write PHY register.\n",
     "<port> <offset> <hex_data>\n"
     "  - port : Specify the port number to write. Valid range is 1~24 (or 48).\n"
     "  - offset : Specify the register offset.\n"
     "  - hex_data : Specify the register value in hexadecimal format.\n"
);

U_BOOT_CMD(
 	ieeetest,	CONFIG_SYS_MAXARGS,	1,	do_IEEEtest,
 	"ieeetest \t- Config PHY to start IEEE test.\n",
    "<mode> <port>\n"
 	"  - Config PHY to start IEEE test.\n"
    "    mode: Specify IEEE test mode. Valid values are <1-4>\n"
    "    port: Specify test RJ45 port.\n"
);

