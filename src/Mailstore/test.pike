#!/usr/bin/pike7.2

int main() {
  //object o=.Mailstore.Mailbox("imap://user:pass@host/folder");
  object o=.Mailstore.Mailbox("/home/redax/Maildir");
  //object o=.Mailstore.Mailbox("/home/redax/mbox");
  o->debug(255);
  int mails=sizeof(o);
  for(int i=0;i<mails;i++) {
	mapping m=o->get_header(i);
	write("%-30s %s\n",(m->env->from[0][1]), m->subject[..48] );
  }
}
