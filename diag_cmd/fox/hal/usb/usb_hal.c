/***************************************************************************
***
***    Copyright 2014  Hon Hai Precision Ind. Co. Ltd.
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
***      usb_hal.c
***
***    DESCRIPTION :
***      for USB verification
***
***    HISTORY :
***       - 2014/03/05, 16:00:52, Wed Chen
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
#include "usb_hal.h"
#include "err_type.h"
#include "log.h"
#include "list.h"
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "i2c_fpga.h"
#include "gpio_hal.h"
#include "sys_utils.h"

/*======================================================================
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
static S_USB_FLASH_NODE     g_usbDevInfolist;
static S_USB_PROBE_DEV_NODE g_usbProbeDevList;

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
static int initUsbExtFlashList(void)
{
    S_USB_FLASH_NODE *pNewNode = NULL;
    DIR *dir;
    FILE *fd;
    struct dirent *ptr;
    int port = 0;
    char *location = NULL;
    char *path, *buf;
    char temp[PORT_NUM_LEN];

    path = (char *)malloc(FILE_NAME_LENGTH);
    buf = (char *)malloc(FILE_NAME_LENGTH);

    dir = opendir("/sys/block");
    while ((ptr = readdir(dir)) != NULL)
    {
        if(!(strncmp("sd", ptr->d_name, 2)))
        {
            log_dbgPrintf("device name:%s\n", ptr->d_name);
            memset(path,(char)0,sizeof(path));
            memset(buf,(char)0,sizeof(buf));
            sprintf(path, "/sys/block/%s/device/max_sectors", ptr->d_name);

            fd = fopen(path, "r");
            if(fd == NULL)
            {
                log_printf("Open /sys/block/%s/device/max_secotrs file failed\n", ptr->d_name);
                return E_TYPE_REG_READ;
            }

            memset(temp, (char)0, sizeof(temp));
            sscanf(temp,"1-1.%d", &port);
            log_dbgPrintf("the port num:%d\n", port);

            pNewNode = (S_USB_FLASH_NODE *)malloc(sizeof(S_USB_FLASH_NODE));
            memset(pNewNode, (char)0, sizeof(S_USB_FLASH_NODE));
            pNewNode->portId = port;
            sprintf(pNewNode->dev, "%s", ptr->d_name);
            pNewNode->testResult = FALSE;

            list_add(&(pNewNode->list), &(g_usbDevInfolist.list));
            log_dbgPrintf("Add node %d on %s\n", pNewNode->portId, pNewNode->dev);

            fclose(fd);
        }
    }

    free(path);
    free(buf);
    closedir(dir);
    return E_TYPE_SUCCESS;
}

static int rwExtFlash(int testPortId)
{
    int result;
    char *testFileName = "/tmp/usb/testfile";
    S_USB_FLASH_NODE *pNode=NULL, *pList=NULL;
    char testFileDir[STRING_MAX_LENGTH];
    FILE *writeFd, *readFd;
    int foundDev = FALSE;

    pList = &g_usbDevInfolist;
    list_for_each_entry(pNode, &(pList->list), list)
    {
        log_dbgPrintf("port num in list:%d \n", pNode->portId);
        if(pNode->portId != testPortId)
        {
            continue;
        }

        foundDev = TRUE;
        log_printf("Test port %d on dev %s\n", pNode->portId, pNode->dev);

        /* open usb device */
        mkdir("/tmp/usb",0777);
        memset(testFileDir, (char)0, STRING_MAX_LENGTH);

        sprintf(testFileDir,"/dev/%s", pNode->dev);

        /* mount /dev/xxx, retry once to mount /dev/xxx1, retry once to mount ext3 */
        result = mount( testFileDir,"/tmp/usb","vfat",0,0);
        umount(testFileDir);
        if(result != 0)
        {
            log_printf("Retry to mount %s1\n", testFileDir);
            sprintf(testFileDir, "%s1", testFileDir);
            umount(testFileDir);
            result = mount( testFileDir,"/tmp/usb","vfat",0,0);
            if(result != 0)
            {
                log_printf("Retry to mount ext3\n");
                result = mount( testFileDir,"/tmp/usb","ext3",0,0);
            if(result != 0)
                {
                    log_printf("Mount %s ext3 failed\n", testFileDir);
                    return E_TYPE_REG_WRITE;
                }
            }
        }

        memset(testFileDir, (char)0, STRING_MAX_LENGTH);
        memcpy(testFileDir,testFileName, strlen(testFileName));

        writeFd = fopen(testFileDir, "w");
        if(writeFd == NULL)
        {
            log_printf("Open test file %s failed\n", testFileDir);
            pNode->testResult = FALSE;
            umount("/tmp/usb");
            return E_TYPE_REG_WRITE;
        }

        /* write file name string to the device */
        if((fputs(testFileDir,writeFd)) == EOF)
        {
            log_printf("Write file to %s failed\n", testFileDir);
            fclose(writeFd);
            pNode->testResult = FALSE;
            umount("/tmp/usb");
            return E_TYPE_REG_WRITE;
        }
        log_printf("Write string %s to test file\n",testFileDir);

        if((fclose(writeFd)) == -1)
        {
            log_printf("Close file %s failed\n", testFileDir);
            pNode->testResult = FALSE;
            umount("/tmp/usb");
            return E_TYPE_REG_WRITE;
        }

        /* read file */
        readFd = fopen(testFileDir, "r");
        if(readFd == NULL)
        {
            log_printf("Open Diag test file %s failed\n", testFileDir);
            pNode->testResult = FALSE;
            umount("/tmp/usb");
            return E_TYPE_REG_READ;
        }

        /* read data just write into device*/
        char testBuffer[STRING_MAX_LENGTH];
        memset(testBuffer, (char)0, STRING_MAX_LENGTH);
        if((fgets(testBuffer,strlen(testFileDir)+1,readFd)) == NULL)
        {
            log_printf("Read file %s failed.\n", testFileDir);
            fclose(readFd);
            pNode->testResult = FALSE;
            umount("/tmp/usb");
            return E_TYPE_REG_READ;
        }
        if((fclose(readFd)) == -1)
        {
            log_printf("Close read file %s failed\n", testFileDir);
            pNode->testResult = FALSE;
            umount("/tmp/usb");
            return E_TYPE_REG_READ;
        }

        log_printf("Diag read string = %s\n",testBuffer);
        log_printf("Remove test file %s\n", testFileDir);
        remove(testFileDir);
        sleep(5);
        umount("/tmp/usb");

        /*?Verify String */
        if(memcmp(testBuffer,testFileDir,strlen(testFileDir)) == 0)
        {
            pNode->testResult = TRUE;
            break;
        }
    else
    {
            return E_TYPE_REG_READ;
    }
}

    if(foundDev == FALSE)
{
        log_printf("Not found flash device on port %d\n", testPortId);
        return E_TYPE_REG_READ;
    }

    return E_TYPE_SUCCESS;
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
 *      usb_halCheckProcStat
 *
 *  DESCRIPTION :
 *      Check if USB mount into proc/bus/usb
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halCheckProcStat
(
    void
)
{
    struct stat st;

    /*  Check if usb device info is exsited on board */
    if (stat("/proc/bus/usb/devices",&st) != 0)
    {
        pid_t childPid, waitPid;
        char   *cmdStr[] = {(char*)"/bin/mount", (char*)"-t", (char*)"usbfs", (char*)"none", (char*)"/proc/bus/usb", NULL};
        // mount usbfs to /proc
        childPid = fork();
        if(childPid == 0)
        {
            if(execvp("mount", cmdStr) < 0)
            {
                log_printf("[ERR]:Mount usbfs failed\n");
                return E_TYPE_REG_READ;
            }
        }
        do
        {
            waitPid = waitpid(childPid, NULL, 0);
        }while(waitPid == 0);
    }

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeListDump
 *
 *  DESCRIPTION :
 *      Dump node info in usb dev list
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
void usb_halProbeListDump
(
    IN S_USB_PROBE_DEV_NODE *pList
)
{
    S_USB_PROBE_DEV_NODE *pNode;

    printf("Current USB device is:\r\n");
    printf("Bus Lev  Port  Device\r\n");
    printf("-------------------------\r\n");

    list_for_each_entry(pNode, &(pList->list), list)
    {
        printf(" %d  ", pNode->busId);
        printf(" %d  ", pNode->level);
        printf("   %d  ", pNode->portId);
        printf(" %s  \n", pNode->devInfo);
    }
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeListAdd
 *
 *  DESCRIPTION :
 *      Add node to usb dev list
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halProbeListAdd
(
    IN int bus,
    IN int lev,
    IN int port,
    IN char *devInfo
)
{
    S_USB_PROBE_DEV_NODE *pNewNode = NULL, *pList = NULL;
    S_USB_PROBE_DEV_NODE *pNode;

    pList = &g_usbProbeDevList;

    list_for_each_entry(pNode, &(pList->list), list)
    {
        if ((pNode->busId == bus) && (pNode->portId == port) && (pNode->level == lev))
        {
            return E_TYPE_SUCCESS;
    }
    }

    pNewNode = (S_USB_PROBE_DEV_NODE *)malloc(sizeof(S_USB_PROBE_DEV_NODE));
    memset(pNewNode, (char)0, sizeof(S_USB_PROBE_DEV_NODE));
    pNewNode->busId = bus;
    pNewNode->level = lev;
    pNewNode->portId = port;
    strcpy(pNewNode->devInfo, devInfo);

    list_add(&(pNewNode->list), &(g_usbProbeDevList.list));
    log_dbgPrintf("Add node bus %d port %d to list\n", bus, port);

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeDevInfo
 *
 *  DESCRIPTION :
 *      Probe current USB device and add them into the list
 *      Log is as below:
 *
 *        T:   Bus=01 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#= 2 Spd=12 Mxch= 0
 *        D:   Ver= 2.00 Cls=00(>ifc) Sub=00 Prot=00 MxPS=64 $Cfgs= 1
 *        P:   Vendor=10c4 ProdID=ea60 Rev= 1.00
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
 INT32 usb_halProbeDevInfo
(
    void
)
{
    FILE *fd;
    char buf[PROC_INFO_LENGTH];
    int bus = 0, lev = 0, port = 0, temp1 = 0, prod = 0 ;
    char devInfo[CMD_MAX_LENGTH];

    memset(buf, (char)0, sizeof(buf));
    memset(devInfo, (char)0, sizeof(devInfo));

    /* 1. check if /proc file is existed */
    if((usb_halCheckProcStat()) != E_TYPE_SUCCESS)
    {
        return E_TYPE_REG_READ;
    }

    /* 2. open linux usb info file*/
    fd = fopen("/proc/bus/usb/devices", "r");
    if(fd == NULL)
    {
        log_printf("Open /proc/bus/usb/devices file failed\n");
        return E_TYPE_REG_READ;
    }

    /* 3. parse /lsusb info */
     while (fgets(buf, sizeof(buf), fd) != NULL)
    {
        if (buf[0] != '\n')      /* not blank line, on board is 0 */
        {
            if(strncmp(buf, "T: ", 3) == 0)
            {
                log_dbgPrintf("line is # %s\n", buf);
                sscanf(buf, "%[^C]", buf);
                sscanf(buf, "T:  Bus=%d Lev=%d Prnt=%d Port=%d \n", &bus, &lev, &temp1, &port);
            }
            else if(strncmp(buf, "S:  P", 5) == 0)
            {
                log_dbgPrintf("line is # %s\n", buf);
                memcpy(devInfo, strstr(buf, "Product"),  CMD_MAX_LENGTH);
                sscanf(devInfo, "Product=%s", devInfo);
                usb_halProbeListAdd(bus, lev, port, devInfo);
            }
        }

        memset(buf, (char)0, sizeof(buf));
        memset(devInfo, (char)0, sizeof(devInfo));
    }

    fclose(fd);
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeTest
 *
 *  DESCRIPTION :
 *      probe USB devices
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
 INT32 usb_halProbeTest
(
    void
)
{
    S_USB_PROBE_DEV_NODE *pNode=NULL, *pList=NULL;
    struct list_head *pPos, *pNextPos;
    int result = E_TYPE_SUCCESS;

    /* enable usb */
    usb_halPortEnable(1);

    /* give some time for linux to probe usb */
    sleep(10);

    /* 1. init usb dev list */
    INIT_LIST_HEAD(&g_usbProbeDevList.list);
    pList = &g_usbProbeDevList;

    /* 2. probe current usb device */
    usb_halProbeDevInfo();

    /* 3. dump all usb device */
    usb_halProbeListDump(pList);

    /* 4. Free the list */
    list_for_each_safe(pPos, pNextPos, &g_usbProbeDevList.list)
    {
        pNode = list_entry(pPos, S_USB_PROBE_DEV_NODE, list);
        list_del(pPos);
        free(pNode);
    }

    /* disable usb */
    usb_halPortEnable(0);
    udelay(1000);

    return result;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halConnectTest
 *
 *  DESCRIPTION :
 *      Connect to USB devices
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halConnectTest
(
    UINT32 testPortId
)
{
    S_USB_FLASH_NODE *pNode=NULL;
    struct list_head *pPos, *pNextPos;
    int result = E_TYPE_SUCCESS;

    /* enable usb */
    usb_halPortEnable(1);

    /* give some time for linux to probe usb */
    sleep(10);

    /* 1. init usb dev list */
    INIT_LIST_HEAD(&g_usbDevInfolist.list);

    /* 2. scan available external usb flash, add dev into dev list */
    initUsbExtFlashList();

    /* 3. r/w test on each usb flash*/
    result = rwExtFlash(testPortId);

    /* 4. free usb dev list */
    list_for_each_safe(pPos, pNextPos, &g_usbDevInfolist.list)
    {
        pNode = list_entry(pPos, S_USB_FLASH_NODE, list);
        list_del(pPos);
        free(pNode);
    }
    /* disable usb */
    usb_halPortEnable(0);
    udelay(1000);

    return result;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbLbTest
 *
 *  DESCRIPTION :
 *      Connect to mini USB devices
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halMiniUsbLbTest
(
    void
)
{
    fd_set readfs;    /* file descriptor set */
    struct timeval Timeout;
    int fd, retry_times = 0;
    char *tty = "/dev/ttyACM0";
    char j[2];
    char *btr = NULL;
    int opt,count=32;
    char i;
    int ret = E_TYPE_SUCCESS;

    do
    {
        /* enable usb */
        usb_halPortEnable(1);
        sleep(1);

        /* set miniusb mux gpio 6 to low, for led turn on */
        usb_halMiniUsbMux(0);

        /* give some time for linux to probe usb */
        sleep(10);

        if ((fd = open (tty, O_RDWR | O_NOCTTY)) < 0) {
            log_dbgPrintf("(re)try_time %d, no detect.\n", retry_times);
            ret = -1;
        }

        close(fd);

        /* disable miniusb mux gpio 6, for led turn off */
        usb_halMiniUsbMux(1);
        sleep(1);

         /* disable usb */
        usb_halPortEnable(0);

        udelay(1000);

        /*no detect, retry again*/
        if(ret != E_TYPE_SUCCESS)
        {
            ret = E_TYPE_SUCCESS;
            continue;
        }
        else
        {
            /*deteced ttyACM0, return back*/
            break;
        }

    }while((++retry_times)<= USB_TEST_RETRY);

    log_dbgPrintf("retry_times=%d\n",retry_times);
    if (retry_times == USB_TEST_RETRY+1)
    {
        log_printf("retry %d times, still no detect\n",retry_times);
        ret = -1;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbDetectTest
 *
 *  DESCRIPTION :
 *      Check the mini USB devices is detected in GPIO18
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halMiniUsbDetectTest
(
    void
)
{
    int ret = E_TYPE_SUCCESS;
    UINT32 hwRev;
    UINT32 boardId;
    UINT8  gpioVal, gpio18;
    int fd, retry_times = 0;
    char *tty = "/dev/ttyACM0";

    do {
        /* enable usb */
        usb_halPortEnable(1);
        sleep(1);

        /* give some time for linux to enable usb port */
        sleep(10);

        if ((fd = open (tty, O_RDWR | O_NOCTTY)) < 0) {
            log_dbgPrintf("(re)try_time %d, no detect.\n", retry_times);
            ret = -1;
        } else {
            log_dbgPrintf("(re)try_time %d, detected the USB device (/dev/ttyACM0).\n", retry_times);
            ret = E_TYPE_SUCCESS;
        }

        close(fd);
        
        /* check HW rev
          * miniUSB detect is supported GPIO 18 for PP build
          * 48/48T rev = 0x2
          * Others SKU rev = 0x1
          */
        hwRev = sys_utilsHWRevGet();
    
        gpio_halGetVal(MINI_USB_DETECT_GPIO, &gpioVal);
        gpio18 = (UINT32)gpioVal;
    
        log_dbgPrintf("MiniUSB Dectected with GPIO 18 %d\n", gpio18);
    
        boardId = sys_utilsDevBoardIdGet();

#if 0  /* 02182017, delete the HWrev check from Haywards 2 */
        if ((boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId ==
        E_BOARD_ID_HAYWARDS_48G4G_P)){
              if (hwRev<2)
                  log_dbgPrintf("The Haywards 48 T/P rev %d may not support the miniUSB detection\n", hwRev);
        }else{
              if (hwRev<1)
                  log_dbgPrintf("The Haywards 8/16/24 T/P rev %d may not support the miniUSB detection\n", hwRev);
        }
#endif  
        
         /* disable usb */
        usb_halPortEnable(0);
    
        udelay(1000);
      
        /* GPIO 18 is low when miniUSB detected */
        if ((gpio18 == 1) || (ret!=E_TYPE_SUCCESS)){
            ret = E_TYPE_MINIUSB_NO_DETECT;
            continue;
        } else {
            ret = E_TYPE_SUCCESS;
            break;
        }  
    }while((++retry_times)<= USB_TEST_RETRY);
    
    
    /* GPIO 18 is low when miniUSB detected */
    if ((gpio18 == 0) && (ret==E_TYPE_SUCCESS)){
        log_printf("The miniUSB port detected with GPIO 18\n");
    } else {
        log_dbgPrintf("The miniUSB port failed to detected /dev/ttyACM0 and gpio-18 is high\n");
    }
        
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbLED
 *
 *  DESCRIPTION :
 *      Test the mini USB LED in Random (blink, solid light on)
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halMiniUsbLED
(
    INT32 timeout
)
{
    int ret = E_TYPE_SUCCESS;
    UINT32 hwRev;
    UINT32 boardId;
    UINT32 st = 0;
    UINT32 random_num = 0;

    /* check HW rev
      * miniUSB detect is supported GPIO 18 for PP build
      * 48/48T rev = 0x2
      * Others SKU rev = 0x1
      */
    hwRev = sys_utilsHWRevGet();
    boardId = sys_utilsDevBoardIdGet();

#if 0  /* 02182017, delete the HWrev check from Haywards 2 */
    if ((boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId ==
        E_BOARD_ID_HAYWARDS_48G4G_P)){
          if (hwRev<2)
              log_dbgPrintf("The Haywards 48 T/P rev %d may not support the miniUSB LED test\n", hwRev);
    }else{
          if (hwRev<1)
              log_dbgPrintf("The Haywards 8/16/24 T/P rev %d may not support the miniUSB LED test\n", hwRev);
    }
 #endif

    random_num = (UINT8)rand();

    /* test miniUSB LED random mode (solid light on or blinking) */
    if (random_num%2 == 0)
    {
        usb_halMiniUsbMux(0);
        sleep(timeout);
        log_printf("miniUSB LED test is Solid light on mode.\n");
    }
    else
    {
        for (st=0; st<timeout; st++)
        {
            /* set miniusb mux gpio 6 to low, for led turn on */
            usb_halMiniUsbMux(0);
            /* give some time to light on minuUSB */
            udelay(1000);

            /* set gpio 6 to high for led off */
            usb_halMiniUsbMux(1);
            udelay(1000);
            sleep(1);
        }
        log_printf("miniUSB LED test is Blinking mode.\n");
    }

    /* disable miniusb mux gpio 6, for led turn off and control console to RJ45 */
    usb_halMiniUsbMux(1);

    udelay(1000);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halPortEnable
 *
 *  DESCRIPTION :
 *      enable usb port
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halPortEnable(UINT16 en)
{
    int ret = E_TYPE_SUCCESS;

    ret = fpgaRegWrite(FPGA_USB_CTRL_REG, en);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbMux
 *
 *  DESCRIPTION :
 *      enable mini usb port mux with gpio 6
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
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
INT32 usb_halMiniUsbMux(UINT16 en)
{
    int ret = E_TYPE_SUCCESS;

    ret = gpio_halSetVal(MINI_USB_MUX_GPIO, en);

    return ret;
}
