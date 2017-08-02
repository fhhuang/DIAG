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
***      cmd_smitest.c
***
***    DESCRIPTION :
***      for switch smi interface test
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
#include "switch_hal.h"
#include "err_type.h"
#include "log.h"
#include "foxCommand.h"

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
INT32 do_smitest
(
	IN cmd_tbl_t *cmdtp,
	IN INT32 flag,
	IN INT32 argc,
	IN INT8 *argv[]
)
{
	INT32 ret=0, repeat;;
	
    if( strcmp("smitest", argv[0]) == 0 )
    {
    	if( argc != 2 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        
        repeat = simple_strtoul(argv[1], NULL, 10);
    	
    	if( (ret=smitest(repeat)) < 0 )
    	{
		    log_cmdPrintf(E_LOG_MSG_FAIL, "SMI test\r\n");
		    goto __CMD_ERROR;
        }
    	else
    	{
    		log_cmdPrintf(E_LOG_MSG_PASS, "SMI test\r\n");
        }
    }
    
    return ret;
    
__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
 	smitest,	CONFIG_SYS_MAXARGS,	1,	do_smitest,
 	"smitest \t- This test validates the MDC/MDIO interfaces between MAC and PHY.\n",
    "<repeat>\n"
 	"  - This test validates the MDC/MDIO interfaces between MAC and PHY.\n"
    "    repeat: repeat times for the SMI tests.\n"
);
