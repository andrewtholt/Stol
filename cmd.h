/*
 * Header file for comms to spd (screen paint daemon when it exists)
 */


/*
 * offsets into m.msg
 */
#define CMD	0		/* spd command */
#define CMD_MOD	1		/* command modifier */
#define DATA	2		/* data (surprised huh) */

/*
 * commands
 */

#define CREATE	01		/* create field */
#define EDIT	02		/* edit field attributes */
#define DELETE	03		/* delete field */
#define LIST	04		/* list field (currently to server tty) */
#define SYSTEM	05		/* System config/control commands */
/*
	Connect and disconnect are the two commands that have an effect
	both for the agent and spd.  Therefore commands < CONECT are passed
	thru' to spd and commands > DISCONNECT are actioned by the agent.
*/
#define CONNECT 80		/* client connect to server	 */
#define AGENT 81		/* commands for the agent */
#define DISCONNECT 84		/* client disconnect from server */
#define SHUTDOWN 8		/* shutdown server on last client disconnect */
#define GET	9		/* get a fields attribute(s) */
#define FILE_OPS 10		/* file operations */
/*
 * command modifiers
 * 
 * for create
 */

#define FIELD		10
#define ALL_LIST	11
#define LIST_ITEM	12
#define FORM		13
/*
 * for edit and GET
 */

#define POSITION	20
#define TYPE		21
#define LEN		22
#define FORMAT		23
#define DEFAULT		24
#define RANGE		25
#define TABLE		26
#define COLUMN		27
#define PERMISSIONS	28
#define VALUE		29
#define DIRECTION	30
#define KEY		34
/*
 * For GET only
 */

#define BY_POSITION	35	/* Return a field name givin a position. If
				 * position is on any part of a field return
				 * name otherwise return (char *)NULL */
/*
 * The following allow you to get the name of the first field, the field
 * defined after, and the field defined before a specified field
 */

#define FIRST		31
#define NEXT		32
#define PREV		33
/*
 * For list
 */
#define ALL		40
#define ONE		41
/*
 * for delete use same definitions as edit
 * 
 * SYSTEM command modifiers start at 50
 */
#define VERBOSE 50		/* annouince all commands on stderr */
#define QUIET 51		/* error messages only */
#define NOW 52			/* on shutdown dont wait */
#define DBQ 53			/* Data base query */
#define ENABLE 54		/* enable connect */
#define DISABLE 55		/* disable connect */
#define WHO 56			/* list connections */
#define BREAK 57		/* Forcibly disconnect a process */
#define SYNC 58		/* Remove any connections to dead process's */
#define UCOUNT 59		/* Return no if connections */
#define DBUPDATE 60		/* used by database query process's.  Allows
				 * the update of screen fields not owned by
				 * the requesting process */

/*
 * File ops
 */
#define SAVE 70		/* save */
#define LOAD 71		/* load */

#define EXIT DISCONNECT+1	/* agent to exit */
