
/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * Copyright © 2000-2002 Endre Hirling <endre@interware.hu>
 *                       András Horváth <andras.horvath@cern.ch>
 *                       Tamás Tevesz <ice@extreme.hu>
 */


constant cvs_version= "$Id$";

#define MD_MAX_CREAT_TRY	10
#define MD_TRY_SLEEP		2
#define MD_INFO_FLAGS		"FRST"

#define MD_FLAGGED		"F"
#define MD_READ			"R"
#define MD_SEEN			"S"
#define MD_TRASHED		"T"

#define MD_FILE(x)		(string)time() + "." + (string)getpid() + (string)sprintf("%03d", random(999)) + (string)sprintf("%02d", x) + "." + gethostname()
#define E(x)			(x) / ""
#define I(x)			(x) * ""
#define ERROR(x)		werror("%s", x + "\n")

#define FAILURE			0
#define SUCCESS			1

// UGLY!! XXX FIXME XVAZZEG.. but how???
// this is the dir where user mail homes are located
// using get_maildir_from_userhost() 
//#define MAILDIR_USER_HOME	"/tmp/ikifiki"
// better idea: ask Roxen auth module about user's home dir

/*
	a new_to_cur mostan minden list() elott meghivodik. ez imigyen lenni jo ? (eset: pop session,
	kozepen kliens ujra kiad list. ilyenkor latnia kell vagy nem kell latnia a session kezdete
	ota erkezett leveleket ?

	maildir++ nem leszen egyelore, csak a message_size tag.
	pop3-ra kigyezve for now

	public methods:
		deliver(string message)			*
		list(void|string flags,void|string noflags)	*
		get_message(string id)			*
		get_headers(string id)			*
		del_message(string id)			*
		undel_message(string id)		*
		get_message_size(string id)		*
		purge_trashed(void)			*
*/
		


