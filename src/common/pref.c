/*
 * pref.c / PulinaPussi 0.12
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

#include "../pp.h"

int
PPopt_add(LLItem *ll, char *desc, int vtype, int lvalue, char *svalue) {
  PPopt *ppnew;

  if (LList_exists(ll, desc)) return 0;

  ppnew= (PPopt*)Calloc(1, sizeof(PPopt));

  ppnew->vtype = vtype;
  ppnew->lvalue= lvalue;
  if (svalue) {
    ppnew->svalue= Strdup(svalue);
  } else {
    ppnew->svalue= NULL;
  }
  ppnew->cb= NULL;
  ppnew->cbdata=NULL;

  LList_add(ll, desc, (void*)ppnew);
  return 1;
}

int
PPopt_setvalue(LLItem *ll, char *desc, int lvalue, char *svalue) {
  LLItem *lli= LList_get(ll, desc);
  PPopt *ppo;

  if (!lli || !lli->data) return 0;
  ppo= (PPopt*)lli->data;

  ppo->lvalue= lvalue;
  if (svalue) {
    Free(ppo->svalue);
    ppo->svalue= Strdup(svalue);
  }

  if (ppo->cb) ppo->cb((void*)ppo);

  return 1;
}

void *
PPopt_getvalue(LLItem *ll, char *desc, int *vtype) {
  LLItem *lli= LList_get(ll, desc);
  PPopt *ppo;

  if (vtype) *vtype= 0;

  if (!lli || !lli->data) return 0;
  ppo= (PPopt*) lli->data;

  if (vtype) *vtype= ppo->vtype;

  if (ppo->vtype == OPTTYPE_STRING || ppo->vtype==OPTTYPE_MSG) {
    return ppo->svalue;
  }else {
    return (void*)ppo->lvalue;
  }
  return 0;
}

int
PPopt_print(IRCSession *IRCs, LLItem *ll, char *ds) {
  void *v;
  int vtype;

  v= PPopt_getvalue(ll, ds, &vtype);
  if (vtype) {
    char s[128];
    if (vtype== OPTTYPE_MSG) {
      snprintf(s, 126, "%s", (char*)v);
    } 
    if (vtype== OPTTYPE_STRING) {
      snprintf(s, 126, "%20s = \"%s\"\n", ds,(char*)v);      
    }
    if (vtype== OPTTYPE_LONG) {
      snprintf(s, 126, "%20s = %d\n", ds, (long)v);
    }
    if (vtype== OPTTYPE_BOOL) {
	snprintf(s, 126, "%20s = %s\n", ds, 
		 ((long)v)?"TRUE":"FALSE");      
    }
    Client_msgf(IRCs, 0, "[server]", s, 0);
  }
  return 1;
}

int
PPopt_printall(IRCSession *IRCs, LLItem *ll) {
  while (ll) {
    ll= (LLItem*)ll->next;
    if (ll && ll->name) {
      PPopt_print(IRCs, ll, ll->name);
    }
  }
  return 1;
}

/*ugly*/
void 
PPopt_logging(void *_ppo) {
  IRCSession *IRCs;
  PPopt *ppo= (PPopt*)_ppo;
  
  IRCs= (IRCSession*)ppo->cbdata;

  if (!ppo || !IRCs) return;

  /* /SET LOGGING TRUE/FALSE */
  if (ppo->vtype == OPTTYPE_BOOL) {

    if (ppo->lvalue == TRUE){ 
      if (IRCs->logfilename) IRCs->logfile= fopen(IRCs->logfilename, "a");
      if (!IRCs->logfile) {
	Client_msgf(IRCs, 0, "[server]", 
		    strdup_printf("can't open %s for writing\n", 
				  IRCs->logfilename), TRUE);
	Free(IRCs->logfilename);
	ppo->lvalue= FALSE;
      }
    }else {
      if (IRCs->logfile) {
	fclose(IRCs->logfile);
	IRCs->logfile=NULL;
      }
    }
  }

  /* /SET LOGFILENAME [LOGFILENAME] */
  if (ppo->vtype == OPTTYPE_STRING) { 
    Free(IRCs->logfilename);
    IRCs->logfilename= Strdup(ppo->svalue);
    if (IRCs->logfile) {
      fclose(IRCs->logfile);
      IRCs->logfile=NULL;
      if (IRCs->logfilename) {
	IRCs->logfile= fopen(IRCs->logfilename, "a");
	if (!IRCs->logfile) {
	  Client_msgf(IRCs, 0, "[server]", 
		      strdup_printf("can't open %s for writing\n", 
				    IRCs->logfilename), TRUE);
	  Free(IRCs->logfilename);
	  ppo->lvalue= FALSE;
	}
      }
    }  
  }

  if (IRCs->logfile) {
    Client_msgf(IRCs, 0, "[server]", 
		strdup_printf("Logging is on(%s)\n", IRCs->logfilename), TRUE);
  }else {
    if (ppo->vtype != OPTTYPE_STRING) 
    Client_msgf(IRCs, 0, "[server]", 
	      strdup_printf("Logging is off(%s).\n", IRCs->logfilename), TRUE);
  }
}
int
PPopt_addcb(LLItem *ll, char *desc, void (*cb)(void*), void *data) {
  PPopt *ppo;
  LLItem *lli= LList_get(ll, desc);

  if (!lli || !lli->data) return 0;
  ppo= (PPopt*)lli->data;
  ppo->cbdata=data;
  ppo->cb= cb;

  if (ppo->cb) ppo->cb((void*)ppo);
  return TRUE;
}

int
PPopt_initdefaults(IRCSession *IRCs, LLItem *ll) {
  PPopt_add(ll, "msg1",           OPTTYPE_MSG   , FALSE, 
	    "- Common shit -----------------\n");

  PPopt_add(ll, "paranoid",       OPTTYPE_BOOL  , TRUE , NULL);

  PPopt_add(ll, "show_colors",    OPTTYPE_BOOL  , TRUE , NULL);
  PPopt_add(ll, "show_hilightown",OPTTYPE_BOOL  , TRUE , NULL);
  PPopt_add(ll, "timestamp_logs", OPTTYPE_BOOL  , TRUE , NULL);
  PPopt_add(ll, "CTCP",           OPTTYPE_BOOL  , TRUE , NULL);
  PPopt_add(ll, "DCC",            OPTTYPE_BOOL  , FALSE, NULL);  

  PPopt_add(ll, "logfilename",    OPTTYPE_STRING, 0,    "vammalog"); 
  PPopt_addcb(ll, "logfilename",   PPopt_logging, IRCs);
  PPopt_add(ll, "logging",        OPTTYPE_BOOL  , FALSE, NULL);  
  PPopt_addcb(ll, "logging",       PPopt_logging, IRCs);

  PPopt_add(ll, "show_awaymsg",   OPTTYPE_BOOL  , FALSE, NULL);
  PPopt_add(ll, "automatic_away", OPTTYPE_BOOL  , FALSE, NULL);
  PPopt_add(ll, "away_timeout",   OPTTYPE_LONG  , 60,    NULL);
  PPopt_add(ll, "away_reason",    OPTTYPE_STRING, 60,    "gone");

  PPopt_add(ll, "DCC_timeout",    OPTTYPE_LONG  , 60,    NULL);
  PPopt_add(ll, "max_channels",    OPTTYPE_LONG  , 50,    NULL);

  return TRUE;
}
