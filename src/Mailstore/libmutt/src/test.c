#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "mutt.h"
#include "mutt_regex.h"
#include <pcreposix.h>

#include "mailbox.h"
#include "rfc822.h"

int main(int argc, char **argv) {
	CONTEXT *ctx;
	HEADER *header=0;
	FILE *f;
	
	char folder[_POSIX_PATH_MAX];
	long i;
	int index_hint,check;
	
	if( argc < 2 ) {
		perror("usage: test <Maildir>\n");
		exit(1);
	}
	
	strcpy(folder,argv[1]);
	
	FileMask.rx = (regex_t *) safe_malloc (sizeof (regex_t));
	REGCOMP(FileMask.rx,"!^\\.[^.]",0);
	ReplyRegexp.rx = (regex_t *) safe_malloc (sizeof (regex_t));
	REGCOMP(ReplyRegexp.rx,"^(re([\\[0-9\\]+])*|aw):[ \t]*",0);

	
	ctx=mx_open_mailbox(folder,0,NULL);
//	index_hint=ctx->vcount;
//	check = mx_check_mailbox(ctx, &index_hint,0);
//	printf("index_hint: %d, check: %d\n",index_hint,check);

	printf("\nmsgs: %d\nnew: %d\nunread: %d\ntagged: %d\n",
		ctx->msgcount, ctx->new, ctx->unread, ctx->tagged );

	for( i=ctx->msgcount-1; i>=0;i-- ) {
		header=ctx->hdrs[i];
		printf("%d,%d: '%s'\n",
			header->msgno, header->lines,
			( header->env->to && header->env->to->mailbox ? header->env->to->mailbox : "" ) );
	}
	mx_close_mailbox(ctx,NULL);
	exit(0);

}


void
mutt_exit(int code)
{
}


void
mutt_message(const char *fmt, ...)
{
	va_list va_args;
	va_start(va_args, fmt);
	vfprintf(stderr,  fmt, va_args);
	va_end(va_args);
}


void
mutt_clear_error(void)
{
}

int
mutt_yesorno(const char *msg, int def)
{
	fprintf(stderr,"YES/NO: %s (%d)", msg, def);
	return 1;
}


void
mutt_perror (const char *s)
{
	char *p = strerror (errno);
	fprintf(stderr,"%s: %s (errno = %d)\n",s,p ? p : "unknown error", errno );
	
	//mutt_error ("%s: %s (errno = %d)", s, p ? p : _("unknown error"), errno);
}

