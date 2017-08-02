/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 *  Command Processor Table
 */

#include <stdio.h>
#include <string.h>
#include <foxCommand.h>
#include "err_type.h"
#include "cmn_type.h"
#include "log.h"
#include "porting.h"
#include "sys_utils.h"
#include "mcu_hal.h"
#include "i2c_fpga.h"
#include "i2c_hal.h"
#include "fox_diagVer.h"

/*cmd_tbl_t __u_boot_cmd_start, __u_boot_cmd_end;*/

int
do_version (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    UINT8   poeSwVersion[64] = "";
    UINT8   ubootVersion[128] = "";
    UINT16  index = 0;
    UINT16  buf[4]={0};
    INT32   mcuData=0, ret=E_TYPE_SUCCESS;
    E_POE_MODULE poeModuleType;
    UINT8 cmd[] = "strings /dev/mtd0 | grep U-Boot | grep \"Haywards\"";
    FILE* fp = popen(cmd, "r");
    if (pipe) {
        fgets(ubootVersion, sizeof(ubootVersion), fp);
    }
    pclose(fp);

    if (ubootVersion[0]!='\0')
       log_printf("UBOOT Version \t: %s", ubootVersion);
    log_printf("DIAG Version  \t: %s (%s - %s)\n", FOX_DIAG_VERSION, __DATE__, __TIME__);

    poeModuleType = sys_utilsPoeModuleGet();
    /* check if PoE module is supported */
    if(poeModuleType != E_POE_MODULE_NOT_SUPPORTED)
    {
        poe_halGetSoftwareVersion(poeSwVersion);
        log_printf("PoE FW Version \t: %s\n", poeSwVersion);
    }

    /* Display FPGA version */
    for ( index=FPGA_BOARD_ID_REG; index<=FPGA_VER_REG; index++ )
    {
        /* Read the version */
        ret = i2c_halRegGet(FPGA_I2C_ADDR, index, FPGA_REG_LEN, (UINT8 *)&buf[index-FPGA_BOARD_ID_REG], 2);
        if ( ret != E_TYPE_SUCCESS )
        {
            log_printf("Fail to read the FPGA version ID.\n");
            return ret;
         }
    }
    log_printf("FPGA Version \t: V%d.%d\n", (buf[2]>>8)&0xFF, buf[2]&0xFF);

    /* Display MCU version */
    mcuData=0x0;
    mcu_halDataSet(0x90, MCU_ADDR_HIGH, mcuData);
    if((ret = mcu_halDataGet(0x1, 0, &mcuData)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }
    else
    {
        log_printf("MCU Version \t: V%d.%d.%d\n", (mcuData&0x00ff0000)>>16, (mcuData&0x0000ff00)>>8, (mcuData&0x000000ff));
    }
    return 0;
}

U_BOOT_CMD(
  version,  1,    1,  do_version,
  "version \t- print diagnostic firmware version\n",
  NULL
);

/* add by Brian Lu, 20151004 */
int
do_setenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int logMode, i;
    unsigned char log_mode_str[2][10] = { "console", "file" }, log_debug_str[2][10] = { "off", "on" };
    S_LOG_ENV_INFO logEnvConf[1];
    unsigned int ret = E_TYPE_INVALID_CMD_FORMAT;

    if(argc != 4)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        return E_TYPE_INVALID_CMD_FORMAT;
    }

    if ( strcmp(argv[1], "log") == 0 )
    {
        memset(logEnvConf,0,sizeof(S_LOG_ENV_INFO));
        for (i=0; i<2; i++)
            if ( !strcmp(argv[2], log_mode_str[i]) )
                break;
        if ( i < 2 )
            logMode = i;
        else
            logMode = simple_strtoul(argv[2], NULL, 10);

        switch(logMode)
        {
            case E_LOG_OUTPUT_CONSOLE:
              logEnvConf[0].logOutputType = E_LOG_OUTPUT_CONSOLE;
              break;
            case E_LOG_OUTPUT_FILE:
              logEnvConf[0].logOutputType = E_LOG_OUTPUT_FILE;
              break;
            default:
              logEnvConf[0].logOutputType = E_LOG_OUTPUT_CONSOLE;
              break;
        }

        if ( !strcmp(argv[3], "on") )
            logEnvConf[0].logDebugType = E_LOG_DEBUG_ON;
        else if ( !strcmp(argv[3], "off") )
            logEnvConf[0].logDebugType = E_LOG_DEBUG_OFF;

        log_envInfoSet(logEnvConf);
        log_dbgPrintf("log output to %s, debug log is %s\n", log_mode_str[logEnvConf[0].logOutputType], log_debug_str[logEnvConf[0].logDebugType]);
    }
    return 0;
}

