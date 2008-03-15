/*

  Konnekt UI

  Obs³uga akcji i ikon.

*/

#include "stdafx.h"
#pragma hdrstop

#include <direct.h>
#include <io.h>
#include <Richedit.h>

#include "include\preg.h"

#include "include\dirdialog.h"
#include "include\colorpicker.h"
#include "include\fontpicker.h"
//#include "include\dbtable.h"
//#include "include\dtablebin.h"
#include "include\win_registry.h"

#include "ui_main.h"



#include "ui_actions.h"
#include "ui_cntlist.h"
#include "ui_cntlistfilter.h"
#include "ui_ui.h"


#include "ui_ico.hpp"
#include "ui_actions.hpp"
#include "ui_actwnd.hpp"
#include "ui_dbg.hpp"
#include "ui_actions_set.hpp"
#include "ui_actmod.hpp"
                     
             

/*
title - pogrubiona, wiêksza czcionka - wyœrodkowane
subtitle - mniejszy ni¿ title
copy - informacja o prawach autorskich - mniejsza, przyszarzona, po prawej
warn - wiêksze, pogrubione, czerwone
info - ciut wiêksza czcionka, justowane
note - lekko wyszarzone, justowane
var - nazwa zmiennej
value - wartoœæ zmiennej - pogrubione
under_dot - podkreœlone kropkowan¹ lini¹
*/

void HTMLInfoStyleCB(const CStdString & token , const CStdString & style , cRichEditFormat::cSetStyle & ss) {
	if (style == "title") {
		ss.cf.dwMask |= CFM_BOLD | CFM_SIZE;
		ss.cf.yHeight = PXToHeight(12);
		ss.cf.dwEffects |= CFE_BOLD;
		ss.pf.dwMask |= PFM_ALIGNMENT;
		ss.pf.wAlignment = PFA_CENTER;
	} else if (style == "subtitle") {
		ss.cf.dwMask |= CFM_BOLD | CFM_SIZE | CFM_UNDERLINE | CFM_UNDERLINETYPE;
		ss.cf.bUnderlineType = CFU_UNDERLINEHAIRLINE;
		ss.cf.yHeight += 1;//PXToHeight(10);
		ss.cf.dwEffects |= CFE_BOLD | CFE_UNDERLINE;
		ss.pf.dwMask |= PFM_ALIGNMENT | PFM_SPACEAFTER | PFM_SPACEBEFORE | PFM_STARTINDENT;
		ss.pf.wAlignment = PFA_CENTER;
		ss.pf.dySpaceAfter = ss.pf.dySpaceBefore = PXToHeight(6);
		ss.pf.dxStartIndent = 0; //PXToHeight(10);
	} else if (style == "copy") {
		ss.cf.dwMask |= CFM_SIZE | CFM_COLOR;
		ss.cf.yHeight = PXToHeight(10);
		ss.cf.dwEffects &= ~CFE_AUTOCOLOR;
		ss.cf.crTextColor |= GetSysColor(COLOR_BTNSHADOW);
		ss.pf.dwMask |= PFM_ALIGNMENT;
		ss.pf.wAlignment = PFA_RIGHT;
	} else if (style == "warn") {
		ss.cf.dwMask |= /*CFM_SIZE |*/ CFM_COLOR;
		//ss.cf.yHeight = PXToHeight(10);
//		ss.cf.dwEffects |= CFE_BOLD;
		ss.cf.dwEffects &= ~CFE_AUTOCOLOR;
		ss.cf.crTextColor = 0x0000FF;
	} else if (style == "info") {
		//ss.cf.dwMask |= CFM_SIZE;
		//ss.cf.yHeight = PXToHeight(10);
		ss.pf.dwMask |= PFM_ALIGNMENT;
		ss.pf.wAlignment = PFA_JUSTIFY;
	} else if (style == "note") {
		ss.cf.dwMask |= CFM_COLOR;
		ss.cf.crTextColor |= GetSysColor(COLOR_BTNSHADOW);
		ss.cf.dwEffects &= ~CFE_AUTOCOLOR;
		ss.pf.dwMask |= PFM_ALIGNMENT;
		ss.pf.wAlignment = PFA_JUSTIFY;
	} else if (style == "var") {
		ss.pf.dwMask |= PFM_TABSTOPS;
		ss.pf.cTabCount = 2;
		ss.pf.rgxTabs[0] = PXToHeight(80);
		ss.pf.rgxTabs[1] = PXToHeight(140);
	} else if (style == "value") {
		ss.cf.dwMask |= CFM_BOLD;
		ss.cf.dwEffects |= CFE_BOLD;
	} else if (style == "under_dot") {
		ss.cf.dwMask |= CFM_UNDERLINE | CFM_UNDERLINETYPE;
		ss.cf.dwEffects |= CFE_UNDERLINE;
		ss.cf.bUnderlineType = CFU_UNDERLINEDOTTED;
	}
}


           
int cUIAction::call(unsigned int code , int notify1,int notify2,int cnt) {
  sUIActionNotify_2params an = notify(code , notify1 , notify2);
  if (cnt != AC_CURRENT) an.act.cnt = cnt;
  return IMessageDirect(IM_UIACTION , (tPluginId)owner , (int)&an);
}

