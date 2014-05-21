#include "cmd.h"
#include "exit_stat.h"

#define STOL_VERSION "1.0.0"

#define EMPTY '\0'
#define MENU_COUNT 25

static char     cbuf = EMPTY;

extern struct registers regs;
extern struct header *latest;
extern struct header *cword;
extern struct variable *rptr;
extern int      BackDropLoaded;

extern int      first;
extern int      slines;
extern int      scolumns;
extern int      fps;
extern char    *back_drop;

extern int      rs[];
extern int      ds[];
extern int      cs[];

// extern struct   sstack ss[512];
// extern struct   sstack *ss;

extern float    fs[];
char           *NumFind();

void            (**
		 poprs()) ();
extern void     (**mem[]) ();

extern char    pad[];
/*
 * extern char     ipformat[]; 
 */
extern void     restore();
extern char     keystroke();
extern int      TermOK;

int             docolon();
int             next();
int             exec();
int             semi();

int             IntRead();
int             IntWrite();

#if defined(FLOATS)
int             FloatRead();
int             FloatWrite();
#endif
int             StrRead();
int             StrWrite();

int             ArrayRead();
int             ArrayWrite();

char           *spop();
float           fpop();
char           *strsave();

char           *spdGetValue();
char           *spdGetPic();
struct position spdGetPos();
char           *spdGetList();

struct position
{
	int             x, y;
	int             status;
};

struct
{
	int             from, to;
	int             status;
}               range;


