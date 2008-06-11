/*

  Konnekt UI

  Obs³uga listy kontaktów.

*/


#include "stdafx.h"
#pragma hdrstop

#include "include\win_listview.h"
#include "include\preg.h"
#include "include\fontpicker.h"

#include "ui_main.h"

#include "ui_actions.h"
#include "ui_msgcontrol.h"
#include "ui_cntlist.h"
#include "ui_cntlistfilter.h"
#include "ui_window.h"
#include "ui_dialogs.h"
#include "ui_ui.h"

#define CLINFO_MARGIN 4

cCntList cntList;   
cCntListWindow * cntListWnd = 0;
cCntListTip * cntTip = 0;
  void sUICnt::close () {
       if (hwndMsg) DestroyWindow(hwndMsg);
       if (hwndInfo) DestroyWindow(hwndInfo);
       hwndMsg = hwndInfo = 0;
       }

  void sUICnt::MsgGetHandles() {
  }
  int sUICnt::MsgUpdate(bool getHandles) {
    if (!hwndMsg) return 0;
	ActionStatus(Act[IMIG_MSGTB](IMIA_MSG_INFO).act(this->ID),-1,0,this->GetStatusIcon(false));
    SendMessage(hwndMsg , MYWM_REFRESH , 0 , 0);
    return 1;
  }

void sUICnt::setStatus(int status , string info) {
    if (this->hwndMsg && status!=-1 && GETINT(CFG_UIMSGSHOWSTATUS) && _msgControl) {
        this->_msgControl->statusInsert(status & ST_MASK , info);
    }
}
int  sUICnt::GetStatusIcon(bool sameStatus) {
  int icon = GETCNTI(this->ID , Contact::colStatusIcon);
	if (this->status & ST_COMPOSING)
		icon = UIIcon(IT_STATUS , 0 , ST_COMPOSING , 0);
	if (!icon) {
    if (this->net == Net::none)
			icon = IDI_NET_NONE;
		else
			icon = sameStatus
				?UIIcon(IT_STATUS,0,this->status & CNTM_STATUS,0)
				:UIIcon(IT_STATUS,this->net,this->status & CNTM_STATUS,0);
	}
// Sprawdzamy czy ikonka w ogole istnieje, a jak nie to bierzemy sobie inna...
	if (!Ico.find(icon)) 
		icon=UIIcon(IT_STATUS,0,this->status & CNTM_STATUS,0);
	return icon;
}

void sUICnt::cfgSet() {
    // Kolorki
    if (!this->hwndMsg) return;
//    LOGFONTEX lfeBg = StrToLogFont(GETSTR(CFG_UIF_MSG)); //tlo
    LOGFONTEX lfe = StrToLogFont(GETSTR(CFG_UIF_MSGSEND)); //czcionka
    //HWND hwnd = GetDlgItem(this->hwndMsg , IDC_EDIT);
/*    SetProp(hwnd , "Color" , (HANDLE)lfe.color);
    SetProp(hwnd , "BGColor" , (HANDLE)lfe.bgColor);
    SendMessage(hwnd , WM_SETFONT , (WPARAM)fontMsgSend , 1);*/

}


void sUICnt::MsgSend() {
	// Szykujemy obiekt wiadomoœci...

  CStdString buff;

  char UIDbuff[500];
  Message m;

  m.setId(0);
  m.setNet((Net::tNet)GETCNTI(this->ID , Contact::colNet));
  m.setType(Message::typeMessage);
  m.setFromUid("");
  m.setToUid(GETCNTC(this->ID , Contact::colUid , UIDbuff , 500));
  m.setExt("");
  m.setFlags(Message::flagSend);
  m.setNotify(0);
  m.setAction(NOACTION);
  m.setTime(Time64(true));

	// Wczytujemy treœæ	
	sUIAction sendAct = sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend , this->ID);
	Konnekt::UI::Notify::_getMessage gm (0,0);
	Act(sendAct).setCnt(this->ID);
	gm._size = Act(sendAct).call(Konnekt::UI::Notify::getMessageSize , 0 , 0);
	if (!gm._size) gm._size = 65000;
	gm._size ++;
	gm._message = &m;
	Act(sendAct).call(&gm);
	// Czyscimy
    RegEx r;
    buff = r.replace("/(^[\\r\\n ]+)/g" , "" , m.getBody().a_str());
    buff = r.replace("/([\\r\\n ]+)$/g" , "" , buff);
    if (!buff.size()) {
        Message m;
        m.setNet((Net::tNet)this->net);
        m.setType(Message::typeQuickEvent);
        m.setFromUid(GETCNTC(this->ID , Contact::colUid));
        m.setToUid("");
        m.setBody("Wpisz wiadomoœæ!");
        m.setExt("");
        m.setFlags(Message::flagHandledByUI);
        m.setAction(NOACTION);
        m.setNotify(0);
        ICMessage(Message::IM::imcNewMessage, (int)&m,0);
        return;
	}
	if (msgHistory.size()<2 || msgHistory[1].body!=buff || msgHistory[1].flag!=m.getFlags() || msgHistory[1].type!=m.getType()) {
    this->msgHistory.insert(msgHistory.begin()+1 , cMsgHistoryEntry(buff , m.getExt().a_str() , m.getType() , m.getFlags())); // wrzucamy nowy "wpis"
        // czyscimy jak jest ich za duzo...
        if (msgHistory.size()>MSGSENDHISTORY_LIMIT)
            msgHistory.erase(msgHistory.begin()+MSGSENDHISTORY_LIMIT , msgHistory.end());
    }
    msgHistoryPos = 0;
    ActionStatus(sUIAction(IMIG_MSGTB , IMIA_MSG_SEND , this->ID) , ACTS_DISABLED);
    SetProp(hwndMsg , "SENDING" , (void*)1);
    m.setBody(buff.c_str()); //wrzucamy najaktualniejsza wersje
    ICMessage(Message::IM::imcNewMessage, (int)&m,0);
    int session = (int)GetProp(hwndMsg , "MsgSession");
    if (!session) SetProp(hwndMsg , "MsgSession" , (void*)1);
    //  sTime64 * st = (sTime64*)(&m.time);
	if (!(m.getFlags() & Message::flagHide)) {
		this->_msgControl->msgInsert(&m);
	}
	SetDlgItemText(hwndMsg , Act(sendAct).index , "");
    MessageSelect mSel;
    mSel.id  = m.getId();
    if (mSel.id) ICMessage(MessageSelect::IM::imcMessageQueue, (int)&mSel);
    SetProp(hwndMsg , "SENDING" , 0);
    ActionStatus(sUIAction(IMIG_MSGTB , IMIA_MSG_SEND , this->ID) , 0);
}
void sUICnt::MsgWndOpen(bool queue , bool autoPopup) {
	using namespace Konnekt::UI;
	CFG::enMsgPopup popup = autoPopup ? (CFG::enMsgPopup) GETINT(CFG_UIMSGPOPUP) : CFG::mpFocused;
    if (!hwndMsg) {
        //msgStatus = status & ST_MASK;
        hwndMsg=CreateDialogParam(Ctrl->hDll() , MAKEINTRESOURCE(IDD_MSG)
                                , 0/*hwndTop*/ , (DLGPROC)MsgDialogProc , (long)this);
		cfgSet();
		onSizeMsg(hwndMsg , 0,0);
	}
	if (popup == CFG::mpFocused) {
		if (IsIconic(hwndMsg)) OpenIcon(hwndMsg);
		SetForegroundWindow(hwndMsg);
		SetActiveWindow(hwndMsg);
		SetWindowPos(hwndMsg , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetFocus(GetDlgItem(hwndMsg , Act(sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend , this->ID)).index));
	} else {
		ShowWindow(hwndMsg, popup == CFG::mpBackground ? SW_SHOWNOACTIVATE : popup == CFG::mpMinimized ? SW_SHOWMINNOACTIVE : SW_SHOW);
	}
	if (queue) {
    ICMessage(MessageSelect::IM::imcMessageQueue, (int)&MessageSelect((Net::tNet)net , GETCNTC(this->ID,Contact::colUid) , Message::typeAll ,Message::flagNone , Message::flagSend));
	}
}
void sUICnt::MsgWndClose(){
    this->hwndMsg = 0;
    delete this->_msgControl;
    _msgControl = 0;
/*
	sUIActionNotify_destroyWindow dw;
	Act(sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend , this->ID)).call(&dw);
*/
}