int cUIAction::call(sUIActionNotify_base * an) {
	an->act = this->act((an->act.cnt == AC_CURRENT)? this->cnt : an->act.cnt);
//	this->setCnt(an->act.cnt);
    return IMessageDirect(IM_UIACTION , (tPluginId)owner , (int)an);
}

void cUIAction::getInfo(sUIActionInfo * ai) {
	 setCnt(ai->act.cnt);
     ai->act = sUIAction(parent , id);
     ai->handle=handle;
     ai->pos=0;
     ai->type = type;

     if (ai->mask & UIAIM_POS) ai->pos = this->pparent ? this->pparent->find(this->id) : -1;
     if (ai->mask & UIAIM_STATUS) ai->status=status;
     if (ai->mask & UIAIM_EXSTYLE) ai->exStyle=exstyle;
     if (ai->mask & UIAIM_P1) ai->p1=p1;
     if (ai->mask & UIAIM_P2) ai->p2=p2;
     if (ai->mask & UIAIM_PARAM) ai->param = userParam;
     if (ai->mask & UIAIM_SIZE) {ai->w = w; ai->h = h;}
     if (ai->mask & UIAIM_TXT) {
       if (!ai->txt) {ai->txt = TLS().buff;ai->txtSize = MAX_STRING;}
       strncpy(ai->txt , txt.c_str() , ai->txtSize);
     }
}



sUIActionNotify cUIAction::notify(unsigned int code , int notify1,int notify2){
     sUIActionNotify an;
     an.act = sUIAction(parent , id , cnt);
     an.code = code;
     an.notify1 = notify1;
     an.notify2 = notify2;
     return an;
}

void cUIAction_cfg::getInfo(sUIActionInfo * ai) {
     cUIAction::getInfo(ai);
     if (ai->s_size == sizeof(*ai)) return;
     sUIActionInfo_cfg * aic = (sUIActionInfo_cfg*)ai;
     if (aic->mask & UIAIM_POS) {aic->x = x; aic->y = y;}
}



class cUIGroup_cfg : public cUIGroup {

};

cUIGroup::~cUIGroup() {
  //for (t_Act_it Act_it=Act.begin(); Act_it!=Act.end(); Act_it++) {
  //  if (!((*Act_it)->type & ACT_ISGROUP)) delete *Act_it;
  //}

  int c  = Act.size() - 1;
  while (c > 0) {
    remove(c);
    c--;
  }
}


cUIGroup & cUIAction::getGroup() {
   if (!(type & ACT_ISGROUP)) throw exception_plug2(stringf(EMSG_UI "Action %d isn't a group" , "Action to Group conv." , id) , Ctrl->ID() , 1);
   return *((cUIGroup*)this);
}


