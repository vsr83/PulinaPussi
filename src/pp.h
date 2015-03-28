/*
 * pp.h / PulinaPussi 0.13
 * written by Ville Räisänen <raivil@geek.com> 2001-2003
 * tällätteet kyl kannattais varmaan tehä pärlil :)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */


#ifndef __PP_H__
#define __PP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "wrap.h"
#include "common/parse.h"
#include "common/llist.h"
#include "types.h"   /*IRCSession*/
#include "plugin.h"
#include "common/pref.h"
#include "common/socket.h"
#include "common/ctcp.h"
#include "common/fout.h"
#include "common/fin.h"

#ifdef WIN32
#include <windows.h>
#include <commctrl.h>

//#define Strcasecmp(a,b) strcasecmp(a,b)

#define bzero(a,b)   memset(a, 0, b)
#define bcopy(a,b,c) memcpy(b, a, c)
#else
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#endif


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define MAXLEN  1024
#define SMAXLEN 512

#define DEFAULTPORT 6667
#define DEFAULTSERVER "irc.inet.fi"

#define IRCO_MESSAGE          0
#define IRCO_MESSAGE_ALL      1
#define IRCO_MESSAGE_USER_ALL 2
#define IRCO_USERLIST_ADD     3
#define IRCO_USERLIST_REMOVE  4
#define IRCO_USERLIST_SETMODE 5
#define IRCO_USERLISTS_ADD    6
#define IRCO_USERLISTS_REMOVE 7
#define IRCO_USERLISTS_NICK   8
#define IRCO_SETTOPIC         9
#define IRCO_NICKNAME_UPDATE  64

#define DELD(s) ((s && s[0]==':') ? 1:0)
#define VERSION "PulinaPussi v0.12 (c) Ville Räisänen 2001, 2002, 2003 -- http://www.sourceforge.org/projects/pulinapussi"

#define IS_NICKC(c) (isalpha(c) || isdigit(c) || c=='_' || c=='-'|| \
	      c=='[' || c==']' || c=='\\' || c=='{' || c=='}' ||\
	      c=='\'' || c=='^')

#define Free(p) if (p){free(p);\
p=NULL;}

/*UI-wrappers for compability with multiple platforms.*/
int Client_msgf     (IRCSession *, int , char *, char *, int);
int Debug_msgf      (IRCSession *, char *, int);

/*CURSES-redraw(replace this!)*/
int Client_drawp    (IRCSession *);
/******************************************************/

#define ERR_NOSUCHNICK       401
#define ERR_NOSUCHSERVER     402
#define ERR_NOSUCHCHANNEL    403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_TOOMANYCHANNELS  405
#define ERR_WASNOSUCHNICK    406
#define ERR_TOOMANYTARGETS   407
#define ERR_NOORIGIN         408
#define ERR_NORECIPIENT      411
#define ERR_NOTEXTTOSEND     412
#define ERR_NOTOPLEVEL       413
#define ERR_WILDTOPLEVEL     414
#define ERR_UNKNOWNCOMMAND   421
#define ERR_NOMOTD           422
#define ERR_NOADMININFO      423
#define ERR_FILEERROR        424
#define ERR_NONICKNAMEGIVEN  431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE    433
#define ERR_NICKCOLLISION    436
#define ERR_USERNOTINCHANNEL 441
#define ERR_NOTONCHANNEL     442
#define ERR_USERONCHANNEL    443
#define ERR_NOLOGIN          444
#define ERR_SUMMONDISABLED   445
#define ERR_USERSDISABLED    446
#define ERR_NOTREGISTERED    451
#define ERR_NEEDMOREPARAMS   461
#define ERR_ALREADYREGISTRED 462
#define ERR_NOPERMFORHOST    463
#define ERR_PASSWDMISCMATCH  464
#define ERR_YOUREBANNEDCREEP 465
#define ERR_KEYSET           467
#define ERR_CHANNELISFULL    471
#define ERR_UNKNOWNMODE      472
#define ERR_INVITEONLYCHAN   473
#define ERR_BANNEDFROMCHAN   474
#define ERR_BADCHANNELKEY    475
#define ERR_NOPRIVILEGES     481
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_CHANKILLSERVER   483
#define ERR_NOOPERHOST       491
#define ERR_UMODEUNKNOWNFLAG 501
#define ERR_USERSDONTMATCH   502