void sUICnt::InfoWndOpen(bool newUser) {
    if (!hwndInfo) {
        hwndInfo=CreateDialogParam(Ctrl->hDll() , MAKEINTRESOURCE(IDD_INFO)
                            , /*hwndTop*/ 0 , (DLGPROC)InfoDialogProc , (long)this);
        if (newUser) {SetProp(hwndInfo , "NEWContact" , (void*)1);}
        SendMessage(hwndInfo , MYWM_REFRESH , 0 , 0 );
        ShowWindow(hwndInfo, SW_SHOW);

        }
    else SetForegroundWindow(hwndInfo);
}



bool sUICnt::haveMsgHistory(bool next) {
    if (next)
		return (this->msgHistoryPos > 0);
	else 
        return (this->msgHistoryPos < (int)this->msgHistory.size()-1);
}

void sUICnt::useMsgHistory(bool next) {
	if (!haveMsgHistory(next)) return;
	cUIAction & sendAct = Act(sUIAction(IMIG_MSGWND , UI::ACT::msg_ctrlsend , this->ID));
	sendAct.setCnt(this->ID);
	int length = sendAct.call(UI::Notify::getMessageSize , 0 , 0);
	UI::Notify::_getSelection gs;
	sendAct.call(&gs);
    if (!next) { // Poprzedni
		// Trzeba zapamiêtaæ aktualnego...
        if (this->msgHistoryPos==0) {
			Message m;
			UI::Notify::_getMessage gm (&m,0);
      gm._size = length + 1;
			cMsgHistoryEntry & mhe = this->msgHistory[0];
			sendAct.call(&gm);
      mhe.ext = m.getExt().a_str();
			mhe.flag = m.getFlags();
			mhe.type = m.getType();
		}
		++this->msgHistoryPos;
	} else { // Nastêpny
		--this->msgHistoryPos;
	}
	Message m;
	cMsgHistoryEntry & mhe = this->msgHistory[this->msgHistoryPos];
	m.setBody(mhe.body.c_str());
	m.setExt(mhe.ext.c_str());
	m.setType((Message::enType)mhe.type);
	m.setFlags((Message::enFlags)mhe.flag);
	UI::Notify::_insertMsg im(&m , 0 , false);
	sendAct.call(&im);
}


void sUICnt::clearMsgHistory() {
	this->msgHistory.clear();
	this->msgHistoryPos=0;
  this->msgHistory.push_front(cMsgHistoryEntry("" , "" , Message::typeMessage , Message::flagSend));
}

