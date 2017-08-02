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
***      act2Test.c
***
***    DESCRIPTION :
***      for FPGA ACT2 test
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
***       - 2016/06/23, 11:15:00, Alex Huang
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
#include "sys_utils.h"
#include "err_type.h"
#include "porting.h"
#include "cmn_type.h"
#include "mpp_i2c.h"
#include "act2test.h"
#include "tam_library.h"
#include "tam_lib_manufacturing.h"

#include "log.h"
#include "foxCommand.h"

#include "mvTypes.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
uint8_t  *sudi_cert = NULL;
uint8_t  *sudi_cert_chain = NULL;
uint32_t delaytime = 10000;

/* added devmem 0620216*/
int devmemFd;

/* Added by Foxconn Alex, 2016/06/13 */
MV_SPI_TYPE_INFO *currSpiInfo = NULL;

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
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      atohex
 *
 *  DESCRIPTION :
 *      atohex
 *
 *  INPUT :
 *      None
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
static char atohex
(
    char c
)
{
    if (c >= '0' && c <= '9')
        return(c - '0');
    if (c >= 'A' && c <= 'F')
        return(c - ('A' - 10));
    if (c >= 'a' && c <= 'f')
        return(c - ('a' - 10));

  return (-1);
}


/*************************TEST INTERFACE FUNCTIONS***************************/