U_BOOT_CMD(
  setenv, CONFIG_SYS_MAXARGS,   1,  do_setenv,
  "setenv \t\t- set environment variables\n",
        "<log> <output> <debug>\n"
        "    log : Indicates log mode to be modified.\n"
        "    output: Specify where to output logs, Valid values are <console|file>.\n"
        "    debug: Enable debug message or not <on|off>.\n"
        "\n"
);

#if defined(CONFIG_CMD_ECHO)

int
do_echo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
  int i, putnl = 1;

  for (i = 1; i < argc; i++) {
    char *p = argv[i], c;

    if (i > 1)
      putc(' ', stdout);
    while ((c = *p++) != '\0') {
      if (c == '\\' && *p == 'c') {
        putnl = 0;
        p++;
      } else {
        putc(c, stdout);
      }
    }
  }

  if (putnl)
    putc('\n', stdout);
  return 0;
}

U_BOOT_CMD(
  echo, CONFIG_SYS_MAXARGS, 1,  do_echo,
  "echo \t\t- echo args to console\n",
  "[args..]\n"
  "    - echo args to console; \\c suppresses newline\n"
);

#endif


/*
 * Use fputs() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */
int do_help (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
  int i;
  int rcode = 0;

  if (argc == 1) {  /*show list of commands */

    int cmd_items = &__u_boot_cmd_end -
        &__u_boot_cmd_start;  /* pointer arith! */
    cmd_tbl_t *cmd_array[cmd_items];
    int i, j, swaps;

    /* Make array of commands from .uboot_cmd section */
    cmdtp = &__u_boot_cmd_start;
    for (i = 0; i < cmd_items; i++) {
      cmd_array[i] = cmdtp++;
    }

    /* Sort command list (trivial bubble sort) */
    for (i = cmd_items - 1; i > 0; --i) {
      swaps = 0;
      for (j = 0; j < i; ++j) {
        if (strcmp (cmd_array[j]->name,
              cmd_array[j + 1]->name) > 0) {
          cmd_tbl_t *tmp;
          tmp = cmd_array[j];
          cmd_array[j] = cmd_array[j + 1];
          cmd_array[j + 1] = tmp;
          ++swaps;
        }
      }
      if (!swaps)
        break;
    }

    /* print short help (usage) */
    for (i = 0; i < cmd_items; i++) {
      const char *usage = cmd_array[i]->usage;
#if 0 /* TBP, by Ken Hsu 20100622 */
      /* allow user abort */
      if (ctrlc ())
        return 1;
#endif
      if (usage == NULL)
        continue;
      fputs (usage, stdout);
    }
    return 0;
  }
  /*
   * command help (long version)
   */
  for (i = 1; i < argc; ++i) {
    if ((cmdtp = find_cmd (argv[i])) != NULL) {
#ifdef  CONFIG_SYS_LONGHELP
      /* found - print (long) help info */
      fputs (cmdtp->name, stdout);
      putc (' ', stdout);
      if (cmdtp->help) {
        fputs (cmdtp->help, stdout);
      } else {
        fputs ("- No help available.\n", stdout);
        rcode = 1;
      }
      putc ('\n', stdout);
#else /* no long help available */
      if (cmdtp->usage)
        fputs (cmdtp->usage, stdout);
#endif  /* CONFIG_SYS_LONGHELP */
    } else {
      printf ("Unknown command '%s' - try 'help'"
        " without arguments for list of all"
        " known commands\n\n", argv[i]
          );
      rcode = 1;
    }
  }
  return rcode;
}


U_BOOT_CMD(
  help, CONFIG_SYS_MAXARGS, 1,  do_help,
  "help \t\t- print online help\n",
  "[command ...]\n"
  "    - show help information (for 'command')\n"
  "'help' prints online help for the monitor commands.\n\n"
  "Without arguments, it prints a short usage message for all commands.\n\n"
  "To get detailed help information for specific commands you can type\n"
  "'help' with one or more command names as arguments.\n"
);