void cUICnts::closeAll() {
/*    for (Cnt_it_t Cnt_it = Cnt.begin(); Cnt_it != Cnt.end(); Cnt_it++) {
      if (Cnt_it->second) {
        Cnt_it->second->close();
        delete Cnt_it->second;
      }
      Cnt_it->second=0;
    }
    */
    while (Cnt.size()) {
        Cnt_it_t Cnt_it = Cnt.begin();
		sUICnt * temp = Cnt_it->second;
		if (temp) 
            temp->close();
        Cnt.erase(Cnt_it);
        if (temp) 
            delete temp;
    }

}


  bool cUICnts::exists(int id) {
    if (!DT_ISROWID(id)) id = Ctrl->DTgetID(DTCNT , id);
    return (Cnt.find(id)!=Cnt.end());
  }

  sUICnt & cUICnts::operator [] (int n) {
    if (!DT_ISROWID(n)) n = Ctrl->DTgetID(DTCNT , n);
    return *Cnt[n];
  }

  cUICnts::~cUICnts() {
    //closeAll();
  }

  int cUICnts::compare(sUICnt * a, sUICnt * b) {
#define COMPARE(A , B) { \
   if ((##A) != (##B)) {\
      if ((##A)) return 1; \
      else if ((##B)) return -1;}\
   }

    COMPARE(a->status & ST_NOTINLIST , b->status & ST_NOTINLIST);
    COMPARE(a->status & ST_IGNORED , b->status & ST_IGNORED);

    if (a->net != b->net)
      if (a->net == Net::none) return 1;
      else if (b->net == Net::none) return -1;


//    COMPARE(status & CNTM_STATUS == ST_OFFLINE);
    if ((a->status & CNTM_STATUS)/0x20 != (b->status & CNTM_STATUS)/0x20) {
        return ((a->status & CNTM_STATUS)/0x20 > (b->status & CNTM_STATUS)/0x20)?-1:1;
      }

	if ((a->status & CNTM_STATUS) < ST_SEMIONLINE && GETINT(CFG_UISORTACTIVE)) {COMPARE(!a->active , !b->active);}
#undef COMPARE
//   const char * c1 = GETCNTC(a->ID , Contact::colDisplay);
//   const char * c2 = GETCNTC(b->ID , Contact::colDisplay);
//   const char * c1 = "Przyk³ad";
//   const char * c2 = "Dupek";
   return _stricoll(a->display.c_str() , b->display.c_str());

//    return stricmp(GETCNTC(a->ID , Contact::colDisplay) , GETCNTC(b->ID , Contact::colDisplay));
  }

  void cUICnts::sort() {
//    std::sort(Cnt.begin()+1 , Cnt.end() , &compare );
  }

  void cUICnts::cfgSet() {
      for (Cnt_it_t it=this->Cnt.begin(); it != Cnt.end(); it++)
          it->second->cfgSet();
      imPrepare();
  }

  void cUICnts::checkActivity(bool force) {
	  static int lastCheck = time(0);
	  if (force || time(0)-lastCheck > 600) {
		  for (Cnt_it_t it=this->Cnt.begin(); it!=Cnt.end(); it++)
			  it->second->checkActivity();
		  this->sort();
	  }
  }

  void cUICnts::imPrepare ()
  {
    int c=IMessage(IMC_CNT_COUNT);
//    if (c>Cnt.size()) {
//      for (int i = 0; i<Cnt.size() - c; i++) imAdd(-1);
//    }
//    Cnt.resize(c);
    for (int i=0;i<c;i++) {
      int ID = Ctrl->DTgetID(DTCNT , i);
      if (ID == -1) continue;
      if (Cnt.find(ID)==Cnt.end()) {imAdd(ID);Cnt[ID]->hidden=false;}
      if (i) Cnt[ID]->imRefresh();
        else Cnt[ID]->user = 1;
    }
  }

  void cUICnts::imAdd (int id , bool hidden) {
    if ((id = Ctrl->DTgetID(DTCNT , id))==-1) return;
    if (!this->exists(id)) {
    Cnt[id]=new sUICnt;
    Cnt[id]->hidden = hidden;
    Cnt[id]->ID = (Ctrl->DTgetPos(DTCNT , id))?id : 0;
    Cnt[id]->imRefresh();
    } else if (!Cnt[id]->hidden) return;
	UI::CntList::filters.ApplyToCnt(Cnt[id]);
  }

  void cUICnts::imRemove (int id , bool massive=false) {
    if ((id = Ctrl->DTgetID(DTCNT , id))==-1) return;
    cntList.removeCnt(Cnt[id]);
    Cnt[id]->close();
    delete Cnt[id];
    Cnt.erase(id);
//    for (Cnt_it=Cnt.begin(); Cnt_it!=Cnt.end(); Cnt_it++) {
//      if ((*Cnt_it)->pos > pos) (*Cnt_it)->pos--; // zmniejsza wszystkie pozycje o jeden
//    }
  }
  void sUICnt::ApplyFilters() {
      Konnekt::UI::CntList::filters.ApplyToCnt(this);
  }
  CStdString sUICnt::getDisplayFormatted() {
	  char * format = (char*)GETSTR(CFG_UICL_DISPLAY_FORMAT);
	  if (!*format) { // standardowa obs³uga
		  format = (char*)GETCNTC(this->ID,Contact::colDisplay);
		  if (!*format)
			  return GETCNTC(this->ID, Contact::colUid);
		  return format;
	  }
	  // formatujemy linijkami... bierzemy pierwsz¹ nie pust¹...
	  format = strdup(format);
	  format = removeChar(format, '\r');
	  char * line = format;
	  CStdString result;
	  while (((int)line > 1) && result.empty()) {
		  char * newline = strchr(line, '\n');
		  if (newline) *newline = 0;
		  result = formatTitle(line, this->ID, FT_DISPLAY);
		  line = newline+1;
	  }
	  free(format);
	  return result;
  }

  void sUICnt::imRefresh () {
    int id=ID;
    net=GETCNTI(id,Contact::colNet);
//    Cnt[id]->UID=GETCNTC(pos,Contact::colUid);
    status=GETCNTI(id,Contact::colStatus);

	display=getDisplayFormatted();
    MsgUpdate();
	checkActivity();
    if (cntTip->docked && cntTip->cnt==Ctrl->DTgetPos(DTCNT , this->ID)) 
        cntTip->refresh();
	ApplyFilters();
  }
  void sUICnt::checkActivity() {
	if (active && Ctrl->DTgetInt64(DTCNT , this->ID , Contact::colLastActivity) < _time64(0)-1200)
      active=false;
  }

  int cUICnts::findBy(int val , char by) {
    for (Cnt_it_t Cnt_it = Cnt.begin(); Cnt_it != Cnt.end(); Cnt_it++) {
      switch(by) {
        case 1: if (Cnt_it->second->hwndInfo==(void*)val) break;
        case 2: if (Cnt_it->second->hwndMsg==(void*)val) break;
      }
    }
    return -1;
  }











void cCntListWindow::callDefaultAction() {
//	if (>1) return 0;
	int selStart = SendMessage(hwndList , LVM_GETSELECTIONMARK , 0 , 0);
	int selCount = ListView_GetSelectedCount(this->_hwnd);
	if (selStart < 0) return;
	sUICnt * pCnt = (sUICnt*)ListView_GetItemData(this->_hwnd , selStart);
	sUIAction act;
	if (!pCnt) return;
	if (selCount == 1 && pCnt->action.id) {
		act = pCnt->action;
	} else {/* Ustalamy defaultowa akcje */
		act.parent = act.id = 0;
		Act[IMIG_CNT].setCnt(selCount > 1 ? -1 : pCnt->ID , true);
		cUIAction * action = Act[IMIG_CNT].getDefaultAction();
		if (action) act = action->act();
	}
	if (!Act.exists(act)) {/*IMLOG("NO_PARENT!");*/return;}
	Act(act).setCnt(selCount > 1 ? -1 : pCnt->ID);
	Act(act).call(ACTN_ACTION,0,0);
}






  INT_PTR CALLBACK cCntListWindow::ListControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   cCntListWindow * clw = (cCntListWindow *)GetProp(hwnd , "cCntListWindow");
   if (!clw) return 0;
   TRACKMOUSEEVENT tme;
   RECT rc;
   static bool cursorInList = false;
   static bool canShow = false;
   switch (message)
   {

       case WM_ERASEBKGND: {

//             HDC DC = GetDC(hwnd);
             HDC DC = (HDC)wParam;
             GetChildRect(hwnd , &rc);
             rc.left=rc.top=0;
             int last , count;
             last=ListView_GetTopIndex(hwnd)+ListView_GetCountPerPage(hwnd);
             count=ListView_GetItemCount(hwnd);
             HBRUSH hbr = CreateSolidBrush(clw->parent->_lfItem.bgColor);
             if (clw->parent->_flickerFree && count) {
               if (last >= count) {
                 RECT rc2;
                 ListView_GetItemRect(hwnd , count-1 , &rc2 , LVIR_BOUNDS);
                 rc.top=rc2.bottom;
                 FillRect(DC , &rc , hbr);
               }
               rc.top=0;
               rc.bottom = 5;
               FillRect(DC , &rc , hbr); // Czyscimy gorna ramke
             } else { // calosc
               FillRect(DC , &rc , hbr);
             }
             DeleteObject(hbr);
//             FloodFill(DC , 0 , 0 , GetSysColor(COLOR_HIGHLIGHT));
//             ReleaseDC(hwnd , DC);

             return 1;}   

        case WM_MOUSEMOVE:
             if (!cursorInList) {
               cursorInList = true;
//                 IMLOG("SETTING HOVER");
                 tme.cbSize = sizeof (tme);
                 tme.dwFlags = TME_LEAVE | TME_HOVER;
                 tme.hwndTrack = hwnd;
                 tme.dwHoverTime = GETINT(CFG_UITIPDELAY);
                 _TrackMouseEvent(&tme);
             }
             if (canShow && clw->tip && !clw->tip->docked) clw->tip->show(-2 , hwnd);
             break;
        case WM_MOUSELEAVE:
             cursorInList = false;
//             GetCursorPos(&pt);
//             GetWindowRect(hwndListTip , &rc);
//             rc.right-=30;
//             pt.x++;pt.y++;
//             IMLOG("PT %d %d , RC %d %d %d %d" , pt.x , pt.y , rc.left , rc.top , rc.right , rc.bottom);
//             if (PtInRect(&rc , pt)) {
//               ListControlProc(hwnd , WM_MOUSEMOVE , 0 , 0);
//               return 0;
//             }
//             IMLOG("LEAVE");
             if (clw->tip && !clw->tip->docked) clw->tip->show(-1);
             canShow = false;
             break;
        case WM_MOUSEHOVER:
//             IMLOG("HOVER");
             canShow = true;
             tme.cbSize = sizeof (tme);
             tme.dwFlags = TME_LEAVE;
             tme.hwndTrack = hwnd;
             _TrackMouseEvent(&tme);
//             ListTipShow(-3);
             if (clw->tip && !clw->tip->docked) clw->tip->show(-2,hwnd);
             return 0;
        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:
            clw->lookupLast = 0;
            clw->lookupString = "";
             canShow = false;
             cursorInList = false;
             if (clw->tip && !clw->tip->docked) clw->tip->show(-1);
             break;
        case WM_CONTEXTMENU:
            if (wParam == (unsigned int)clw->_hwnd) {
                POINT pt;
                int cnt;
                if (lParam != -1) {
                    pt.x=LOWORD(lParam);pt.y=HIWORD(lParam);
                    ScreenToClient(clw->_hwnd , &pt);
                    cnt = ListView_ItemFromPoint(hwndList , pt);
                    if (cnt<0) return 0;
                    if (!ListView_GetItemState(clw->_hwnd , cnt , LVIS_SELECTED)) {
                        ListView_Deselect(clw->_hwnd);
                        ListView_SetItemState(clw->_hwnd , cnt , LVIS_SELECTED , LVIS_SELECTED);
                        ListView_SetSelectionMark(clw->_hwnd , cnt);
                    }

                } else {
                    cnt = ListView_GetSelectionMark(clw->_hwnd);
                    if (cnt<0) return 0;
                    ListView_GetItemPosition(clw->_hwnd , cnt , &pt);
                    pt.x += 20;
                    pt.y += 16;
                }

                ClientToScreen(clw->_hwnd , &pt);

                if (ListView_GetSelectedCount(clw->_hwnd)>1) Act[IMIG_CNT].setCnt(-1 , true);
                else Act[IMIG_CNT].setCnt(((sUICnt *)ListView_GetItemData(clw->_hwnd , cnt))->ID);
                UIPopupMenu(Act[IMIG_CNT],TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD,
                    pt.x,pt.y,0,GetParent(clw->_hwnd) , 0);
            }
            return 0;
        case WM_ACTIVATE:
            clw->lookupString = "";
            clw->lookupLast = 0;
            break;
        case WM_KEYUP:
            if (wParam > 0x20 && wParam <=0x2F) {clw->lookupString = "";clw->lookupLast=0;}
            if (wParam == VK_DELETE) {
                if (ListView_GetSelectedCount(clw->_hwnd)<1) return 0;
//                Act[IMIG_CNT].setCnt(((sUICnt *)ListView_GetItemData(clw->_hwnd , ListView_GetSelectionMark(clw->_hwnd)))->ID);
                Act[IMIG_CNT].setCnt(-1);
                Act(IMIG_CNT , IMIA_CNT_DEL).call(ACTN_ACTION,0,0);
                SetActiveWindow(clw->_hwnd);
			} else if (wParam == VK_RETURN) {
				if (!SendMessage(clw->_hwnd , LVM_GETEDITCONTROL , 0 , 0))
					clw->callDefaultAction();
			}
            break;
        case WM_CHAR: {
            if (wParam == VK_SPACE) {clw->lookupLast++;}
            else {clw->lookupString += (char)wParam;}
            // Przeszukiwanie po liscie
            int c=ListView_GetItemCount(clw->_hwnd);
            if (clw->lookupLast > c) clw->lookupLast = 0;
            int i=clw->lookupLast;
            bool found = false;
            do {
                sUICnt * cnt = (sUICnt *)ListView_GetItemData(clw->_hwnd , i);
                if (!cnt) break;
                if (!strnicmp(clw->lookupString , cnt->display.c_str() , clw->lookupString.size())) {
                    ListView_SetCurSel(clw->_hwnd , i);
                    ListView_SetItemState(clw->_hwnd , i , LVIS_FOCUSED , LVIS_FOCUSED);
                    clw->lookupLast = i;
                    found = true;
                    break;

                }
                i++;
                if (i>=c) i=0;
            } while (i!=clw->lookupLast); // Dopoki nie dojedzie do poczatku
            if (!found) {
                int t = clw->lookupString.size();
                clw->lookupLast = 0;
                clw->lookupString = "";
                if (t>1) ListControlProc(hwnd , message , wParam , lParam);
            }
            return 0;}

   }
   return CallWindowProc((WNDPROC)clw->_oldwndproc , hwnd , message , wParam , lParam);
}



// ---------------------------------------------------------------------------


int CALLBACK sortList_compare(LPARAM p1, LPARAM p2, LPARAM lParam) {
   return Cnt.compare((sUICnt*)p1 , (sUICnt*)p2);
}









int dragdropList(HWND hwnd, UINT message , WPARAM wParam , LPARAM lParam )
{
   static bool dragging = false;
   static int dragIndex = -1;
   static HIMAGELIST dragImage;
   static int dropIndex = -1;
   POINT pt;
   if (!hwnd) return dragging?1:0;
   switch (message)
   {
        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             switch (nm->code) {
               case LVN_BEGINDRAG:
           	    dragIndex = ((NM_LISTVIEW *)lParam)->iItem;

        	    pt.x = 0;
        	    pt.y = 0;
        	    ImageList_BeginDrag(Ico.iml[0] , Ico[IDI_NET_NONE].index[0], pt.x , pt.y);
                    GetCursorPos(&pt);
        	    ImageList_DragEnter(
        	 	    GetDesktopWindow(), pt.x , pt.y);
        	    dragging = true;
        	    SetCapture(hwndMain);
             }
             break;
        case WM_LBUTTONUP:
	     ReleaseCapture();
	     ImageList_DragLeave(hwndMain);
             ImageList_EndDrag();
             if (dropIndex>=0) {
               int c = ListView_GetItemCount(cntListWnd->_hwnd);
               TCITEM tci;
               *TLS().buff = 0;
               if (dropIndex) {
                 tci.mask = TCIF_TEXT;
                 tci.pszText = TLS().buff;
                 tci.cchTextMax = MAX_STRING;
                 TabCtrl_GetItem(hwndGroups , dropIndex , &tci);
               }
               for (int i = 0; i<c ; i++) {
                 if (ListView_GetItemState(cntListWnd->_hwnd , i , LVIS_SELECTED)) {
                    sUICnt * cnt = (sUICnt *)ListView_GetItemData(cntListWnd->_hwnd , i);
                    if (!cnt) continue;
                    SETCNTC(cnt->ID , Contact::colGroup , TLS().buff);
					sIMessage_CntChanged cc(IMC_CNT_CHANGED , cnt->ID);
					cc._changed.group = 1;
					IMessage(&cc);
                 }
               }
               ICMessage(IMI_REFRESH_LST , 1);

             }
//             ImageList_Destroy(dragImage);
             dragging = false;
             break;
        case WM_MOUSEMOVE:
             GetCursorPos(&pt);
             ImageList_DragMove(pt.x , pt.y - 14);
             ScreenToClient(hwndGroups , &pt);
             TCHITTESTINFO tchti;
             tchti.pt = pt;
             dropIndex = TabCtrl_HitTest(hwndGroups , &tchti);
             if (tchti.flags & (TCHT_ONITEMLABEL | TCHT_ONITEMICON)) SetCursor(LoadCursor(0 , IDC_ARROW));
               else SetCursor(LoadCursor(0 , IDC_NO));

             break;

   }
   return 0;
}



void fillGroups() {
   deque <string> items;
   TabCtrl_DeleteAllItems(hwndGroups);
   char * grps2 = strdup(GETSTR(CFG_GROUPS)) , * grp = grps2 , * grps = grps2;
   unsigned int i = 0;
   TCITEM tci;
   tci.mask = TCIF_IMAGE | TCIF_TEXT;
   while ((grps = strchr(grps , '\n'))!=0) {
      *grps = 0;
      grps++;
      if (*grp) {
        items.push_back(grp);
      }
      grp = grps;
   }
   sort(items.begin() , items.end() , strcompare);

   for (i = 0; i < items.size() ; i++) {
      tci.pszText = (char *)items[i].c_str();
      tci.iImage = -1;
      TabCtrl_InsertItem(hwndGroups , i , &tci);
      if (!stricmp(GETSTR(CFG_CURGROUP) , items[i].c_str()))
        TabCtrl_SetCurSel(hwndGroups , i);
   }


   if (i) {
        tci.pszText = (char*)GETSTR(CFG_UIALLGROUPSDESC);
        tci.iImage = Ico[UIIcon(IT_STATUS , Net::none , 0 , 0)].index[0];
        TabCtrl_InsertItem(hwndGroups , 0 , &tci);
        if (!*GETSTR(CFG_CURGROUP))
          TabCtrl_SetCurSel(hwndGroups , 0);
   }
   free(grps2);
}

void selectGroups() {
   int pos = TabCtrl_GetCurSel(hwndGroups);
   char * ch = "";
   if (pos > 0) {
     TCITEM tci;
     tci.mask = TCIF_TEXT;
     tci.pszText = TLS().buff;
     tci.cchTextMax = MAX_STRING;
     TabCtrl_GetItem(hwndGroups , pos , &tci);
     if (ICMessage(IMC_GRP_FIND , (int)TLS().buff))
       ch = TLS().buff;
   }
   SETSTR(CFG_CURGROUP , ch);
   cntList.refill();
   ListView_SetColumnWidth(hwndList , 0 , LVSCW_AUTOSIZE_USEHEADER);
}





// -------------------------------------------------------

void cCntList::registerWnd(class cCntListWindow * wnd) {
    _wnd.push_back(wnd);
}
void cCntList::unregisterWnd(class cCntListWindow * wnd){
    _wnd.erase(find(_wnd.begin(), _wnd.end() , wnd));
}
bool cCntList::running(){
    return _wnd.size()>0;
}
void cCntList::refresh(bool sort){
    for (_wnd_it_t it=_wnd.begin(); it!=_wnd.end(); it++) {
        (*it)->refresh(sort);
    }
}
void cCntList::refill(){
    for (_wnd_it_t it=_wnd.begin(); it!=_wnd.end(); it++) {
        (*it)->refill();
    }
}
void cCntList::removeCnt(sUICnt * cnt){
    for (_wnd_it_t it=_wnd.begin(); it!=_wnd.end(); it++) {
        (*it)->removeCnt(cnt);
    }
}
void cCntList::addCnt(sUICnt * cnt){
    for (_wnd_it_t it=_wnd.begin(); it!=_wnd.end(); it++) {
        (*it)->addCnt(cnt);
    }
}

void cCntList::cfgSet() {
    _right = GETINT(CFG_UICL_ICONSONRIGHT) != 0;
    _iconson = GETINT(CFG_UICL_ICONSON) != 0;
    _icons = GETINT(CFG_UICL_ICONS);
    _offset = GETINT(CFG_UICL_ICONSOFFSET) != 0;
    _samestatus = GETINT(CFG_UISAMECNTSTATUS) != 0;
    _flickerFree = GETINT(CFG_UICL_BUFFEREDPAINT) != 0;
	_info = (UI::CFG::enCntListInfo)GETINT(CFG_UICL_INFO);
	_infoFormat = GETSTR(CFG_UICL_INFO_FORMAT);

    _lfItem = StrToLogFont(GETSTR(CFG_UIF_CLIST));
    _lfItemSel = StrToLogFont(GETSTR(CFG_UIF_CLISTSEL));
	_lfItemInfo = StrToLogFont(GETSTR(CFG_UIF_CLISTINFO));
    if (_hfItem) DeleteObject(_hfItem);
    _hfItem = CreateFontIndirect(&_lfItem.lf);
    if (_hfItemOnline) DeleteObject(_hfItemOnline);
    LOGFONT lf = _lfItem.lf;
    if (GETINT(CFG_UIONLINEBOLD))
        lf.lfWeight = FW_BOLD;
    _hfItemOnline = CreateFontIndirect(&lf);
    if (_hfItemSel) DeleteObject(_hfItemSel);
    _hfItemSel = CreateFontIndirect(&_lfItemSel.lf);
    if (_hfItemInfo) DeleteObject(_hfItemInfo);
    _hfItemInfo = CreateFontIndirect(&_lfItemInfo.lf);

    for (_wnd_it_t it=_wnd.begin(); it!=_wnd.end(); it++) {
        (*it)->cfgSet();
    }
}


// ----------------------

cCntListWindow::cCntListWindow(cCntList  * owner , int x , int y , int w , int h , HWND parent , HMENU id) {
    this->parent = owner;
    _hwnd = CreateWindowEx(0,"SysListView32", "",
      WS_VISIBLE | WS_CHILD | WS_VSCROLL |
      LVS_REPORT | LVS_SHOWSELALWAYS | LVS_EDITLABELS
      | LVS_OWNERDRAWFIXED | LVS_NOCOLUMNHEADER
      ,x, y, w, h , parent, id, 0, NULL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT;
    lvc.iSubItem = 0;
    lvc.pszText = "Kontakty";
    lvc.cx = LVSCW_AUTOSIZE_USEHEADER;
    ListView_InsertColumn(_hwnd, 0, &lvc);
    ListView_SetColumnWidth(_hwnd , 0 , LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetImageList(_hwnd , LVSIL_SMALL , Ico.iml[0]);
    SetProp(_hwnd , "cCntListWindow" , (HANDLE)this);
    _oldwndproc = (WNDPROC)SetWindowLong(_hwnd , GWL_WNDPROC , (long)ListControlProc);
    this->parent->registerWnd(this);
}

cCntListWindow::~cCntListWindow() {
    this->parent->unregisterWnd(this);
    DestroyWindow(_hwnd);
}


void cCntListWindow::cfgSet() {
    SendMessage(_hwnd, WM_SETFONT , (WPARAM)this->parent->_hfItem , 1);
    RECT rc;
    GetClientRect(_hwnd , &rc);
	WINDOWPOS wp;
    wp.hwnd  = _hwnd;
    wp.cx    = rc.right;
    wp.cy    = rc.bottom;
    wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
    SendMessage(_hwnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );
    ListView_SetBkColor(_hwnd , parent->_lfItem.bgColor);

}


void cCntListWindow::sort() {
   int s = GetTickCount();
   ListView_SortItems(_hwnd , sortList_compare , 0);
//   IMLOG("* sortList.time = %d ms" , GetTickCount() - s);
}
int cCntListWindow::find(sUICnt * cnt) { // podaje pozycjê danego kontaktu na liœcie
  LVFINDINFO vfi;
  vfi.flags = LVFI_PARAM;
  vfi.lParam = (LPARAM)cnt;
  return ListView_FindItem(_hwnd , -1 , &vfi);
}

void cCntListWindow::refresh(bool sort){ // Odœwie¿a listê
    if (sort) this->sort();
    else
        RedrawWindow(_hwnd,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE);
}

void cCntListWindow::refill() { // Wype³nia na nowo
    if (!_hwnd) return;
   SendMessage(_hwnd , WM_SETREDRAW , 0,0);
//   SendMessage(_hwnd , LVM_DELETEALLITEMS, 0,0);
   IMLOG("*fillList Start");

   for (int i=0;i<Cnt.size();i++) {
	   Konnekt::UI::CntList::filters.ApplyToCnt(&Cnt[i]);
//       if (ICMessage(IMC_CNT_INGROUP , i) && !Cnt[i].hidden)
//           addCnt(&Cnt[i]);
   }


   IMLOG("*fillList End");
   sort();
   SendMessage(_hwnd , LVM_SETCOLUMNWIDTH , 0 , MAKELPARAM(LVSCW_AUTOSIZE_USEHEADER , 0));
//   onSizeMain(0,0);
   SendMessage(_hwnd , WM_SETREDRAW , 1,0);
}
void cCntListWindow::removeCnt(sUICnt * cnt) {
	if (!cnt->onList)
		return;
     int pos = this->find(cnt);
	 cnt->onList = false;
     if (pos > -1) ListView_DeleteItem(_hwnd , pos);
}
int cCntListWindow::addCnt(sUICnt * cnt) {
	if (cnt->onList)
		return 0;
     LVITEM lvi;
     lvi.mask = LVIF_PARAM | LVIF_TEXT;
     lvi.iItem = 0xFFFF;
     lvi.iSubItem = 0;
     lvi.lParam = (LPARAM)cnt;
     lvi.pszText = "";
	 cnt->onList = true;
     return ListView_InsertItem(_hwnd , &lvi);
}

void cCntListWindow::measureListItem(MEASUREITEMSTRUCT * mis) {
    mis->itemHeight=abs(parent->_lfItem.lf.lfHeight) + 2 + (parent->_lfItem.lf.lfHeight<0?4:0);
	mis->itemHeight = max(mis->itemHeight , 20);
	if (this->parent->_info != UI::CFG::clinfoInline && this->parent->_info != UI::CFG::clinfoOff) {
		CStdString format = GETSTR(CFG_UICL_INFO_FORMAT);
		format += "j_^M|";
		HDC hdc = GetDC(this->_hwnd);
		SelectObject(hdc , this->parent->_hfItemInfo);
        RECT rt = Rect(0,0,1000,100);
		this->parent->_infoHeight = DrawText(hdc, format, -1, &rt, DT_LEFT | DT_NOPREFIX | DT_CALCRECT) + (2 * CLINFO_MARGIN);
		mis->itemHeight += this->parent->_infoHeight;
		ReleaseDC(_hwnd, hdc);
	} else {
		this->parent->_infoHeight = 0;
	}
    mis->itemWidth=30;
}
 
void cCntListWindow::drawListItem(DRAWITEMSTRUCT * dis) {
   sUICnt * pCnt;
   bool notify=false;
   HBRUSH hBrush;
   RECT rc;
   if (!dis->itemData) return;
   pCnt=(sUICnt *)dis->itemData;
   // Poszerzamy "trochê" nasz item
   GetClientRect(dis->hwndItem , &rc);
   //dis->rcItem.left = 0;
   dis->rcItem.right = rc.right;
   HDC origDC;
   RECT rcItem;
   HBITMAP memBmp;
   RECT rcName; // fragment z nazw¹
   RECT rcInfo; // fragment z opisem
   // FlickerFree
   if (this->parent->_flickerFree) {
        origDC = dis->hDC; // Zapisujemy oryginalne DC...
        rcItem = dis->rcItem; // zapisujemy oryginalne wspolrzedne
        dis->hDC = CreateCompatibleDC(origDC);
        memBmp = CreateCompatibleBitmap(origDC , dis->rcItem.right - dis->rcItem.left , dis->rcItem.bottom - dis->rcItem.top);
        HGDIOBJ old = SelectObject(dis->hDC , memBmp);
        if (old) DeleteObject(old);
   // Ustawiam sobie Clipping...
/*   HRGN memRgn = CreateRectRgn(0,0,1000,1000);
   test = GetRandomRgn(origDC , memRgn , SYSRGN);
   OffsetRgn(memRgn , 0 , -dis->rcItem.top);
   SelectObject(dis->hDC , memRgn);
   GetUpdateRgn
   */
       dis->rcItem.bottom = dis->rcItem.bottom - dis->rcItem.top;
       dis->rcItem.top = 0;
   }
   // Wczytujemy kilka przydatnych wartoœci dla pewnoœci...
   pCnt->status = GETCNTI(pCnt->ID , Contact::colStatus);
   rcName = rcInfo = dis->rcItem;
   rcName.bottom = rcInfo.top = (rcName.bottom - this->parent->_infoHeight);
   bool selected = dis->itemState & ODS_SELECTED;
   // Robimy tlo...
   if (selected) {
       SelectObject(dis->hDC , parent->_hfItemSel);
       hBrush = CreateSolidBrush(parent->_lfItemSel.bgColor);
       SetTextColor(dis->hDC , parent->_lfItemSel.color);
	   FillRect(dis->hDC , &dis->rcItem , hBrush);
	   DeleteObject(hBrush);
   } else {
       SelectObject(dis->hDC , ((pCnt->status & ST_MASK) >= ST_ONLINE && (pCnt->status & ST_MASK) < ST_SPECIAL)?parent->_hfItemOnline : parent->_hfItem);
       if (pCnt->ex & CNT_EX_NEW) {
           hBrush = CreateSolidBrush(parent->_lfItem.bgColor);
           SetTextColor(dis->hDC 
               , RGB_ALPHA(parent->_lfItem.color 
                   , parent->_lfItem.bgColor /*(GetRValue(parent->_lfItem.color)+GetGValue(parent->_lfItem.color)*1.5+GetBValue(parent->_lfItem.color)) > 450 ? 0x0 : 0xFFFFFF */
                   , ceil(80 - (cos((float(pCnt->ex & CNT_EX_NEW) / CNT_NEW_TIME)*1.5707))*40) /*alpha*/)
                 /* (~parent->_lfItem.bgColor)&0xFFFFFF*/
               );
       } else {             
           hBrush = CreateSolidBrush(parent->_lfItem.bgColor);
           SetTextColor(dis->hDC , parent->_lfItem.color);
       }
	   FillRect(dis->hDC , &rcName , hBrush);
	   DeleteObject(hBrush);
	   if (this->parent->_infoHeight) {
			HBRUSH hBrushInfo = CreateSolidBrush(parent->_lfItemInfo.bgColor);
			FillRect(dis->hDC , &rcInfo , hBrushInfo);
			DeleteObject(hBrushInfo);
	   }

   }
   // Dobieramy ikonke statusu
   int icon = 0;
   if (pCnt->notify>0 && !ISICON_BLANK(pCnt->notify)) {
       icon = pCnt->notify; 
       notify=true;
   } else 
	   icon = pCnt->GetStatusIcon(parent->_samestatus);
   // Ustawiamy kilka przydatnych rzeczy
   int icoXOffset = parent->_offset?6 : 1;
   int icoYOffset = (rcName.bottom - rcName.top - 16) / 2;
   // Jezeli nie jest to notify, rysujemy pod ikonkami... 
   if (!notify)
	   Ico[icon].draw(dis->hDC , dis->rcItem.left+icoXOffset , dis->rcItem.top+icoYOffset , 0, 0 , 0 , 0 , 0x0);
   // Piszemy...
   rc=rcName;
   int nameXOffset = 16 + 2*icoXOffset + 4;
   rc.left += nameXOffset;
   if (parent->_iconson) rc.right-=16;
   SetBkMode(dis->hDC , TRANSPARENT);
   
   const char * display = pCnt->display.c_str();
   if (!*display) display = (char*)GETCNTC(pCnt->ID , Contact::colUid);

   DrawText(dis->hDC , display , -1 , &rc , DT_WORD_ELLIPSIS|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX);

   // Trzeba ustawic jeszcze kilka rzeczy
   rc.right = rc.left + 5;
   int ovrXOffset = 0;
   // £adujemy do rc faktyczny rozmiar tekstu nazwy
   DrawText(dis->hDC , display , -1 , &rc , DT_SINGLELINE|DT_CALCRECT);
   if (!parent->_right) {
       if (rc.right > dis->rcItem.right - 30) ovrXOffset = 0;
       else ovrXOffset = rc.right + 2;
   }

   // Rysujemy ikonki dodatkowe
   // Ikonki, ktore i tak sa zawsze po lewej...
   int ovrCount = 0; // liczba ikonek
   int ovr [4]; // ID ikonek...

   if (pCnt->status & ST_NOTINLIST)
       ovr[ovrCount++]=UIIcon(IT_OVERLAY,0,OVR_NOTINLIST,0);
   if (pCnt->status & ST_IGNORED)
       ovr[ovrCount++]=UIIcon(IT_OVERLAY,0,OVR_IGNORE,0);
   if ((pCnt->status & ST_HIDEBOTH) == ST_HIDEBOTH)
	   ovr[ovrCount++]=UIIcon(IT_OVERLAY,0,OVR_HIDEBOTHSTATUS,0);
   else if (pCnt->status & ST_HIDEMYSTATUS)
       ovr[ovrCount++]=UIIcon(IT_OVERLAY,0,OVR_HIDEMYSTATUS,0);
   else if (pCnt->status & ST_HIDESHISSTATUS)
       ovr[ovrCount++]=UIIcon(IT_OVERLAY,0,OVR_HIDESHISSTATUS,0);
   int ovrSize = (ovrCount*8>dis->rcItem.bottom - dis->rcItem.top)?(dis->rcItem.bottom - dis->rcItem.top)/ovrCount : 8;
   int ovrStart = (rcName.bottom - rcName.top - ovrSize*ovrCount) / 2;
   for (int i=0; i<ovrCount; i++) {
       Ico[ovr[i]].draw(dis->hDC , dis->rcItem.left+1 , dis->rcItem.top+ovrStart+ovrSize*i);
   }
   int ovrXCount = 0;
   ovrCount=0;

   // Rysujemy pozosta³e ikonki...
#define DO_OVR(id , off) \
   if (((parent->_icons >> 2*(off))&0x3) == 2) ovr[ovrCount++]=(id); \
   else if (((parent->_icons >> 2*(off))&0x3) == 1) {                \
       Ico[(id)+1].draw(dis->hDC , (ovrXOffset)?ovrXOffset + (ovrXCount++)*9:dis->rcItem.right-((++ovrXCount)*9) , dis->rcItem.top+icoYOffset+4); \
   }
// #define
	CStdString statusInfo = GETCNTC(pCnt->ID,Contact::colStatusInfo);
   if (parent->_iconson) {
       if (*GETCNTC(pCnt->ID,Contact::colMail)) 
           DO_OVR(UIIcon(IT_OVERLAY,0,OVR_MAIL,0) , 1);
       if (*GETCNTC(pCnt->ID,Contact::colCellPhone)) 
           DO_OVR(UIIcon(IT_OVERLAY,0,OVR_SMS,0) , 2);
       if (!statusInfo.empty()) 
           DO_OVR(UIIcon(IT_OVERLAY,0,OVR_INFO,0) , 0);
   }
   /*ovrCount>1? min(18 , dis->rcItem.bottom - dis->rcItem.top) / ovrCount : 8*/
   ovrSize = 6;
   ovrStart = (rcName.bottom - rcName.top - ovrSize*ovrCount) / 2;
   for (int i=0; i<ovrCount; i++) {
       Ico[ovr[i]].draw(dis->hDC , dis->rcItem.left+icoXOffset+(16 + icoXOffset - 7) , dis->rcItem.top+ovrStart+ovrSize*i);
   }
   /* info, 
      w rc mamy rozmiar tekstu, ovrXCount oznacza iloœæ ikonek po prawej
   */
   if (parent->_info != UI::CFG::clinfoOff) {
	   RECT rcSi;
	   if (parent->_info == UI::CFG::clinfoInline) {
			rcSi = rcName;
			rcSi.left = rc.right + 10;
			rcSi.right = dis->rcItem.right - 10;
			if (ovrXOffset) 
				rcSi.left += 9*ovrXCount;
			else
				rcSi.right -= 9*ovrXCount;
	   } else {
		   rcSi = rcInfo;
		   rcSi.top += CLINFO_MARGIN;
		   rcSi.left += nameXOffset; // tak samo jak zwyk³y tekst
		   rcSi.bottom -= CLINFO_MARGIN;
	   }
	   if (rcSi.right - rcSi.left > 30)  {// mamy miejsce
		   CStdString info = formatTitle(parent->_infoFormat, pCnt->ID, FT_CONTANCTLIST);
		   SelectObject(dis->hDC , parent->_hfItemInfo);
		   if (!(dis->itemState & ODS_SELECTED)) {
			   SetTextColor(dis->hDC , parent->_lfItemInfo.color);
		   }
		   if (parent->_info == UI::CFG::clinfoInline) {
			    info.Replace('\r',' ');
			    info.Replace('\n',' ');
			    DrawText(dis->hDC , info , info.length() , &rcSi , DT_WORD_ELLIPSIS|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_RIGHT);
		   } else {
			    DrawText(dis->hDC , info , info.length() , &rcSi , DT_NOPREFIX|DT_LEFT|(parent->_info == UI::CFG::clinfoMultilineWrap ? DT_WORDBREAK : 0));
		   }
	   }
   }

   // Rysujemy ikonke aktywnosci
   if (!notify && pCnt->active && ((pCnt->status & CNTM_STATUS)==ST_OFFLINE) && GETINT(CFG_UISORTACTIVE))
       Ico[UIIcon(IT_OVERLAY,0,OVR_ACTIVE,0)].draw(dis->hDC , dis->rcItem.left+icoXOffset , dis->rcItem.top+icoYOffset);

   // Skoro to notify, to rysujemy go NA wszystkim...
   if (notify)
       Ico[icon].draw(dis->hDC , dis->rcItem.left+icoXOffset , dis->rcItem.top+icoYOffset , 0, 0 , 0 , 0 , 0x0);
   // Nakladka zalezna od sieci...
   if (pCnt->net != Net::none && parent->_samestatus) {
      Ico[UIIcon(IT_OVERLAY,pCnt->net,0,0)].draw(dis->hDC , dis->rcItem.left+icoXOffset , dis->rcItem.top+icoYOffset);
   }

   // Flicker-free
   // Wyciagamy wszystko z buforka
   if (this->parent->_flickerFree) {
        BitBlt(origDC , 0 , rcItem.top , rcItem.right-rcItem.left , dis->rcItem.bottom
            , dis->hDC , 0 , 0 , SRCCOPY);
        //   DeleteObject(memRgn);
        DeleteDC(dis->hDC);
        DeleteObject(memBmp);
        dis->hDC = origDC;
   }
   dis->rcItem = rcItem;
}


//--------------------------------------------------

cCntListTip::cCntListTip(HWND parent , bool docked) {
    this->docked = docked;
    _hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,"LISTTIP","",
        WS_POPUP/* | WS_BORDER*/,
        0,0,100,108,
        parent,NULL,0,NULL);
    SetProp(_hwnd , "cCntListTip" , (HANDLE)this);
}
cCntListTip::~cCntListTip(){
    DestroyWindow(_hwnd);
    _hwnd = 0;
}
// -1 - ukryj
// -2 - wg. myszki
// -3 - wg. listy kontaktow
void cCntListTip::show(int pos , HWND list){
    if (!hwndMain) return; // Nie mozemy rysowac po zamknieciu!
    static bool timer;
    if (docked && pos==-2) pos = -3;
    if (pos==-2) {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(list , &pt);
        pos = ListView_ItemFromPoint(list , pt);
        if (pos >= 0)
            pos = ((sUICnt*)ListView_GetItemData(list , pos))->ID;
    } else 
        if (pos==-3) {
            pos = ListView_GetSelectionMark(list);
            if (pos >= 0) {
                sUICnt * cnt = ((sUICnt*)ListView_GetItemData(list , pos));
                if (cnt)
                    pos = cnt->ID;
                else pos = -1;
            }
        }
    if (pos==-1) {
        if (!IsWindowVisible(_hwnd)) return;
        SendMessage(_hwnd , MYWM_REFRESH , -1 , 0);
        if (!docked) ShowWindow(_hwnd , SW_HIDE);
        return;
    }
    if (!GETINT(CFG_UIINFOTIP)) return;

    if (!IsWindowVisible(_hwnd)) {
        SendMessage(_hwnd , MYWM_REFRESH , pos , 0);
        ShowWindow(_hwnd , SW_SHOWNOACTIVATE);
        return;
    } else {
        SendMessage(_hwnd , MYWM_REFRESH , pos , 0);
    }
}

#define TIP_VSPACE 2

void cCntListTip::refresh(int newPos) {
    if (!hwndMain) return; // Nie mozemy rysowac po zamknieciu!
    RECT rt;
    if (newPos <= 0) IMLOG("Refresh %d" , newPos);
    if (newPos==-2) {RedrawWindow(_hwnd,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE); return;} // refresh
    newPos = Ctrl->DTgetPos(DTCNT , newPos);
    if (newPos != this->cnt) {
        this->cnt = newPos;
        if (this->cnt ==-1) return;
        // Sprawdzamy wysokosc znaczkow
        LOGFONTEX lfe = StrToLogFont(GETSTR(CFG_UICNTTIPFONT));
        //charHeight = abs(lfe.lf.lfHeight);
        drawIcon = GETINT(CFG_UICNTTIPICONS);
        HDC hdc = GetDC(_hwnd);
        SelectObject(hdc , fontTip);
        rt = Rect(0,0,1000,100);
		DrawText(hdc, "0Ajg^_|", -1, &rt, DT_LEFT | DT_NOPREFIX | DT_CALCRECT);
        charHeight = rt.bottom;
        realCharHeight = charHeight;
        charHeight = drawIcon?max(16,charHeight):charHeight;
        if (!docked) {
            int h = GETINT(CFG_UICNTTIPHEADER)?charHeight + TIP_VSPACE + 4 : 3;
			if (GETINT(CFG_UICNTTIP_SHOWDEFAULT)) {
				toDraw = ((Cnt[this->cnt].net!=0)+(*GETCNTC(this->cnt , Contact::colCellPhone)!=0)
							+(*GETCNTC(this->cnt , Contact::colMail)!=0)
							+((*GETCNTC(this->cnt , Contact::colName)!=0)||(*GETCNTC(this->cnt , Contact::colSurname)!=0))
							+(GETINT(CFG_UILASTACTIVITY)&&Cnt[this->cnt].net)
							+(GETINT(CFG_UISHOWIP)&&*GETCNTC(this->cnt , Contact::colHost))
							);
			} else
				toDraw = 0;

            h += (charHeight+TIP_VSPACE) * toDraw;
//               h += 12 * (min(6 , max((int)(*GETCNTC(pos , Contact::colStatusInfo)!=0),(int)(strlen(GETCNTC(pos , Contact::colStatusInfo)) / 13))));
            //GetWindowRect(_hwnd , &rt);
#define TIP_WIDTH 118
#define TIP_SPACE 26
            rt.left=TIP_SPACE;
            rt.top=0;
            rt.bottom = 0;
            int w = TIP_WIDTH;

			CStdString info;
			if (GETINT(CFG_UICNTTIP_SHOWDEFAULT)) {
				rt.right = w;
				info = GETCNTC(this->cnt , Contact::colStatusInfo);
				if (!info.empty()) {
					toDraw++;
					DrawText(hdc, info, info.length(), &rt, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
					w = max (TIP_WIDTH, rt.right);
					h += max(charHeight, rt.bottom) + 4;
				}
			}
			const char * infoFormat = GETSTR(CFG_UICNTTIPINFO);
			if (*infoFormat) {
				rt.right = w;
				info = formatTitle(infoFormat, this->cnt, FT_TIP);
				if (!info.empty()) {
					toDraw++;
					DrawText(hdc, info, info.length(), &rt, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
					w = max (w, rt.right);
					h += max(charHeight, rt.bottom) + 4;
				}
			}
			w = min(w , 2*TIP_WIDTH + 4*TIP_SPACE);

            // sprawdza czy okno siê mieœci
//             GetWindowRect(GetDesktopWindow() , &rt);
//             if (x+w > rt.right) x = rt.right - w - 4;
            // ustawia
            SetWindowPos(_hwnd , 0 , 0, 0, w , h , SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        } // !clt->docked
        ReleaseDC(_hwnd , hdc);
        RedrawWindow(_hwnd,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE);
    } // newPos != cnt
    if (!docked){
        RECT rt2;
        POINT pt;
        // przesuwamy
        GetCursorPos(&pt);
        GetClientRect(_hwnd , &rt);
        SystemParametersInfo(SPI_GETWORKAREA,0,&rt2,0);
        pt.x+=5;
        pt.y+=5;
        if (pt.x+rt.right+5 > rt2.right) pt.x = rt2.right - rt.right - 5;
        if (pt.y+rt.bottom+5 > rt2.bottom) pt.y = pt.y - rt.bottom - 10;
        SetWindowPos(this->_hwnd , 0 , pt.x , pt.y , 0 , 0 , SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void cCntListTip::drawItem(HDC hdc , RECT * rc , const char * name , const char * text , unsigned int ico , int maxWidth, bool multiline) {
    if (*text) {
        RECT rt;
        int w = 8; // szerokosc
        if (this->docked) { // Ustalamy szerokoœæ
            rt = Rect(0,0,1000,1000);
            DrawText(hdc, text, -1, &rt, DT_LEFT | DT_NOPREFIX | DT_CALCRECT | DT_SINGLELINE);
            w = rt.right + 2 + (drawIcon?16:18);
        }
		if (rc->left > 4) { // Na pewno poza poczatkiem linijki
            if (!docked || (rc->left + w > rc->right && ((rc->bottom - rc->top >= 2*charHeight) ||  (rc->right-rc->left < charHeight))))
            {//IMLOG("NEXTLINE [%s]" , name);
				// nowa linijka jest tam, gdzie skonczyla sie poprzednia oczywiscie...
				rc->left = 3; rc->top = rc->bottom/*(drawIcon?charHeight:charHeight)*/ + TIP_VSPACE;
			} // Przechodzimy do nastêpnej linijki
		}
        if (drawIcon) {
			Ico[ico].draw(hdc , rc->left , rc->top); rc->left+=18;
		} else {
            SelectObject(hdc , fontTipB);
			DrawText(hdc, name, -1, rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
            rc->left += 18;
            SelectObject(hdc , fontTip);
        }
        rt = *rc;
        if (!multiline) 
			rt.bottom = rt.top + charHeight;
		else { 
			rt.bottom = 1000;
			if (realCharHeight < charHeight)
				rt.top += (charHeight - realCharHeight) / 2;
		}
        //if (drawIcon && charHeight < 16) { rt.top += (16 - charHeight) / 2;}
        rt.right = min(rt.right , maxWidth+rt.left);
        int h = DrawText(hdc, text, -1, &rt, DT_LEFT | (multiline?DT_WORDBREAK:DT_SINGLELINE | DT_VCENTER) | DT_NOPREFIX);
		rc->bottom = rt.top + max(charHeight, h);
        rc->left+=w + 2 - 16;
    }

}

void cCntListTip::paint(){
    if (!hwndMain) return; // Nie mozemy rysowac po zamknieciu!
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc , rt;
    HBRUSH hbr;
   	hdc = BeginPaint(_hwnd, &ps);
    LOGFONTEX lfe = StrToLogFont(GETSTR(CFG_UICNTTIPFONT));
    hbr = CreateSolidBrush(lfe.bgColor);
    //SelectObject(hdc, hbr);
    GetClientRect(_hwnd, &rc);
    FillRect(hdc , &rc , hbr);
    if (this->cnt == -1 || !this->cnt || !Cnt.exists(this->cnt)) {
        DeleteObject(hbr);
       	EndPaint(_hwnd, &ps);
        return;
    }

    bool header = GETINT(CFG_UICNTTIPHEADER) != 0;
    if (header) {
        rt = rc;
        rt.bottom = realCharHeight + 4;
        DeleteObject(hbr);
        // Naglowek
        hbr = CreateSolidBrush(RGB_ALPHA(lfe.bgColor , lfe.color , 70));
        FillRect(hdc , &rt , hbr);
        // Kreska
/*        hpen = CreatePen(PS_SOLID , 1 , lfe.color);
        MoveToEx(hdc , 2 , charHeight + 2 , 0);
        LineTo(hdc , rc.right - 2 , charHeight + 2);
        DeleteObject(hpen);*/
    }
    rt = rc;
    rt.top +=2;
    rt.left+=2;
    rt.right-=2;
    rt.bottom-=2; // <- przestrzen po ktorej mozemy bazgrac
    int trimWidth = (docked && toDraw>2)?(rt.right - rt.left) / 2 - 16 : rt.right - rt.left;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc , lfe.color);
    if (header) {
        SelectObject(hdc, fontTipB);
        // Nag³ówek
        DrawText(hdc, GETCNTC(this->cnt , Contact::colDisplay), -1, &rt, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS|DT_NOPREFIX);
        rt.top += realCharHeight + 1 + 3;
    }
    SelectObject(hdc, fontTip);
	if (GETINT(CFG_UICNTTIP_SHOWDEFAULT)) {
		if (Cnt[this->cnt].net)
			drawItem(hdc , &rt , "#" , GETCNTC(this->cnt , Contact::colUid) , UIIcon(IT_LOGO , Cnt[this->cnt].net , 0 , 0) , trimWidth);

		if ((*GETCNTC(this->cnt , Contact::colName)!=0)||(*GETCNTC(this->cnt , Contact::colSurname)!=0)) 
			drawItem(hdc , &rt , "IN" , (GETCNTC(this->cnt , Contact::colName) + string(" ") + GETCNTC(this->cnt , Contact::colSurname)).c_str() , IDI_TIP_NAME , trimWidth);
		drawItem(hdc , &rt , "@" , GETCNTC(this->cnt , Contact::colMail) , IDI_TIP_MAIL , trimWidth);
		drawItem(hdc , &rt , "P" , GETCNTC(this->cnt , Contact::colCellPhone) , IDI_TIP_PHONE , trimWidth);

		if (GETINT(CFG_UILASTACTIVITY) && Cnt[this->cnt].net) {
			Time64 t(GETCNTI64(cnt , Contact::colLastActivity));
			drawItem(hdc , &rt , "?" , t.empty()&&Cnt[this->cnt].active? "nieaktywny" : (char*)t.strftime("%a %H:%M:%S").c_str() , 0 , trimWidth);
		}
		if (GETINT(CFG_UISHOWIP) && (*GETCNTC(this->cnt,Contact::colHost) && GETCNTI(this->cnt,Contact::colPort))) {
			drawItem(hdc , &rt , "IP" , _sprintf("%s:%d",GETCNTC(this->cnt,Contact::colHost),GETCNTI(this->cnt,Contact::colPort)) , 0 , trimWidth);
		}

		drawItem(hdc , &rt , "*" , GETCNTC(this->cnt , Contact::colStatusInfo) , IDI_TIP_REASON , 1000, true);
	}	
	const char * infoFormat = GETSTR(CFG_UICNTTIPINFO);
	if (*infoFormat) {
		drawItem(hdc , &rt , "i" , formatTitle(infoFormat, this->cnt, FT_TIP) , IDI_TIP_INFO , 1000, true);
	}

	FrameRect(hdc, &rc, GetSysColorBrush(COLOR_INFOTEXT));

    //SelectObject(hdc, holdfnt);
    DeleteObject(hbr);

   	EndPaint(_hwnd, &ps);

}



// ------------------------------


LRESULT CALLBACK ListTipClassProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam){
   cCntListTip * clt = (cCntListTip *)GetProp(hwnd , "cCntListTip");
   switch (message)

   {
   case WM_PAINT: {
       clt->paint();
       break;}

       // Ustawia kilka pierdó³ek i przede wszystkim
       case MYWM_REFRESH:{
           clt->refresh(wParam);
           break;}
       case WM_LBUTTONUP:{
/*           RECT rc;
           GetClientRect(hwnd , &rc);
           rc.bottom += 10;
           SetWindowPos(hwnd , 0 , 0 , 0 , rc.right , rc.bottom , SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
*/
           break;}



	default:

	        return DefWindowProc(hwnd, message, wParam, lParam);
   }
   return 0;
}
