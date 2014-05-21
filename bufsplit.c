#define NULL 0

/*
 * int SCMatch (char Text, char *Delim);
 */
int             SCMatch();

int bufsplit(char *Text, int ArrayLen) {

	int count = 1;
	int i;
	int j;
	char *t=(char *)NULL;
    char x[255];

	static char     Delim[32] = "";

	Delim[31] = '\0';

	if (0 == ArrayLen) {
		if (*Text == (char )0 ) {
			return (0);
        } else {
			strncpy(Delim, Text, 31);
			return (0);
		}
	}

	ArrayLen--;

	while (0 != ArrayLen) {
		if(0 != (SCMatch((char) *Text, Delim)))
			Text++;
			
		t=Text;
		
		while (((char)0 != *Text) && (0 != (SCMatch((char) *Text, Delim)))) {
			Text++;
        }

		if ((char) 0 != *Text) {
			*Text++ = '\0';
			if(strlen(t) == 0) {
				spush("<NULL>");
			} else {
                strcpy( &x[1],t);
                x[0]=(char)strlen( t );
				spush(x);
            }
			ArrayLen--;
			count++;
		} else {
			break;
		}
	}

	if(strlen(Text) == 0) {
        strcpy( &x[1],t);
        x[0]=strlen( t );
		spush(x);
	} else {
        strcpy( &x[1],Text);
        x[0]=strlen( Text );
		spush(x);
    }

	sdepth();
	j=pop();

	for(i=(j - count); i < count;i++) {
		push(i);
		sinsert();
	}
/*
	for (i = 0; i < count; i++)
	{
		t=(char *)pop();
		if(strlen(t) == 0)
			spush("<NULL>");
		else
		{
			spush(t);
		}
	}

*/
	return (count);
}


int 
SCMatch(Text, Delim)
	char            Text;
	char           *Delim;
{
	while (*Delim)
		if (Text == *Delim++)
			return (1);

	return (0);
}

/*
 * Name strstuff - fill a char array with a defauld char Syntax void
 * strstuff(char *string, char fill, int count);
 */

void
strstuff(ptr, fill, count)
	char           *ptr;
	char            fill;
	int             count;
{
	int             i;

	for (i = 0; i < count; i++)
		*(ptr + i) = fill;
	*(ptr + i) = NULL;
}