/* This do not ust the U_BOOT_CMD macro as ? can't be used in symbol names */
#ifdef  CONFIG_SYS_LONGHELP
cmd_tbl_t __u_boot_cmd_question_mark Struct_Section = {
  "?",  CONFIG_SYS_MAXARGS, 1,  do_help,
  "? \t\t- alias for 'help'\n",
  NULL
};
#else
cmd_tbl_t __u_boot_cmd_question_mark Struct_Section = {
  "?",  CONFIG_SYS_MAXARGS, 1,  do_help,
  "? \t\t- alias for 'help'\n"
};
#endif /* CONFIG_SYS_LONGHELP */

/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd (const char *cmd)
{
  cmd_tbl_t *cmdtp;
  cmd_tbl_t *cmdtp_temp = &__u_boot_cmd_start;  /*Init value */
  const char *p;
  int len;
  int n_found = 0;

  /*
   * Some commands allow length modifiers (like "cp.b");
   * compare command name only until first dot.
   */
  len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

  for (cmdtp = &__u_boot_cmd_start;
       cmdtp != &__u_boot_cmd_end;
       cmdtp++) {
    if (strncmp (cmd, cmdtp->name, len) == 0) {
      if (len == strlen (cmdtp->name))
        return cmdtp; /* full match */

      cmdtp_temp = cmdtp; /* abbreviated command ? */
      n_found++;
    }
  }
  if (n_found == 1) {     /* exactly one match */
    return cmdtp_temp;
  }

  return NULL;  /* not found or ambiguous command */
}

#ifdef CONFIG_AUTO_COMPLETE

int var_complete(int argc, char *argv[], char last_char, int maxv, char *cmdv[])
{
  static char tmp_buf[512];
  int space;

  space = last_char == '\0' || last_char == ' ' || last_char == '\t';

  if (space && argc == 1)
    return env_complete("", maxv, cmdv, sizeof(tmp_buf), tmp_buf);

  if (!space && argc == 2)
    return env_complete(argv[1], maxv, cmdv, sizeof(tmp_buf), tmp_buf);

  return 0;
}

static void install_auto_complete_handler(const char *cmd,
    int (*complete)(int argc, char *argv[], char last_char, int maxv, char *cmdv[]))
{
  cmd_tbl_t *cmdtp;

  cmdtp = find_cmd(cmd);
  if (cmdtp == NULL)
    return;

  cmdtp->complete = complete;
}

void install_auto_complete(void)
{
  install_auto_complete_handler("printenv", var_complete);
  install_auto_complete_handler("setenv", var_complete);
#if defined(CONFIG_CMD_RUN)
  install_auto_complete_handler("run", var_complete);
#endif
}

/*************************************************************************************/