cUIAction & cUIGroup::operator ()(int id) {
  int pos = find(id);
  if (pos == -1) throw exception_plug2(stringf(EMSG_UI "Action %d/%d doesn't exist!" , "action_get" , this->id , id) , Ctrl->ID() , 1);
  return *Act[pos];
}

cUIAction & cUIGroup::operator [](int pos) {
  if (pos <0||pos>=(int)Act.size()) throw exception_plug2(stringf(EMSG_UI "Action %d/%d doesn't exist!" , "action_get" , this->id , id) , Ctrl->ID() , 1);
  return *Act[pos];
}



/*
cUIAction * cUIGroup::getId(int id) {
  int pos=find(id);
  if (pos < 0) throw 1;
  return &Act[pos];
}

cUIGroup::setId(int id,cUIAction & v) {
  int pos=find(id);
  if (pos<0) throw 1;
  Act[pos]=v;
  return 1;
}
*/

cUIAction * cUIGroup::insert (int id , int pos , const char * txt, int status,int p1 , short w , short h , int p2 , int param , int exstyle , tPluginId owner) {
  sUIActionInfo ai;
  ai.act.id = id;
  ai.act.parent = this->id;
  ai.mask = UIAIM_ALL;
  ai.pos = pos;
  ai.txt = (char*)txt;
  ai.status = status;
  ai.exStyle = 0;
  ai.p1 = p1;
  ai.p2 = p2;
  ai.param = 0;
  ai.w = w;
  ai.h = h;
  return insert(&ai , owner);
}

cUIAction * cUIGroup::cfginsert (int id , int pos , const char * txt, int status,int p1 , short x , short y , short w , short h , int p2 , int param , int exstyle , tPluginId owner) {
  sUIActionInfo_cfg ai;
  ai.act.id = id;
  ai.act.parent = this->id;
  ai.mask = UIAIM_ALL | UIAIM_POS;
  ai.pos = pos;
  ai.txt = (char*)txt;
  ai.status = status;
  ai.exStyle = 0;
  ai.p1 = p1;
  ai.p2 = p2;
  ai.param = 0;
  ai.w = w;
  ai.h = h;
  ai.x = x;
  ai.y = y;
  return insert(&ai , owner);
}




  cUIActions::cUIActions()
    {lastIndex = 0x1000; //1/*0x8000*/;
     cUIGroup * root = new cUIGroup;
     root->id = 0;
     root->parent = 0;
     root->pparent = 0;
     Grp[0] = root;

//     actList = this;
     }

  cUIActions::~cUIActions() {
    for (t_Grp_it Grp_it=Grp.begin(); Grp_it!=Grp.end(); Grp_it++) {
      delete Grp_it->second;
   }
  }

cUIAction & cUIActions::operator ()(sUIAction & uia) {
  if (!exists(uia.parent , uia.id)) throw exception_plug2(stringf(EMSG_UI "Action %d/%d doesn't exist!" , "get group" , uia.parent , uia.id) , Ctrl->ID() , 1);
  cUIAction & act = (*this)[uia.parent](uia.id);
  act.setCnt(uia.cnt);
  return act;
  //return (*this)[uia.parent](uia.id);
}

cUIAction * cUIActions::getIndex (int index) {
  cUIAction * act = 0;
  for (t_Grp_it Grp_it = Grp.begin(); Grp_it != Grp.end(); Grp_it++)
    {
      act = Grp_it->second->getIndex(index);
      if (act) break;
    }
  return act;
}


cUIAction * cUIActions::findId (int id) {
  int found;
  for (t_Grp_it Grp_it = Grp.begin(); Grp_it != Grp.end(); Grp_it++)
    {
      found = Grp_it->second->find(id);
      if (found != -1) return &(*Grp_it->second)[found];
    }
  return 0;
}


