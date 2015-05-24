#include "primary.h"
#include "build_no.h"

#ifdef UNIX
#include <fcntl.h>
#endif

struct position pos;

#ifdef ANSI
void           *Find();
#endif

extern char    *getenv();
#define STRINGS 1

#if defined(STRINGS)
extern int      bufsplit(char *, int);
#endif

void            Creat();

int oldf;

#ifdef UNIX
int tstc() {
    char ch;

    fcntl(0, F_SETFL, oldf | O_NONBLOCK);

    ch = GETC();

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif

void Startup() {
    char ptr[32];
    int r=-1;

    strcpy(&ptr[1],"nvramrc");
    ptr[0]=7;

    spush(ptr);
    Getenv();

    r=pop();

    if(r ==0 ) {
        Eval();
        Load();
    }
}

next()
{
    if (regs.Trace)
    {
        char           *ptr;
        int             rep;

        ptr = NumFind(mem[regs.ip]);

        printf("\n%s\n", ptr);
#if defined(STRINGS)
        dotss();
#endif
        dots();
#if defined(FLOATS)
        fdots();
#endif
        if (regs.Trace != 2)
        {
            printf("q to quit 0 to turn off trace 1 to trace with no pause \n\n");
            key();
            rep = pop();
            if (rep == 'q')
                bye();
            else if (rep == '0')
                regs.Trace = 0;
            else if (rep == '1')
                regs.Trace = 2;
        }
    }
    regs.wa = (unsigned int) mem[regs.ip++];
    regs.ca = VOIDCAST mem[regs.wa++];

    (*regs.ca) ();
}

exec() {
    regs.wa = pop();
    regs.ca = VOIDCAST mem[regs.wa++];

    (*regs.ca) ();
}

void Exec() {
    if (regs.mode) {
        mem[regs.dp++] = Find("exec");
    } else {
        exec();
    }
}

void Eval () {
    if (regs.mode) {
        mem[regs.dp++] = Find("eval");
    } else {
        char *cmd;
        void *ca;

        regs.lbp = lb;
        regs.lbp[0] = 0x00;
        regs.lbp[1] = 0x00;

        cmd = spop();

        //        memset(regs.lbp,0x00,255);
        strncpy(regs.lbp, (cmd + 1),cmd[0]+1);
        regs.lbp[ cmd[0] ] = 0x00;
        regs.lbp[ cmd[0]+1 ] = ' ';
        tst();
        regs.lbp = lb;

        regs.lbp[0] = 0x00;
        regs.lbp[1] = 0x00;

        //        memset(regs.lbp,0x00,255);
    }
}

void Load() {
    char *ptr;

    int i=0;
    int j=0;
    int run=1;

    ptr=pop();

    while( run ) {
        regs.lbp = lb;
        for(i=0; i<255;i++) {
            if ( (ptr[i] < 0x7f) && (ptr[i] != 0xffffffff) ) {
                regs.lbp[i]= ptr[i];
                j++;

                if (ptr[i] == 0x0a) {
                    i++;
                    break;
                }
            } else if ( ptr[i] == 0xffffffff) {
                run=0;
            }
        }
        tst();
        ptr=ptr+i;
    }
}



void tst() {
    int len;

    while(token()) {
        len = pop();
        if (len > 0) {
            FindHeader();
            if(pop()) {
                regs.ip = pop();
                push(regs.ip);
                if ((mem[regs.ip] == VOIDCAST docolon) && regs.mode) {
                    mem[regs.dp++] = VOIDCAST pop();
                } else {
                    do {
                        if (regs.rsp > 0) {
                            next();
                        } else {
                            exec();
                        }
                    } while (regs.rsp > 0);
                }
            } else {
                int tmp;

                tmp=STRTOL(pad,NULL,regs.base);

                if ( ((tmp == 0) && (*pad == 0x30)) || ( tmp != 0)) {
                    if (!regs.mode) {
                        push(tmp);
                    } else {
                        lit();
                    }
                } else {
                    printf("Unknown word :%s:\n",pad);
                }
            }
        } else {
            drop();
        }
    }
}

dosemi() {
    regs.ip = (unsigned int) poprs();

    if (regs.rsp > 0) {
        regs.wa = (unsigned int) mem[regs.ip++];
        regs.ca = VOIDCAST mem[regs.wa++];

        (*regs.ca) ();
    }
}

semi() {
    if (regs.mode) {
        regs.mode = 0;
        mem[regs.dp++] = Find("(;)");
    } else
        printf("\n; Compile mode only\n");
}

docolon() {
    pushrs(regs.ip);
    regs.ip = regs.wa;

    regs.wa = (unsigned int) mem[regs.ip++];
    regs.ca = VOIDCAST mem[regs.wa++];

    (*regs.ca) ();
}


/*
pushrs(p)
    void            (**p) ();
    */
pushrs(void (**p) () )
{
    rs[regs.rsp] = (int) p;
    regs.rsp++;
}

    void            (**
            poprs()) ()
{
    regs.rsp--;
    return (VOIDCAST rs[regs.rsp]);
}

void tor() {
    if (regs.mode) {
        mem[regs.dp++] = Find(">r");
    } else {
        pushrs( pop() );
    }
}

void fromr() {
    if (regs.mode) {
        mem[regs.dp++] = Find("r>");
    } else {
        push( poprs() );
    }
}

push(int d) {
    ds[regs.dsp] = d;
    regs.dsp++;
}

#if defined(STRINGS)
#warning "STRINGS"
spush(char *d) {
    strcpy(&ss[regs.ssp], d);

    regs.ssp++;
}

char *spop() {
    char           *ptr;

    regs.ssp--;

    if (regs.ssp < 0) {
        regs.ssp = 0;
        printf("\nString Stack Underflow\n");
    } else {
        ptr = &(ss[regs.ssp].Entry);
    }

    return (ptr);
}
//
// Take a pointer to memory, on the data stack and the length and
// move to a counted string on the string stack
//
void mem2string() {
    char *ptr;          // Pointer to memory area
    char tmp[1024];
    unsigned char term; // The charcter that marks the end
    int maxLen;         // Maximum, anticipated lemgth (i.e. <= )
    int i=0;

    maxLen=(int)pop();

    maxLen = ( maxLen > 1024 ) ? 1024 : maxLen;

    term = (pop() & 0xff);
    ptr=(char *)pop();

    for(i=0; i < maxLen; i++) {
        if(ptr[i] == 0xffffffff) {
            tmp[i] = 0x00;
            break;
        } else {
            tmp[i] = *(ptr + i);
        }
    }
    printf("len=%d\n",strlen(tmp));
    spush( &tmp[0] );
}
//
// On the data stack place a pointer to the string currently 
// on top of the string stack.
//
void stringPtr() {
    if (regs.mode) {
        mem[regs.dp++] = Find("string-ptr");
    } else {
        char *ptr;
        ptr = &(ss[regs.ssp-1].Entry);

        push( ptr );
    }
}

#ifdef UBOOT
void ubootSystem() {    

    if (regs.mode) {
        mem[regs.dp++] = Find("$system");
    } else {
        char *ptr;

        ptr=spop();

        //        printf("U-Boot cmd is [%02d] >%s<\n",ptr[0],ptr+1);
        push( RunCmd(ptr+1) );
    }
}

void ubootSaveenv() {
    if (regs.mode) {
        mem[regs.dp++] = Find("saveenv");
    } else {
        //        saveenv();
        spush( RunCmd( "saveenv") );
    }
}

void ubootSetenv() {
    if (regs.mode) {
        mem[regs.dp++] = Find("$setenv");
    } else {
        char *name;
        char *value;

        value=spop();
        name=spop();
        setenv((name+1),(value+1));
    }
}

#endif

#endif

dotVersion() {
    if (regs.mode) {
        mem[regs.dp++] = Find(".version");
    } else {
        printf("\n\tVersion Number: %s", STOL_VERSION);
        printf("\n\tBuild   Number: %d", BUILD_NO);
        printf("\n");
    }
}

getBuildNo() {
    if (regs.mode) {
        mem[regs.dp++] = Find("build-no");
    } else {
        push( BUILD_NO );
    }
}

#if defined(FLOATS)

fequal()
{
    if (regs.mode)
        mem[regs.dp++] = Find("f=");
    else
    {
        float           t;

        t = fpop();

        push(t == fpop());
    }
}

fmul()
{
    float           t;

    if (regs.mode)
        mem[regs.dp++] = Find("f*");
    else
    {
        t = fpop();
        t *= fpop();
        fpush(t);
    }
}

fsub()
{
    if (regs.mode)
        mem[regs.dp++] = Find("f-");
    else
    {
        float           a;
        float           b;
        float           t;

        b = fpop();
        a = fpop();
        t = a - b;

        fpush(t);
    }
}

fdiv()
{
    float           t;

    if (regs.mode)
        mem[regs.dp++] = Find("f/");
    else
    {
        t = fpop();

        t = fpop() / t;
        fpush(t);
    }
}

fgt()
{

    if (regs.mode)
        mem[regs.dp++] = Find("f>");
    else
    {
        float           a, b;
        a = fpop();
        b = fpop();
        push(b > a);
    }
}

Flt()
{
    if (regs.mode)
        mem[regs.dp++] = Find("f<");
    else
    {
        float           a, b;

        a = fpop();
        b = fpop();
        push(b < a);
    }
}


fadd()
{
    float           t;

    if (regs.mode)
        mem[regs.dp++] = Find("f+");
    else
    {
        t = fpop();
        t += fpop();
        fpush(t);
    }
}

fdot()
{
    if (regs.mode)
        mem[regs.dp++] = Find("f.");
    else
    {
        float           t;

        if (regs.fsp == 0)
            printf("\nStack empty\n");
        else
        {
            t = fpop();
            printf(regs.fpformat, t);
        }
    }
}


fdots()
{
    if (regs.mode)
        mem[regs.dp++] = Find(".fs");
    else
    {
        int             i;
        if (regs.fsp > 0)
        {
            printf("Top");
            for (i = regs.fsp - 1; i >= 0; i--)
            {
                PUTC('\t');
                printf(regs.fpformat, fs[i]);
                PUTC('\n');
            }
        } else
            printf("Empty\n");
    }
}


fempty()
{
    if (regs.mode)
        mem[regs.dp++] = Find("fempty");
    else
        regs.fsp = 0;
}


fswap()
{
    if (regs.mode)
        mem[regs.dp++] = Find("fswap");
    else
    {
        float           a;
        float           b;

        a = fpop();
        b = fpop();
        fpush(a);
        fpush(b);
    }
}

fdepth()
{
    if (regs.mode)
        mem[regs.dp++] = Find("fdepth");
    else
        push(regs.fsp);
}


fdup()
{
    float           t;

    if (regs.mode)
        mem[regs.dp++] = Find("fdup");
    else
    {
        t = fpop();
        fpush(t);
        fpush(t);
    }
}


fpick()
{
    if (regs.mode)
        mem[regs.dp++] = Find("fpick");
    else
    {
        int             n;
        float           t;

        n = pop();
        t = fs[regs.fsp - n];
        fpush(t);
    }
}


frot()
{
    if (regs.mode)
        mem[regs.dp++] = Find("frot");
    else
    {
        float           n;

        n = fs[2];
        fs[2] = fs[0];
        fs[0] = fs[1];
        fs[1] = n;
    }
}


fdrop()
{
    if (regs.mode)
        mem[regs.dp++] = Find("fdrop");
    else
    {
        regs.fsp--;
        if (regs.fsp < 0)
            regs.fsp = 0;
    }
}

fpush(d)
    float           d;
{
    fs[regs.fsp] = d;
    regs.fsp++;
}

    float
fpop()
{
    regs.fsp--;
    if (regs.fsp < 0)
    {
        regs.fsp = 0;
        printf("\nFloating point stack Underflow\007\n");
    }
    return (fs[regs.fsp]);
}
#endif

pop()
{
    regs.dsp--;
    if (regs.dsp < 0)
    {
        regs.dsp = 0;
        printf("\nData (integer) stack Underflow\007\n");

    }
    return (ds[regs.dsp]);
}


pushcs(d)
    int             d;
{
    cs[regs.csp] = d;
    regs.csp++;
}

popcs()
{
    regs.csp--;
    return (cs[regs.csp]);
}

drop()
{
    if (regs.mode) {
        mem[regs.dp++] = Find("drop");
    } else {
        regs.dsp--;
        if (regs.dsp < 0)
            regs.dsp = 0;
    }
}

#if defined(STRINGS)
sdrop() {
    char           *p;

    if (regs.mode) {
        mem[regs.dp++] = Find("sdrop");
    } else {
        char           *tmp;

        if (regs.ssp > 0)
            regs.ssp--;
    }
}

sdup() {
    char           *t, *p;

    if (regs.mode) {
        mem[regs.dp++] = Find("sdup");
    } else {
        strcpy(&ss[regs.ssp], &ss[regs.ssp - 1]);
        regs.ssp++;

    }
}

sswap() {
    if (regs.mode)
        mem[regs.dp++] = Find("sswap");
    else
    {
        char            scratch[512];


        strcpy(scratch, ss[regs.ssp - 1].Entry);
        strcpy(ss[regs.ssp - 1].Entry, ss[regs.ssp - 2].Entry);
        strcpy(ss[regs.ssp - 2].Entry, scratch);
    }
}

#endif

void rot() {
    if (regs.mode) {
        mem[regs.dp++] = Find("rot");
    } else {
        int             n;

        n = ds[regs.dsp - 2];
        ds[regs.dsp - 2] = ds[regs.dsp - 1];
        ds[regs.dsp - 1] = ds[regs.dsp - 3];
        ds[regs.dsp - 3] = n;
    }
}

pick() {
    if (regs.mode) {
        mem[regs.dp++] = Find("pick");
    } else {
        int             n;
        int             t;

        n = pop();
        t = ds[regs.dsp - n];
        push(t);
    }
}

void nip() {
    if (regs.mode) {
        mem[regs.dp++] = Find("nip");
    } else {
        unsigned int a,b;

        a=pop();
        b=pop();
        push(a);
    }
}

void tuck() {
    if (regs.mode) {
        mem[regs.dp++] = Find("tuck");
    } else {
        unsigned a,b;

        a=pop();
        b=pop();

        push(a);
        push(b);
        push(a);
    }
}

void Dup() {
    if (regs.mode) {
        mem[regs.dp++] = Find("dup");
    } else {
        int a;
        a=pop();
        push(a);
        push(a);
    }
}

void bounds() {
    if (regs.mode) {
        mem[regs.dp++] = Find("bounds");
    } else {
        unsigned int a,b;

        b=pop();    // len
        a=pop();    // start

        push(a+b);
        push(a);
    }
}

void swap() {
    if (regs.mode) {
        mem[regs.dp++] = Find("swap");
    } else {
        int             a;
        int             b;

        a = pop();
        b = pop();
        push(a);
        push(b);
    }
}

void over() {
    if (regs.mode) {
        mem[regs.dp++] = Find("over");
    } else {
        unsigned int a,b;

        b=pop();
        a=pop();

        push(a);
        push(b);
        push(a);
    }
}

void depth() {
    if (regs.mode) {
        mem[regs.dp++] = Find("depth");
    } else {
        push(regs.dsp);
    }
}

#if defined(STRINGS)
sdepth()
{
    if (regs.mode)
        mem[regs.dp++] = Find("sdepth");
    else
        push(regs.ssp);
}
#endif

void empty() {
    if (regs.mode) {
        mem[regs.dp++] = Find("empty");
    } else {
        regs.dsp = 0;
    }
}

#if defined(STRINGS)
sempty()
{
    if (regs.mode)
        mem[regs.dp++] = Find("sempty");
    else
        regs.ssp = 0;
}
#endif

dotq()
{
    if (regs.mode) {
        char           *ptr, *keep;
        int             count = 0;

        mem[regs.dp++] = Find(".\"");

        regs.lbp++;
        keep = regs.lbp;

        while (*(regs.lbp++) != '"')
            count++;

        ptr = (char *) malloc(count + 1);

        if (!ptr)
            printf("Malloc in dotq");

        strncpy(ptr, keep, count);
        *(ptr + count) = '\0';

        mem[regs.dp++] = VOIDCAST ptr;
    } else {
        /*
         * fputs((char *) mem[regs.ip++], stdout);
         */
        printf((char *) mem[regs.ip++]);
    }
}

#ifdef UBOOT
void ubootABI() {
    if (regs.mode) {
        mem[regs.dp++] = Find("?abi");
    } else {
        push( (int)get_version());
    }
}
#endif

dots() {
    if (regs.mode) {
        mem[regs.dp++] = Find(".s");
    } else {
        int             i;

        if (regs.dsp > 0) {
            printf("Top");
            for (i = regs.dsp - 1; i >= 0; i--)
                printf("\t%d\t%04x\n", ds[i], ds[i]);
        } else {
            printf("Empty\n");
        }
    }
}

dot() {
    if (regs.mode) {
        mem[regs.dp++] = Find(".");
    } else {
        int             t;

        if (regs.dsp == 0) {
            printf("\nStack empty\n");
        } else {
            t = pop();
            printf(&regs.ipformat[1], t);
            fflush(stdout);
        }
    }
}

#if defined(STRINGS)
void sdot() {
    char           *t;
    int l=0;

    if (regs.mode) {
        mem[regs.dp++] = Find("type");
    } else {
        if (regs.ssp == 0) {
            printf("\nString Stack empty\n");
        } else {
            t = spop();
            l = t[0];
            t++;
            printf("%s", t);
        }
    }
}
#endif

lit() {

    if (regs.mode) {
        mem[regs.dp++] = Find("(lit)");
        mem[regs.dp++] = VOIDCAST STRTOL(pad,NULL,regs.base);
    } else {
        push(mem[regs.ip++]);
    }
}

slit() {

    if (regs.mode) {
        mem[regs.dp++] = Find("(lit)");
        mem[regs.dp++] = strsave(pad);
    } else {
        char           *ptr;

        /*
           ptr = (char *) strsave(mem[regs.ip++]);
           push(ptr);
           */
        ptr = (char *) mem[regs.ip++];
        spush(ptr);
    }
}

void Min() {
    if (regs.mode) {
        mem[regs.dp++] = Find("min");
    } else {
        int a,b;

        a = pop();
        b = pop();

        push( (a>b) ? b : a );
    }
}

oneplus() {
    int             t;

    if (regs.mode) {
        mem[regs.dp++] = Find("1+");
    } else {
        t = pop();
        t++;
        push(t);
    }
}

oneminus()
{
    int             t;

    if (regs.mode)
        mem[regs.dp++] = Find("1-");
    else
    {
        t = pop();
        t--;
        push(t);
    }
}

sub()
{
    if (regs.mode)
        mem[regs.dp++] = Find("-");
    else
    {
        int             a;
        int             b;
        int             t;

        b = pop();
        a = pop();
        t = a - b;

        push(t);
    }
}

add()
{
    int             t;

    if (regs.mode)
        mem[regs.dp++] = Find("+");
    else
    {
        t = pop();
        t += pop();
        push(t);
    }
}

mul()
{
    int             t;

    if (regs.mode)
        mem[regs.dp++] = Find("*");
    else
    {
        t = pop();
        t *= pop();
        push(t);
    }
}

div()
{
    int             t;

    if (regs.mode)
        mem[regs.dp++] = Find("/");
    else
    {
        t = pop();

        t = pop() / t;
        push(t);
    }
}

mod()
{
    int             t;

    if (regs.mode)
        mem[regs.dp++] = Find("mod");
    else
    {
        t = pop();

        t = pop() % t;
        push(t);
    }
}

gt()
{
    int             a, b;

    if (regs.mode)
        mem[regs.dp++] = Find(">");
    else
    {
        a = pop();
        b = pop();
        push(b > a);
    }
}

lt()
{
    int             a, b;

    if (regs.mode)
        mem[regs.dp++] = Find("<");
    else
    {
        a = pop();
        b = pop();
        push(b < a);
    }
}

zequ() {
    if (regs.mode) {
        mem[regs.dp++] = Find("0=");
    } else {
        if (pop())
            push(0);
        else
            push(1);
    }
}

void znequ() {
    if (regs.mode) {
        mem[regs.dp++] = Find("0<>");
    } else {
        if (pop()) {
            push(1);
        } else {
            push(0);
        }
    }
}

void not() {
    int             t;

    if (regs.mode) {
        mem[regs.dp++] = Find("not");
    } else {
        t = pop();

        push(~t);
    }
}

void and() {
    if (regs.mode) {
        mem[regs.dp++] = Find("and");
    } else {
        int             t;
        t = pop();
        push(t & pop());
    }
}


void or() {
    int             t;
    if (regs.mode) {
        mem[regs.dp++] = Find("or");
    } else {
        t = pop();
        push(t | pop());
    }
}

void xor() {
    int             a;
    int             b;

    if (regs.mode) {
        mem[regs.dp++] = Find("xor");
    } else {
        a = pop();
        b = pop();

        push((a & ~b) | (~a & b));
    }
}

mon() {
    extern int rc;
    //    printf("MON\n");

    if (regs.mode) {
        mem[regs.dp++] = Find("mon");
    } else {
        exitFlag=1;
        rc=pop();
        //        return(0);
    }
}

bye() {
    extern int rc;
    //    printf("BYE\n");
    if (regs.mode) {
        mem[regs.dp++] = Find("bye");
    } else {
        exitFlag=1;
        rc=0;
    }
}

#if defined(STRINGS)
/*
   System()
   {
   if (regs.mode)
   mem[regs.dp++] = Find("system");
   else
   {
   char           *ptr;

   ptr = spop();
   push(system(ptr + 1));
//free(ptr);
}
}
*/

void Getenv() {
    if (regs.mode) {
        mem[regs.dp++] = Find("$getenv");
    } else {
        char           *ptr;
        char *tmp;
        char *n;
        int i=0;
        int l;
        char env[1024];

        tmp=spop();

        strcpy(pad,&tmp[1]);
        ptr = getenv(pad);


        if (!ptr) {
            push(1);
        } else {
            env[0]=strlen(ptr);
            strcpy(&env[1],ptr);
            spush(env);
            push(0);
        }
    }
}
#endif
/*
 * Function : token
 *
 * Description:
 *
 * breaks an input line into tokens.  Returns the token length as the top stack
 * entry. if no more tokens on line returns 0.
 */
int token() {
    char            sep;
    char           *ptr;
    int             count = 0;

    ptr = pad;

    while (*regs.lbp <= 0x20)
    {
        if ( (*regs.lbp == '\n' ) || (*regs.lbp == '\r')) {
            return (0);
        } else if (*regs.lbp == 0x00) {
            return(0);
        }
        regs.lbp++;
    }

    while (*regs.lbp > ' ')
    {
        count++;
        *ptr = *regs.lbp;
        ptr++;
        regs.lbp++;
    }
    *ptr = '\0';

    push(count);
    return (count);
}

colon()
{
    int             len;
    struct header  *ptr;

    token();
    len = pop();

    if (len > 0)
    {
        ptr = latest;
        while ((strcmp("(:)", ptr->name)) && ptr != 0)
            ptr = ptr->lfa;

        regs.mode = 1;
        MakeHeader(pad, regs.dp);
        mem[regs.dp] = mem[ptr->cfa];
        regs.dp++;
    }
}

equal()
{
    if (regs.mode)
        mem[regs.dp++] = Find("=");
    else
    {
        int             t;

        t = pop();

        push(t == pop());
    }
}


void regdump()
{
    int i=0;


    printf("\t\tSTOL register dump\n\n");
    printf("\tip\t\t%d\n", regs.ip);
    printf("\twa\t\t%d\n", regs.wa);
    printf("\tca\t\t%d\t%08x\n", regs.ca, regs.ca);
    printf("\tdsp\t\t%d\n", regs.dsp);
    printf("\trsp\t\t%d\n", regs.rsp);
    printf("\tssp\t\t%d\n", regs.ssp);
    printf("\tcsp\t\t%d\n", regs.csp);
    printf("\tmode\t\t%d\n", regs.mode);
    printf("\tstate\t\t%d\n", regs.state);
    printf("\tcurrent\t\t%d\n", regs.current);
    printf("\tcontext\t\t%d\n", regs.context);
    printf("\tlatest\t\t%d\t%08x\n", latest, latest);
    printf("\tdp\t\t%d\n", regs.dp);
    printf("\tfence\t\t%d\n", regs.fence);
    printf("\tlbp\t\t%x\n",regs.lbp);
    status();

    /*
       if( regs.ssp > 0) {
       printf("  String Stack top >%s<\n", ss[0]);
       }
       */
}

status()
{
    printf("\tbase\t\t%2d\n", regs.base);
    printf("\tfpformat\t%s\n", regs.fpformat);
    printf("\tipformat\t%s\n", regs.ipformat);
    printf("\tVerbose\t\t");
    if (regs.verbose)
        printf("Yes");
    else
        printf("No");
    printf("\n");

}

plist()
{
    int             count = 0;
    char            rep;
    struct header  *ptr;

#if defined(TERMCAP)
    cls();
#endif
    ptr = latest;

    PUTC('\n');

    while (ptr)
    {
        {
            count++;
            if (count >= (slines - 1))
            {
                count = 0;
                printf("\tPress RETURN to continue, or q to quit ");

                rep = GETC();

                if (rep == 'q' || rep == 'Q')
                    return;
            }
            printf("%-32s\t%4d\n", ptr->name, ptr->cfa);
        }
        ptr = ptr->lfa;
    }
}

vlist() {
    struct header  *ptr;

#if defined(TERMCAP)
    cls();
#endif

    ptr = latest;
    PUTC('\n');
    while (ptr)
    {
        printf("%-32s\t%4d\n", ptr->name, ptr->cfa);
        ptr = ptr->lfa;
    }
}

#ifdef ANSI
    void           *
Find(name)
#else
Find(name)
#endif
    char           *name;
{
    struct header  *ptr;

    ptr = latest;

    while ((strcmp(name, ptr->name)) && ptr->lfa != 0)
        ptr = ptr->lfa;

    return (VOIDCAST ptr->cfa);
}
TraceOn()
{
    if (regs.mode)
        mem[regs.dp++] = Find("traceon");
    else
        regs.Trace = 1;
}


TraceOff()
{
    if (regs.mode)
        mem[regs.dp++] = Find("traceoff");
    else
        regs.Trace = 0;
}

TraceNoPause()
{
    if (regs.mode)
        mem[regs.dp++] = Find("trace");
    else
        regs.Trace = 2;
}



    char           *
NumFind(num)
    int             num;
{
    struct header  *ptr;
    ptr = latest;

    while ((ptr->cfa != num) && (ptr->lfa != 0))
        ptr = ptr->lfa;
    return (ptr->name);
}


/*
 * Conditional constructs
 */

fif()
{
    if (regs.mode)
    {
        mem[regs.dp++] = Find("if");
        pushcs(regs.dp);
        regs.dp++;
    } else
    {
        int             t;
        int             a;
        int             b;

        t = pop();

        if (t == 0)
        {
            a = regs.ip;
            b = (int) mem[regs.ip];
            regs.ip = a + b;
        } else
            regs.ip++;
    }
}

ffi()
{
    if (regs.mode)
    {
        int             a, b;

        b = popcs();
        a = regs.dp - b;
        mem[b] = VOIDCAST a;
    } else
        printf("fi has no immediate action\n");
}

mdump() {
    if (regs.mode) {
        mem[regs.dp++] = Find("dump");
    } else {

        unsigned char  *from;
        int             count;
        unsigned int    i;
        int             j;
        unsigned char c;

        count = pop();
        from = pop();

        count= (count+0x0f) & 0xfff0;


        for(i=from; i < (from+count); i=i+0x10) {
            printf("%08x:",i);
            //
            // Print the hex values
            //
            for (j=0;j<0x10;j++) {
                printf("%02x ",(*(char *)(i + j)) & 0xff);
            }


            printf(":");
            for (j=0;j<0x10;j++) {
                c=(*(char *)(i + j)) & 0xff;

                if (c < 0x20) {
                    PUTC('.');
                } else {
                    PUTC(c);
                }
            }
            printf(":\n");
        }
    }
}

felse()
{
    if (regs.mode)
    {
        int             a, b;

        /*
         * mem[regs.dp++] = Find("(;)");
         */
        mem[regs.dp++] = Find("else");
        b = popcs();
        pushcs(regs.dp);

        regs.dp++;
        a = regs.dp - b;
        mem[b] = VOIDCAST a;
    } else
    {
        int             t;
        int             a;
        int             b;

        /*
         * t = pop(); if (t == 0) {
         */
        a = regs.ip;
        b = (int) mem[regs.ip];
        regs.ip = a + b;
        /*
         * } else regs.ip++;
         */
    }
}

fbegin()
{
    if (regs.mode)
        pushcs(regs.dp);
}

funtil()
{
    if (regs.mode)
    {
        int             a;
        int             b;

        mem[regs.dp++] = Find("until");
        a = regs.dp;
        b = popcs();
        mem[regs.dp++] = VOIDCAST(b - a);
    } else
    {
        int             t;
        int             a, b;

        t = pop();

        if (!t)
        {
            a = (int) mem[regs.ip];
            b = regs.ip;

            regs.ip = a + b;
        } else
            regs.ip++;
    }
}

again()
{
    if (regs.mode)
    {
        int             a;
        int             b;

        mem[regs.dp++] = Find("again");
        a = regs.dp;
        b = popcs();
        mem[regs.dp++] = VOIDCAST(b - a);
    } else
    {
        int             a, b;

        a = (int) mem[regs.ip];
        b = regs.ip;

        regs.ip = a + b;
    }
}

End()
{
    if (regs.mode)
    {
        int             i, j;

        mem[regs.dp++] = Find("end");

        i = popcs();
        mem[i] = VOIDCAST((regs.dp + 1) - i);

        i = popcs();
        mem[regs.dp++] = VOIDCAST(i - regs.dp + 1);
    } else
    {
        int             a, b;

        a = (int) mem[regs.ip];
        b = regs.ip;

        regs.ip = a + b;
    }
}

fdo()
{
    if (regs.mode)
    {
        mem[regs.dp++] = Find("do");
        pushcs(regs.dp);
    } else
    {
        int             term, from;

        term = pop();
        from = pop();

        pushcs(from);
        pushcs(term);
    }
}

ploop()
{
    int             a, b, c;
    if (regs.mode)
    {
        mem[regs.dp++] = Find("+loop");
        a = popcs();
        b = regs.dp;
        mem[regs.dp++] = VOIDCAST(a - b);
    } else
    {
        a = pop();
        b = popcs();
        c = popcs();

        b = b + a;

        if (b > c)
            regs.ip++;
        else
        {
            pushcs(c);
            pushcs(b);
            a = (int) mem[regs.ip];
            b = regs.ip;
            regs.ip = a + b;
        }
    }
}

Break()
{
    if (regs.mode)
        mem[regs.dp++] = Find("break");
    else
    {
        int             count, term;
        count = popcs();
        term = popcs();

        pushcs(term);
        pushcs(term);
    }
}

loop()
{
    int             a, b, c;
    if (regs.mode)
    {
        mem[regs.dp++] = Find("loop");
        a = popcs();
        b = regs.dp;
        mem[regs.dp++] = VOIDCAST(a - b);
    } else
    {
        b = popcs();
        c = popcs();

        b++;

        if (b > c)
            regs.ip++;
        else
        {
            pushcs(c);
            pushcs(b);
            a = (int) mem[regs.ip];
            b = regs.ip;
            regs.ip = a + b;
        }
    }
}


findex()
{
    if (regs.mode)
        mem[regs.dp++] = Find("i");
    else
    {
        int             a;
        a = popcs();
        pushcs(a);
        push(a);
    }
}

cr()
{
    if (regs.mode)
        mem[regs.dp++] = Find("cr");
    else
        printf("\n");
}

hex()
{
    if (regs.mode)
        mem[regs.dp++] = Find("hex");
    else
    {
        strcpy(regs.ipformat, "%x");
        regs.base = 16;
    }
}

decimal()
{
    if (regs.mode)
        mem[regs.dp++] = Find("decimal");
    else
    {
        strcpy(regs.ipformat, "%d");
        regs.base = 10;
    }
}

octal()
{
    if (regs.mode)
        mem[regs.dp++] = Find("octal");
    else
    {
        strcpy(regs.ipformat, "%o");
        regs.base = 8;
    }
}

emit()
{
    if (regs.mode)
        mem[regs.dp++] = Find("emit");
    else
    {
        int             a;
        a = pop();
        PUTC(a);
    }
}

key()
{
    if (regs.mode)
        mem[regs.dp++] = Find("key");
    else
    {
        int i;

        if (cbuf != EMPTY)
        {
            push(cbuf);
            cbuf = EMPTY;
        } else {
            //            tty_raw();
            //            nonblock(0);
            i=GETC();
            push(i);
        }
    }
}

qterm()
{
    if (regs.mode)
        mem[regs.dp++] = Find("?terminal");
    else
    {
        /*
           tty_raw();
        //            nonblock(0);

        if(kbhit() !=0 )
        {
        cbuf=fGETC(stdin);
        push(-1);
        } else {
        cbuf = EMPTY;
        push(0);
        }
        //            tty_reset();
        //            nonblock(1);
        */
    }
}



cget()
{
    char            c;

    if (cbuf != EMPTY)
    {
        c = cbuf;
        cbuf = EMPTY;
        return (c & 0377);
    }
    //    setblock(0, 1);
    switch (GETC())
    {
        case -1:
            printf("cget read fail\n");
            return(1);
        case 0:
            return (-1);
        default:
            return (c & 0377);
    }
}

Ascii()
{
    int             len;
    int             c;

    token();
    len = pop();
    if (len <= 0)
    {
        printf("No token following ascii\n");
        return(1);
    }
    c = *pad;

    if (regs.mode)
    {
        mem[regs.dp++] = Find("(lit)");
        mem[regs.dp++] = VOIDCAST c;
    } else
        push(c);
}


integer()
{
    push(INTEGER);
}

str()
{
    push(STRING);
}

flt()
{
    push(FLOAT);
}


variable()
{
    fvariable(VARIABLE);
}

constant() {
    fvariable(CONSTANT);
}

fvariable( int mode) {
    if (regs.mode == 0) {
        token(); // get the variables name

        if (pop())
        {
            int             n;
            struct variable *vptr;
            vptr = (struct variable *) malloc(sizeof(struct variable));
            if (!vptr) {
                printf("variable: Failed to allocate memory\n");
            } else {
                vptr->type = pop();

                switch (vptr->type) {
                    case REGISTER:
                        rptr = vptr;
                        break;
                    case INTEGER:
                        vptr->Read = IntRead;
                        if (mode == VARIABLE) {
                            vptr->Write = IntWrite;
                        } else {
                            vptr->Write = NULL;
                            vptr->value.ivar = pop();
                        }
                        break;
#if defined(STRINGS)
                    case STRING:
                        vptr->Read = StrRead;
                        if (mode == VARIABLE) {
                            vptr->Write = StrWrite;
                        } else {
                            vptr->Write = NULL;
                            /*
                             * vptr->value.string =
                             * spop();
                             */
                            strcpy(vptr->value.string, spop());
                        }
                        break;
#endif
#if defined(FLOATS)
                    case FLOAT:
                        vptr->Read = FloatRead;
                        if (mode == VARIABLE) {
                            vptr->Write = FloatWrite;
                        } else {
                            vptr->Write = NULL;
                            vptr->value.fvar = fpop();
                        }
                        break;
#endif
                    case CHARACTER:
                    default:
                        vptr->Read = NULL;
                        vptr->Write = NULL;
                        break;
                }

                rptr = vptr;
                MakeHeader(pad, regs.dp);
                mem[regs.dp++] = VOIDCAST docolon;
                mem[regs.dp++] = Find("(lit)");

                if(mode == VARIABLE) {
                    mem[regs.dp++] = VOIDCAST vptr;
                } else {
                    mem[regs.dp++] = VOIDCAST vptr->value.ivar;
                }
                mem[regs.dp++] = Find("(;)");
            }
        }
    } else
        printf("variable has no compile time action\b");
}

farray()
{
    if (regs.mode == 0)
    {
        token();

        if (pop())
        {
            int             n;

            struct array   *vptr;
            vptr = (struct array *) malloc(sizeof(struct array));
            if (!vptr)
                printf("array: Failed to allocate memory\n");
            else
            {
                int             i;
                struct variable *tmp;

                vptr->type = pop();

                /* get number of elements */
                n = pop();
                vptr->member = (struct variable *) malloc(n * sizeof(struct variable));
                if (vptr->member == 0)
                {
                    printf("array: Failed to allocate memory for members\n");
                    return;
                }
                for (i = 0; i < n; i++)
                {
                    vptr->member[i] = (struct variable *) malloc(sizeof(struct variable));
                    if (vptr->member[i])
                    {
                        vptr->member[i]->type = vptr->type;
                    } else
                        printf( "array: Failed to allocate memory for member %d\n", i);


                    switch (vptr->member[i]->type)
                    {
                        case REGISTER:
                            rptr = vptr;
                            break;
                        case INTEGER:
                            vptr->member[i]->Read = IntRead;
                            vptr->member[i]->Write = IntWrite;
                            break;
#if defined(STRINGS)
                        case STRING:
                            vptr->member[i]->Read = StrRead;
                            vptr->member[i]->Write = StrWrite;
                            break;
#endif
#if defined(FLOATS)
                        case FLOAT:
                            vptr->member[i]->Read = FloatRead;
                            vptr->member[i]->Write = FloatWrite;
                            break;
#endif
                        case CHARACTER:
                        default:
                            vptr->member[i]->Read = NULL;
                            vptr->member[i]->Write = NULL;
                            break;
                    }
                }

                vptr->Read = ArrayRead;
                vptr->Write = ArrayWrite;


                MakeHeader(pad, regs.dp);
                mem[regs.dp++] = docolon;
                mem[regs.dp++] = Find("(lit)");
                mem[regs.dp++] = vptr;
                mem[regs.dp++] = Find("(ArrayFix)");
                mem[regs.dp++] = Find("(;)");
            }
        }
    } else
        printf( "variable has no compile time action\b");
}

ArrayFix()
{
    int             Element;
    struct array   *ArrayPtr;
    struct array   *VarPtr;

    ArrayPtr = (struct array *) pop();
    Element = pop();

    VarPtr = ArrayPtr->member[Element];
    push(VarPtr);
}

bvariable()
{
    if (regs.mode)
        mem[regs.dp++] = Find(pad);
    else
    {
        regs.ip++;
        push(mem[regs.ip++]);
    }
}

IntRead()
{
    struct variable *ptr;

    ptr = (struct variable *) pop();

    push(ptr->value.ivar);
}

IntWrite()
{
    struct variable *ptr;

    ptr = (struct variable *) pop();
    ptr->value.ivar = pop();
}


#if defined(FLOATS)
FloatRead()
{
    struct variable *ptr;

    ptr = (struct variable *) pop();

    fpush(ptr->value.fvar);
}

FloatWrite()
{
    struct variable *ptr;

    ptr = (struct variable *) pop();
    ptr->value.fvar = fpop();
}

#endif

#if defined(STRINGS)
StrRead()
{
    struct variable *ptr;
    char           *str;

    ptr = (struct variable *) pop();
    /*
       str = (char *) strsave(ptr->value.string);
       if (!str)
       printf( "StrRead malloc fail\n");
       */
    spush(&(ptr->value.string));
}

StrWrite()
{
    struct variable *ptr;
    char           *sptr;

    ptr = (struct variable *) pop();
    /*
     * if (ptr->value.string) free(ptr->value.string);
     *
     * ptr->value.string = spop();
     regs.ssp--;
     strcpy(ptr->value.string, ss[regs.ssp].Entry);
     */
    strcpy(ptr->value.string, spop());

}
#endif

ArrayRead()
{
    struct array   *ptr;
    int             n;

    ptr = (struct array *) pop();
    n = pop();
    push(ptr->member[n]);
    (*(ptr->member[n]->Read)) ();
}

ArrayWrite()
{
    struct array   *ptr;
    int             n;

    ptr = (struct array *) pop();
    n = pop();
    push(ptr->member[n]);
    (*(ptr->member[n]->Write)) ();
}

wmemRead()
{
    if (regs.mode)
        mem[regs.dp++] = Find("w@");
    else
    {
        unsigned short int *ptr;
        unsigned short int val;

        ptr = pop();

        val= *ptr;
        push( val & 0xffff);

        //        ptr = ((unsigned short int *) pop() & (unsigned short int *)0xffff);
        //        push(*ptr);

    }
}

wmemWrite()
{
    if (regs.mode) {
        mem[regs.dp++] = Find("c!");
    } else {
        unsigned short int *ptr;
        unsigned short int val;

        ptr = (unsigned short int *) pop();
        val = (unsigned short int )  pop();

        val &= 0xffff;

        *ptr = val;

        // *ptr = pop() & 0xffff;
    }
}

cat()
{
    if (regs.mode)
        mem[regs.dp++] = Find("c@");
    else
    {
        char           *ptr;

        ptr = (char *) pop();
        push(*ptr);

    }
}

cstore()
{
    if (regs.mode)
        mem[regs.dp++] = Find("c!");
    else
    {
        char           *ptr;
        int             i;

        ptr = (char *) pop();
        *ptr = pop() & 0xff;
    }
}

void sysReset() {
    if (regs.mode) {
        mem[regs.dp++] = Find("reset");
    } else {
#ifdef UBOOT
        do_reset();
#else
        printf("Unimplemented\n");
#endif

    }
}

void cmemRead() {
    if (regs.mode) {
        mem[regs.dp++] = Find("c@");
    } else {
        unsigned char *ptr;
        unsigned c;

        ptr=pop();
        c=*ptr;
        push(c);
    }
}

void cmemWrite() {
    if (regs.mode) {
        mem[regs.dp++] = Find("!");
    } else {
        unsigned int *ptr;
        unsigned char c;

        ptr=pop();
        c=pop() & 0xff;

        *ptr=c;
    }
}
void memRead() {
    if (regs.mode) {
        mem[regs.dp++] = Find("@");
    } else {
        unsigned int *ptr;

        ptr=pop();
        push(*ptr);
    }
}

void memWrite() {
    if (regs.mode) {
        mem[regs.dp++] = Find("!");
    } else {
        unsigned int *ptr,*data;

        ptr=pop();
        data=pop();

        *ptr=data;
    }
}

at() {
    if (regs.mode) {
        mem[regs.dp++] = Find("get");
    } else {
        struct variable *ptr;

        Dup();
        ptr = (struct variable *) pop();
        if (ptr->Read) {
            (*(ptr->Read)) ();
        } else {
            drop();
            switch (ptr->type) {
#if defined(STRINGS)
                case STRING:
                    sdrop();
                    break;
#endif
                case INTEGER:
                    drop();
                    break;
#if defined(FLOATS)
                case FLOAT:
                    fdrop();
                    break;
#endif
                default:
                    break;
            }
        }
    }
}

put()
{
    if (regs.mode)
        mem[regs.dp++] = Find("put");
    else
    {
        struct variable *ptr;

        Dup();
        ptr = (struct variable *) pop();
        if (ptr->Write)
            (*(ptr->Write)) ();
        else
        {
            drop();
            switch (ptr->type)
            {
#if defined(STRINGS)
                case STRING:
                    sdrop();
                    break;
#endif
                case INTEGER:
                    drop();
                    break;
#if defined(FLOATS)
                case FLOAT:
                    fdrop();
                    break;
#endif
                default:
                    break;
            }
        }
    }
}


#if defined(FLOATS)
Float()
{
    float           num;
    int             i;
    int             len;

    token();
    len = pop();
    i = sscanf(pad, "%f", &num);
    if (i != 0)
        fpush(num);
    else
        printf( "Invalid real number %s\n", pad);
}
#endif

#if defined(STRINGS)
void string() {
    char           *start;
    char           *ptr;

    while (*(regs.lbp) != '"') {
        regs.lbp++;
    }

    regs.lbp++;

    start = regs.lbp;

    while (*(regs.lbp) != '"') {
        regs.lbp++;
    }

    *(regs.lbp) = '\0';
    regs.lbp++;

    ptr = (char *) strsave(start);


    if (!ptr) {
        printf( "string Malloc fail\n");
    }

    if (regs.mode)
    {
        mem[regs.dp++] = Find("(slit)");
        mem[regs.dp++] = VOIDCAST ptr;
    } else
        spush(ptr);
    free(ptr);
}

void squot() {
    char           *start;
    char           *ptr;

    regs.lbp++;

    start = regs.lbp;

    while (*(regs.lbp) != '"') {
        regs.lbp++;
    }

    *(regs.lbp) = '\0';
    regs.lbp++;

    ptr = (char *) strsave(start);

    if (!ptr) {
        printf("s\" Malloc fail\n");
    }

    if (regs.mode) {
        mem[regs.dp++] = Find("(slit)");
        mem[regs.dp++] = VOIDCAST ptr;
    } else {
        spush(ptr);
        free(ptr);
    }
}

void CFind() {
    if (regs.mode) {
        mem[regs.dp++] = Find("find");
    } else {
        char *name;

        name=spop();

        push(Find( name+1 ));
    }
}


dotss()
{
    if (regs.mode)
        mem[regs.dp++] = Find(".ss");
    else
    {
        int             i;

        if (regs.ssp > 0) {
            printf("Top");
            for (i = regs.ssp - 1; i >= 0; i--) {
                printf("\t%08x\t>%s<\n", ss[i].Entry, &((ss[i].Entry)[1]));
            }
        } else {
            printf("Empty\n");
        }
    }
}
#endif

nop()
{
}

forget()
{
    token();

    if (pop())
    {
        struct header  *ptr;
        int             cfa;
        int             term;

        term = (int) Find(pad);
        if (term < regs.fence)
        {
            printf( "Can't forget below fence\n");
            return;
        }
        ptr = latest;

        while ((ptr->cfa > regs.fence) && (ptr->cfa != term))
        {
            latest = ptr->lfa;
            regs.dp = ptr->cfa;
            free(ptr);
            ptr = latest;
        }
    } else
        printf( "Forget what ?\n");
}

expect()
{
    if (regs.mode)
        mem[regs.dp++] = Find("expect");
    else
    {
        int             count;
        int             i;
        int             j = 0;
        int             done = 0;
        char           *ptr;
        char            c;

        i = 0;
        count = pop();
        ptr = (char *) malloc(count + 1);
        if (!ptr)
            printf( "expect malloc fail\n");
        else
        {
            //            tty_reset();
            //            nonblock(1);
            do
            {
                UDELAY(1);
                j = tstc();

                if (j != 0)
                {
                    c = GETC();

                    switch (c)
                    {
                        case 127:   /* delete */
                        case '\b':
                            if (i > 0)
                            {
                                printf( "\b \b");
                                i--;
                            }
                            break;
                        case '\n':
                        case '\r':
                            *(ptr + i) = '\0';
                            done = 1;
                            break;
                        default:
                            *(ptr + i) = c;
                            i++;
                            PUTC(c);
                    }
                }
            }
            while (!done && i < count);
        }
        push(ptr);
        push(i);
    }
}

#if defined(STRINGS)
ctoi()
{
    if (regs.mode)
        mem[regs.dp++] = Find("ctoi");
    else
    {
        char           *ptr;

        ptr = pop();
        push(*(ptr + 1));
        free(ptr);
    }
}

itoc()
{
    if (regs.mode)
        mem[regs.dp++] = Find("itoc");
    else
    {
        int             n;
        char           *ptr;

        n = pop();
        ptr = (char *) malloc(2);
        if (!ptr)
        {
            printf( "malloc failed in litoc\n");
            return;
        }
        *ptr = n & 0xff;
        *(ptr + 1) = '\0';
        push(ptr);
    }
}
#endif

#if defined(FLOATS)
itof()
{
    if (regs.mode)
        mem[regs.dp++] = Find("itof");
    else
        fpush((float) pop());
}

ftoi()
{
    if (regs.mode)
        mem[regs.dp++] = Find("ftoi");
    else
        push((int) fpop());
}
#endif

#if defined(FLOATS) && defined(STRINGS)
Ftoa()
{
    if (regs.mode)
        mem[regs.dp++] = Find("ftoa");
    else
    {
        char            line[128];
        char           *ptr;
        float           val;

        val = fpop();

        sprintf(line, regs.fpformat, val);

        ptr = (char *) strsave(line);
        spush(ptr);
    }
}

Atof()
{
    if (regs.mode)
        mem[regs.dp++] = Find("atof");
    else
    {
        char           *ptr;
        float           val;

        ptr = spop();

        sscanf(ptr, "%f", &val);
        free(ptr);
        fpush(val);
    }
}
#endif

outc(c)
    int             c;
{
    /*
       putchar(c);
       */

    PUTC(c);
}


#if defined(TERMCAP)
cls()
{
    if (regs.mode)
        mem[regs.dp++] = Find("cls");
    else
        Cls();
}

Cls()
{
}

Goto()
{
    if (regs.mode)
        mem[regs.dp++] = Find("goto");
    else
    {

        int             line, column;

        line = pop();
        column = pop();

        stolgoto(column, line);
    }

}

stolgoto(column, line)
    int             line, column;
{
    extern char    *Move;
    char           *s;

    s = (char *) tgoto(Move, column, line);
    tputs(s, 1, outc);
    fflush(out);
}
#endif

#if defined(STRINGS) && defined(TERMCAP)
picexpect()
{
    if (regs.mode)
        mem[regs.dp++] = Find("picexpect");
    else
    {
        int             y, x, len;
        char           *pic;
        char           *ptr;

        y = pop();
        x = pop();

        pic = spop();

        ptr = (char *) PicExpect(x, y, pic);
        len = strlen(ptr);

        push(len);
        spush(ptr);
    }
}

PicExpect(x, y, pic)
    int             y, x;
    char           *pic;
{
    extern char    *Move;
    int             count;
    int             i;
    int             done = 0;
    int             been = 0;

    char           *ptr=(char *)NULL;
    char           *s;
    char            c;

    /*
       i = 0;

       count = strlen(pic);
       ptr = (char *) malloc(count + 1);
       if (!ptr)
       printf( "picexpect malloc fail\n");

       s = (char *) tgoto(Move, y, x);
       tputs(s, 1, outc);
       fflush(out);

       Line();
       do
       {
       c = keystroke(1);
       switch (c)
       {
       case '\b':
       if (i > 0)
       {
       printf( "\b \b");

       i--;
       }
       break;
       case '\n':
       case '\r':
     *(ptr + i) = '\0';
     done = 1;
     break;
     default:
     if (been == 0)
     {
     int             j;

     for (j = 0; j < count; j++)
     PUTC(' ', out);

     s = (char *) tgoto(Move, y, x);
     tputs(s, 1, outc);

     been++;
     }
     switch (*(pic + i))
     {
    //                a-z A-Z 
    case 'A':
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
    {
     *(ptr + i) = c;
     i++;
     fPUTC(c, out);

     }
     break;
     case '#':
     if (c >= '0' && c <= '9')
     {
     *(ptr + i) = c;
     i++;
     fPUTC(c, out);

     }
     break;
     case 'X':
     *(ptr + i) = c;
     i++;
     fPUTC(c, out);

     break;
}
}
}

//     * while (!done && i < count);
while (!done);

tty_reset();
//    restore();
*/
return (ptr);
}

#endif
spaces()
{
    if (regs.mode)
        mem[regs.dp++] = Find("spaces");
    else
    {
        int             count;
        int             i;

        count = pop();

        for (i = 0; i < count; i++)
            PUTC(' ');
    }
}

void fsleep() {

    if (regs.mode) {
        mem[regs.dp++] = Find("sleep");
    } else {
        int             time;

        time = pop();
        UDELAY(1000*time);
    }
}


/*
   Getpid()
   {
   if (regs.mode)
   mem[regs.dp++] = Find("getpid");
   else
   {
   int             pid;

   pid = getpid();
   push(pid);
   }
   }

   Getuid()
   {
   if (regs.mode)
   mem[regs.dp++] = Find("getuid");
   else
   {
   uid_t           uid;

   uid = getuid();
   push(uid);
   }
   }
   */
#if defined(STRINGS)
/*
   Itoa()
   {
   if (regs.mode)
   mem[regs.dp++] = Find("itoa");
   else
   {
   char            line[128];
   char           *ptr;
   int             val;

   val = pop();

   sprintf(line, regs.ipformat, val);

   ptr = (char *) strsave(line);
   spush(ptr);
   }
   }
   */

Atoi()
{
    if (regs.mode)
        mem[regs.dp++] = Find("atoi");
    else
    {
        char           *ptr;
        int             val;

        ptr = spop();
        //        val = atoi(ptr + 1);

        val=STRTOL(ptr+1,NULL,regs.base);
        push(val);
        free(ptr);
    }
}



stoi()
{
    if (regs.mode)
        mem[regs.dp++] = Find("s>i");
    else
    {
        char           *ptr;
        int             i;

        ptr = spop();

        i = (unsigned int) ptr;
        push(i);
    }
}

sfromi()
{
    if (regs.mode)
        mem[regs.dp++] = Find("s<i");
    else
    {
        char           *ptr;
        int             i;

        i = pop();
        ptr = (char *) i;
        spush(ptr);
    }
}

/*
   Returns true if the strings are equal, false ,otherwise.
   */
void Strcmp() {

    if (regs.mode) {
        mem[regs.dp++] = Find("strcmp");
    } else {
        char           *p1, *p2;
        int             i;

        p1 = spop();
        p2 = spop();

        if( p1[0] != p2[0] ) {
            push(0);
        } else {
            i = strcmp(p1, p2);
            if(i == 0) {
                i=-1;
            } else { 
                i=0;
            }
            push(i);
        }
        /*
           free(p1);
           free(p2);
           */
    }
}

Strlen()
{
    if (regs.mode)
        mem[regs.dp++] = Find("strlen");
    else
    {
        char           *ptr;
        int             len;

        ptr = spop();
        len = strlen(ptr);
        push(len);
        free(ptr);
    }
}

Strcut()
{
    if (regs.mode)
        mem[regs.dp++] = Find("strcut");
    else
    {
        char           *str, *str1, *str2;
        int             pos;

        pos = pop();

        str = spop();
        str2 = strsave(str + pos);

        *(str + pos) = '\0';
        str1 = strsave(str);
        spush(str1);
        spush(str2);
        free(str);
    }
}

/*
   Strcat()
   {
   if (regs.mode)
   mem[regs.dp++] = Find("strcat");
   else
   {
   char            scratch[512];

   sswap();
   strcpy(scratch, spop());
   strcat(scratch, spop());
   spush(scratch);
   }
   }
   */
/*
   Strtok()
   {
   if (regs.mode)
   mem[regs.dp++] = Find("strtok");
   else
   {
   int             count = 0;
   int             i;

   char           *str, *str1, *str2, *sep;

   sep = spop();
   str = spop();

   str1 = (char *) strtok(str, sep);
   while (str1 != NULL)
   {
   count++;
   str2 = strsave(str1);
   push(str2);
   str1 = (char *) strtok(NULL, sep);
   }
   free(str1);
   free(sep);

   for (i = 0; i < count; i++)
   spush(pop());
   push(count);
   }
   }
   */

Token() {
    if (regs.mode) {
        mem[regs.dp++] = Find("token");
    } else {
        int count=1;
        int i=0;
        int exitFlag=0;
        //        regs.lbp++;
        char buff[32];


        while( (*regs.lbp <= 0x20) || (*regs.lbp >= 0x7f))  {
            regs.lbp++;
        }
#ifdef UNIX
        bzero( (void *)&buff[0],32);
        while( (isprint(*regs.lbp) ) && (exitFlag == 0)) {
#endif

#ifdef UBOOT
            for(i=0;i<32;i++) {
                buff[i]=0x00;
            }

            while( (*regs.lbp > 0x1f) && (*regs.lbp < 0x7f) && (exitFlag == 0)) {
#endif
                if( *regs.lbp != ' ' ) {
                    buff[count++]= (*regs.lbp);
                } else {
                    exitFlag=1;
                }
                regs.lbp++;
            }
            buff[0]=strlen( &buff[1]);
            spush( buff );
        }
    }

    BufSplit() {
        if (regs.mode) {
            mem[regs.dp++] = Find("bufsplit");
        } else {
            int             count = 0;
            int             tokCount = 0;
            char           *str, *str1, *str2, *sep;

            sep = spop();
            str = spop();
            tokCount = pop();

            if(strlen(sep) > 0) {
                sep++;
            }

            if(strlen(str) > 0) {
                str++;
            }

            VOIDCAST        bufsplit(sep, 0);
            count = bufsplit(str, tokCount);
            push(count);
        }
    }
#endif

    Malloc()
    {
        if (regs.mode)
            mem[regs.dp++] = Find("malloc");
        else
        {
            char           *ptr;
            int             size;

            size = pop();
            ptr = (char *) malloc(size);
            push(ptr);
        }
    }

    Free()
    {
        if (regs.mode)
            mem[regs.dp++] = Find("free");
        else
        {
            void *ptr;
            ptr = (void *)pop();
            free(ptr);
        }
    }

    hash()
    {
        *regs.lbp = '\n';
        *(regs.lbp + 1) = '\0';
    }

    verbose()
    {
        regs.verbose = 0xff;
    }

    quiet()
    {
        regs.verbose = 0;
    }


    void mm() {
        unsigned int             *addr;

        addr = pop();

        printf("mm:FIX ME \n");
        /*
           printf("%d\t%d\t", addr, mem[addr]);
           scanf("%d", &mem[addr]);
           */
    }

    /*
       lines()
       {
       if (regs.mode)
       mem[regs.dp++] = Find("lines");
       else
       push(slines);
       }

       columns()
       {
       if (regs.mode)
       mem[regs.dp++] = Find("columns");
       else
       push(scolumns);
       }
       */

#if defined(STRINGS)
    strtoc()
    {
        if (regs.mode)
            mem[regs.dp++] = Find("strtoc");
        else
        {
            char           *ptr;
            char            c;

            int             len;
            int             i;

            ptr = spop();
            len = strlen(ptr);

            for (i = (len - 1); i >= 0; i--)
                push(*(ptr + i));

            push(len);
        }
    }
#endif

    /*
       fargc()
       {
       if (regs.mode)
       mem[regs.dp++] = Find("argc");
       else
       {
       extern int      Argc;
       push(Argc);
       }
       }

       fargv()
       {
       if (regs.mode)
       mem[regs.dp++] = Find("argv");
       else
       {
       extern char   **Argv;
       char           *ptr;
       char           *p;

       int             position;
       position = pop();
       p = *(Argv + position);
       ptr = (char *) strsave(p);
       push(ptr);
       }
       }
       */

#if defined(STRINGS)

    void athToken() {
    }

    void
        sinsert()
        {
            if (regs.mode)
                mem[regs.dp++] = Find("sinsert");
            else
            {
                char            buffer[512];
                int             insertAs = 0;
                int             t;
                int             i;

                t = regs.ssp - 1;
                strcpy(buffer, ss[t].Entry);
                insertAs = pop();

                for (i = t; i >= insertAs; i--)
                {
                    strcpy(ss[i].Entry, ss[i - 1].Entry);
                }
                strcpy(ss[insertAs].Entry, buffer);
            }
        }
#endif

#if defined(SIGNALS)
    void
        alarmHandler()
        {
            regs.ev_sigalarm++;
        }

    void
        getev_sigalarm()
        {
            if (regs.mode)
                mem[regs.dp++] = Find("ev_sigalarm_get");
            else
            {
                push(regs.ev_sigalarm);
            }
        }

    void
        clrev_sigalarm()
        {
            if (regs.mode)
                mem[regs.dp++] = Find("ev_sigalarm_clr");
            else
            {
                regs.ev_sigalarm = 0;
            }
        }


    void
        Alarm()
        {
            if (regs.mode)
                mem[regs.dp++] = Find("alarm");
            else
            {
                int             value;
                int             ret;

                value = pop();
                ret = signal(SIGALRM, alarmHandler);
                alarm(value);
                push(ret);
            }
        }

    void
        sigioHandler()
        {
            regs.ev_sigio++;
        }

    void
        getev_sigio()
        {
            if (regs.mode)
                mem[regs.dp++] = Find("ev_sigio_get");
            else
            {
                push(regs.ev_sigio);
            }
        }

    void
        clrev_sigio()
        {
            if (regs.mode)
                mem[regs.dp++] = Find("ev_sigio_clr");
            else
            {
                regs.ev_sigio = 0;
            }
        }

    void
        SigIO()
        {
            void           *ret;
            ret = (void *) signal(SIGIO, sigioHandler);
            if (ret == -1)
                perror("SigIO");
        }
#endif