static int complete_cmdv(int argc, char *argv[], char last_char, int maxv, char *cmdv[])
{
  cmd_tbl_t *cmdtp;
  const char *p;
  int len, clen;
  int n_found = 0;
  const char *cmd;

  /* sanity? */
  if (maxv < 2)
    return -2;

  cmdv[0] = NULL;

  if (argc == 0) {
    /* output full list of commands */
    for (cmdtp = &__u_boot_cmd_start; cmdtp != &__u_boot_cmd_end; cmdtp++) {
      if (n_found >= maxv - 2) {
        cmdv[n_found++] = "...";
        break;
      }
      cmdv[n_found++] = cmdtp->name;
    }
    cmdv[n_found] = NULL;
    return n_found;
  }

  /* more than one arg or one but the start of the next */
  if (argc > 1 || (last_char == '\0' || last_char == ' ' || last_char == '\t')) {
    cmdtp = find_cmd(argv[0]);
    if (cmdtp == NULL || cmdtp->complete == NULL) {
      cmdv[0] = NULL;
      return 0;
    }
    return (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
  }

  cmd = argv[0];
  /*
   * Some commands allow length modifiers (like "cp.b");
   * compare command name only until first dot.
   */
  p = strchr(cmd, '.');
  if (p == NULL)
    len = strlen(cmd);
  else
    len = p - cmd;

  /* return the partial matches */
  for (cmdtp = &__u_boot_cmd_start; cmdtp != &__u_boot_cmd_end; cmdtp++) {

    clen = strlen(cmdtp->name);
    if (clen < len)
      continue;

    if (memcmp(cmd, cmdtp->name, len) != 0)
      continue;

    /* too many! */
    if (n_found >= maxv - 2) {
      cmdv[n_found++] = "...";
      break;
    }

    cmdv[n_found++] = cmdtp->name;
  }

  cmdv[n_found] = NULL;
  return n_found;
}

static int make_argv(char *s, int argvsz, char *argv[])
{
  int argc = 0;

  /* split into argv */
  while (argc < argvsz - 1) {

    /* skip any white space */
    while ((*s == ' ') || (*s == '\t'))
      ++s;

    if (*s == '\0')   /* end of s, no more args */
      break;

    argv[argc++] = s; /* begin of argument string */

    /* find end of string */
    while (*s && (*s != ' ') && (*s != '\t'))
      ++s;

    if (*s == '\0')   /* end of s, no more args */
      break;

    *s++ = '\0';    /* terminate current arg   */
  }
  argv[argc] = NULL;

  return argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep, int linemax, char *argv[])
{
  int ll = leader != NULL ? strlen(leader) : 0;
  int sl = sep != NULL ? strlen(sep) : 0;
  int len, i;

  if (banner) {
    fputs("\n", stdout);
    fputs(banner, stdout);
  }

  i = linemax;  /* force leader and newline */
  while (*argv != NULL) {
    len = strlen(*argv) + sl;
    if (i + len >= linemax) {
      fputs("\n", stdout);
      if (leader)
        fputs(leader, stdout);
      i = ll - sl;
    } else if (sep)
      fputs(sep, stdout);
    fputs(*argv++, stdout);
    i += len;
  }
  printf("\n");
}

static int find_common_prefix(char *argv[])
{
  int i, len;
  char *anchor, *s, *t;

  if (*argv == NULL)
    return 0;

  /* begin with max */
  anchor = *argv++;
  len = strlen(anchor);
  while ((t = *argv++) != NULL) {
    s = anchor;
    for (i = 0; i < len; i++, t++, s++) {
      if (*t != *s)
        break;
    }
    len = s - anchor;
  }
  return len;
}

static char tmp_buf[CONFIG_SYS_CBSIZE]; /* copy of console I/O buffer */

int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp)
{
  int n = *np, col = *colp;
  char *argv[CONFIG_SYS_MAXARGS + 1];   /* NULL terminated  */
  char *cmdv[20];
  char *s, *t;
  const char *sep;
  int i, j, k, len, seplen, argc;
  int cnt;
  char last_char;

  if (strcmp(prompt, CONFIG_SYS_PROMPT) != 0)
    return 0; /* not in normal console */

  cnt = strlen(buf);
  if (cnt >= 1)
    last_char = buf[cnt - 1];
  else
    last_char = '\0';

  /* copy to secondary buffer which will be affected */
  strcpy(tmp_buf, buf);

  /* separate into argv */
  argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

  /* do the completion and return the possible completions */
  i = complete_cmdv(argc, argv, last_char, sizeof(cmdv)/sizeof(cmdv[0]), cmdv);

  /* no match; bell and out */
  if (i == 0) {
    if (argc > 1) /* allow tab for non command */
      return 0;
    putc('\a', stdout);
    return 1;
  }

  s = NULL;
  len = 0;
  sep = NULL;
  seplen = 0;
  if (i == 1) { /* one match; perfect */
    k = strlen(argv[argc - 1]);
    s = cmdv[0] + k;
    len = strlen(s);
    sep = " ";
    seplen = 1;
  } else if (i > 1 && (j = find_common_prefix(cmdv)) != 0) {  /* more */
    k = strlen(argv[argc - 1]);
    j -= k;
    if (j > 0) {
      s = cmdv[0] + k;
      len = j;
    }
  }

  if (s != NULL) {
    k = len + seplen;
    /* make sure it fits */
    if (n + k >= CONFIG_SYS_CBSIZE - 2) {
      putc('\a', stdout);
      return 1;
    }

    t = buf + cnt;
    for (i = 0; i < len; i++)
      *t++ = *s++;
    if (sep != NULL)
      for (i = 0; i < seplen; i++)
        *t++ = sep[i];
    *t = '\0';
    n += k;
    col += k;
    fputs(t - k, stdout);
    if (sep == NULL)
      putc('\a', stdout);
    *np = n;
    *colp = col;
  } else {
    print_argv(NULL, "  ", " ", 78, cmdv);

    fputs(prompt, stdout);
    fputs(buf, stdout);
  }
  return 1;
}

#endif
