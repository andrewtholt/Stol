/*
 */
#ifndef UNIX
#define UBOOT 1
#endif

#ifdef UBOOT
#include <common.h>
#include <exports.h>
#include <asm/io.h>
#include <asm/processor.h>

#define PUTC putc
#define GETC getc
#define UDELAY udelay
#define STRTOL simple_strtol
#endif

#ifdef UNIX
#include <stdio.h>

#define PUTC putchar
#define GETC getchar
#define UDELAY usleep
#define STRTOL strtol
#endif
#include "env.h"
#include "f16.h"

#ifdef SUPERH
#define SCASMR5 (vu_short *) 0xA4E40000
#define SCASCR5 (vu_short *) 0xA4E40018
#define SCASSR5 (vu_short *) 0xA4E40014
#define SCAFTDR5 (vu_short *) 0xA4E50020
#define SCABRR5 0xA4E50004
#define SCAFCR5 0xA4E50018
#define SCR_RE 0x10
#define SCR_TE 0x20

#define FCR_RFRST 0x0002
#define FCR_TFRST 0x0004

#define SCBRR_VALUE(bps, clk) (((clk * 2 * 2) + 16 * bps) / (32 * bps) - 1)
#endif

#define VOIDCAST (void *)
#define PROMPT "OK>\n"
int             rs[128];
unsigned int             ds[128];
int             cs[128];    // Compile stack 
struct header *latest;
char pad[255];
char lb[255];
struct header  *cword;
int exitFlag;

struct variable *rptr;
int slines = 23;

void (**mem[4000]) ();

void sysReset();

#ifdef UBOOT
void ubootABI();
#endif

void squot();

void znequ();
void bounds();
void over();
void swap();
void empty();
void depth();

char  *strsave(char *s) {
    char *p=(char *)NULL;
    size_t len=0;
    
    len=strlen(s);
    
    p = (char *) malloc(len+2);

    if (p ==0) {
        printf("\n\nMalloc failure in strsave\n");
        return((char *)1);
    } else {
        *p=(char)len;
        strcpy((p+1), s);
    }
    return (p);
}

void printFeatures();

// #include "bufsplit.c"
#include "primary.c"

int rc;