cUIGroup & cUIActions::operator [](int n) {
      t_Grp_it Grp_it=Grp.find(n);
      if (Grp_it==Grp.end()) throw exception_plug(_sprintf(EMSG_UI "Group %d doesn't exist!" , "get group" , n) , Ctrl->ID() , 1);
      return *Grp_it->second;
}

cUIAction & cUIActions::operator ()(int id) {
    cUIAction * act = this->findId(id);
    if (!act) throw exception_plug(_sprintf(EMSG_UI "Action %d doesn't exist!" , "get unique action" , id) , Ctrl->ID() , 1);
    return *act;
}


int cUIGroup::find (int id) {
  for (t_Act_it Act_it=Act.begin() ; Act_it!=Act.end(); Act_it++) {
    if ((*Act_it)->id==id) return Act_it-Act.begin();
  }
  return -1;
}

cUIAction * cUIGroup::findAction (int id) {
  for (t_Act_it Act_it=Act.begin() ; Act_it!=Act.end(); Act_it++) {
      if ((*Act_it)->id==id) return *Act_it;
      if ((*Act_it)->type & ACT_ISGROUP) {
          cUIAction * found = (static_cast<cUIGroup*>(*Act_it))->findAction(id);
          if (found) return found;
      }
  }
  return 0;
}

int cUIGroup::findIndex (int index) {
  for (t_Act_it Act_it=Act.begin() ; Act_it!=Act.end(); Act_it++) {
    if ((*Act_it)->index==index) return Act_it-Act.begin();
  }
  return -1;
}


cUIAction * cUIGroup::getIndex (int index) {
  for (t_Act_it Act_it=Act.begin() ; Act_it!=Act.end(); Act_it++) {
    if ((*Act_it)->index==index) return *Act_it;
  }
  return 0;
}

int cUIGroup::findHandle (void * handle) {
  for (t_Act_it Act_it=Act.begin() ; Act_it!=Act.end(); Act_it++) {
    if ((*Act_it)->handle==handle) {
      return Act_it-Act.begin();
      }
  }
  return -1;
}

cUIAction * cUIGroup::insert (sUIActionInfo * ai , tPluginId owner) {
  if (ai->act.id==-1 || ai->act.id == IMIB_CFG) ai->act.id = IMIB_CFG|ai->p1;
  else if (ai->act.id==-2 || ai->act.id == IMIB_CNT) ai->act.id = IMIB_CNT|ai->p1;

  if (!::Act.exists(ai->act.parent) || (ai->act.id && exists(ai->act.id))) 
    return 0;

  cUIAction * v;
  if (ai->status & ACTS_GROUP) {
    if (type & ACT_CFGBRANCH)
      v = new cUIGroup_cfg;
    else
      v = new cUIGroup;
  } else {
    if (type & ACT_CFGBRANCH)
      v = new cUIAction_cfg;
    else
      v = new cUIAction;
  }

  v->type = 0;
  v->type |= type & (ACT_BRANCHMASK|ACT_WNDMASK|ACT_FORCNT); // Dziedziczy typ ga³êzi
  if (ai->status & ACTS_GROUP)
    v->type |= ACT_ISGROUP;
  v->index = ++::Act.lastIndex;
  v->id=ai->act.id;
  v->parent=this->id;
  v->cnt=(type & ACT_FORCNT)?-1:0;
  v->pparent = this;
  v->handle=0;
  v->owner=owner;
  if (ai->pos > Act.size()) ai->pos=Act.size();
  ai->statusMask = -1;
  v->setInfo(*ai , false);

  Act.insert(Act.begin()+ai->pos , v);
  if (ai->status & ACTS_GROUP) {
    ::Act.Grp[ai->act.id] = (cUIGroup*)v;
  }
  return Act[ai->pos];
}

int cUIGroup::remove(int pos) {
	if (pos == -1 || Act[pos]->handle) return 0;
	if (Act[pos]->status & ACTS_GROUP) {
		// wywalamy z listy grup...
		::Act.Grp.erase(Act[pos]->id);
	}
    delete Act[pos];
    Act.erase(Act.begin()+pos);
    return 1;
}

