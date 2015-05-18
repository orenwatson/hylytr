#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <unistd.h>

/* A highlighter script consists of tuples
color color /regex/ /regex/
where color is a color name from the following:
(normal)(bright)
black	grey
maroon	red
green	lime
olive	yellow
navy	blue
purple	fuchsia,magenta
teal	cyan,aqua
silver	white
default	bold
The regex may begin and end with any one non-whitespace character.
*/

#define DEBUG 1
typedef char bool;

struct hylight{
	int fg,bg;
	regex_t beg,end;
	bool noend,stillon;
	struct hylight*next;
}*hllist;

int getcolor(char *s){
	if(!strcmp(s,"black"))return 0;
	if(!strcmp(s,"maroon"))return 1;
	if(!strcmp(s,"green"))return 2;
	if(!strcmp(s,"olive"))return 3;
	if(!strcmp(s,"navy"))return 4;
	if(!strcmp(s,"purple"))return 5;
	if(!strcmp(s,"teal"))return 6;
	if(!strcmp(s,"silver"))return 7;
	if(!strcmp(s,"default"))return 9;
	if(!strcmp(s,"grey"))return 10;
	if(!strcmp(s,"red"))return 11;
	if(!strcmp(s,"lime"))return 12;
	if(!strcmp(s,"yellow"))return 13;
	if(!strcmp(s,"blue"))return 14;
	if(!strcmp(s,"magenta"))return 15;
	if(!strcmp(s,"fuchsia"))return 15;
	if(!strcmp(s,"cyan"))return 16;
	if(!strcmp(s,"aqua"))return 16;
	if(!strcmp(s,"white"))return 17;
	if(!strcmp(s,"bold"))return 19;
	return -1;
}

int main(int argc, char **argv){
	if(argc<2)fprintf(stderr,"You didn't specify a hylytr script.\n");
	char buf[1000],rx1[500],rx2[500];
	FILE *hlf=fopen(argv[1],"r");
	hllist=malloc(sizeof(struct hylight));
	struct hylight*cur=hllist;
	int linenum=0;
	while(fgets(buf,1000,hlf)){
		int i=0;
		while(buf[i]<=' '&&buf[i]!=0)i++;
		if(buf[i]==0){fprintf(stderr,"No fg color string on line %d\n",linenum);continue;}
		int j=i;
		while(buf[j]>' ')j++;
		if(buf[j]==0){fprintf(stderr,"No bg color string on line %d\n",linenum);continue;}
		buf[j]=0;
		cur->fg = getcolor(buf+i);
		if(cur->fg==-1)fprintf(stderr,"Unrecognized fg color %s on line %d\n",buf+i,linenum),cur->fg=9;
		i=j+1;
		while(buf[i]<=' '&&buf[i]!=0)i++;
		if(buf[i]==0){fprintf(stderr,"no bg color string on line %d\n",linenum);continue;}
		j=i;
		while(buf[j]>' ')j++;
		if(buf[j]==0){fprintf(stderr,"No regex string on line %d\n",linenum);continue;}
		buf[j]=0;
		cur->bg = getcolor(buf+i);
		if(cur->bg==-1)fprintf(stderr,"Unrecognized bg color %s on line %d\n\n",buf+i, linenum),cur->bg=9;
		i=j+1;
		while(buf[i]<=' '&&buf[i]!=0)i++;
		if(buf[i]==0){fprintf(stderr,"No regex string on line %d\n",linenum);continue;}
		j=i+1;
		while(buf[j]!=buf[i]&&buf[j]!=0)j++;
		if(buf[j]==0)fprintf(stderr,"Unterminated regex starting with %c on line %d\n",buf[i],linenum);
		buf[j]=0;
		regcomp(&(cur->beg),buf+i+1,REG_EXTENDED);
		i=j+1;
		while(buf[i]<=' '&&buf[i]!=0)i++;
		if(buf[i]==0){goto noend;}
		j=i+1;
		while(buf[j]!=buf[i]&&buf[j]!=0)j++;
		if(buf[j]==0)fprintf(stderr,"Unterminated regex starting with %c on line %d\n",buf[i],linenum);
		if(j==i+1){noend:;
			cur->noend=1;
		}else{
			buf[j]=0;
			regcomp(&(cur->end),buf+i+1,REG_EXTENDED);
			cur->noend=0;
		}
		cur->stillon=0;
		cur->next=malloc(sizeof(struct hylight));
		cur = cur->next;
	}
	cur->next=0;
	if(DEBUG) fprintf(stderr,"Finished compiling regexes. Ready.\n");
	int cbuf[1000][2];
	while(fgets(buf,1000,stdin)){
		int i;
		for(i=0;buf[i]!=0;i++){
			cbuf[i][0]=9;
			cbuf[i][1]=9;
		}
		cur=hllist;
		while(cur->next!=0){
			int m,e,s,i,j;
			i=0;
			nextmatch:;
			regmatch_t match;
			if(!cur->stillon){
			m=regexec(&(cur->beg),buf+i,1,&match,REG_NOTBOL*(i==0));
			if(m)goto nextrule;
			s = i+match.rm_so;
			}else{
				s=0;
			}
			if(cur->noend)e = i+match.rm_eo;
			else{
				m=regexec(&(cur->end),buf+s,1,&match,REG_NOTBOL*(s==0));
				if(m){
					e = 1000;
					cur->stillon=1;
				}else{
					e = s+match.rm_eo;
					cur->stillon=0;
				}
			}
			for(j=s;j<e;j++){
				if(buf[j]==0)goto nextrule;
				cbuf[j][0]=cur->fg;
				cbuf[j][1]=cur->bg;
			}
			i=e;
			goto nextmatch;
			nextrule:;
			cur=cur->next;
		}
		int curfg=9,curbg=9;
		printf("\e[0m");
		for(i=0;buf[i]!=0;i++){
			if(cbuf[i][0]!=curfg){
				if(cbuf[i][0]>=10)printf("\e[%d;1m",cbuf[i][0]+20);
				else printf("\e[%d;22m",cbuf[i][0]+30);
			}
			if(cbuf[i][1]!=curbg){
				printf("\e[%dm",cbuf[i][1]%10+40);
			}
			curfg=cbuf[i][0];
			curbg=cbuf[i][1];
			putchar(buf[i]);
		}
	}
}
