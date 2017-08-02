/***************************************************
File: cmdif_ext.h
Author: CMLAI
Date: 
Modified by:
          1. CMLAI 2006/11/07 prototype 
          2. CMLAI 2006/11/07 handle SIGINT
          3. CMLAI 2006/11/07 handle SIGQUIT
          4. CMLAI 2006/11/07 terminal prototype
          5. CMLAI 2008/02/05 add server client mode
          6. CMLAI 2008/02/05 add stadnalone mode
          7. CMLAI 2008/02/05 remodify cmd structure 
          8. CMLAI 2012/01/31 user input function independently
Last update:
Destription:
          A terminal user input interface for special key handler

****************************************************/


#ifndef __CMDIF_EXT_H
#define __CMDIF_EXT_H

char *cmd_process(int istab,char *inbuf);

char * tab_sh_proc(char *buf);
char *qm_sh_proc(char *buf);


#endif



