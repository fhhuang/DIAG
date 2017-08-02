/***************************************************
File: cmdif.h
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
          A terminal user input interface

****************************************************/
#ifndef __CMD_INTERFACE_H__
#define __CMD_INTERFACE_H__


#define COMBUFSIZ 1024

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif



enum {
	SET_ATEXIT = 1,				
	SET_WCHG_HANDLERS = 2,  	
	SET_RESET_TERM = 4,     		
};


enum {
	ESC = 27,
	DEL = 127,
};


typedef void (*sighandler_t) (int);


int cmdifinput(char *, char*);

#endif
