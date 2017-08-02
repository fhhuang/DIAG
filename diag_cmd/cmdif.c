/***************************************************
File: cmdif.c
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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>
#include <termios.h>



#include "cmdif.h"
#include "cmdif_ext.h"

#define setTermSettings(fd,argp) tcsetattr(fd,TCSANOW,argp)
#define getTermSettings(fd,argp) tcgetattr(fd, argp);

#define xmalloc malloc
#define xstrdup strdup

#define Isprint(c) ( (c) >= ' ' && (c) != ((unsigned char)'\233') )

static int cmdedit_x;		/* x position */
static int cmdedit_y;		/* y terminal */
static int cmdedit_prmt_len;	
char *cmdedit_prompt;		

static int cursor;		
static int len;			
static char *command_ps;	
volatile int handlers_sets = 0;	
static volatile int cmdedit_termw = 80;

static struct termios term_initial_settings, term_new_settings;


static void cmdif_reset_term(void);
static void win_changed(int nsig);
ssize_t safe_read(int fd, void *buf, size_t count);

struct history {
	char *s;
	struct history *p;
	struct history *n;
};

/* Maximum list for history */
static const int MAX_HISTORY = 15;
static int history_counter = 0;	/* Number of commands in history list */
/* First element in command line list */
static struct history *his_front = NULL;

/* Last element in command line list */
static struct history *his_end = NULL;


static void cmdif_init(void);
static void cmdif_setwidth(int w, int redraw_flg);
static void cmdif_reset_term(void);
static void cmdif_set_out_char(int next_char);

static void input_end(void);
static void input_backward(int num);
static void input_delete(void);
static void input_backspace(void);
static void input_forward(void);
static void input_tab(int *lastWasTab,char * in_prompt);
static void input_qm(int *lastWasTab,char * in_prompt);

static void goto_new_line(void);
static void parse_prompt(const char *prmt_ptr);
static void put_prompt(void);

static inline void out1str(const char *s);
static void redraw(int y, int back_cursor);
static inline void get_next_history(struct history **hp);
static void get_previous_history(struct history **hp, struct history *p);
static inline void beep(void);
static void win_changed(int nsig);


