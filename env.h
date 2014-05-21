/* for ANSI C */


#define ANSI	1	


#ifdef ANSI
#define VOIDCAST (void *)
#else
#define VOIDCAST 
#endif
struct sstack 
{
	char Entry[512];
};

struct sstack ss[32];