void cUIGroup::move(int old_pos , int pos) {
    if (old_pos < 0 || old_pos >= (int) Act.size()) return;
    cUIAction * temp = Act[old_pos];
    Act.erase(Act.begin()+old_pos);
    if (pos < 0)
        pos = max(0 , Act.size()+pos+1);
    if (pos >= (int) Act.size())
        Act.push_back(temp);
    else
        Act.insert(Act.begin()+pos , temp);
}


cUIAction::cUIAction() {type = parent = id = status = exstyle = w = h = 0;
                        handle = 0;
                        pparent = 0;
                        txt="";
                        params="";
                        index=p1=p2=userParam=0;
						owner=(tPluginId)0;
                        #ifdef __DEBUG
                          dbg=0;
                        #endif
                       }




cUIGroup * cUIAction::getRoot() {
  if (!pparent || !pparent->pparent) return (cUIGroup*)this;
  if (!pparent) return (cUIGroup*)this;
  return pparent->getRoot();
}




bool cUIAction::setCnt(unsigned int _cnt  , bool recursive) {
	if (_cnt == AC_CURRENT) return false;
	if (_cnt != AC_NONE) _cnt = Ctrl->DTgetID(DTCNT , _cnt);
	if (!pparent || (/*this->handle && */this->cnt==_cnt) || !(this->type & ACT_FORCNT)) return false;
	if (_cnt==-1) {
		handle=0;
		this->cnt=_cnt;
		#ifdef __DEBUG
		DbgActionsSet(this , 0);
		#endif
		return true;
	}
	if (status & ACTR_SETCNT) { // Pytamy bezpoœrednio akcji...
		sUIActionNotify_2params an;
		an.code = ACTN_SETCNT;
		an.notify1 = this->cnt;
		an.notify2 = (int)this->handle;
		this->cnt=_cnt;
		(void*)this->call(&an);
		this->handle = (void*)an.notify2;
		#ifdef __DEBUG
		DbgActionsSet(this , 0);
		#endif
		return true;
	}
	this->cnt=_cnt;
	if (pparent && pparent->type & ACT_FORCNT && pparent->cnt!=_cnt)
		{pparent->setCnt(_cnt,false);}
  
	switch (this->getType()) {
		case ACT_CFG:
			if (type & ACT_ISGROUP) {
			} else {
				switch (this->statusType()) {
					case ACTT_GROUPEND: handle=(HANDLE)-1;break;
					default:handle = GetDlgItem((HWND)pparent->ghandle , this->index);
				}
			}
			break;
		default: handle = 0;
	}
	#ifdef __DEBUG
	DbgActionsSet(this , 0);
	#endif
	//IMLOG("handleSET %p cnt=%x [%d]" , handle , this->cnt , id);
	return true;
//  this->cnt=_cnt;
}
bool cUIGroup::setCnt(unsigned int _cnt  , bool recursive) {
  if (_cnt == AC_CURRENT) return false;
  bool renew = cUIAction::setCnt(_cnt , recursive);
  if (!pparent) return false;
  if (renew && (this->type & ACT_FORCNT) && _cnt!=-1) {
    if (pparent && !pparent->pparent) { // Root grupy akcji. Trzeba pobraæ HWND
      switch(this->type & ACT_WNDMASK) {
        case ACT_WNDMAIN: this->ghandle=hwndMain; break;
        case ACT_WNDCFG: this->ghandle=hwndConfig; break;
        case ACT_WNDMSG: this->ghandle=Cnt[_cnt].hwndMsg; break;
        case ACT_WNDNFO: this->ghandle=Cnt[_cnt].hwndInfo; break;
        default: this->ghandle=0;
      }
    } else { // pparent ....
      switch (this->getType()) {
        case ACT_CFG: this->ghandle=GetDlgItem((HWND)getRoot()->ghandle , this->index);break;
        case ACT_BAR:this->ghandle=GetDlgItem((HWND)pparent->ghandle , this->index);break;
        case ACT_TB:this->ghandle=this->handle=GetDlgItem((HWND)pparent->ghandle , this->index);break;
      }
    }
  }
  if (_cnt==-1) {this->ghandle=0;} // this->cnt!=_cnt
  for (t_Act_it Act_it=Act.begin() ; Act_it!=Act.end(); Act_it++) {
     (*Act_it)->setCnt(_cnt , recursive);
  }
  //if (renew) IMLOG("GhandleSET %p cnt=%x [%d]" , ghandle , this->cnt , id);  
  return renew;
}