int cmdifinput(char *prompt, char command[COMBUFSIZ])
{
	int break_out = 0;
	int lastWasTab = FALSE;
	unsigned char c = 0;
	struct history *hp = his_end;

	cmdedit_y = 0;	
	len = 0;
	command_ps = command;

	getTermSettings(0, (void *) &term_initial_settings);
	memcpy(&term_new_settings, &term_initial_settings, sizeof(struct termios));
	term_new_settings.c_lflag &= ~ICANON;        /* unbuffered input */

	/* Turn off echoing and CTRL-C, so we can trap it */
	term_new_settings.c_lflag &= ~(ECHO | ECHONL | ISIG);
	command[0] = 0;

	setTermSettings(0, (void *) &term_new_settings);
	handlers_sets |= SET_RESET_TERM;

	/* Now initialize terminal windows */
	cmdif_init();

	/* Print out the command prompt */
	parse_prompt(prompt);

	while (1) {

		fflush(stdout);			/* buffered out to fast */
		
		if (safe_read(0, &c, 1) < 1)
			/* if we can't read input then exit */
			goto prepare_to_die;

		switch (c) {
		case '\n':
		case '\r':
			/* Enter */
			goto_new_line();
			break_out = 1;
			break;
		case 1:
			/* Control-a -- Beginning of line */
			input_backward(cursor);
			break;
		case 2:
			/* Control-b -- Move back one character */
			input_backward(1);
			break;
		case 3:
			/* Control-c -- stop gathering input */
			goto_new_line();
			command[0] = 0;
			len = 0;
			lastWasTab = FALSE;
			put_prompt();
			break;
		case 4:
			/* Control-d -- Delete one character, or exit
			 * if the len=0 and no chars to delete */
			if (len == 0) {
prepare_to_die:
				break_out = -1; /* for control stoped jobs */
				break;
			} else {
				input_delete();
			}
			break;
		case 5:
			/* Control-e -- End of line */
			input_end();
			break;
		case 6:
			/* Control-f -- Move forward one character */
			input_forward();
			break;
		case '\b':
		case DEL:
			/* Control-h and DEL */
			input_backspace();
			break;
		#ifdef CONFIG_TAB_FUNCTION
		case '\t':
			input_tab(&lastWasTab,prompt);
			break;
		#endif
		#ifdef CONFIG_QM_FUNCTION
		case '?': /* question mark */
			input_qm(&lastWasTab,prompt);
			break;
                    #endif
                                
		case 11:
			/* Control-k -- clear to end of line */  
			*(command + cursor) = 0;
			len = cursor;
			printf("\033[J");
			break;
		case 12: 
			{
				/* Control-l -- clear screen */
				int old_cursor = cursor;
				printf("\033[H");
				redraw(0, len-old_cursor);
			}
			break;
		case 14:
			/* Control-n -- Get next command in history */
			if (hp && hp->n && hp->n->s) {
				get_next_history(&hp);
				goto rewrite_line;
			} else {
				beep();
			}
			break;
		case 16:
			/* Control-p -- Get previous command from history */
			if (hp && hp->p) {
				get_previous_history(&hp, hp->p);
				goto rewrite_line;
			} else {
				beep();
			}
			break;
		case 21:
			/* Control-U -- Clear line before cursor */
			if (cursor) {
				strcpy(command, command + cursor);
				redraw(cmdedit_y, len -= cursor);
			}
			break;
		case ESC:{
			/* escape sequence follows */
			if (safe_read(0, &c, 1) < 1)
				goto prepare_to_die;
			/* different vt100 emulations */
			if (c == '[' || c == 'O') {
				if (safe_read(0, &c, 1) < 1)
					goto prepare_to_die;
			}
			switch (c) {
			case '\t':			/* Alt-Tab */

				//input_tab(&lastWasTab,prompt);
				break;
			case 'A':
				/* Up Arrow -- Get previous command from history */
				if (hp && hp->p) {
					get_previous_history(&hp, hp->p);
					goto rewrite_line;
				} else {
					beep();
				}
				break;
			case 'B':
				/* Down Arrow -- Get next command in history */
				if (hp && hp->n && hp->n->s) {
					get_next_history(&hp);
					goto rewrite_line;
				} else {
					beep();
				}
				break;

				/* Rewrite the line with the selected history item */
			  rewrite_line:

				/* change command */
				len = strlen(strcpy(command, hp->s));
				/* redraw and go to end line */
				redraw(cmdedit_y, 0);
				break;

			case 'C':
				/* Right Arrow -- Move forward one character */
				input_forward();
				break;
			case 'D':
				/* Left Arrow -- Move back one character */
				input_backward(1);
				break;
			case '3':
				/* Delete */
				input_delete();
				break;
			case '1':
			case 'H':
				/* Home (Ctrl-A) */
				input_backward(cursor);
				break;
			case '4':
			case 'F':
				/* End (Ctrl-E) */
				input_end();
				break;
			default:
				if (!(c >= '1' && c <= '9'))
					c = 0;
				//beep();
			}
			if (c >= '1' && c <= '9')
				do
					if (safe_read(0, &c, 1) < 1)
						goto prepare_to_die;
				while (c != '~');
			break;
		}

		default:	/* If it's regular input, do the normal thing */
			if (!Isprint(c))	/* Skip non-printable characters */
				break;

			if (len >= (BUFSIZ - 2))	/* Need to leave space for enter */
				break;

			len++;

			if (cursor == (len - 1)) {	/* Append if at the end of the line */
				*(command + cursor) = c;
				*(command + cursor + 1) = 0;
				cmdif_set_out_char(0);
			} else {			/* Insert otherwise */
				int sc = cursor;

				memmove(command + sc + 1, command + sc, len - sc);
				*(command + sc) = c;
				sc++;
				/* rewrite from cursor */
				input_end();
				/* to prev x pos + 1 */
				input_backward(cursor - sc);
			}

			break;
		}
		if (break_out)			/* Enter is the command terminator, no more input. */
			break;

		if (c != '\t')
			lastWasTab = FALSE;
	}

	setTermSettings(0, (void *) &term_initial_settings);
	handlers_sets &= ~SET_RESET_TERM;

	if (len) {					/* no put empty line */

		struct history *h = his_end;
		char *ss;

		ss = xstrdup(command);	/* duplicate */

		if (h == 0) {
			/* No previous history -- this memory is never freed */
			h = his_front = xmalloc(sizeof(struct history));
			h->n = xmalloc(sizeof(struct history));

			h->p = NULL;
			h->s = ss;
			h->n->p = h;
			h->n->n = NULL;
			h->n->s = NULL;
			his_end = h->n;
			history_counter++;
		} else {
			/* Add a new history command -- this memory is never freed */
			h->n = xmalloc(sizeof(struct history));

			h->n->p = h;
			h->n->n = NULL;
			h->n->s = NULL;
			h->s = ss;
			his_end = h->n;

			/* After max history, remove the oldest command */
			if (history_counter >= MAX_HISTORY) {

				struct history *p = his_front->n;

				p->p = NULL;
				free(his_front->s);
				free(his_front);
				his_front = p;
			} else {
				history_counter++;
			}
		}
	}	

	/* Handle command history log */
	cmdif_reset_term();

	return 0;
}