#define RPL_NONE             300
#define RPL_AWAY             301
#define RPL_USERHOST         302
#define RPL_ISON             303
#define RPL_UNAWAY           305
#define RPL_NOAWAY           306
#define RPL_WHOISUSER        311
#define RPL_WHOISSERVER      312
#define RPL_WHOISOPERATOR    313
#define RPL_WHOISIDLE        317
#define RPL_ENDOFWHOIS       318
#define RPL_WHOISCHANNELS    319
#define RPL_WHOWASUSER       314
#define RPL_ENDOFWHOWAS      369
#define RPL_LISTSTART        321
#define RPL_LIST             322
#define RPL_LISTEND          323
#define RPL_CHANNELMODEIS    324
#define RPL_NOTOPIC          331
#define RPL_TOPIC            332
#define RPL_INVITING         341
#define RPL_SUMMONING        342
#define RPL_VERSION          351
#define RPL_WHOREPLY         352
#define RPL_ENDOFWHO         315
#define RPL_NAMREPLY         353
#define RPL_ENDOFNAMES       366
#define RPL_LINKS            364
#define RPL_ENDOFLINKS       365
#define RPL_BANLIST          367
#define RPL_ENDOFBANLIST     368
#define RPL_INFO             371
#define RPL_ENDOFINFO        374
#define RPL_MOTDSTART        375
#define RPL_MOTF             372
#define RPL_ENDOFMOTD        376
#define RPL_YOUREOPER        381
#define RPL_RESHAKING        382
#define RPL_TIME             391
#define RPL_USERSSTART       392
#define RPL_ENDOFUSERS       394
#define RPL_NOUSERS          395
#define RPL_TRACELINK        200
#define RPL_TRACECONNECTING  201
#define RPL_TRACEHANDSHAKE   202
#define RPL_TRACEUNKNOWN     203
#define RPL_TRACEOPERATOR    204
#define RPL_TRACEUSER        205
#define RPL_TRACESERVER      206
#define RPL_TRACENEWTYPE     208
#define RPL_TRACELOG         261
#define RPL_STATSLINKINFO    211
#define RPL_STATSCOMMANDS    212
#define RPL_STATSCLINE       213
#define RPL_STATSNLINE       214
#define RPL_STATSILINE       215
#define RPL_STATSKLINE       216
#define RPL_STATSYLINE       218
#define RPL_ENDOFSTATS       219
#define RPL_STATSLLINE       241
#define RPL_STATSUPTIME      242
#define RPL_STATSOLINE       243
#define RPL_STATSHLINE       244
#define RPL_UMODEIS          221
#define RPL_LUSERCLIENT      251
#define RPL_LUSEROP          252
#define RPL_LUSERUNKNOWN     253
#define RPL_LUSERCHANNELS    254
#define RPL_LUSERME          255
#define RPL_ADMINME          256
#define RPL_ADMINLOC1        257
#define RPL_ADMINLOC2        258
#define RPL_ADMINEMAIL       259

#define HELPMSG \
"-Built-in commands --------------------------------------------------\n"\
"/FINGER /PING /TIME /VERSION    [USER] : CTCP operations     \n"\
"/OP /DEOP /VOICE /DEVOICE /KICK [USER] : CHANNEL operations  \n"\
"/ME /MSG /LEAVE /PART /NICK            : Common stuff        \n"\
"/CHANNEL [CHANNEL]                     : Switch Channel(TEXT)\n"\
"/CHANLIST                              : List Channels\n"\
".......................................:.............................\n"\
"/SERVER [HOST:PORT]                    : Connect to a server \n"\
"/QUIT   [MESSAGE]                      : Quit IRC            \n"\
".......................................:.............................\n"\
"/SET    [OPTION]/NULL   [VALUE]        : Preferences         \n"\
"/PLUGIN LOAD/CLOSE/NULL [FILENAME]     : Plugin Operations   \n"\
".......................................:.............................\n"\
"/CLOSE                                 : Close Current Window(GUI)\n"\
"/CLEAR                                 : Clear Current Window\n"\
"/ME \n"\
"---------------------------------------------------------------------\n"
/*
"/QUIT [MSG]                    /LICENSE /HELP               \n\n"\
"/CHANNEL /CHANLIST                 \n"\
"/CLOSE /CLEAR                      \n"\
"/FINGER /PING /TIME /VERSION      [USER]  CTCP-shit\n"\
"/OP /DEOP /VOICE /DEVOICE /KICK   [USER]  CHANNEL-shit \n"\
"/ME                               [MSG]\n\n"
*/


#define LICENSEMSG \
"PulinaPussi 0.12\n"\
"written by Ville Räisänen <spuuki@koti.mbnet.fi> 2001, 2002, 2003\n"\
"tällätteet kyl kannattais varmaan tehä pärlil :)\n"\
"\n"\
"This program is free software; you can redistribute it and/or modify\n"\
"it under the terms of the GNU General Public License as published by\n"\
"the Free Software Foundation; either version 2 of the License, or\n"\
"(at your option) any later version.\n"\
"\n"\
"This program is distributed in the hope that it will be useful,\n"\
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
"GNU General Public License for more details.\n"\
"\n"\
"You should have received a copy of the GNU General Public License\n"\
"along with this program; if not, write to the Free Software\n"\
"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"\
"\n"\
"For more details see the file COPYING.\n\n"
 

#define WMSG \
"PulinaPussi 0.12, Copyright (c) 2001, 2002, 2003 Ville Räisänen(spuuki@koti.mbnet.fi)\n"\
"PulinaPussi comes with ABSOLUTELY NO WARRANTY; for details type '/LICENSE'\n\n"

#endif