#ifdef UBOOT
int stol (int argc, char *argv[]) {
#endif

#ifdef UNIX
int main (int argc, char *argv[]) {
#endif
	int i;
    char *ptr;
    char t;
    int speed = 0;
    char tib[255];
    int len=0;
    
    regs.dsp = 0;
    regs.rsp = 0;
#ifdef STRINGS
    regs.ssp = 0;
#endif
    regs.Trace = 0;
    regs.lbp = &tib[0];
    regs.mode = 0;
    regs.ip = 0;
    regs.dp = 0;
    regs.base = 10;
    
    rc=0;
    latest = (struct header *) NULL;

    for(i=0;i<4000;i++) {
        mem[i] = 0;
    }

    #ifdef UBOOT
	/* Print the ABI version */
	app_startup(argv);
    /*
	printf ("Example expects ABI version %d\n", XF_VERSION);
	printf ("Actual U-Boot ABI version %d\n", (int)get_version());
    
	printf ("\n");
    
     printf("Clock       :%d:\n",CONFIG_SYS_CLK_FREQ);
     printf("Value(115)  :%02x\n",SCBRR_VALUE(115200, CONFIG_SYS_CLK_FREQ));
     printf("Mode        :%04x:\n",readw(SCASMR5));
     printf("Status      :%04x:\n",readw(SCASSR5));
    */ 
     writew((SCR_RE | SCR_TE), SCASCR5);
     writew(0, SCASMR5);
     writew(0, SCASMR5);
     writew((FCR_RFRST | FCR_TFRST), SCAFCR5);
//     printf("Fifo Control:%04x:\n",readw(SCASCR5));
     writew(0, SCAFCR5);
     speed=SCBRR_VALUE(38400, CONFIG_SYS_CLK_FREQ);
//     printf("Speed(38400):%04x:\n",speed );
    #endif    
    
    MakePrim("-", sub);
    MakePrim("*", mul);
    MakePrim("2*", TwoTimes);
    MakePrim("/", div);
    MakePrim("2/", TwoDiv);
    MakePrim("mod", mod);
    MakePrim("char", Char);
    MakePrim("cell", Cell);
    MakePrim("cells", Cells);
    MakePrim("cell+", CellPlus);
    MakePrim("+", add);
    MakePrim("=", equal);
    MakePrim(">", gt);
    MakePrim("<", lt);
    MakePrim("min",Min);
    MakePrim("1+", oneplus);
    MakePrim("1-", oneminus);
    MakePrim("0=", zequ);
    MakePrim("0<>", znequ);
    MakePrim("not", not);
    MakePrim("and", and);
    MakePrim("or", or);
    MakePrim("xor", xor);
    MakePrim("(lit)", lit);
    MakePrim("(slit)", slit);
    MakePrim("hex", hex);
    MakePrim("decimal", decimal);
    MakePrim("octal", octal);
    
    MakePrim("if", fif);
    MakePrim("while", fif);
    MakePrim("else", felse);
    MakePrim("fi", ffi);
    MakePrim("then", ffi);
    MakePrim("begin", fbegin);
    MakePrim("until", funtil);
    MakePrim("again", again);
    MakePrim("end", End);
    MakePrim("repeat", End);
    
    MakePrim("do", fdo);
    MakePrim("+loop", ploop);
    MakePrim("loop", loop);
    MakePrim("break", Break);
    MakePrim("leave", Break);
    MakePrim("i", findex);
    MakePrim("sleep",fsleep);
    
    MakePrim("swap", swap);
    MakePrim("drop", drop);
    MakePrim("dup", Dup);
    MakePrim("over", over);
    MakePrim("nip", nip);
    MakePrim("tuck", tuck);
    MakePrim("rot",rot);

    MakePrim(">r",tor);
    MakePrim("r>",fromr);

    MakePrim("bounds",bounds);
    MakePrim("empty",empty);
    MakePrim("depth",depth);
    
    #ifdef UNIX
    MakePrim("mon", mon);
    MakePrim("bye", bye);
    #endif

    MakePrim("traceon", TraceOn);
    MakePrim("traceoff", TraceOff);
    MakePrim("trace", TraceNoPause);
    
    MakePrim("emit", emit);
    MakePrim("key", key);
    MakePrim("?terminal", qterm);
    MakePrim(".", dot);
    MakePrim(".version",dotVersion);
    MakePrim("build-no",getBuildNo);
    MakePrim("cr", cr);
    MakePrim(".s", dots);
    MakePrim(".\"", dotq);
    MakePrim("(:)", docolon);
    MakePrim("(;)", dosemi);
    MakePrim(":", colon);
    MakePrim(";", semi);
    MakePrim("rdump", regdump);
    MakePrim("status", status);
    MakePrim("vlist", vlist);
    MakePrim("words", plist);
    MakePrim("dump", mdump);
    MakePrim("mm", mm);
    MakePrim("INTEGER", integer);

    MakePrim("variable", variable);
    MakePrim("array", farray);
    MakePrim("(ArrayFix)", ArrayFix);
    MakePrim("(variable)", bvariable);

    MakePrim("@", memRead);
    MakePrim("!", memWrite);

    MakePrim("w@", wmemRead);
    MakePrim("w!", wmemWrite);

    MakePrim("c@", cmemRead);
    MakePrim("c!", cmemWrite);

    MakePrim("get", at);
    MakePrim("put", put);

    MakePrim("malloc", Malloc);
    MakePrim("free", Free);
    MakePrim("spaces", spaces);
    MakePrim("bl", Blank);
    MakePrim("'", Tick);
#if defined(STRINGS)
    MakePrim("token", Token); 
//    MakePrim("bufsplit", BufSplit); 
    MakePrim("$getenv", Getenv); 
    MakePrim("string", string);
    MakePrim("s\"", squot);

    MakePrim("type", sdot);
    MakePrim(".ss", dotss);

    MakePrim("sempty", sempty);

    MakePrim("sdrop", sdrop);
    MakePrim("sswap", sswap);
    MakePrim("sdup", sdup);
    MakePrim("evaluate", Eval);
    MakePrim("mem2string",mem2string);
    MakePrim("string-ptr",stringPtr);
    MakePrim("load",Load);
    MakePrim("find",CFind);
#ifdef UBOOT
    MakePrim("$system",ubootSystem);
    MakePrim("saveenv",ubootSaveenv);
    MakePrim("$setenv",ubootSetenv);
    MakePrim("?abi", ubootABI);
#endif
    MakePrim("strcmp",Strcmp);
#endif

    MakePrim(".features",printFeatures);
    MakePrim("#", hash);
    MakePrim("\\",hash);

    MakePrim("reset", sysReset);
    MakePrim("forget", forget);
    MakePrim("expect", expect);
    MakePrim("(constant)", constant);
    MakePrim("c@", cat);
    MakePrim("c!", cstore);
    MakePrim("execute", Exec);
    
    MakeVariable("test", CONSTANT, 1, NULL);

    MakePrim("nop", nop);
    
    regs.base = 16;
    regs.fence = Find("nop");
    
    regs.fpformat = strsave( "%f");
    
    regs.mode = regs.state = 0;
    regs.ipformat= strsave( "%d");
    
    regs.lbp = lb;
    strcpy(lb, "Startup");

//    Startup();
    
    for(i=0;i<255;i++) {
        lb[i]=0x20;
    }
//    printf("\n->%s<-\n",lb);
    
    exitFlag=0;

    while(!exitFlag) {
        regs.lbp = &lb[0];

        printf("\n:%d:OK>",regs.dsp);
        Inline();

        while(token()) {
            len = pop();
            if (len > 0)
            {
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
                    unsigned long long tmp;
                    
//                    tmp=STRTOL(pad,NULL,regs.base);
                    tmp=strtoll(pad,NULL,regs.base);
                    
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
            }
        }
    }
	return (rc);
}

int athGetline(char *ptr,int cnt,int echo) {
    
    int exitFlag=0;
    int idx=0;
    int c;
    
    while( !exitFlag && (idx <= cnt) ) {
        c=GETC();

        switch(c) {
            #ifdef UBOOT
            case 0x0d:
            #endif

            #ifdef UNIX
            case 0x0a:
            #endif
                PUTC(c);
                exitFlag=1;
                ptr[idx++]=c;
                ptr[idx]=0x00;
                break;
            case 0x7f:
            case 0x08:
                PUTC(0x08);
                PUTC(' ');
                PUTC(0x08);
                if(idx > 0) {
                    idx--;
                }
                break;
            default:
                if(echo) {
                    PUTC(c);
                }
                ptr[idx]=c;
                idx++;
                break;
        }
    }
    return( idx );
}

#ifdef UBOOT
void sc5_putchar(char c) {
    unsigned int status =-1;
    
    status=(readw(SCASSR5) & 0x0000ffff);
    
    status &= 0x40;
    printf("I_TDRE        :%04x:\n",status);
    writeb(c,SCAFTDR5);
    UDELAY(1000);
    printf("O_TDRE        :%04x:\n",status);
    
    return ;
}
#endif

#ifdef UBOOT
__kernel_size_t strlen(const char *s) {
#else
size_t strlen(const char *s) {
#endif
    int i=0;
    
    while(s[i] != 0x0) {
        i++;
    }
    return(i);
}

void Inline() {
    char           *c;
    
    regs.lbp = lb;
    
    #ifdef UBOOT
    c=athGetline(regs.lbp,255,1);
    PUTC('\n');
    #endif

    #ifdef UNIX
    c=athGetline(regs.lbp,255,0);
    #endif
    
    regs.lbp = lb;
}

void FindHeader() {
    struct header  *ptr;
    int             found = 0;
    
    ptr = latest;
    
    while (ptr != (struct header *) NULL) {
        if (!strcmp(pad, ptr->name)) {
            cword = ptr;
            push(ptr->cfa);
            found = 1;
            break;
        } else
            ptr = ptr->lfa;
    }
    push(found);
}

MakeHeader(char *name, int (*func) () ) {
    struct header  *hp;
    
    hp = (struct header *) malloc(sizeof(struct header));
    
    if (!hp) {
        printf("Malloc fail in MakeHeader\n");
        return(1);
    }
    hp->len = strlen(name);
    hp->name = (char *) malloc(hp->len + 1);

    if (!hp->name) {
        printf("Malloc2 fail in MakeHeader\n");
        return(1);
    }

    hp->cfa = (int) func;
    strcpy(hp->name, name);

    if (!latest) {
        latest = hp;
        hp->lfa = (struct header *) NULL;
    } else {
        hp->lfa = latest;
        latest = hp;
    }
}

void MakePrim(char *name, int (*func)()) {
    MakeHeader(name, regs.dp + 1);
    regs.dp++;
    mem[regs.dp++] = VOIDCAST func;
    mem[regs.dp++] = VOIDCAST next;
}

void MakeVariable( char *name, int type, int (*rd) (), int (*wr) ()) {
    push(type);
    regs.lbp = lb;
    strcpy(lb, name);
    variable();

    rptr->Read = rd;
    rptr->Write = wr;
}



