#define REGISTER 0
#define STRING 1
#define CHARACTER 2
#define INTEGER 3
#define FLOAT 4
#define MENU 5

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/*
	This is going to soound strange but here goes ...
	if the function variable is called with CONSTANT
	then it creates a constant.  If called with 
	VARIABLE it creates a variable.
	clear ?
*/

#define CONSTANT 0
#define VARIABLE 1
#define STRINGS 1

struct header
{
    unsigned char   len;
    char   *name;
    struct header  *lfa;
    int     cfa;
};

struct registers
{
    unsigned int    ip;
    void (*ca) ();
    unsigned int    wa;
    int     dsp;
    int     rsp;
    int     csp;		/* compile stack pointer */
#if defined(STRINGS)
    int     ssp;		/* string stack pointer */
#endif
    int     fence;		/* protect words below here */
    int     mode;
    int     state;
    int     current;
    int     context;
    int     base;
    int     dp;
    int     verbose;
    int     Trace;
    char   *lbp;
    char *ipformat;
    char *fpformat;
};

struct variable
{
    int     type;
            union
    {
	int     ivar;
	float   fvar;
	/*
	char   *string;
	*/
	char   string[1024];
	char    cvar;
    }               value;
    int     (*Write) ();
    int     (*Read) ();
};

struct array
{
    int     type;
    struct variable **member;
    int     (*Write) ();
    int     (*Read) ();
};

struct registers    regs;