static void win_changed(int nsig)
{
	struct winsize win = { 0, 0, 0, 0 };
	static sighandler_t previous_SIGWINCH_handler;	/* for reset */

	/*   emulate      || signal call */
	if (nsig == -SIGWINCH || nsig == SIGWINCH) {
		ioctl(0, TIOCGWINSZ, &win);
		if (win.ws_col > 0) {
			cmdif_setwidth(win.ws_col, nsig == SIGWINCH);
		} 
	}
	/* Unix not all standart in recall signal */

	if (nsig == -SIGWINCH)		/* save previous handler   */
		previous_SIGWINCH_handler = signal(SIGWINCH, win_changed);
	else if (nsig == SIGWINCH)	/* signaled called handler */
		signal(SIGWINCH, win_changed);	/* set for next call       */
	else						/* nsig == 0 */
		/* set previous handler    */
		signal(SIGWINCH, previous_SIGWINCH_handler);	/* reset    */
}


static void cmdif_init(void)
{
	cmdedit_prmt_len = 0;
	if ((handlers_sets & SET_WCHG_HANDLERS) == 0) {
		/* emulate usage handler to set handler and call yours work */
		win_changed(-SIGWINCH);
		handlers_sets |= SET_WCHG_HANDLERS;
	}

	if ((handlers_sets & SET_ATEXIT) == 0) {

		handlers_sets |= SET_ATEXIT;
		atexit(cmdif_reset_term);	/* be sure to do this only once */
	}
}


static void cmdif_setwidth(int w, int redraw_flg)
{
	cmdedit_termw = cmdedit_prmt_len + 2;
	if (w <= cmdedit_termw) {
		cmdedit_termw = cmdedit_termw % w;
	}
	if (w > cmdedit_termw) {
		cmdedit_termw = w;

		if (redraw_flg) {
			/* new y for current cursor */
			int new_y = (cursor + cmdedit_prmt_len) / w;

			/* redraw */
			redraw((new_y >= cmdedit_y ? new_y : cmdedit_y), len - cursor);
			fflush(stdout);
		}
	} 
}


