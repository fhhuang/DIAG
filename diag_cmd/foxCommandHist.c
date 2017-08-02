/**********************************************************************
 *	Command history
 *
 *	AUTOR:	SELETZ
 *
 *	Implements a simple command history
 *
 * Copyright (C) 2001 Stefan Eletzhofer <stefan.eletzhofer@www.eletztrick.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/**********************************************************************
 * Includes
 */
#include <string.h>
#include <foxCommandHist.h>

/**********************************************************************
 * Defines / Makros
 */

#define CMDHIST_DEBUG			0

#define MAX_HIST				32

/**********************************************************************
 * Programmglobale Variable
 */

/**********************************************************************
 * Modulglobale Variable
 */

static int	cmdhist_entries		= 0;
static int	cmdhist_read		= 0;
static int	cmdhist_write		= 0;
static char cmdhistory[MAX_HIST][MAX_COMMANDLINE_LENGTH];


/**********************************************************************
 * Prototypen
 */

/**********************************************************************
 * Exported functions
 */

/*********************************************************************
 * cmd_push
 *
 * AUTOR:		SELETZ
 * REVISED:
 *
 * Push a command to the history buffer
 *
 */
int cmdhist_push( char *cmd )
{
	if ( !cmd )
		return -EINVAL;

	if ( strlen( cmd ) > MAX_COMMANDLINE_LENGTH )
		return -EINVAL;

	if ( strlen( cmd ) == 0 )
		return 0;

	strncpy( cmdhistory[ cmdhist_write ], cmd, MAX_COMMANDLINE_LENGTH );

	cmdhist_write ++;
	cmdhist_write = cmdhist_write % MAX_HIST;

	if ( cmdhist_entries < MAX_HIST )
		cmdhist_entries++;

	return 0;
}


/*********************************************************************
 * cmdhist_reset
 *
 * AUTOR:		SELETZ
 * REVISED:
 *
 * Resets read ptr
 *
 */
int cmdhist_reset( void )
{
	cmdhist_read = cmdhist_write;

	return 0;
}



/*********************************************************************
 * cmd_next
 *
 * AUTOR:		seletz
 * REVISED:
 *
 * Gets next command in history
 *
 */
int cmdhist_next( char **cmd )
{
	int ptr;

	if ( !cmdhist_entries )
		return -EINVAL;

	if ( !cmd )
		return -EINVAL;

	ptr = cmdhist_read;
	
	if ( ptr == 0 ) {
		if ( cmdhist_entries != MAX_HIST )
			return -EINVAL;
		ptr = MAX_HIST - 1;
	} else {
		ptr--;
	}

	if ( !cmdhistory[ptr][0] )
		return -EINVAL;

	*cmd = cmdhistory[ptr];

	cmdhist_read = ptr;

	return 0;
}


/*********************************************************************
 * cmd_prev
 *
 * AUTOR:		SELETZ
 * REVISED:
 *
 * Gets previous command from history
 *
 */
int cmdhist_prev( char **cmd )
{
	int ptr;

	if ( !cmd )
		return -EINVAL;

	if ( !cmdhist_entries )
		return -EINVAL;
	
	ptr = cmdhist_read + 1;
	ptr = ptr % MAX_HIST;

	if ( ptr == cmdhist_write )
		return -EINVAL;

	if ( !cmdhistory[ptr][0] )
		return -EINVAL;

	*cmd = cmdhistory[ptr];

	cmdhist_read = ptr;

	return 0;
}

/**********************************************************************
 * Static functions
 */
/*********************************************************************
 * cmdhist_init
 *
 * AUTOR:		SELETZ
 * REVISED:
 *
 * Initializes this module
 *
 */
void cmdhist_init( void )
{
	int		i;

	cmdhist_read = 0;
	cmdhist_write = 0;
	cmdhist_entries = 0;

	i=MAX_HIST - 1;
	while ( i-- ) {
		cmdhistory[i][0]=0;
	}
}