// zwraca domyslna akcje (ACTN_DEFAULT)
cUIAction * cUIGroup::getDefaultAction(void)
{
    for (Act_it_t act=Act.begin(); act != Act.end(); act++) {
        if ((*act)->call(&sUIActionNotify_2params(NOACTION , ACTN_DEFAULT , 0 , 0)))
            return *act;
    }
    return NULL;
}



ToolTipX::TipPtr TargetAction::getTip(ToolTipX * ttx) {
	//IMLOG("getTip() - _tip = %x _act = %x", _tip.get(), _act);
	if (_tip) return _tip;
	if (_act) {
		//OutputDebugStr("\nNewTip");
		this->_tip = ToolTipX::TipPtr(_act->getTipObject(this->_useText));
	}
	return _tip;
}

bool TargetActionRebar::mouseAtTarget(ToolTipX * ttx, HWND window, POINT screen, POINT local) {
	HWND target = WindowFromPoint(screen);
	DWORD targetProcId = 0;
	GetWindowThreadProcessId(target, &targetProcId);
	//IMLOG("tip::mat t=%x tpid=%x cpid=%x tact=%x cact=%x", target, targetProcId, GetCurrentProcessId(), GetProp(target, "ACTION"), this->_act);
	if (targetProcId != GetCurrentProcessId())
		return false;
//	if ((HINSTANCE)GetWindowLong(target, GWL_HINSTANCE) != Ctrl->hInst())
//		return false;
	cUIAction * action = (cUIAction *) GetProp(target, "ACTION");
	if (!action) 
		return false;
	if (action->getType() == ACT_TB) {
		//IMLOG("TB");
		POINT tbLocal = screen;
		ScreenToClient(target, &tbLocal);
		int index = abs(SendMessage(target, TB_HITTEST, 0, (LPARAM)&tbLocal));
		TBBUTTON tbb;
		if (!SendMessage(target, TB_GETBUTTON, index, (LPARAM)&tbb) || (tbb.fsState & TBSTATE_HIDDEN))
			return false;
		this->_useText = !(tbb.fsStyle & BTNS_SHOWTEXT);
		action = Act.getIndex(tbb.idCommand);
		if (!action)
			return false;
	} else {
		this->_useText = false;
	}
	//IMLOG("_act = %x action = %x useText = %d tip = %d", _act, action, _useText, _tip.get());
	if (this->_act == action)
		return true;
	if (!action || !action->hasTipObject(this->_useText))
		return false;
	this->_act = action;
	this->_tip.reset();
	ttx->refresh();
	return true;
}

void TipAction::fillCache() {
	this->_cachedTip = ToolTipX::TipPtr(_act->getTipObject());
}


bool TargetActionTaskbar::mouseAtTarget(ToolTipX * ttx, HWND window, POINT screen, POINT local) {
	if (local.y == -1 && local.x == this->_id) {
		this->_screen = screen;
		return true;
	} else {
		if (this->_screen.x == screen.x && this->_screen.y == screen.y)
			return true;
		else {
			this->_screen = Point(0,0);
			return false;
		}
	}
}
void TargetActionTaskbar::onHide(ToolTipX * ttx) {
	ttx->reset();
}
void TargetActionTaskbar::onShow(ToolTipX * ttx) {
	ttx->setPos(this->_screen, false, ToolTipX::positionFixed, ToolTipX::pRightTop);
}