static void cmdif_reset_term(void)
{
	if ((handlers_sets & SET_RESET_TERM) != 0) {
/* sparc and other have broken termios support: use old termio handling. */
		setTermSettings(fileno(stdin), (void *) &term_initial_settings);
		handlers_sets &= ~SET_RESET_TERM;
	}
	if ((handlers_sets & SET_WCHG_HANDLERS) != 0) {
		/* reset SIGWINCH handler to previous (default) */
		win_changed(0);
		handlers_sets &= ~SET_WCHG_HANDLERS;
	}
	fflush(stdout);
}

/* special for recount position for scroll and remove terminal margin effect */
static void cmdif_set_out_char(int next_char)
{

	int c = (int)((unsigned char) command_ps[cursor]);

	if (c == 0)
		c = ' ';	/* destroy end char? */
	putchar(c);
	if (++cmdedit_x >= cmdedit_termw) {
		/* terminal is scrolled down */
		cmdedit_y++;
		cmdedit_x = 0;

		if (!next_char)
			next_char = ' ';
		/* destroy "(auto)margin" */
		putchar(next_char);
		putchar('\b');
	}
	cursor++;
}

static inline void out1str(const char *s)
{
	fputs(s, stdout);
}


/* draw promt, editor line, and clear tail */
static void redraw(int y, int back_cursor)
{
	fflush(stdout);
	if (y > 0)				/* up to start y */
		printf("\033[%dA", y);
	putchar('\r');
	put_prompt();
	input_end();				/* rewrite */
	printf("\033[J");			/* destroy tail after cursor */
	input_backward(back_cursor);
}


static void parse_prompt(const char *prmt_ptr)
{
	cmdedit_prompt = (char *)prmt_ptr;
	cmdedit_prmt_len = strlen(prmt_ptr);
	put_prompt();
}

static void put_prompt(void)
{
	out1str(cmdedit_prompt);
	cmdedit_x = cmdedit_prmt_len;	/* count real x terminal position */
	cursor = 0;
	cmdedit_y = 0;                  /* new quasireal y */
}

/* Go to the next line */
static void goto_new_line(void)
{
	input_end();
	if (cmdedit_x)
		putchar('\n');
}

/* Move to end line. Bonus: rewrite line from cursor */
static void input_end(void)
{
	while (cursor < len)
		cmdif_set_out_char(0);
}


static void input_backward(int num)
{
	if (num > cursor)
		num = cursor;
	cursor -= num;		/* new cursor (in command, not terminal) */

	if (cmdedit_x >= num) {		/* no to up line */
		cmdedit_x -= num;
		if (num < 4)
			while (num-- > 0)
				putchar('\b');

		else
			printf("\033[%dD", num);
	} else {
		int count_y;

		if (cmdedit_x) {
			putchar('\r');		/* back to first terminal pos.  */
			num -= cmdedit_x;	/* set previous backward        */
		}
		count_y = 1 + num / cmdedit_termw;
		printf("\033[%dA", count_y);
		cmdedit_y -= count_y;
		/*  require  forward  after  uping   */
		cmdedit_x = cmdedit_termw * count_y - num;
		printf("\033[%dC", cmdedit_x);	/* set term cursor   */
	}
}

/* Delete the char in front of the cursor */
static void input_delete(void)
{
	int j = cursor;

	if (j == len)
		return;

	strcpy(command_ps + j, command_ps + j + 1);
	len--;
	input_end();			/* rewtite new line */
	cmdif_set_out_char(0);	/* destroy end char */
	input_backward(cursor - j);	/* back to old pos cursor */
}

/* Delete the char in back of the cursor */
static void input_backspace(void)
{
	if (cursor > 0) {
		input_backward(1);
		input_delete();
	}
}


/* Move forward one charactor */
static void input_forward(void)
{
	if (cursor < len)
		cmdif_set_out_char(command_ps[cursor + 1]);
}