/* This function is available in Linux, but not in Stardust2.  */
/* 4096 was returned by the Linux function.  Seems to work ok. */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      get_page_size
 *
 *  DESCRIPTION :
 *      get_page_size
 *
 *  INPUT :
 *      None
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
static int get_page_size (void)
{
    return (ACT2_PAGESIZE);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      getline_act2
 *
 *  DESCRIPTION :
 *      getline_act2
 *
 *  INPUT :
 *      None
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
static int getline_act2
(
    char *buf,
    uint16_t size
)
{
    int xx=0;

    if (size >= (getpagesize()-1))
    {
        log_printf("read size is %d; must be less than %d \n", size, getpagesize()-1);
    }

    fgets(buf, size, stdin);
    xx = strlen(buf);
    if (xx && (buf[xx-1] == '\n')) {
        buf[xx-1] = '\0';
        xx--;
    }

    return (xx);
}

/*cptr : character buffer pointer */
/*longret : for the result */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      getnnum
 *
 *  DESCRIPTION :
 *      getnnum
 *
 *  INPUT :
 *      None
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
static int getnnum (
    char *cptr,
    int base,
    uint32_t *longret,
    int maxchars)
{
    char cval;
    uint32_t value = 0; /* init */
    int count = 0; /* init */

    while(1) {
        cval = atohex(*cptr);
        if (cval < 0 || cval >= base) {
            break;  /* invalid character encountered */
        }
        value = (value * base) + cval;
        cptr++;
        count++;

        if (maxchars && count == maxchars) {
            break;
        }
    }
    *longret = value;  /* place result */
    return (count);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      getnum
 *
 *  DESCRIPTION :
 *      getnum
 * Convert the ascii string pointed to by cptr to binary according to base.
 * Result is placed in *longret.
 * Return value is the number of characters processed.
 * Maxchars defines the maximum number of characters to process.  If
 * maxchars == 0, process until an invalid character occurs.
 * Getnum exists for historical reasons.
 * cptr : character buffer pointer
 *longret : for the result
 *  INPUT :
 *      None
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
static int getnum
(
    char *cptr,
    int base,
    uint32_t *longret
)
{
  return (getnnum(cptr, base, longret, 0));
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      get_line
 *
 *  DESCRIPTION :
 *      get_line
 *
 *  INPUT :
 *      None
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
static int get_line
(
    char *ptr,
    uint32_t size
)
{
    int i;
    uint32_t len = 0;

    fgets(ptr, size, stdin);

    len = strlen(ptr);
    for (i = 0; i < 2; i++) {
        if (len) {
            if (ptr[len - 1] == '\r' || ptr[len - 1] == '\n') {
                ptr[len - 1] = '\0';
                len--;
            }
        }
    }

    return (len);
}

/*****************TEST INTERFACE FUNCTIONS ABOVE THIS LINE*******************/

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_local_open_device
 *
 *  DESCRIPTION :
 *      act2_local_open_device
 *
 *   Function: act2_local_open_device
 *   Description: open act2 tam device and get tam_handle
 *   Parameters: platform_opaque_handle - pointer to act2 device private
 *   Returns: 0 - success, 1 - error
 *
 *  INPUT :
 *      None
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
uint32_t act2_local_open_device
(
    void *platform_opaque_handle,
    void **tam_handle
)
{
    tam_lib_status_t rc = TAM_RC_OK;
    uint8_t use_interrupt = 0;
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = 0x0000C000;

    if (gSpiFlag) {
        log_printf("\n[SPI Init]: ");
        if (mvSysSpiInit(0, 0x1000000) != MV_OK) {
            log_printf("Failed\n");
            return -1;
        }
        log_printf("Success\n[Setting Params]: ");
        if (mvSpiParamsSet(0, 0, 4) != MV_OK) {
            log_printf("\nSPI param set failed\n");
            return -1;
        }
        log_printf("Success\n\n");

        /* Initialize Mailbox */
        rc = tam_lib_device_open_mailbox(platform_opaque_handle, use_interrupt,
                                  mbx_msg_size, mbx_reg_base_addr, tam_handle);

        if (rc != TAM_RC_OK) {
            log_printf("\n%s-%u Platform-SCC:Mailbox INIT failed! Status=0x%0x-%s ",
                __FUNCTION__, __LINE__, rc, tam_lib_rc2string(rc));
        }
    } else {
       /* tam_handle must be NULL prior to the open call */
       rc = tam_lib_device_open(platform_opaque_handle,
               TAM_LIB_PLATFORM_BUF_SIZE,
               tam_handle);
       if (rc != TAM_RC_OK) {
           log_printf("\n%s-%u ERROR open status=0x%0x-%s ",
               __FUNCTION__, __LINE__,
                rc, tam_lib_rc2string(rc));
       }
    }

    return (rc);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_local_close_device
 *
 *  DESCRIPTION :
 *      act2_local_close_device
 *
 *    Function: act2_local_close_device
 *    Description: open act2 tam device and get tam_handle
 *    Parameters: platform_opaque_handle - pointer to act2 device private
 *    Returns: 0 - success, 1 - error
 *
 *  INPUT :
 *      None
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
int act2_local_close_device
(
    void **tam_handle
)
{
    tam_lib_status_t rc = TAM_RC_OK;

    rc = tam_lib_device_close(tam_handle);
    if (rc != TAM_RC_OK) {
        log_printf("\n%s-%u ERROR close rc=0x%0x-%s ",
                __FUNCTION__, __LINE__,
                rc, tam_lib_rc2string(rc));
        return (rc);
    }

    return (rc);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_set_simple_mode
 *
 *  DESCRIPTION :
 *      act2_set_simple_mode
 *
 *  INPUT :
 *      None
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
static boolean act2_set_simple_mode( void *tam_handle )
{
    tam_lib_status_t result=TAM_RC_OK;
    uint8_t act2Mode;

    /*Check ACT2 Mode and set it to SimpleMode */
    act2Mode = tam_lib_check_mode(tam_handle);
    if (BUS_MODE_SIMPLE != act2Mode) {
        result = tam_lib_set_simple(tam_handle);
        if (TAM_RC_OK != result) {
            return (result);
        }
    }

    return (result);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      read_and_cmp_sudi_cert
 *
 *  DESCRIPTION :
 *      read_and_cmp_sudi_cert
 *
 *  INPUT :
 *      None
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
static int read_and_cmp_sudi_cert
(
    void *tam_handle,
    uint8_t *key_type,
    uint32_t session_id
)

{
    tam_lib_status_t status=TAM_RC_OK;
    uint16_t i;
    uint8_t *idevid_cert_chain = NULL;
    uint8_t *idevid_cert = NULL;
    uint8_t object_type = 0;
    uint16_t object_length = 0;
    uint16_t cert_chain = 0, cert = 0;

    /* define cert and cert_chain */
    if( !strncmp("RSA", key_type, 3) )
    {
        cert = TAM_LIB_IDEVID_RSA_CERT;
        cert_chain = TAM_LIB_IDEVID_RSA_CERT_CHAIN;
    }
    else if( !strncmp("ECC", key_type, 3) )
    {
        cert = TAM_LIB_IDEVID_ECC_CERT;
        cert_chain = TAM_LIB_IDEVID_ECC_CERT_CHAIN;
    }
    else
    {
        log_printf("Unsupport mode.\n");
        return (-1);
    }

    log_dbgPrintf("cert = 0x%x, cert_chain=0x%x\n", cert, cert_chain);

    /* Display SUDI Cert Chain */
    status = tam_lib_object_readinfo(tam_handle,
                                     session_id,
                                     cert_chain,
                                     &object_type,
                                     &object_length);

    log_printf("\nA2L SUDI Cert Chain Read Object Info Status = 0x%02x\n", status);

    if (status) {
        return (int) status;
    }

    idevid_cert_chain = (uint8_t *) malloc(object_length);
    if (!idevid_cert_chain) {
        log_printf("***ERR Malloc failed for idevid_cert_chain\n");
        return (-1);
    }


    memset(idevid_cert_chain, 0x00, object_length);

    status = tam_lib_object_read(tam_handle,
                                 session_id,
                                 cert_chain,
                                 idevid_cert_chain,
                                 &object_length);

    log_printf("\nA2L Object Read Status = 0x%02x\n", status);

    if (status) {
        free(idevid_cert_chain);
        return (int) status;
    }

    for (i = 0; i < object_length; i++ ) {
        if ( i % 32 == 0) {
            log_printf("\n[0x%08x] ", i);
        }
        log_printf("%02x", idevid_cert_chain[i]);
    }
    log_printf("\n");

    for (i = 0; i < object_length; i++ ) {
        if (idevid_cert_chain[i] != sudi_cert_chain[i]) {
            log_printf("SUDI Cert Chain read mis-compare byte %d\n",i);
            log_printf("Expected: 0x%x  Actual: 0x%x\n",
                   sudi_cert_chain[i],idevid_cert_chain[i]);
            free(idevid_cert_chain);
            return (-1);
        }
    }

    /* Display SUDI Cert */
    status = tam_lib_object_readinfo(tam_handle,
                                     session_id,
                                     cert,
                                     &object_type,
                                     &object_length);

    log_printf("\nA2L SUDI Cert Object Read Info Status = 0x%02x\n", status);

    if (status) {
        free(idevid_cert_chain);
        return (int) status;
    }

    idevid_cert = (uint8_t *) malloc(object_length);
    if (!idevid_cert) {
        log_printf("***ERR Malloc failed for idevid_cert\n");
        free(idevid_cert_chain);
        return (-1);
    }


    memset(idevid_cert, 0x00, object_length);

    status = tam_lib_object_read(tam_handle,
                                 session_id,
                                 cert,
                                 idevid_cert,
                                 &object_length);

    log_printf("\nA2L Object Read Status = 0x%02x\n", status);

    if (status) {
        free(idevid_cert);
        free(idevid_cert_chain);
        return (int) status;
    }

    for (i = 0; i < object_length; i++ ) {
        if ( i % 32 == 0) {
            log_printf("\n[0x%08x] ", i);
        }
        log_printf("%02x", idevid_cert[i]);
    }
    log_printf("\n");

    for (i = 0; i < object_length; i++ ) {
        if (idevid_cert[i] != sudi_cert[i]) {
            log_printf("SUDI Cert read mis-compare byte %d\n",i);
            log_printf("Expected: 0x%x  Actual: 0x%x\n",
                   sudi_cert[i],idevid_cert[i]);
            free(idevid_cert);
            free(idevid_cert_chain);
            return (-1);
        }
    }

    free(idevid_cert_chain);
    free(idevid_cert);

    return (int) status;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_get_chipSN
 *
 *  DESCRIPTION :
 *      act2_get_chipSN
 *
 *  INPUT :
 *      None
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
int act2_get_chipSN
(
    void *act2_device
)
{
    int i;
    tam_lib_status_t result=TAM_RC_OK;
    uint8_t csn[CHIP_SERIAL_NUMBER_LENGTH];
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;

    memset (&csn[0],0,CHIP_SERIAL_NUMBER_LENGTH);

//    result = act2_local_open_device(&act2_device, &tam_handle);
    result = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != result) {
        log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                result,tam_lib_rc2string(result));
        return (result);
    }

    if (act2_set_simple_mode(tam_handle) != TAM_RC_OK) {

        log_printf("\n Error: Unable to set ACT2 chip to simple mode.\n");
        act2_local_close_device(&tam_handle);
        return (result);
    }

    /* get the ACT2 device serial number */
    result = tam_lib_get_chip_serial_number(tam_handle, csn);
    if (TAM_RC_OK != result) {
         log_printf("\n Error: Unable to get ACT2 chip serial number, status = 0x%0x-%s\n",
                 result,tam_lib_rc2string(result));
         act2_local_close_device(&tam_handle);
         return (result);
    }

    log_printf("CSN = ");
    for (i = 0; i < CHIP_SERIAL_NUMBER_LENGTH; i++) {
        log_printf("%02X", csn[i]);
    }
    log_printf("\n");
    log_printf("\n");

    act2_local_close_device(&tam_handle);

    return (result);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_closeSession
 *
 *  DESCRIPTION :
 *      act2_closeSession
 *
 *  INPUT :
 *      None
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
int act2_closeSession
(
    void *act2_device,
    uint32_t session_id
)
{
    tam_lib_status_t result=TAM_RC_OK;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;

    result = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != result) {
        log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                result,tam_lib_rc2string(result));
        return (result);
    }

    if (act2_set_simple_mode(tam_handle) != TAM_RC_OK) {

        log_printf("\n Error: Unable to set ACT2 chip to simple mode.\n");
        act2_local_close_device(&tam_handle);
        return (result);
    }

    result = tam_lib_session_end(tam_handle, session_id);
    if (TAM_RC_OK != result) {
        log_printf("\n***ERR Session End FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    }

    log_printf("\n> ACT2 Session Ended for ID 0x%X\n",session_id);
    act2_local_close_device(&tam_handle);

    return (result);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_get_chipInfo
 *
 *  DESCRIPTION :
 *      act2_get_chipInfo
 *
 *  INPUT :
 *      None
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
int act2_get_chipInfo
(
    void *act2_device
)
{
    tam_lib_chip_info_t chip_info;
    uint8_t csn[CHIP_SERIAL_NUMBER_LENGTH];
    tam_lib_status_t result=TAM_RC_OK;
    int count;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;

    result = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != result) {
        log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                result,tam_lib_rc2string(result));
        return (result);
    }

    if (act2_set_simple_mode(tam_handle) != TAM_RC_OK) {

        log_printf("\n Error: Unable to set ACT2 chip to simple mode.\n");
        act2_local_close_device(&tam_handle);
        return (result);
    }
    usleep(5000000);

    memset (&csn[0],0,CHIP_SERIAL_NUMBER_LENGTH);

    /*get device info*/
    result = tam_lib_get_chip_info(tam_handle, &chip_info);
    if (TAM_RC_OK != result) {
        log_printf("ERR: Unable to get chip info. ErrCode:%d\n", result);
        act2_local_close_device(&tam_handle);
        return (result);
    }

    log_printf("FW Version: %d.%d.%d\n", chip_info.fw_version[0],
           chip_info.fw_version[1], chip_info.fw_version[2]);
    log_printf("Metal Rand: ");
    for (count = 0; count < TAM_LIB_METAL_RAND_LENGTH; count++) {
        log_printf("%02x ",chip_info.metal_rand[count]);
    }
    log_printf("\n");
    log_printf("Last Reset Status: ");
    for (count = 0; count < TAM_RESET_STATUS_LENGTH; count++) {
        log_printf("%02x ",chip_info.latest_reset_status[count]);
    }
    log_printf("\n");
    log_printf("Reset Counts: ");
    for (count = 0; count < TAM_RESET_COUNTS_BYTES; count++) {
        log_printf("%02x ",chip_info.reset_counts[count]);
    }
    log_printf("\n");
    log_printf("Total RAM: %d\n",chip_info.total_ram);
    log_printf("Total ROM: %d\n",chip_info.total_rom);

    result = act2_local_close_device(&tam_handle);
    if(result)
    {
        REPORT_TAM_ERROR(result);
        return result;
    }
    return (result);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_get_pid_ssn
 *
 *  DESCRIPTION :
 *      act2_get_pid_ssn
 *
 *  INPUT :
 *      None
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
static int act2_get_pid_ssn
(
    char *pPid,
    char *pSsn
)
{
    int ret = E_TYPE_SUCCESS;

    /* Fetch PID and SN from pcamap */
    ret = hal_pcamapItemGet("model_num", pPid);
    if(ret != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get the pid content.\n");
        return ret;
    }

    /*Lowell: change the mother board sn to system_sn 2016/06/20*/
    ret = hal_pcamapItemGet("system_sn", pSsn);
    if(ret != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get the psn content.\n");
        return ret;
    }

    return ret;
}


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      GetOATData
 *
 *  DESCRIPTION :
 *      GetOATData
 *
 *  INPUT :
 *      None
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
static uint16_t GetOATData
(
    uint16_t max_cnt,
    uint8_t *buffer,
    uint16_t length,
    char *prompt
)
{
    uint16_t cnt,xx,bytes;
    uint16_t transfer_size;

    xx = 0;
    bytes = 0;
    transfer_size = 0;
    for (cnt=0; cnt<max_cnt; cnt++) {

        transfer_size = length - bytes;
        if (transfer_size > get_page_size() -2) {
            transfer_size = get_page_size() -4;
        }
        log_printf("\n%s",prompt);
        xx = getline_act2(&buffer[bytes], transfer_size);
        bytes +=xx;
        log_printf("\n..count %d; total bytes entered so far is %d\n", cnt, bytes);
    }
    return (bytes);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      ConvertOATToHex
 *
 *  DESCRIPTION :
 *      ConvertOATToHex
 *
 *  INPUT :
 *      None
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
static boolean ConvertOATToHex
(
    uint8_t *buffer,
    uint16_t length
)

{
    uint16_t cnt;

    for (cnt = 0; cnt < length; cnt++) {
        buffer[cnt] = atohex(buffer[cnt]);
    }
    return (TRUE);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      getdec_answer
 *
 *  DESCRIPTION :
 *      getdec_answer
 *
 *  INPUT :
 *      None
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
static int getdec_answer
(
    char *msgstr,
    uint32_t currentval,
    uint32_t min,
    uint32_t max
)
{
    char buffer[32];
    uint32_t newval;

    memset(buffer,0,32);
    while(1) {

        log_printf("%s [%d]:  ", msgstr, currentval);

        get_line(buffer,sizeof(buffer));

        if (buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n') {
            return(currentval); /* null line returns current value */
        }
        if ((getnum(buffer,10, (uint32_t *)&newval)) <= 0 ||
                    (newval < min) || (newval > max)) {
            log_printf("valid entry %d to %d ... try again\n", min, max);
            continue;
        }
        else {
            return (newval);
        }
    }
}

//*****************TEST INTERFACE FUNCTIONS ABOVE THIS LINE******************

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_diags_manufacturing_login
 *
 *  DESCRIPTION :
 *      act2_diags_manufacturing_login
 *
 *   Get chip serial number.
 *   Init Mfg login
 *     Receive (cert chain + nonce) length from OAT
 *     Receive 32 byte Nonce from ACT2 library
 *   Finalize Mfg login
 *     Receive (nonce + cert chain) from OAT
 *     Receive RSA 2048 bit signature (over the nonce and cert chain) from OAT
 *     Receive Mfg Login Session ID from ACT2 library
 *
 *   Note that the prints in these routines are expected by the OAT system.
 *  INPUT :
 *      None
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
static int act2_diags_manufacturing_login
(
    void *tam_handle,
    uint32_t *session_id
)

{
    tam_lib_status_t status = TAM_RC_OK;
    int cert_chain_length = 0, i;
    /* This buffer is allocated to receive the data from
     * the Open Autotest (OAT) server. In this reference
     * implementation, the data is received as ascii characters
     * for each digit of the hex data.
     */
    uint8_t *cert_chain_from_oat = NULL;
    uint16_t cert_chain_from_oat_length = 0;

    /* This buffer will be populated with the binary data to send
     * to the ACT-2Lite chip. It is the destination buffer for the
     * nonce returned by the act_lib_mfg_login_init() function and
     * the converted token credential data received from OAT.
     */
    uint8_t *credentials = NULL;
    uint8_t *ptr_credentials = NULL;
    uint16_t credentials_length = 0;

    uint8_t signature_from_oat[SIG_FROM_OAT_LENGTH]={0};
    uint8_t signature[SIG_LENGTH]={0};
    size_t  signature_length = 0;

    /* Validate input parameters */
    if (!tam_handle || !session_id) {
        return (-1);
    }

    memset(signature_from_oat, '0', sizeof(signature_from_oat));
    memset(signature, '0', sizeof(signature));

    /* Initialize the Manufacturing Session ID to 0 */
    *session_id = 0;

    /* Get the length of the eToken certificate chain from OAT */
    do {
        cert_chain_length = getdec_answer("\n Enter cert chain length >",
                                          0,1,16384);
    } while (!cert_chain_length);

    log_printf("\ncert chain length: %d\n", cert_chain_length);

    /* Allocate a buffer large enough to hold the certificate chain
       represented in ASCII format (2 bytes per hex byte value) */
    /* plus a little extra */
    cert_chain_from_oat_length = (uint16_t) cert_chain_length * 2 + 10;
    cert_chain_from_oat = (uint8_t *) malloc(cert_chain_from_oat_length);

    if (cert_chain_from_oat == NULL) {
        log_printf("***ERR Malloc failed for cert_chain_from_oat\n");
        return (-1);
    }

    credentials_length = (uint16_t) cert_chain_length + MFG_NONCE_SIZE;
    credentials = (uint8_t *) malloc(credentials_length);

    if (credentials == NULL) {
        log_printf("***ERR Malloc failed for credentials\n");
        free(cert_chain_from_oat);
        return (-1);
    }

    memset(cert_chain_from_oat, '0', cert_chain_from_oat_length);
    memset(credentials, '0', credentials_length);

    status = tam_lib_mfg_login_init(tam_handle,
                                    (uint16_t) credentials_length,
                                    &credentials[MFG_NONCE_OFFSET]);
    if (status) {
        free(credentials);
        free(cert_chain_from_oat);
        return (status);
    }

    log_printf("\nA2L Mfg Login Init Status = 0x%02x\n", status);

    /*Receive the Manufacturing User Authentication Credentials (MUAC) from the Test Station*/
    GetOATData(CERT_CHAIN_MAX_COUNT,
               cert_chain_from_oat,
               cert_chain_from_oat_length,
               " Read (cert chain) > ");

    ConvertOATToHex(cert_chain_from_oat,cert_chain_from_oat_length);

    ptr_credentials = credentials + TOKEN_CERT_CHAIN_OFFSET;

    /* ZZZ_ARRAY */
    for (i = 0; i < cert_chain_length; i++) {
        ptr_credentials[i] = ((cert_chain_from_oat[2 * i] << 4) & 0xf0) |
                             ((cert_chain_from_oat[2 * i + 1]) & 0x0f);
    }

    /*Send the nonce to the Test Station*/
    log_printf("\ncredentials: \n");
    for (i = 0; i < credentials_length; i++)
    {
        log_printf("%02x", credentials[i]);
    }

    /*Send the nonce and the MUAC to the chip*/
    status = tam_lib_mfg_login_credentials(tam_handle, credentials_length,
                                            credentials);
    if (status) {
        free(credentials);
        free(cert_chain_from_oat);
        return (status);
    }

    log_printf("\nA2L Credential Status = 0x%02x\n", status);

    /* OBTAIN THE SIGNATURE AND ITS LENGTH HERE */

    /* Display the NONCE for OAT */
    log_printf("\nNonce Number is: ");

    for (i = 0; i < MFG_NONCE_SIZE; i++) {
    log_printf("%02x", credentials[i]);
    }
    log_printf("\n");

    /*Receive the signature of the Concatenated Authentication Credentials (nonce+MUAC) data from the Test Station*/
    do {
        log_printf("\n Read signature > ");

        signature_length = getline_act2(signature_from_oat,
                                        sizeof(signature_from_oat));
    } while (!signature_length);

    ConvertOATToHex(signature_from_oat,SIG_LENGTH * 2);

    /* ZZZ_ARRAY */
    memset(signature, '0', SIG_LENGTH);
    for (i = 0; i < (256); i++) {
        signature[i] = ((signature_from_oat[2*i] << 4) & 0xf0) |
            ((signature_from_oat[2*i + 1]) & 0x0f);
    }

    log_printf("\nSignature data is: ");
    /* ZZZ_ARRAY display */
    for (i = 0; i < (256); i++) {
        log_printf("%02x", signature[i]);
    }
    log_printf("\n");

    /*Verify the signature, create the manufacturing session ID*/
    status = tam_lib_mfg_login_signature(tam_handle,
                                         sizeof(signature),
                                         signature, session_id);

    if (status) {
        free(credentials);
        free(cert_chain_from_oat);
        return (status);
    }

    log_printf("\nA2L Signature Status = 0x%02x\n", status);

    free(credentials);
    free (cert_chain_from_oat);

    return (0);

} /* act2_diags_manufacturing_login */


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_diags_cliip_install
 *
 *  DESCRIPTION :
 *      act2_diags_cliip_install
 * Chip Level Identity Insertion Process
 * Cliip data is retreived from upstream by OAT and passed thru here
 * to the chip.
 *    Receive CLIIP and CLIIP length from OAT
 *    Pass Session ID from Mfg Login
 *  INPUT :
 *      None
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
static int act2_diags_cliip_install
(
    void *tam_handle,
    uint32_t session_id
)
{
    tam_lib_status_t status = TAM_RC_OK;
    uint8_t *cliip_from_oat = NULL;
    uint16_t cliip_from_oat_length = 0;
    uint8_t *cliip = NULL;
    uint16_t cliip_length = 0;
    uint16_t i,bytes=0;

    /*Receive CLIIP length from the Test Station*/
    do {
        cliip_length = getdec_answer("\n Enter CLIIP data length >",
                              0,1,16384);
    } while (!cliip_length);

    log_printf("\ncliip length: %d\n", cliip_length);

    /* Allocate a buffer large enough to hold the certificate chain
       represented in ASCII format (2 bytes per hex byte value) */
    /* plus a little extra */
    cliip_from_oat_length = cliip_length * 2 + 10;
    cliip_from_oat = (uint8_t *) malloc(cliip_from_oat_length);
    if (!cliip_from_oat) {
        log_printf("***ERR Malloc failed for cliip_from_oat\n");
        return (-1);
    }

    cliip = (uint8_t *) malloc(cliip_length);
    if (!cliip) {
        log_printf("***ERR Malloc failed for cliip\n");
        free(cliip_from_oat);
        return (-1);
    }

    /*Receive CLIIP from the Test Station*/
    bytes = GetOATData(CLIIP_MAX_COUNT,
                       cliip_from_oat,
                       cliip_from_oat_length,
                       " Read CLIIP data > ");

    if (bytes != (uint32_t)(cliip_length * 2)) {
        log_printf("\nthe cliip entered is %d bytes; it has to be %d bytes",
           bytes, cliip_length * 2);
        free(cliip);
        free(cliip_from_oat);
        return (-1);
    }

    ConvertOATToHex(cliip_from_oat,cliip_length * 2);

    /*ZZZ_ARRAY*/
    for (i = 0; i < (uint32_t)cliip_length; i++) {
        cliip[i] = ((cliip_from_oat[2 * i] << 4) & 0xf0) |
                   ((cliip_from_oat[2 * i + 1]) & 0x0f);
    }

    /* CLIIP can only be install once, return of 0 indicate
     * install successfully, 1 indicate cliip already installed */
    status = tam_lib_mfg_cliip_install(tam_handle, session_id,
            cliip_length, cliip);

    log_printf("\nA2L CLIIP Status = 0x%02x\n", status);

    free(cliip);
    free(cliip_from_oat);

    return (status);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_diags_sudi_request
 *
 *  DESCRIPTION :
 *      act2_diags_sudi_request
 *    Get Product SN, Name and PID from System
 *    Pass the Session ID from Mfg Login
 *    Receive CMS SUDI request and pass on to OAT
 *
 *  INPUT :
 *      None
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
static int act2_diags_sudi_request
(
    void *tam_handle,
    uint8_t *key_type,
    uint32_t session_id
)
{
    tam_lib_status_t status = TAM_RC_OK;
    uint32_t i;
    sudi_info_t board_info;
    uint8_t *sudi_request = NULL;
    uint16_t sudi_request_length = 0;
    act2_device_t *act2Dev;

    char pSsn[CHASSIS_SERIAL_NUM_SIZE+1]={0};
    char pPid[PID_SIZE+1]={0};
    char pPnm[PID_SIZE+1]={0};

    memset(pPid, 0x20, PID_SIZE);
    memset(pSsn, 0x20, CHASSIS_SERIAL_NUM_SIZE);
    memset(pPnm, 0x20, PID_SIZE);

    if (act2_get_pid_ssn(pPid, pSsn) != TAM_RC_OK) {
        log_printf("***ERR Unable to get PID/SSN\n");
        return (-1);
    }

    /* We need to pass the exact string without any extra character,
     * Max String Length can be 18, but orginal string is only 12byte
     * we need to terminate the string there
     */
    i = 0;
    while (pPid[i] != 0x20 && i < PID_SIZE) {
        i++;
    }
    pPid[i] = '\0';

    /* Copy PID to ProductName */
    memcpy(pPnm,pPid,PID_SIZE);
    pPnm[i] = '\0';

    /*Initialize the sudi_info_t structure */
    board_info.product_name_ptr = &pPnm[0];
    board_info.serial_number_ptr = &pSsn[0];
    board_info.pid_ptr = &pPid[0];
    board_info.product_name_length = strlen(pPnm);
    board_info.serial_number_length = strlen(pSsn);
    board_info.pid_length = strlen(pPid);
    board_info.sudi_key_type_ptr = key_type;
    board_info.sudi_key_type_length = strlen(key_type);

    /* TODO - Will remove after the authentication is verified in IOS */
    log_printf("\n%s: sudi_info_t structure\n", key_type);
    log_printf("Serial Number : %s\n", board_info.serial_number_ptr);
    log_printf("Serial Number Length : %d\n", board_info.serial_number_length);
    log_printf("Product Name : %s\n", board_info.product_name_ptr);
    log_printf("Product Name Length : %d\n", board_info.product_name_length);
    log_printf("PID : %s\n", board_info.pid_ptr);
    log_printf("PID Length : %d\n", board_info.pid_length);
    log_printf("Sudi Key Type : %s\n", board_info.sudi_key_type_ptr);
    log_printf("Sudi Key Type Length : %d\n", board_info.sudi_key_type_length);

    /*Generate the SUDI Request*/
    status = tam_lib_mfg_create_sudi_request(tam_handle, session_id,
                                             &board_info, &sudi_request,
                                             &sudi_request_length);

    log_printf("\nA2L SUDI Request Status = 0x%02x\n", status);

    if (status) {
        return (int) status;
    }

    if (!sudi_request) {
        log_printf("\n***ERR SUDI request is NULL\n");
    }

    if (sudi_request) {
        log_printf("\nSUDI Request: ");
        for (i = 0; i < sudi_request_length; i++) {
            log_printf("%02x", sudi_request[i]);
        }
        log_printf("\n");
    }

    log_printf("\nIDevID Req Length: %d", sudi_request_length);

    if (sudi_request) {
        free(sudi_request);
    }

    return (int) status;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_diags_sudi_install
 *
 *  DESCRIPTION :
 *      act2_diags_sudi_install
 *
 *    Receive SUDI Leaf Cert, SUDI Cert chain (CA + ROOT)
 *    Pass the Session Id from Mfg Login
 *
 *  INPUT :
 *      None
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
static int act2_diags_sudi_install
(
    void *tam_handle,
    uint8_t *key_type,
    uint32_t session_id
)
{
    tam_lib_status_t status = TAM_RC_OK;
    uint16_t i, bytes=0, sudi_max_counter = SUDI_MAX_COUNT;

    uint8_t  *sudi_cert_from_oat = NULL;
    uint16_t sudi_cert_length = 0;
    uint16_t sudi_cert_from_oat_length = 0;

    uint8_t  *sudi_cert_chain_from_oat = NULL;
    uint16_t sudi_cert_chain_length = 0; /* Cert Chain = SubCA+Root */
    uint16_t sudi_cert_chain_from_oat_length = 0;
    uint16_t cert = 0, cert_chain = 0;

    /* define cert and cert_chain */
    if( !strncmp("RSA", key_type, 3) ) {
        cert = TAM_LIB_IDEVID_RSA_CERT;
        cert_chain = TAM_LIB_IDEVID_RSA_CERT_CHAIN;
    } else if( !strncmp("ECC", key_type, 3) ) {
        cert = TAM_LIB_IDEVID_ECC_CERT;
        cert_chain = TAM_LIB_IDEVID_ECC_CERT_CHAIN;
    } else {
        log_printf("Unsupport mode.\n");
        return (-1);
    }

    /* Acquire SUDI Certificate from Autotest and convert to binary */
    do {
     sudi_cert_length = getdec_answer("\n Enter SUDI Cert length >",
                                      0,1,16384);
    } while (!sudi_cert_length);

    log_printf("\nSUDI Cert length: %d\n", sudi_cert_length);

    sudi_cert_from_oat_length = sudi_cert_length * 2 + 10;
    sudi_cert_from_oat = (uint8_t *) malloc(sudi_cert_from_oat_length);
    if (!sudi_cert_from_oat) {
        log_printf("***ERR Malloc failed for sudi_cert_from_oat\n");
        return (-1);
    }

    bytes = GetOATData(SUDI_MAX_COUNT,
                       sudi_cert_from_oat,
                       sudi_cert_from_oat_length,
                       " Read SUDI Cert data > ");

    if (bytes != (uint32_t)(sudi_cert_length * 2)) {
        log_printf("\nthe SUDI Cert entered is %d bytes; it has to be %d bytes",
           bytes, sudi_cert_length * 2);
        free(sudi_cert_from_oat);
        return (-1);
    }

    ConvertOATToHex(sudi_cert_from_oat,sudi_cert_length * 2);

    sudi_cert = (uint8_t *) malloc(sudi_cert_length);
    if (!sudi_cert) {
        log_printf("***ERR Malloc failed for sudi_cert\n");
        free(sudi_cert_from_oat);
        return (-1);
    }

    /*ZZZ_ARRAY*/
    for (i = 0; i < (uint32_t)sudi_cert_length; i++) {
        sudi_cert[i] = ((sudi_cert_from_oat[2 * i] << 4) & 0xf0) |
                       ((sudi_cert_from_oat[2 * i + 1]) & 0x0f);
    }

    /* Done with this one */
    free(sudi_cert_from_oat);

    /* Acquire SUDI Certificate Chain from Autotest and convert to binary */
    do {
     sudi_cert_chain_length = getdec_answer("\n Enter SUDI Cert Chain length >",
                                     0,1,16384);
    } while (!sudi_cert_chain_length);

    log_printf("\nSUDI Cert Chain length: %d\n", sudi_cert_chain_length);

    sudi_cert_chain_from_oat_length = sudi_cert_chain_length * 2 + 10;
    sudi_cert_chain_from_oat = (uint8_t *)
                               malloc(sudi_cert_chain_from_oat_length);
    if (!sudi_cert_chain_from_oat) {
        log_printf("***ERR Malloc failed for sudi_cert_chain_from_oat\n");
        free(sudi_cert);
        return (-1);
    }

    /*ECC mode should receive for twice */
    if( !strncmp("ECC", key_type, 3) )
        sudi_max_counter = SUDI_MAX_COUNT+1;

    bytes = GetOATData(sudi_max_counter,
                       sudi_cert_chain_from_oat,
                       sudi_cert_chain_from_oat_length,
                       " Read SUDI Cert Chain data > ");

    if (bytes != (uint32_t)(sudi_cert_chain_length * 2)) {
        log_printf("\nthe SUDI Cert Chain entered is %d bytes; it has to be %d bytes",
           bytes, sudi_cert_chain_length * 2);
        free(sudi_cert);
        free(sudi_cert_chain_from_oat);
        return (-1);
    }

    ConvertOATToHex(sudi_cert_chain_from_oat,sudi_cert_chain_length * 2);

    sudi_cert_chain = (uint8_t *) malloc(sudi_cert_chain_length);
    if (!sudi_cert_chain) {
        log_printf("***ERR Malloc failed for sudi_cert_chain\n");
        free(sudi_cert);
        free(sudi_cert_chain_from_oat);
        return ( -2 );
    }

    /*ZZZ_ARRAY*/
    for (i = 0; i < sudi_cert_chain_length; i++) {
        sudi_cert_chain[i] = ((sudi_cert_chain_from_oat[2 * i] << 4) & 0xf0) |
                             ((sudi_cert_chain_from_oat[2 * i + 1]) & 0x0f);
    }

    /* Done with this one */
    free(sudi_cert_chain_from_oat);

    /* Write the RSA/ECC SUDI Cert */
    status = tam_lib_object_write(tam_handle, session_id,
                 cert,
                 sudi_cert, sudi_cert_length);

    if (TAM_RC_OK != status) {
        REPORT_TAM_ERROR(status);
        free(sudi_cert);
        free(sudi_cert_chain);
        return (status);
    }

    /* Write the RSA/ECC SUDI Cert Chain */
    status = tam_lib_object_write(tam_handle, session_id,
                 cert_chain,
                 sudi_cert_chain, sudi_cert_chain_length);

    if (TAM_RC_OK != status) {
        REPORT_TAM_ERROR(status);
        free(sudi_cert);
        free(sudi_cert_chain);
        return (status);
    }

    log_printf("\nA2L SUDI Status = 0x%02x\n", status);

    return ((int)status);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_diags_signature
 *
 *  DESCRIPTION :
 *      act2_diags_signature
 *
 *  INPUT :
 *      None
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
int act2_diags_signature
(
    void *act2_device
)
{
    uint8_t csn[CHIP_SERIAL_NUMBER_LENGTH]={0};
    int i;
    uint32_t session_id=0;
    tam_lib_status_t result=TAM_RC_OK;
    tam_library_version_t lib_version;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;

    memset (&csn[0],0,CHIP_SERIAL_NUMBER_LENGTH);

    result = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != result) {
        log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                result,tam_lib_rc2string(result));
        return (result);
    }

    result = act2_set_simple_mode(tam_handle);
    if (result != TAM_RC_OK) {
        log_printf("\nERR: Unable to set ACT2 chip to simple mode.\n");
        act2_local_close_device(&tam_handle);
        return (result);
    }

    result = tam_lib_get_library_version(&lib_version);
    if (result) {
        log_printf("\n***ERR Unable to get lib version\n");
    } else {
        log_printf("Using ACT2 Library Version: %d.%d-%d\n",
               lib_version.major,lib_version.minor,
               lib_version.patch);
    }

    /* get the ACT2 device serial number */
    result = tam_lib_get_chip_serial_number(tam_handle, csn);
    if (result) {
        log_printf("\n***ERR Unable to get ACT2 CSN\n");
    } else {
        log_printf("CSN = ");
        for (i = 0; i < CHIP_SERIAL_NUMBER_LENGTH; i++) {
            log_printf("%02X", csn[i]);
        }
        log_printf("\n\n");
    }

    /* manufacture login */
    result = act2_diags_manufacturing_login(tam_handle, &session_id);
    if (result) {
        log_printf("\n***ERR Manufacturing Login FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\nManufacturing login succeeded.\n");
        log_printf("\nManufacturing Session ID: 0x%08X\n", session_id);
    }

    /*cliip install*/
    result = act2_diags_cliip_install(tam_handle, session_id);
    if (result > 0x1) {
        log_printf("\n***ERR CLIIP Install FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\nCLIIP Install Succeeded!! (%s installed)\n", (result == 0x1)? "Already" : "First");
    }

    /* RSA sudi request */
    result = act2_diags_sudi_request(tam_handle, "RSA", session_id);
    if (result) {
        log_printf("\n***RSA: ERR SUDI Request FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\nRSA: SUDI Request Succeeded!!\n");
    }

    /* RSA sudi install */
    result = act2_diags_sudi_install(tam_handle, "RSA", session_id);
    if (result) {
        log_printf("\n***RSA: ERR SUDI Install FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\n> RSA: SUDI Install Succeeded\n\n");
    }

    log_printf("\nRSA: SUDI Verify...\n");
    result = read_and_cmp_sudi_cert(tam_handle, "RSA", session_id);
    if (result) {
        log_printf("\n***ERR SUDI Verify FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        free(sudi_cert);
        free(sudi_cert_chain);
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\n> RSA: SUDI Verified Succeeded.\n");
    }

    /*Make point to NULL after free */
    free(sudi_cert);
    free(sudi_cert_chain);
    sudi_cert = NULL;
    sudi_cert_chain = NULL;

    /* ECC sudi request */
    result = act2_diags_sudi_request(tam_handle, "ECC", session_id);
    if (result) {
        log_printf("\n***ECC: ERR SUDI Request FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\nECC: SUDI Request Succeeded!!\n");
    }

    /* ECC sudi install */
    result = act2_diags_sudi_install(tam_handle, "ECC", session_id);
    if (result) {
        log_printf("\n***ECC: ERR SUDI Install FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\n> ECC: SUDI Install Succeeded\n\n");
    }

    log_printf("\nECC: SUDI Verify...\n");
    result = read_and_cmp_sudi_cert(tam_handle, "ECC", session_id);
    if (result) {
        log_printf("\n***ERR SUDI Verify FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        free(sudi_cert);
        free(sudi_cert_chain);
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\n> ECC: SUDI Verified Succeeded.\n");
    }

    result = tam_lib_session_end(tam_handle, session_id);
    if (result) {
        log_printf("\n***ERR Session End FAILED!!\n");
        tam_lib_mfg_errinfo_print();
        free(sudi_cert);
        free(sudi_cert_chain);
        act2_local_close_device(&tam_handle);
        return (result);
    } else {
        log_printf("\n> ACT2 Session Ended for ID 0x%08X\n",session_id);
    }

    act2_local_close_device(&tam_handle);

    free(sudi_cert);
    free(sudi_cert_chain);

    return (result);
}

/*--------------------------------------------------------------------------
* NAME
*    tam_lib_platform_spi_write
*
* SYNOPSIS
*    #include "tam_library.h"
*    int32_t
*    tam_lib_platform_spi_write(void *platform_opaque_handle,
*                               uint32_t bytes_to_send,
*                               uint8_t *send_buffer)
*
* DESCRIPTION
*    Platform dependent SPI driver wrapper.
*
*
* INPUT PARAMETERS
*    platform_opaque_handle - SPI driver handle
*    bytes_to_send - SPI write length
*    send_buffer - SPI write data
*
* OUTPUT PARAMETERS
*    none
*
* RETURN VALUE
*    Number of bytes write through slave
*
* NOTES
*
*--------------------------------------------------------------------------
*/
int tam_lib_platform_spi_write(void *platform_opaque_handle,
                               uint16_t bytes_to_send,
                               uint8_t *send_buffer)
{
    int ret,i;
    uint8_t handle = 0;
    MV_U8 rxbuff[8];

    if (gDebug) {
        printf("\nSPI Write:");
        for (i = 0; i < bytes_to_send; i++) {
            printf("%02x ", send_buffer[i]);
        }
        printf("\n");
    }

    /* First assert the chip select */
    mvSpiCsAssert(handle);


    ret = mvSpiWrite(handle, send_buffer, bytes_to_send);

    if (ret != MV_OK) {
        printf("ACT2 SPI write failed: status (%d)\n", ret);
        return 0;
    }

    udelay(1);

    /* Finally deassert the chip select */
    mvSpiCsDeassert(handle);

    return (int)bytes_to_send;
}

/*--------------------------------------------------------------------------
* NAME
*    tam_lib_platform_spi_read
*
* SYNOPSIS
*    #include "tam_library.h"
*    int32_t
*    tam_lib_platform_spi_read(void *platform_opaque_handle,
*                              uint32_t bytes_to_send,
*                              uint8_t *send_buffer,
*                              uint16_t bytes_to_read,
*                              uint8_t *read_buffer,
*                              uint16_t *bytes_actually_read)
*
* DESCRIPTION
*    Platform dependent SPI driver wrapper.
*
*
* INPUT PARAMETERS
*    platform_opaque_handle - SPI driver handle
*    bytes_to_send - SPI write length
*    send_buffer - SPI write data
*    bytes_to_read - SPI read response length
*
* OUTPUT PARAMETERS
*    read_buffer - SPI read response data
*    bytes_actually_read - actual read response length
*
* RETURN VALUE
*    TAM_RC_OK
*    Error, otherwise
*
* NOTES
*
*--------------------------------------------------------------------------
*/
tam_lib_status_t tam_lib_platform_spi_read(void *platform_opaque_handle,
                                           uint16_t bytes_to_send,
                                           uint8_t *send_buffer,
                                           uint16_t bytes_to_read,
                                           uint8_t *read_buffer,
                                           uint16_t *bytes_actually_read)
{
    int count, i;
    uint32_t handle = 0;

    if (bytes_to_read != bytes_to_send) {
        printf("%s bytes_to_send(%d) != bytes_to_read(%d)\n",
            __FUNCTION__, bytes_to_send, bytes_to_read);
    }
    if (mvSpiReadAndWrite(0, read_buffer, send_buffer, bytes_to_read) != MV_OK) {
        printf("%s ACT2 SPI write failed\n", __FUNCTION__);
        return TAM_LIB_ERR_READ_FAILURE;
    }

    /* if return MV_OK, bytes_actually_read is equal to bytes_to_read */
    *bytes_actually_read = bytes_to_read;

    if (gDebug) {
        printf("\nSPI Read:");
        for (i = 0; i < bytes_to_read; i++) {
            printf("%02x ", read_buffer[i]);
        }
        printf("\n");
    }
    
    return TAM_RC_OK;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      tam_lib_platform_write
 *
 *  DESCRIPTION :
 *      tam_lib_platform_write
 *
 *  INPUT :
 *      None
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
int tam_lib_platform_write
(
    IN void   *opaque_act2_handle,
    IN uint8_t  *send_buffer,
    IN uint32_t length
)
{
    uint8_t data_buf[10]={0};
    uint8_t data_string[DATA_BUF_LEN]={0};
    uint32_t data_sent = 0;
    uint16_t port;
    uint16_t addr;
    uint16_t write_times;

    /* validate ptr before dereference below */
    if (opaque_act2_handle == NULL)
    {
        return (-1);
    }

    if (send_buffer == NULL)
    {
        return (-1);
    }

    if (length < 1)
    {
        return (-1);
    }

    addr = ((act2_device_t *)opaque_act2_handle)->addr;
    ((act2_device_t *)opaque_act2_handle)->delay_flag = 0;

    data_sent = I2Cgpio_write(ACT2_I2C_ADDR, send_buffer, length);

    memset(data_string, 0x0, DATA_BUF_LEN);
    for(write_times = 0; write_times < length ; write_times++) {
        sprintf(data_buf, "%02X \0", send_buffer[write_times]);
        strcat(data_string, data_buf);
    }
    strcat(data_string, "\0");

    log_dbgPrintf("I2C_WRITE: i2caddr=0x%02X %d bytes, [ data = %s]\n",  addr, length, data_string);


    return (data_sent);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      tam_lib_platform_read
 *
 *  DESCRIPTION :
 *      tam_lib_platform_read
 *
 *  INPUT :
 *      None
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
tam_lib_status_t tam_lib_platform_read
(
    IN void   *opaque_act2_handle,
    IN uint32_t  min_time,
    IN uint32_t  max_time,
    IN uint8_t  *read_buffer,
    IN uint16_t bytes_to_read,
    OUT uint16_t  *bytes_actually_read
)
{
    uint8_t data_buf[10]={0};
    uint8_t data_string[DATA_BUF_LEN]={0};
    uint32_t data_read = 0;
    uint32_t port = 0;
    uint16_t addr, read_times;
    uint32_t retries = 0;

    /* validate ptr before dereference below */
    if (opaque_act2_handle == NULL) {
        return (-1);
    }

    if (read_buffer == NULL) {
        return (-1);
    }

    if (bytes_to_read < 1) {
        return (-1);
    }

    addr = ((act2_device_t *)opaque_act2_handle)->addr;
    if (!((act2_device_t *)opaque_act2_handle)->delay_flag) {
        udelay(delaytime);
        ((act2_device_t *)opaque_act2_handle)->delay_flag = 1;
    }

    do {
        /*set read_buffer data to 0xfff*/
        memset(read_buffer, 0xFF, bytes_to_read);

        /*
         * platform read routine
         */
        data_read = I2Cgpio_read(ACT2_I2C_ADDR, read_buffer, bytes_to_read);
        /* if data_read == 0, the data ACt2 is not ready, retry again */
        if (data_read == 0) {
            udelay(100000);
            retries++;
        }
    } while ((data_read != bytes_to_read) && (retries < ACT2_I2C_READ_RETRIES));

    memset(data_string, 0x0, DATA_BUF_LEN);
    for(read_times=0 ; read_times < bytes_to_read ; read_times++) {
        sprintf(data_buf, "%02X \0", read_buffer[read_times]);
        strcat(data_string, data_buf);
    }
    strcat(data_string, "\0");

    *bytes_actually_read  = bytes_to_read;
    log_dbgPrintf("I2C_READ : i2caddr=0x%02X %d bytes, retries = %d, [ data = %s]\n", addr, bytes_to_read, retries, data_string);

    return (data_read);
}

/*************************TEST INTERFACE FUNCTIONS***************************/
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_diags_configure_and_identify_chip
 *
 *  DESCRIPTION :
 *      act2_diags_configure_and_identify_chip
 *
 *  INPUT :
 *      None
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
int act2_diags_configure_and_identify_chip
(
    IN act2_device_t *act2_device
)
{
    tam_lib_status_t status;
    uint8_t chip_serial_number[CHIP_SERIAL_NUMBER_LENGTH];
    tam_lib_status_t result;
    tam_lib_scc_id_t   scc_id;
    tam_lib_chip_info_t chip_info;
    tam_library_version_t  act2_lib_version;
    int i;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;

    result = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != result) {
       log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                  result,tam_lib_rc2string(result));
       return (result);
    }

    /* Get the SCC_FW_ID; the value controls expected results. */
    log_printf("[Reading SCC ID]:\n");
    if((result = tam_lib_scc_read_id(tam_handle, &scc_id)) != TAM_RC_OK) {
        log_printf("ERROR: tam_lib_scc_read_id - code 0x%X\n", result);
        return result;
    }
    act2_device->firmware_version = scc_id.firmware_version;
    log_printf("SCC ID Frimware Version : 0x%02x \n", scc_id.firmware_version);

    log_printf("\n[Configuring Simple Mode]:\n");
    if ((result = tam_lib_set_simple(tam_handle)) != TAM_RC_OK) {
       REPORT_TAM_ERROR(status);
    }

    if ((result = tam_lib_get_library_version(&act2_lib_version)) != TAM_RC_OK ) {
       REPORT_TAM_ERROR(status);
    }

    log_printf("Using ACT2 Library Version: %d.%d-%d\n", act2_lib_version.major, act2_lib_version.minor, act2_lib_version.patch);

    log_printf("\n[Getting Chip Info]:\n");
    if ((result = tam_lib_get_chip_info(tam_handle, &chip_info)) != TAM_RC_OK)
        REPORT_TAM_ERROR(result);
    act2_device->is_so64 = (chip_info.fw_version[1] | chip_info.fw_version[2]) ? TRUE : FALSE;
    log_printf("Firmware Version : %02X.%02X.%04X \n", chip_info.fw_version[0] & 0x0F, chip_info.fw_version[0] >> 4,
           (uint16_t) (((uint16_t)(chip_info.fw_version[1] << 8)) | (chip_info.fw_version[2])));

    log_printf("\n[Sending CSN pattern]:\n");
    if  ((result = tam_lib_get_chip_serial_number(tam_handle, chip_serial_number)) != TAM_RC_OK)
        REPORT_TAM_ERROR(result);
    log_printf("\nCSN = ");
    for(i=0;i<CHIP_SERIAL_NUMBER_LENGTH;i++)
        log_printf("%02X", chip_serial_number[i]);
    log_printf("\n\n");

    result = act2_local_close_device(&tam_handle);
    if(result)
        REPORT_TAM_ERROR(result);

    return result;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_authenticate
 *
 *  DESCRIPTION :
 *      act2_authenticate
 *
 *  INPUT :
 *      None
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
int act2_authenticate(act2_device_t *act2_device)
{
    int  ret = E_TYPE_SUCCESS;
    tam_lib_status_t result=TAM_RC_OK;
    void *tam_handle = NULL;
    uint8_t pSsn[CHASSIS_SERIAL_NUM_SIZE+1]={0};
    uint8_t pPid[PID_SIZE+1]={0};
    void *platform_opaque_handle = NULL;

    result = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != result) {
        log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                   result,tam_lib_rc2string(result));
        return (result);
    }

    log_printf("\n[Configuring Simple Mode]:");
    ret = tam_lib_check_mode(tam_handle);
    if (ret != BUS_MODE_SIMPLE)  {
        if ((ret = tam_lib_set_simple(tam_handle)) != TAM_RC_OK) {
            log_printf("ERROR: Setting Simple Mode - code 0x%X\n", ret);
            act2_local_close_device(&tam_handle);
            return ret;
        }
    }
    log_printf("  Success\n");

    log_printf("\n[Authenticating]:");
    ret = tam_lib_authentication(tam_handle);
    if (ret != TAM_RC_OK) {
        REPORT_TAM_ERROR(ret);
        act2_local_close_device(&tam_handle);
        return ret;
    }
    log_printf("  Success\n");

    log_printf("\n");
    /* Get the board PID and board SN*/
    if (act2_get_pid_ssn(pPid, pSsn) != TAM_RC_OK) {
        log_printf("***ERR Unable to get PID/SSN\n");
        return (-1);
    }

    log_printf("\n[PID/SN RSA_SUDI Authentication]:");
    /* PID/SN authentication with RSA_SUDI*/
    ret = tam_lib_authentication_udi(tam_handle, pPid, pSsn, RSA_SUDI);
    if (ret != TAM_RC_OK) {
        REPORT_TAM_ERROR(ret);
        act2_local_close_device(&tam_handle);
        return ret;
    }
    log_printf("  Success\n");

    log_printf("[PID/SN ECC_SUDI Authentication]:");
    /* PID/SN authentication with ECC_SUDI*/
    ret = tam_lib_authentication_udi(tam_handle, pPid, pSsn, ECC_SUDI);
    if (ret != TAM_RC_OK) {
        REPORT_TAM_ERROR(ret);
        act2_local_close_device(&tam_handle);
        return ret;
    }
    log_printf("  Success\n");

    act2_local_close_device(&tam_handle);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      act2_generateEcskmp
 *
 *  DESCRIPTION :
 *      act2_generateEcskmp
 *
 *  INPUT :
 *      None
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
INT32 act2_generateEcskmp(act2_device_t *act2_device)
{
    tam_lib_status_t status=TAM_RC_OK;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;
    FILE *fp;
    uint16_t ecskmp_len=0;
    uint8_t repeat=0, ecskmp_buf[ACT2_ECSKMP_SIZE] = {0};

    /* Open the act2 device */
    status = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : act2_device), &tam_handle);
    if (TAM_RC_OK != status) {
        log_printf("\n Error: ACT2 Device Open Status = 0x%0x-%s\n",
                status,tam_lib_rc2string(status));
        return (status);
    }

    /* generate ecskmp */
    log_printf("Generate the ecskmp...\n");
    status = tam_lib_mfg_ecskmp_generate(tam_handle, &repeat);
    if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR: ecskmp generate, status=0x%0x-%s\n",
                __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        return (status);
    }

    /*read the esckmp buffer */
    log_printf("Read the ecskmp buffer...\n");
    status = tam_lib_mfg_ecskmp_read(tam_handle, ecskmp_buf, &ecskmp_len);
    if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR: ecskmp read, status=0x%0x-%s\n",
                __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        return (status);
    }

    /* Close the act device */
    status = act2_local_close_device(&tam_handle);
    if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR: act2 close, status=0x%0x-%s\n",
                __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        return (status);
    }

    log_dbgPrintf("ecskmp_len = %d\n", ecskmp_len);
    log_dbgPrintf("\nAlready Generated=%u \n", repeat);

    /* write buffer data to ecskmp.bin file */
    log_printf("Generate the ecskmp bin file.\n");
    fp = fopen("/usr/bin/ecskmp.bin", "wb");

    fseek(fp, 0, SEEK_SET);

    fwrite(ecskmp_buf, sizeof(uint8_t), ecskmp_len, fp);

    fclose(fp);

    log_printf("Generator the /usr/bin/ecskmp.bin completely.\n");

    return status;
}


/*******************************************************************************
* mvSpiCsAssert - Assert the Chip Select pin indicating a new transfer
*
* DESCRIPTION:
*       Assert The chip select - used to select an external SPI device
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
********************************************************************************/
void mvSpiCsAssert(uint8_t spiId)
{
    udelay(1);
    MV_REG_BIT_SET(MV_SPI_IF_CTRL_REG(spiId), MV_SPI_CS_ENABLE_MASK);
}

/*******************************************************************************
* mvSpiCsDeassert - DeAssert the Chip Select pin indicating the end of a
*         SPI transfer sequence
*
* DESCRIPTION:
*       DeAssert the chip select pin
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
********************************************************************************/
void mvSpiCsDeassert(uint8_t spiId)
{
    MV_REG_BIT_RESET(MV_SPI_IF_CTRL_REG(spiId), MV_SPI_CS_ENABLE_MASK);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaAct2Test
 *
 *  DESCRIPTION :
 *      FPGA ACT2 test
 *
 *  INPUT :
 *      None
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
INT32 fpgaAct2Test
(
    IN ACT2_TEST_TYPE act2Type,
    IN uint32_t debug
)
{
    uint32_t idx=0;
    int  ret = E_TYPE_SUCCESS;
    act2_device_t act2_device;

    /* init the act2_device struct */
    memset((void *) &act2_device, 0x00, sizeof(act2_device));
    act2_device.addr = ACT2_I2C_ADDR;
    act2_device.delay_flag = 0;

    gDebug = debug;

    /* Get the ACT2 chip information */
    if ( act2Type == ACT2_SPITEST) {
        act2spi_cmd();
        return E_TYPE_SUCCESS;
    }
    
    if ((act2Type == ACT2_PROBE) || (act2Type == ACT2_PROBE_I2C)) {
        ret = act2_diags_configure_and_identify_chip(&act2_device);
        if (ret != TAM_RC_OK) {
            log_printf("Failed to get ACT2 chip information.\n");
        } else {
            log_printf("Succeed to get ACT2 chip information.\n");
        }
        return ret;
    }

    /*Generte the ecskmp.bin file */
    if ((act2Type == ACT2_ECSKMP) || (act2Type == ACT2_ECSKMP_I2C) ) {
        /* ecskmp bin file need 2 seconds delay */
        delaytime=2000000;
        ret = act2_generateEcskmp(&act2_device);
        delaytime=10000;
        return ret;
    }

    /* Check authentication after mfg complete CLIIP, SUDI */
    if ((act2Type == ACT2_ATHEN) || (act2Type == ACT2_ATHEN_I2C) ) {
        ret = act2_authenticate(&act2_device);
        if (ret != TAM_RC_OK) {
            log_printf("Failed to authenticate ACT2.\n");
        } else {
            log_printf("Succeed to authenticate ACT2.\n");
        }
        return ret;
    }

    /* signature the act2 chip */
    ret = act2_diags_signature(&act2_device);
    if ( ret != TAM_RC_OK) {
        log_printf("Failed to signature ACT2.\n");
    } else {
        log_printf("Succeed to signature ACT2.\n");
    }

    return ret;
}


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaAikidoTest
 *
 *  DESCRIPTION :
 *      FPGA Aikido Test via SPI interface
 *
 *  INPUT :
 *      None
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
INT32 fpgaAikidoTest
(
    IN ACT2_TEST_TYPE act2Type,
    IN uint32_t debug,
    IN uint32_t gpio5,
    IN uint32_t spireg,
    IN uint32_t wdata
        
)
{
    int  ret = E_TYPE_SUCCESS;

    gDebug = debug;

    /* Get the SPI information */
    if ( act2Type == ACT2_SPIGPIO5) {
         if(gpio5==0) {
             /* Configure the MPP5 to zero */
             MV_REG_BIT_RESET(FPGA_CS_L, (0x1 << 5));
             printf("FPGA Aikido GPIO5=0\n");
         }else{
            MV_REG_BIT_SET(FPGA_CS_L, (0x1 << 5));
             printf("FPGA Aikido GPIO5=1\n");
         }            
        return E_TYPE_SUCCESS;
    } 

    /* Get the ACT2 chip information */
    if ( act2Type == ACT2_SPIREAD) {
        fpgaSpiReadCmd(spireg);
        return E_TYPE_SUCCESS;
    }

        /* Get the ACT2 chip information */
    if ( act2Type == ACT2_SPIWRITE) {
        fpgaSpiWriteCmd(spireg, wdata);
        return E_TYPE_SUCCESS;
    }
    
    return ret;
}

/*******************************************************************************
* mvSpi16bitDataTxRx - Transmt and receive data
*
* DESCRIPTION:
*       Tx data and block waiting for data to be transmitted
*
********************************************************************************/
INT32 mvSpi16bitDataTxRx(uint8_t spiId, uint16_t txData, uint16_t *pRxData)
{
    uint32_t i;
    MV_BOOL ready = MV_FALSE;

    /* First clear the bit in the interrupt cause register */
    MV_REG_WRITE(MV_SPI_INT_CAUSE_REG(spiId), 0x0);

    /* Transmit data */
    MV_REG_WRITE(MV_SPI_DATA_OUT_REG(spiId), MV_16BIT_LE(txData));

    /* wait with timeout for memory ready */
    for (i = 0; i < MV_SPI_WAIT_RDY_MAX_LOOP; i++) {
        if (MV_REG_READ(MV_SPI_INT_CAUSE_REG(spiId))) {
            ready = MV_TRUE;
            break;
        }
        udelay(1);
    }

    if (!ready)
        return MV_TIMEOUT;

    /* check that the RX data is needed */
    if (pRxData) {
        if ((uint32_t)pRxData &  0x1) { /* check if address is not alligned to 16bit */
            /* perform the data write to the buffer in two stages with 8bit each */
            uint8_t *bptr = (uint8_t *)pRxData;
            uint16_t data = MV_16BIT_LE(MV_REG_READ(MV_SPI_DATA_IN_REG(spiId)));
            *bptr = (data & 0xFF);
            ++bptr;
            *bptr = ((data >> 8) & 0xFF);
        } else
            *pRxData = MV_16BIT_LE(MV_REG_READ(MV_SPI_DATA_IN_REG(spiId)));
    }

    return MV_OK;
}


/*******************************************************************************
* mvSpi8bitDataTxRx - Transmt and receive data (8bits)
*
* DESCRIPTION:
*       Tx data and block waiting for data to be transmitted
*
********************************************************************************/
INT32 mvSpi8bitDataTxRx(uint8_t spiId, uint8_t txData, uint8_t *pRxData)
{
    MV_U32 i;
    MV_BOOL ready = MV_FALSE;

    if (currSpiInfo->byteCsAsrt)
        mvSpiCsAssert(spiId);

    /* First clear the bit in the interrupt cause register */
    MV_SPI_REG_WRITE(MV_SPI_INT_CAUSE_REG(spiId), 0x0);

    /* Transmit data */
    MV_SPI_REG_WRITE(MV_SPI_DATA_OUT_REG(spiId), txData);
    if (gDebug)
        log_printf("0x%02X ", txData);
    else
        for(i=0;i<5000;i++) asm("nop");    /* Added delay */

    /* wait with timeout for memory ready */
    for (i = 0; i < MV_SPI_WAIT_RDY_MAX_LOOP; i++) {
        if (MV_SPI_REG_READ(MV_SPI_INT_CAUSE_REG(spiId))) {
            ready = MV_TRUE;
            break;
        }
        udelay(1);
    }

    if (!ready) {
        if (currSpiInfo->byteCsAsrt) {
            mvSpiCsDeassert(spiId);
            udelay(4);
        }
        return MV_TIMEOUT;
    }

    /* check that the RX data is needed */
    if (pRxData) {
        *pRxData = MV_SPI_REG_READ(MV_SPI_DATA_IN_REG(spiId));
        if (gDebug)
            log_printf("0x%02X ", *pRxData);
    }

    if (currSpiInfo->byteCsAsrt) {
        mvSpiCsDeassert(spiId);
        udelay(4);
    }

    return MV_OK;
}

/*******************************************************************************
* mvSpiRead - Read a buffer over the SPI interface
*
* DESCRIPTION:
*       Receive (read) a buffer over the SPI interface in 16bit chunks. If the
*   buffer size is odd, then the last chunk will be 8bits. Chip select is not
*       handled at this level.
*
* INPUT:
*   pRxBuff: Pointer to the buffer to hold the received data
*   buffSize: length of the pRxBuff
*
* OUTPUT:
*   pRxBuff: Pointer to the buffer with the received data
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
INT32 mvSpiRead(uint8_t spiId, uint8_t *pRxBuff, uint32_t buffSize)
{
    INT32 ret;
    uint32_t bytesLeft = buffSize;
    uint16_t *rxPtr = (uint16_t *)pRxBuff;

    /* check for null parameters */
    if (pRxBuff == NULL) {
        log_printf("%s ERROR: Null pointer parameter!\n", __func__);
        return MV_BAD_PARAM;
    }

    /* Check that the buffer pointer and the buffer size are 16bit aligned */
    if ((currSpiInfo->en16Bit) && (((uint32_t)buffSize & 1) == 0) && (((uint32_t)pRxBuff & 1) == 0)) {
        /* Verify that the SPI mode is in 16bit mode */
        MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

        /* TX/RX as long we have complete 16bit chunks */
        while (bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE) {
            /* Transmitted and wait for the transfer to be completed */
            ret = mvSpi16bitDataTxRx(spiId, MV_SPI_DUMMY_WRITE_16BITS, rxPtr);
            if (ret != MV_OK)
                return ret;

            /* increment the pointers */
            rxPtr++;
            bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
        }
    } else {
        /* Verify that the SPI mode is in 8bit mode */
        MV_REG_BIT_RESET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

        /* TX/RX in 8bit chanks */
        if (gDebug)
            log_printf("\nRead Data : ");
        
        while (bytesLeft > 0) {
            /* Transmitted and wait for the transfer to be completed */
            ret = mvSpi8bitDataTxRx(spiId, MV_SPI_DUMMY_WRITE_8BITS, pRxBuff);
            if (ret != MV_OK)
                return ret;
            /* increment the pointers */
            pRxBuff++;
            bytesLeft--;
        }
        if (gDebug)
            log_printf("\n");
    }

  return (buffSize - bytesLeft);
}

/*******************************************************************************
* mvCtrlModelGet - Get Marvell controller device model (Id)
*
* DESCRIPTION:
*       This function returns 16bit describing the device model (ID) as defined
*       in PCI Device and Vendor ID configuration register offset 0x0.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit desscribing Marvell controller ID
*
*******************************************************************************/
uint16_t mvCtrlModelGet()
{
    uint32_t ctrlId = MV_REG_READ(DEV_ID_REG);

    ctrlId = (ctrlId & (DEVICE_ID_MASK)) >> DEVICE_ID_OFFS;
    return ctrlId;
}

/*******************************************************************************
* mvBoardTclkGet - Get the board Tclk (Controller clock)
*
* DESCRIPTION:
*       This routine extract the controller core clock.
*       This function uses the controller counters to make identification.
*   Note: In order to avoid interference, make sure task context switch
*   and interrupts will not occure during this function operation
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
uint32_t mvBoardTclkGet()
{
    return 200000000; /* constant Tclock @ 200MHz (not Sampled@Reset)  */
}

/*******************************************************************************
* mvSpiIfConfigSet -
*
* DESCRIPTION:
* Set the SPI interface parameters.
*
* INPUT:
*       spiId: The SPI controller ID to setup.
* ifParams: The interface parameters.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*******************************************************************************/
INT32 mvSpiIfConfigSet(uint8_t spiId, MV_SPI_IF_PARAMS *ifParams)
{
    uint32_t  ctrlReg;

    ctrlReg = MV_REG_READ(MV_SPI_IF_CONFIG_REG(spiId));

    /* Set Clock Polarity */
    ctrlReg &= ~(MV_SPI_CPOL_MASK | MV_SPI_CPHA_MASK |
        MV_SPI_TXLSBF_MASK | MV_SPI_RXLSBF_MASK);
    if (ifParams->clockPolLow)
        ctrlReg |= MV_SPI_CPOL_MASK;

    if (ifParams->clockPhase == SPI_CLK_BEGIN_CYC)
        ctrlReg |= MV_SPI_CPHA_MASK;

    if (ifParams->txMsbFirst)
        ctrlReg |= MV_SPI_TXLSBF_MASK;

    if (ifParams->rxMsbFirst)
        ctrlReg |= MV_SPI_RXLSBF_MASK;

    MV_REG_WRITE(MV_SPI_IF_CONFIG_REG(spiId), ctrlReg);

    return MV_OK;
}

INT32 mvSpiBaudRateSet(uint8_t spiId, uint32_t serialBaudRate)
{
    uint32_t spr, sppr;
    uint32_t divider;
    uint32_t bestSpr = 0, bestSppr = 0;
    uint8_t exactMatch = 0;
    uint32_t minBaudOffset = 0xFFFFFFFF;
    uint32_t cpuClk = spiHalData.tclk; /*mvCpuPclkGet();*/
    uint32_t tempReg;

    /* Find the best prescale configuration - less or equal */
    for (spr = 1; spr <= 15; spr++) {
        for (sppr = 0; sppr <= 7; sppr++) {
            divider = spr * (1 << sppr);
            /* check for higher - irrelevent */
            if ((cpuClk / divider) > serialBaudRate)
                continue;

            /* check for exact fit */
            if ((cpuClk / divider) == serialBaudRate) {
                bestSpr = spr;
                bestSppr = sppr;
                exactMatch = 1;
                break;
            }

            /* check if this is better than the previous one */
            if ((serialBaudRate - (cpuClk / divider)) < minBaudOffset) {
                minBaudOffset = (serialBaudRate - (cpuClk / divider));
                bestSpr = spr;
                bestSppr = sppr;
            }
        }

        if (exactMatch == 1)
            break;
    }

    if (bestSpr == 0) {
        log_printf("%s ERROR: SPI baud rate prescale error!\n", __func__);
        return MV_OUT_OF_RANGE;
    }

    /* configure the Prescale */
    tempReg = MV_SPI_REG_READ(MV_SPI_IF_CONFIG_REG(spiId)) & ~(MV_SPI_SPR_MASK | MV_SPI_SPPR_0_MASK |
        MV_SPI_SPPR_HI_MASK);
    tempReg |= ((bestSpr << MV_SPI_SPR_OFFSET) |
        ((bestSppr & 0x1) << MV_SPI_SPPR_0_OFFSET) |
        ((bestSppr >> 1) << MV_SPI_SPPR_HI_OFFSET));
    MV_SPI_REG_WRITE(MV_SPI_IF_CONFIG_REG(spiId), tempReg);

    return MV_OK;
}

/*******************************************************************************
* mvSpiCsSet -
*
* DESCRIPTION:
* Set the Chip-Select to which the next SPI transaction will be
* addressed to.
*
* INPUT:
*       csId: The Chip-Select ID to set.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*******************************************************************************/
INT32 mvSpiCsSet(MV_U8 spiId, MV_U8 csId)
{
    MV_U32  ctrlReg;
    static MV_U8 lastCsId = 0xFF;

    if (csId > 7)
        return MV_BAD_PARAM;

    if (lastCsId == csId)
        return MV_OK;

    ctrlReg = MV_SPI_REG_READ(MV_SPI_IF_CTRL_REG(spiId));
    ctrlReg &= ~MV_SPI_CS_NUM_MASK;
    ctrlReg |= (csId << MV_SPI_CS_NUM_OFFSET);
    MV_SPI_REG_WRITE(MV_SPI_IF_CTRL_REG(spiId), ctrlReg);

    lastCsId = csId;

    return MV_OK;
}

/*******************************************************************************
* mvSpiParamsSet
*
* DESCRIPTION:
* Set SPI driver parameters.
* This will affect the behaviour of the SPI APIs after this call.
*
* INPUT:
* spiId - Controller ID
* csId - chip select ID
* type - The type to set.
*
* OUTPUT:
* None.
*
* RETURNS:
* MV_OK on success,
* MV_ERROR otherwise.
*
********************************************************************************/
INT32 mvSpiParamsSet(uint8_t spiId, uint8_t csId, MV_SPI_TYPE type)
{
    MV_SPI_IF_PARAMS ifParams;

    if (MV_OK != mvSpiCsSet(spiId, csId)) {
        log_printf("Error, setting SPI CS failed\n");
        return MV_ERROR;
    }

    if (currSpiInfo != (&(spiTypes[type]))) {
        currSpiInfo = &(spiTypes[type]);
        mvSpiBaudRateSet(spiId, currSpiInfo->baudRate);

        ifParams.clockPolLow = currSpiInfo->clockPolLow;
        ifParams.clockPhase = currSpiInfo->clkPhase;
        ifParams.txMsbFirst = MV_FALSE;
        ifParams.rxMsbFirst = MV_FALSE;
        mvSpiIfConfigSet(spiId, &ifParams);
    }

    return MV_OK;
}

/*******************************************************************************
* mvSpiInit - Initialize the SPI controller
*
* DESCRIPTION:
*       Perform the neccessary initialization in order to be able to send an
*   receive over the SPI interface.
*
* INPUT:
*       serialBaudRate: Baud rate (SPI clock frequency)
*   use16BitMode: Whether to use 2bytes (MV_TRUE) or 1bytes (MV_FALSE)
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
INT32 mvSpiInit(uint8_t spiId, uint32_t serialBaudRate, MV_SPI_HAL_DATA *halData)
{
    MV_STATUS ret;

    memcpy(&spiHalData, halData, sizeof(MV_SPI_HAL_DATA));

    /* Set the serial clock */
    ret = mvSpiBaudRateSet(spiId, serialBaudRate);
    if (ret != MV_OK)
        return ret;

    /* Configure the default SPI mode to be 16bit */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

    /* Configure the MPP5 to zero */
    MV_REG_BIT_RESET(FPGA_CS_L, (0x1 << 5));

    /* Verify that the CS is deasserted */
    mvSpiCsDeassert(spiId);

    mvSpiParamsSet(spiId, 0, 4);

    return MV_OK;
}

/*******************************************************************************
* act2_hal_memMapInit - Initialize the Memory Map
*
* DESCRIPTION:
*
* INPUT:
*       None
* OUTPUT:
*   None
* RETURN:
*       None
*
*******************************************************************************/
void act2_hal_memMapInit()
{
    uint32_t reg_base = INTER_REGS_VIRT_BASE;
    unsigned int *reg_vadr;

    if ((devmemFd=open("/dev/mem", O_RDWR)) < 0)
        log_printf("Failed to open /dev/mem. ");

    reg_vadr = (uint32_t *) mmap(0,
                                 (size_t) MAP_SPACE_SIZE,
                                 PROT_READ|PROT_WRITE,
                                 MAP_SHARED,
                                 devmemFd,
                                 (off_t)reg_base);

    global_virt_reg_addr = reg_vadr;

    if (reg_vadr < 0)
        log_printf("mmap() fail\n");
}

/*******************************************************************************
* act2_hal_memMapClose - Close the Memory Map
*
* DESCRIPTION:
*
* INPUT:
*       None
* OUTPUT:
*   None
* RETURN:
*       None
*
*******************************************************************************/
void act2_hal_memMapClose()
{
    munmap(global_virt_reg_addr, MAP_SPACE_SIZE);
    close(devmemFd);
}
/*******************************************************************************
* mvSysSpiInit - Initialize the SPI subsystem
*
* DESCRIPTION:
*
* INPUT:
*       None
* OUTPUT:
*   None
* RETURN:
*       None
*
*******************************************************************************/
INT32 mvSysSpiInit(uint8_t spiId, uint32_t serialBaudRate)
{
    MV_SPI_HAL_DATA halData;

    halData.ctrlModel = mvCtrlModelGet();
    halData.tclk = mvBoardTclkGet();

    return mvSpiInit(spiId, serialBaudRate, &halData);
}
/*******************************************************************************
* mvSpiWrite - Transmit a buffer over the SPI interface
*
* DESCRIPTION:
*       Transmit a buffer over the SPI interface in 16bit chunks. If the
*   buffer size is odd, then the last chunk will be 8bits. No chip select
*       action is taken.
*
* INPUT:
*   pTxBuff: Pointer to the buffer holding the TX data
*   buffSize: length of the pTxBuff
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
INT32 mvSpiWrite(uint8_t spiId, uint8_t *pTxBuff, uint32_t buffSize)
{
    INT32 ret;
    uint32_t bytesLeft = buffSize;
    uint16_t *txPtr = (uint16_t *)pTxBuff;

    /* check for null parameters */
    if (pTxBuff == NULL) {
        log_printf("%s ERROR: Null pointer parameter!\n", __func__);
        return MV_BAD_PARAM;
    }

    /* Check that the buffer pointer and the buffer size are 16bit aligned */
    if ((currSpiInfo->en16Bit)
        && (currSpiInfo->en16Bit)
        && (((uint32_t)buffSize & 1) == 0)
        && (((uint32_t)pTxBuff & 1) == 0)) {
        /* Verify that the SPI mode is in 16bit mode */
        MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

        /* TX/RX as long we have complete 16bit chunks */
        while (bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE) {
            /* Transmitted and wait for the transfer to be completed */
            ret = mvSpi16bitDataTxRx(spiId, *pTxBuff, NULL);
            if (ret != MV_OK)
                return ret;
            /* increment the pointers */
            pTxBuff++;
            bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
        }
    } else {
        /* Verify that the SPI mode is in 8bit mode */
        MV_REG_BIT_RESET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

        /* TX/RX in 8bit chanks */
        if (gDebug)
            log_printf("\nWrite Data : ");
        while (bytesLeft > 0) {
            /* Transmitted and wait for the transfer to be completed */
            ret = mvSpi8bitDataTxRx(spiId, *pTxBuff, NULL);
            if (ret != MV_OK)
                return ret;

            /* increment the pointers */
            pTxBuff++;
            bytesLeft--;
        }
    }
    
    if (gDebug)
        log_printf("\n");

    return MV_OK;
}

/*******************************************************************************
* mvSpiReadWrite - Read and Write a buffer simultanuosely
*
* DESCRIPTION:
*       Transmit and receive a buffer over the SPI in 16bit chunks. If the
*   buffer size is odd, then the last chunk will be 8bits. The SPI chip
*       select is not handled implicitely.
*
* INPUT:
*       pRxBuff: Pointer to the buffer to write the RX info in
*   pTxBuff: Pointer to the buffer holding the TX info
*   buffSize: length of both the pTxBuff and pRxBuff
*
* OUTPUT:
*       pRxBuff: Pointer of the buffer holding the RX data
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSpiReadWrite(MV_U8 spiId, MV_U8 *pRxBuff, MV_U8* pTxBuff, MV_U32 buffSize)
{
    MV_STATUS ret;
    MV_U32 bytesLeft = buffSize;
    MV_U16 *txPtr = (MV_U16 *)pTxBuff;
    MV_U16 *rxPtr = (MV_U16 *)pRxBuff;

    /* check for null parameters */
    if ((pRxBuff == NULL) || (pTxBuff == NULL)) {
        log_printf("%s ERROR: Null pointer parameter!\n", __func__);
        return MV_BAD_PARAM;
    }

    /* Check that the buffer pointer and the buffer size are 16bit aligned */
    if ((currSpiInfo->en16Bit)
        && (((MV_U32)buffSize & 1) == 0)
        && (((MV_U32)pTxBuff & 1) == 0)
        && (((MV_U32)pRxBuff & 1) == 0)) {
        /* Verify that the SPI mode is in 16bit mode */
        MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

        /* TX/RX as long we have complete 16bit chunks */
        while (bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE) {
            /* Transmitted and wait for the transfer to be completed */
            ret = mvSpi16bitDataTxRx(spiId, *txPtr, rxPtr);
            if (ret != MV_OK)
                return ret;
            /* increment the pointers */
            txPtr++;
            rxPtr++;
            bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
        }
    } else {
        /* Verify that the SPI mode is in 8bit mode */
        MV_REG_BIT_RESET(MV_SPI_IF_CONFIG_REG(spiId), MV_SPI_BYTE_LENGTH_MASK);

        /* TX/RX in 8bit chanks */
        while (bytesLeft > 0) {
            /* Transmitted and wait for the transfer to be completed */
            ret = mvSpi8bitDataTxRx(spiId, *pTxBuff, pRxBuff);
            if (ret != MV_OK)
              return ret;
            pRxBuff++;
            pTxBuff++;
            bytesLeft--;
        }
    }

    return MV_OK;
}


/*******************************************************************************
* mvSpiReadAndWrite - Read and Write a buffer simultanuousely
*
* DESCRIPTION:
*       Transmit and receive a buffer over the SPI in 16bit chunks. If the
*   buffer size is odd, then the last chunk will be 8bits.
*
* INPUT:
*       pRxBuff: Pointer to the buffer to write the RX info in
*   pTxBuff: Pointer to the buffer holding the TX info
*   buffSize: length of both the pTxBuff and pRxBuff
*
* OUTPUT:
*       pRxBuff: Pointer of the buffer holding the RX data
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
INT32  mvSpiReadAndWrite(uint8_t spiId, uint8_t *pRxBuff, uint8_t *pTxBuff, uint32_t buffSize)
{
    INT32 ret;

    /* check for null parameters */
    if ((pRxBuff == NULL) || (pTxBuff == NULL) || (buffSize == 0)) {
        log_printf("%s ERROR: Null pointer parameter!\n", __func__);
        return MV_BAD_PARAM;
    }

    /* First assert the chip select */
    mvSpiCsAssert(spiId);

    ret = mvSpiReadWrite(spiId, pRxBuff, pTxBuff, buffSize);

    /* Finally deassert the chip select */
    mvSpiCsDeassert(spiId);

    return ret;
}

void act2spi_cmd (void)
{
    int i, j;
    MV_STATUS status;
    MV_U8 rxbuff[8];
    MV_U8 txbuff[4][8] = {
        {0x02, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x02, 0x00, 0xC0, 0x00, 0x08, 0x00, 0x00, 0x00},
        {0x03, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    act2_hal_memMapInit();

    printf("\nTesting ACT2 over SPI:\n\n");
    if (mvSysSpiInit(0, 0x1000000) != MV_OK) {
        printf("\nSPI init failed\n");
        return;
    }

    printf("\nSPI init done, setting params...");
    if (mvSpiParamsSet(0, 0, 4) != MV_OK) {
        printf("\nSPI param set failed\n");
    }
    printf("\nDone.\n");

    printf("\nWriting ...\n");
    for (i = 0; i < 4; i++) {
        memset(rxbuff, 0 , 8);
        status = mvSpiReadAndWrite(0, rxbuff, txbuff[i], 8);
        if (status != MV_OK) {
            printf("ACT2 SPI write failed(%d): status (%d)\n", i, status);
            return;
        }
        printf("\n");
        udelay(1000);
    }
    printf("\nACT2 SPI write success\n");
    act2_hal_memMapClose();
}

/* Function fpgaSpiReadCmd
 * read opcode 0x03 and 3 bytes address (example: 0x001028)
 * MOSI (cpu output)03 00 10 28 FF FF FF FF 
 * MSIO (cpu input)  xx xx xx xx D1 D2 D3 D4
 */
MV_STATUS fpgaSpiReadCmd (uint32_t spireg)
{
    int i;
    MV_STATUS status;
    MV_U8 rxbuff[8];
    MV_U8 txbuff[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    

    MV_SPI_HAL_DATA halData;

    halData.ctrlModel = mvCtrlModelGet();
    halData.tclk = mvBoardTclkGet();

    memcpy(&spiHalData, &halData, sizeof(MV_SPI_HAL_DATA));

    /* Set the serial clock */
    status = mvSpiBaudRateSet(0, 0x1000000);
    if (status != MV_OK)
        return status;

    /* Configure the default SPI mode to be 16bit */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(0), MV_SPI_BYTE_LENGTH_MASK);

    /* Verify that the CS is deasserted */
    mvSpiCsDeassert(0);

    mvSpiParamsSet(0, 0, 4);

    /* write opcode 0x02, 3 bytes register address */
    printf("spi register 0x%08x\n", spireg);
     
    txbuff[1] = (spireg >> 16) & 0xFF; 
    txbuff[2] = (spireg  >> 8) & 0xFF; 
    txbuff[3] = spireg & 0xFF; 

    printf("\nFPGA Aikido SPI Write:");
    for (i = 0; i < 8; i++)
        printf("%02X ", txbuff[i]);
    
    printf("\n");
        
    memset(rxbuff, 0 , 8);

    /* read opcode 0x3, 3 bytes spi register */
    status = mvSpiReadAndWrite(0, rxbuff, txbuff, 8);
    if (status != MV_OK) {
        printf("FPGA Aikido SPI Read failed: status (%d)\n", i, status);
        return status;
    }

    udelay(1000);
    
    printf("\nFPGA Aikido SPI Read:");
    for (i = 0; i < 8; i++) {
        printf("%02X ", rxbuff[i]);
    }
    printf("\n");
}

/* Function fpgaSpiWriteCmd
 * write opcode 0x02 and 3 bytes address (example: 0x002304)
 * MOSI (cpu output)02 00 23 04 D1 D2 D3 D4 
 * MSIO (cpu input)  xx xx xx xx xx xx xx xx
 */
MV_STATUS fpgaSpiWriteCmd (uint32_t spireg, uint32_t wdata)
{
    int i;
    MV_STATUS status;
    MV_U8 txbuff[8] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};    

    MV_SPI_HAL_DATA halData;

    halData.ctrlModel = mvCtrlModelGet();
    halData.tclk = mvBoardTclkGet();

    memcpy(&spiHalData, &halData, sizeof(MV_SPI_HAL_DATA));

    /* Set the serial clock */
    status = mvSpiBaudRateSet(0, 0x1000000);
    if (status != MV_OK)
        return status;

    /* Configure the default SPI mode to be 16bit */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG(0), MV_SPI_BYTE_LENGTH_MASK);

    /* Verify that the CS is deasserted */
    mvSpiCsDeassert(0);

    mvSpiParamsSet(0, 0, 4);

    /* write opcode 0x02, 3 bytes register address */
    txbuff[1] = (spireg >> 16) & 0xFF; 
    txbuff[2] = (spireg  >> 8) & 0xFF; 
    txbuff[3] = spireg & 0xFF; 

    txbuff[4] = (wdata >> 24) & 0xFF; 
    txbuff[5] = (wdata >> 16) & 0xFF; 
    txbuff[6] = (wdata  >>8) & 0xFF;
    txbuff[7] = wdata & 0xFF; 

    printf("\nFPGA Aikido SPIWrite:");
    for (i = 0; i < 8; i++) {
        printf("%02X ", txbuff[i]);
    }
    printf("\n");

    /* First assert the chip select */
    mvSpiCsAssert(0);
    status = mvSpiWrite(0, txbuff, 8);
    if (status != MV_OK) {
        printf("FPGA Aikido SPI write failed: status (%d)\n", status);
        return status;
    }

    udelay(1);

    /* Finally deassert the chip select */
    mvSpiCsDeassert(0);
    
}