void TipActionTaskBar::fillCache() {
	ToolTipX::Tip * tip = new ToolTipX::Tip;
	int ico = trayNotify ? trayNotify : ICON_KONNEKT;
	if ( ! trayInfo.empty()) {
        cPreg preg;
		if (ISICON_BLANK(ico)) {ico&=~IT_BLANK_MASK;}
		if (ico & UIIcon(-1,0,0,0)) ico &= UIIcon(-1, -1, -1, 0);
		int width = GetTextSize(tooltip.getTipWindow(), font, preg.replace("/<.*?>/", "", trayInfo).c_str()).cx + 10;
		width = max(width, 100);
		width = min(width, 200);
		tip->add(new ToolTipX::TipOnlyTitle("", Ico.getImageList(IML_16), Ico.getIconIndex(ico, IML_16)), ToolTipX::Tip::alignLeft);
		if (trayInfo.find('<') != trayInfo.npos) {
			tip->add(new ToolTipX::TipTextRich(preg.replace("/\r?\n/", "<br/>", trayInfo), font, HTMLInfoStyleCB, width));
		} else {
			tip->add(new ToolTipX::TipText(trayInfo, font, width));
		}
		//tip->add(new ToolTipX::TipOnlyTitle(trayInfo, Ico.getImageList(IML_16), Ico.getIconIndex(ico, IML_16), font));
	}
	// dodajemy sieci...
	if (GETINT(CFG_UITRAYTOOLTIPSHOWSTATUS)) {
		int c = IMessage(IMC_PLUG_COUNT);
		for (int i=0;i<c;i++) {
			oPlugin plugin = Ctrl->getPlugin((tPluginId)i);
			if (plugin->getType() & (imtProtocol | imtNet)) {
				int status = plugin->IMessageDirect(IM_GET_STATUS);
				if (Ctrl->getError() == errorNoResult) continue;
				int net = plugin->getNet();
				String statusInfo = SAFECHAR((char*)plugin->IMessageDirect(IM_GET_STATUSINFO));
				String netName = plugin->getNetName();
	//			tip->add(new ToolTipX::TipOnlyTitle(netName + string(": ") + getStatusName(status), Ico.getImageList(IML_16), Ico.getIconIndex(UIIcon(IT_STATUS, net, status, 0), IML_16), font));
				tip->add(new ToolTipX::TipOnlyTitle("", (HICON)Ico.iconGet(UIIcon(IT_STATUS, net, status, 0), IML_ICO2), true, font), ToolTipX::Tip::alignWrap);
				String text = netName;
				text += ": ";
				text += getStatusName(status);
				if (!statusInfo.empty()) {
					text += "\r\n" + statusInfo;
				}
				tip->add(new ToolTipX::TipText(text.c_str(), font, 150), ToolTipX::Tip::alignRight);

//				tip->add(new ToolTipX::TipOnlyTitle(netName + string(": ") + getStatusName(status), (HICON)Ico.iconGet(UIIcon(IT_STATUS, net, status, 0), IML_ICO2), true, font));
			}
		}
	}
	if (tip->empty()) {
		tip->add(new ToolTipX::TipOnlyTitle("Konnekt", Ico.getImageList(IML_16), Ico.getIconIndex(ICON_KONNEKT, IML_16), fontB));
	}
	this->_cachedTip = ToolTipX::TipPtr(tip);
}


void TargetActionTipButton::attach(ToolTipX * ttx) {
	SendMessage(this->_button, BM_SETCHECK, BST_CHECKED, 0);
	ToolTipX::TargetFixed::attach(ttx);
}
void TargetActionTipButton::detach(ToolTipX * ttx) {
	SendMessage(this->_button, BM_SETCHECK, BST_UNCHECKED, 0);
	ToolTipX::TargetFixed::detach(ttx);
}