class Maildir
{

string Maildir; 

void create(void|string maildir)
{
	// see if any parameters are given
	if( !maildir || (sizeof(maildir) < 1) ) 
	{
		ERROR("Maildir(\"user@host\" | \"/file/path\")");
		destruct(this_object());
	}
	// see if parameter is a user@host, convert it to pathname
	if(maildir-"@"!=maildir) maildir=get_maildir_from_userhost(maildir); 
	// it must be a pathname then
	if(!file_stat(maildir) || (file_stat(maildir)[1] != -2))
	{
		ERROR("Maildir does not exist or is not a directory");
		destruct(this_object());
	}
	else {
	// bugfix release v1.2+ :)
	if(maildir[(sizeof(maildir)-1)..]!="/") maildir+="/";

	Maildir = maildir;
	}
// no need, user will list() and thus the function be called -- raas
//	new_to_cur();
}

private string get_maildir_from_userhost(string userhost) {
//	array uh=userhost/"@";
	// should use append_path
//	return MAILDIR_USER_HOME+"/"+uh[1]+"/"+uh[0][..0]+"/"+uh[0]+"/"+"Maildir/";
// ask Roxen about 'home dir'
	return userinfo(userhost)[5]+"Maildir/"; 
}

private string create_basename(int try)
{
	return MD_FILE(try);
}

private array parse_filename(string filename)
{
// INPUT:  full filename
// OUTPUT: array ({ unique part, maildir++ flags, maildir flags })
// ACTION: breaks a filename up to it's three components
	string unique, mdpp, flags;
	sscanf(filename, "%s,S=%s:2,%s", unique, mdpp, flags);
	return ({ unique, mdpp, flags });
}

private int new_to_cur()
{
// INPUT:  void
// OUTPUT: SUCCESS|FAILURE
// ACTION: moves messages from new/ to cur/
	array directory_list = get_dir(Maildir + "new/");
	int i;
	for(i=0; i<sizeof(directory_list); i++)
	{
		mv(Maildir + "new/" + directory_list[i], Maildir + "cur/" + directory_list[i]);
	}
	return SUCCESS;
}

private array get_flags(string message_id)
{
// INPUT:  message id (== unqiue part)
// OUTPUT: string flags
	return E(parse_filename(get_filename(message_id))[2]);
}

private string get_filename(string message_id)
{
// INPUT:  message id
// OUTPUT: full filename
	array directory_list = get_dir(Maildir + "cur/");
	for(int i=0; i<sizeof(directory_list); i++)
	{
		if( (directory_list[i] / ",")[0] == message_id )
		{
			return Maildir + "cur/" + directory_list[i];
		}
	}
	return FAILURE; 
}

private int set_flags(string message_id, string flags)
{
	string filename = get_filename(message_id);
	string currentflags;
	int i;
	currentflags = I(get_flags(message_id));
	for(i=0;i<strlen(flags)-1;i++)
	{
		switch(flags[i..i])
		{
			case "+":
				currentflags += sprintf("%c", flags[++i]);
				break;
			case "-":
				currentflags -= sprintf("%c", flags[++i]);
				break;
		}
	}
	currentflags = I(Array.sort(Array.uniq(E(upper_case(currentflags)))));
	if ( I(E(currentflags) - E(MD_INFO_FLAGS)) > "" )
	{
		return FAILURE;
	}
	mv(filename, (parse_filename(filename)[..1] * ",S=") + ":2," + currentflags);
	return SUCCESS;
}

public array list(void|string flags,void|string noflags)
{
// INPUT:  void|string: flags (with these, or with any if empty.)
// 			noflags: w/ flags and w/ NOT these.
// OUTPUT: ({ message ids })
// ACTION: generates message_ids from files in cur/
	array retval, directory_list;
	int i;
	new_to_cur();
	directory_list = get_dir(Maildir + "cur/");
	if(!flags)
		return (Array.map(Array.map(directory_list, `/, ","), `[], 0));
	else	{
		array afl=E(flags); 
		array anfl=noflags?E(noflags):({ }); // nice isn't it? :)
		return Array.filter(
			Array.map(Array.map(directory_list, `/, ","), `[], 0),
			lambda(string message_id) {
				array mfl=get_flags(message_id);
				return ( 
					((afl*"")==((afl&mfl)*"")) && // these must be present
					!sizeof(anfl&mfl)
					);
				}
			);
		}
}

public int get_message_size(string message_id) {
// INPUT: message id
// OUTPUT: message size in octets, -2 for dir's, -3 for links, -4 otherwise 
	return file_stat(get_filename(message_id))[1];
}

string get_headers(string message_id)
{
	string retval="", line;
	object message_file = Stdio.FILE(get_filename(message_id), "r");
	while((line=message_file->gets())!="")
		retval+= line + "\n";
	message_file->close();
	return retval;
}

string get_message(string message_id)
{
	string filename = get_filename(message_id);
	object retrieve_tmp = Stdio.File(get_filename(message_id), "r");
	string retval = retrieve_tmp->read();
	retrieve_tmp->close();
	return retval;
}

int del_message(string message_id)
{
	return set_flags(message_id, "+T");
}

int undel_message(string message_id)
{
	return set_flags(message_id, "-T");
}

int purge_trashed()
{
	int i;
	array trash_these = Array.filter(get_dir(Maildir + "cur/"), Regexp(":2,.*T")->match);
	for(i=0; i<sizeof(trash_these); i++)
	{
		rm(Maildir + "cur/" + trash_these[i]);
	}
	return SUCCESS;
}

int deliver(string maildir_message)
{
/* RANDOM THOUGHTS
   - umask etc?
   - more error checking (wrap everything into catch ? i don't think
     i care about the cause, i just need to know if the given
     operation did succeed or not.. )
*/
	int creat_tries = 0;
	int message_size;
	object deliver_tmp = Stdio.File();
	string maildir_file = create_basename(creat_tries);
	while(!deliver_tmp->open(Maildir + "tmp/" + maildir_file, "wctx"))
	{
		if(creat_tries++ > MD_MAX_CREAT_TRY) return FAILURE; 
		sleep(MD_TRY_SLEEP);
		maildir_file = create_basename(creat_tries++);
	}
	message_size = deliver_tmp->write(maildir_message);
	deliver_tmp->close();
	if(message_size != strlen(maildir_message))
	{
		rm(maildir_file);
		return FAILURE;
	}
	if(catch(hardlink(Maildir + "tmp/" + maildir_file, Maildir + "new/" + maildir_file)))
	{
		rm(Maildir + "tmp/" + maildir_file);
		return FAILURE;
	}
	else
	{
		mv(Maildir + "new/" + maildir_file, Maildir + "new/" + maildir_file + ",S=" + (string)message_size + ":2,");
		rm(Maildir + "tmp/" + maildir_file);
		return SUCCESS;
	}
}


}