static void input_tab(int *lastWasTab,char * in_prompt)
{
	/* do TAB completion */
	//static int matches=0;
	//int i;	
	char *tmp;
	char matchBuf[BUFSIZ];
	char matchtmpBuf[BUFSIZ];
	int len_found;
	int recalc_pos;
	//int spacecount=0;
	
	redraw(0,0);
	if (lastWasTab == 0) {		/* free all memory */		
		//printf("\n");
		//output2console(1);
		tab_sh_proc("");
		parse_prompt(in_prompt);		
		return;
	}	
		//output2console(1);
		tmp = strncpy(matchBuf, command_ps, cursor);		
		tmp[cursor] = 0;
		strcpy(matchtmpBuf,matchBuf);
		if(!strlen(matchBuf))
			input_tab(0,in_prompt);
		else{			
			tmp = tab_sh_proc(matchBuf);
			if(tmp){
				len_found = strlen(tmp);
				recalc_pos= strlen(matchtmpBuf);
				/* have space to placed match? */
				if ((len_found - strlen(matchtmpBuf) + len) < BUFSIZ) {
					/* before word for match   */
					command_ps[cursor - recalc_pos] = 0;
					/* save   tail line        */
					strcpy(matchtmpBuf, command_ps + cursor);
					/* add    match            */
					strcat(command_ps, tmp);
					/* add    tail             */
					strcat(command_ps, matchtmpBuf);
					/* back to begin word for match    */
					input_backward(recalc_pos);
					//input_end();
					/* new pos                         */
					recalc_pos = cursor + len_found;
					/* new len                         */
					len = strlen(command_ps);
					/* write out the matched command   */
					redraw(cmdedit_y, len - recalc_pos);
				}
				//free(tmp);
			}
			redraw(0,0);
		}
	
}


static void input_qm(int *lastWasTab,char * in_prompt)
{
	/* do TAB completion */
	//static int matches=0;
	//int i;	
	char *tmp;
	char matchBuf[BUFSIZ];
	char matchtmpBuf[BUFSIZ];
	int len_found;
	int recalc_pos;
	//int spacecount=0;
	
	redraw(0,0);
	if (lastWasTab == 0) {		/* free all memory */		
		//printf("\n");
		//output2console(1);
		
		qm_sh_proc("");
		parse_prompt(in_prompt);		
		return;
	}	
		//printf("\n");
		//output2console(1);

		tmp = strncpy(matchBuf, command_ps, cursor);		
		tmp[cursor] = 0;
		strcpy(matchtmpBuf,matchBuf);
		if(!strlen(matchBuf))
			input_qm(0,in_prompt);
		else{			
			tmp = qm_sh_proc(matchBuf);
			#if 0
			if(tmp){
				len_found = strlen(tmp);
				recalc_pos= strlen(matchtmpBuf);
				/* have space to placed match? */
				if ((len_found - strlen(matchtmpBuf) + len) < BUFSIZ) {
					/* before word for match   */
					command_ps[cursor - recalc_pos] = 0;
					/* save   tail line        */
					strcpy(matchtmpBuf, command_ps + cursor);
					/* add    match            */
					strcat(command_ps, tmp);
					/* add    tail             */
					strcat(command_ps, matchtmpBuf);
					/* back to begin word for match    */
					input_backward(recalc_pos);
					//input_end();
					/* new pos                         */
					recalc_pos = cursor + len_found;
					/* new len                         */
					len = strlen(command_ps);
					/* write out the matched command   */
					redraw(cmdedit_y, len - recalc_pos);
				}
				//free(tmp);
			}
			#endif
			redraw(0,0);
		}
	
}

static inline void beep(void)
{
	putchar('\007');
}

static void get_previous_history(struct history **hp, struct history *p)
{
	if ((*hp)->s)
		free((*hp)->s);
	(*hp)->s = xstrdup(command_ps);
	*hp = p;
}

static inline void get_next_history(struct history **hp)
{
	get_previous_history(hp, (*hp)->n);
}



ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

