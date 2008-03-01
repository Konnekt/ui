/*

  Konnekt UI

  Okna dialogowe.

*/

#include "stdafx.h"
#pragma hdrstop


#include "ui_main.h"


#include "resource_ui.h"


#include "ui_cntlist.h"
#include "ui_msgcontrol.h"
#include "ui_ui.h"
#include "ui_actions.h"



cMsgControl::cMsgControl(sUIAction act , int x , int y , int w , int h , HWND parent , HMENU id) {
	this->act = act;
	sUIActionNotify_createWindow cw;
	cw.act = act;
	cw.x = x;
	cw.y = y;
	cw.w = w;
	cw.h = h;
	cw.hwndParent = parent;
	Act(act).call(&cw);
	Act(act).handle = cw.hwnd;
	this->hwnd = cw.hwnd;
	SetWindowLong(this->hwnd , GWL_ID , IDC_MSG);
}
cMsgControl::~cMsgControl() {
	/*sUIActionNotify_destroyWindow dw;
	dw.act = act; // ¯eby CNT siê zgadza³o
	Act(act).call(&dw);
	this->hwnd = 0;*/
}

void cMsgControl::clear() {
	Act(act).call(Konnekt::UI::Notify::clear,0,0,act.cnt);
}
void cMsgControl::lock() {
	Act(act).call(Konnekt::UI::Notify::lock,0,0,act.cnt);
}
void cMsgControl::unlock() {
	Act(act).call(Konnekt::UI::Notify::unlock,0,0,act.cnt);
}
//__inline int cMsgControl::parentMessageProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam);
void cMsgControl::msgInsert(Message * m , const char * display , bool scroll) {
	if (!m) return;
	Konnekt::UI::Notify::_insertMsg im(m , display , scroll);
	im.act = act;
	Act(act).call(&im);
}
void cMsgControl::statusInsert(int status , CStdString info) {
	Konnekt::UI::Notify::_insertStatus is(status , info);
	is.act = act;
	Act(act).call(&is);
}



void cMsgControlRich::actionProcess(sUIActionNotify_base * anBase) {
	using namespace Konnekt::UI;
	switch (anBase->code) {
		case Notify::clear: this->clear(); break;
		case Notify::lock: this->lock(); break;
		case Notify::unlock: this->unlock(); break;
		case Notify::insertMsg: {
			Notify::_insertMsg * im = static_cast<Notify::_insertMsg*>(anBase);
			this->msgInsert(im->_message , im->_display , im->_scroll);
			break;}
		case Notify::insertStatus: {
			Notify::_insertStatus * is = static_cast<Notify::_insertStatus*>(anBase);
			this->statusInsert(is->_status , is->_info);
			break;}
	}
}

cMsgControlRich::cMsgControlRich(int x , int y , int w , int h , HWND parent , HMENU id) {
    this->hwnd = CreateWindowEx(0 , RICHEDIT_CLASS  , "" , WS_CHILD|WS_VISIBLE|ES_MULTILINE | ES_AUTOVSCROLL | 
                    ES_READONLY | WS_VSCROLL | WS_TABSTOP
                    ,x,y,w,h,parent,id , 0 , 0);
    SetProp(hwnd , "cMsgControl*" , (HANDLE)this);
    this->_oldwndproc = (WNDPROC) SetWindowLong(hwnd , GWL_WNDPROC , (LONG)WndProc);

    SendMessage(this->hwnd , EM_AUTOURLDETECT , 1 , 0);
    SendMessage(this->hwnd , EM_SETEVENTMASK  , 0 , ENM_SELCHANGE | ENM_LINK);
    CHARFORMAT cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE;
    cf.yHeight = 20;
    SendMessage(this->hwnd , EM_SETCHARFORMAT , SCF_ALL , (LPARAM)&cf);
    LOGFONTEX lfe = StrToLogFont(GETSTR(CFG_UIF_MSG));
    SendMessage(this->hwnd , EM_SETBKGNDCOLOR , 0 , lfe.bgColor);
	HideCaret(this->hwnd);
//  SendMessage(item , EM_SETFONTSIZE , 200 , 0);
    _lock = 0;
}

cMsgControlRich::~cMsgControlRich() {
//    if (hwnd) DestroyWindow(hwnd);
}



void cMsgControlRich::clear() {
	SendMessage(this->hwnd , WM_SETTEXT , 0 , (LPARAM)"");
}
void cMsgControlRich::lock() {
	if (!_lock)
		SendMessage(this->hwnd , WM_SETREDRAW , 0,0);
	_lock++;
}
void cMsgControlRich::unlock() {
	if (_lock) _lock --;
	else return;
	if (!_lock) {
		SendMessage(this->hwnd , WM_SETREDRAW , 1,0);
		RepaintWindow(this->hwnd);
	}
}
int  cMsgControlRich::parentMessageProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam) {
/*    switch (message) {
    }*/
    return 0;
}


#define RE_PREPARE() memset(&cf , 0 , sizeof(cf));memset(&pf , 0 , sizeof(pf));pf.cbSize = sizeof(pf);cf.cbSize = sizeof(cf); \
    {int ndx = GetWindowTextLength (hwnd); SendMessage(hwnd , EM_SETSEL ,  ndx, ndx);}
#define RE_ALIGNMENT(al) pf.dwMask = PFM_ALIGNMENT;pf.wAlignment = al;SendMessage(hwnd , EM_SETPARAFORMAT , 0 , (LPARAM)&pf)
#define RE_FACE(fac) cf.dwMask = CFM_FACE;strcpy(cf.szFaceName , fac);SendMessage(hwnd , EM_SETCHARFORMAT , SCF_SELECTION , (LPARAM)&cf)
#define RE_SIZE(siz) cf.dwMask = CFM_SIZE;cf.yHeight = (siz);SendMessage(hwnd , EM_SETCHARFORMAT , SCF_SELECTION , (LPARAM)&cf)
#define RE_BOLD(bo) cf.dwMask = CFM_BOLD;cf.dwEffects = bo?CFM_BOLD:0;SendMessage(hwnd , EM_SETCHARFORMAT , SCF_SELECTION , (LPARAM)&cf)
#define RE_COLOR(co) cf.dwMask = CFM_COLOR;cf.crTextColor = co;SendMessage(hwnd , EM_SETCHARFORMAT , SCF_SELECTION , (LPARAM)&cf)
#define RE_ADD(txt) SendMessage(hwnd , EM_REPLACESEL , 0 , (LPARAM)(string(txt).c_str()))
#define RE_ADDLINE(txt) RE_ADD(string(txt) + "\r\n")
// Wczytuje z bazy danych definicje fonta
#define RE_SETEXFONT(id) \
{ \
    LOGFONTEX lfe = StrToLogFont(GETSTR(id));\
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_COLOR | /*CFM_BACKCOLOR |*/ CFM_CHARSET;\
    strcpy(cf.szFaceName , lfe.lf.lfFaceName);\
    cf.yHeight = (-lfe.lf.lfHeight) * 15 ;\
    cf.dwEffects = (lfe.lf.lfWeight>=FW_BOLD?CFE_BOLD:0) \
        | (lfe.lf.lfItalic?CFE_ITALIC:0)\
        | (lfe.lf.lfUnderline?CFE_UNDERLINE:0);\
    cf.crTextColor = lfe.color;  \
    /*cf.crBackColor = lfe.bgColor;*/\
    cf.bCharSet = lfe.lf.lfCharSet;\
    SendMessage(hwnd , EM_SETCHARFORMAT , SCF_SELECTION , (LPARAM)&cf);\
	cf.crTextColor;\
}

void cMsgControlRich::SetStyleCB(const CStdString & token , const CStdString & styleClass , cRichEditFormat::cSetStyle & ss) {
	/*TODO: Wstawiæ formatowanie MARK */
	HTMLInfoStyleCB(token , styleClass , ss);
	if (styleClass == "mark") {
		LOGFONTEX lfe = StrToLogFont(GETSTR(CFG_UIF_MSGMARK));
		ss.cf.dwMask |= CFM_FACE | CFM_COLOR | CFM_BACKCOLOR | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE; 
		ss.cf.crTextColor = lfe.color;
		ss.cf.crBackColor = lfe.bgColor;
		/* Wysokoœæ w lfe jest najczêœciej ujemna... * 15 daje m/w dobry rozmiar czcionki */
		ss.cf.yHeight = -lfe.lf.lfHeight * 15;
		strcpy(ss.cf.szFaceName , lfe.lf.lfFaceName);
		if (lfe.lf.lfItalic)
			ss.cf.dwEffects |= CFE_ITALIC;
		if (lfe.lf.lfWeight >= FW_BOLD)
			ss.cf.dwEffects |= CFE_ITALIC;
		if (lfe.lf.lfUnderline)
			ss.cf.dwEffects |= CFE_UNDERLINE;
	}
}


#define COLOR_SWAP(c) ((c & 0x00FF0000)>>16) | (c&0xFF00) | ((c & 0x00FF)<<16)
void cMsgControlRich::insertHTML(CStdString body , int defColor) {
	cRichEditHtml::InsertHTML(this->hwnd , body , SetStyleCB);
}


void cMsgControlRich::msgInsert(Message * m , const char * display , bool scroll){
    CHARFORMAT2 cf;
    PARAFORMAT2 pf;
    RE_PREPARE();
    // Ustawiamy paragraf.
    pf.dwMask = PFM_SPACEBEFORE | PFM_SPACEAFTER | PFM_STARTINDENT;
    pf.dySpaceBefore = 50;
    pf.dySpaceAfter = 0;
    pf.dxStartIndent = 0;
    SendMessage(hwnd , EM_SETPARAFORMAT , 0 , (LPARAM)&pf);
//    RE_ADDLINE("\nCHECK\n");
    string from;
    int cnt = m->getFlags() & Message::flagSend ? 0 : ICMessage(IMC_CNT_FIND , m->getNet() , (int)m->getFromUid().a_str());
    from = m->getExtParam(Message::extDisplay).a_str();
	if (from.empty()) {
		if (display) from=display;
		else 
		from = GETCNTC(cnt , CNT_DISPLAY);
	}

  if (from.empty()) from = (m->getFlags() & Message::flagSend)?"JA" : m->getFromUid().a_str();
  CStdString body = m->getBody().a_str();

	if (m->getType() == Message::typeFile) {
    m->setOneFlag(Message::flagHTML, true);
    body = CStdString("Plik:\t\t<b>") + m->getExtParam(Message::extFile_Path).a_str() + "</b>";
    if (m->getFlags() & Message::flagSend) 
			body += "<br/><b>Wysy³any</b>";
		else
			body += "<br/><b>Odbierany</b>";
    double size = atof(m->getExtParam(Message::extFile_Size).a_str());
		if (size > 0)
			body += stringf("<br/>Rozmiar:\t<b>%.2f KB</b>" , size / 1024);
    double transfered = atof(m->getExtParam(Message::extFile_Transfered).a_str());
		if (transfered > 0)
			body += stringf("<br/>Przes³ano:\t<b>%.2f KB</b>" , transfered / 1024);
    int time = atoi(m->getExtParam(Message::extFile_Transfer_Time).a_str());
		if (time)
			body += stringf("<br/>Czas:\t\t<b>%d</b>m <b>%d</b> s" , time / 60 , time % 60);
		if (time && size > 0)
			body += stringf("<br/>Transfer:\t<b>%.2f KB/s</b>" , transfered / 1024 / time);
    string error = m->getExtParam(Message::extFile_Error).a_str();
		if (!error.empty()) 
			body += "<br/><br/>B³¹d:\t\t<b>" + error + "</b>";
	}

  if (m->getType() != Message::typeQuickEvent && (!from.empty() || m->getTime())) {
        RE_SETEXFONT(CFG_UIF_MSGCNT);
        if (!from.empty()) {RE_BOLD(1);RE_ADD(from+": ");RE_BOLD(0);}
        cTime64 dtime(m->getTime());
        if (m->getTime()/* && m->type==Message::typeMessage*/) {
			if (dtime.sec / 86400 != _time64(0) / 86400)
				RE_ADD(dtime.strftime(" (%a %d-%m'%y %H:%M) ")); 
			else
				RE_ADD(dtime.strftime(" (%H:%M) ")); 
		}
        CStdString AddInfo = m->getExtParam(Message::extAddInfo).a_str();
        if (!AddInfo.empty()) RE_ADD(" "+AddInfo);
        RE_ADD(" \r\n");
    }
    if (m->getType() == Message::typeQuickEvent) {RE_SETEXFONT(m->getFlags() & Message::flag_QE_Normal?CFG_UIF_MSG: CFG_UIF_MSGINFO);}
    else if(m->getFlags() & Message::flagSend) {RE_SETEXFONT(CFG_UIF_MSGSEND);}
    else {RE_SETEXFONT(CFG_UIF_MSGRCV);}
    pf.dwMask = PFM_STARTINDENT | PFM_OFFSET | PFM_SPACEBEFORE;
    pf.dxStartIndent = 100;
    pf.dxOffset = 0;
    pf.dySpaceBefore = 0;
    SendMessage(hwnd , EM_SETPARAFORMAT , 0 , (LPARAM)&pf);
	// Dodajemy BODY -------------------------------------------------
    if (m->getFlags() & Message::flagHTML) {
		insertHTML(body + "<br>", cf.crTextColor);
	} else {
		RE_ADD(body + "\r\n");
	}
//    RE_SIZE(40);
//    RE_ADD("\r\n");
//    if (m->flag & MF_SEND) RE_COLOR(GETINT(CFG_UIC_MSGTXT));
    //  SetWindowText(GetDlgItem(hwnd,IDC_MSG) , body.c_str());
    if (scroll) {
        SendMessage(hwnd , WM_VSCROLL , SB_BOTTOM , 0);
        if (SendMessage(hwnd , EM_GETFIRSTVISIBLELINE , 0 , 0) == SendMessage(hwnd , EM_GETLINECOUNT , 0 , 0)-1) SendMessage(hwnd , EM_SCROLL , SB_PAGEUP , 0);
    }
    //  delete [] buff;
}


void cMsgControlRich::statusInsert(int status , CStdString info) {
    CHARFORMAT2 cf;
    PARAFORMAT2 pf;
    RE_PREPARE();
    // Ustawiamy paragraf.
    pf.dwMask = PFM_SPACEBEFORE | PFM_SPACEAFTER | PFM_STARTINDENT;
    pf.dySpaceBefore = 50;
    pf.dySpaceAfter = 50;
    pf.dxStartIndent = 0;
    SendMessage(hwnd , EM_SETPARAFORMAT , 0 , (LPARAM)&pf);
    RE_SETEXFONT(CFG_UIF_MSG);
    CStdString txt = cTime64(true).strftime("[%H:%M] status ");
    RE_ADD(txt);
    RE_BOLD(1);
    RE_ADD(getStatusName(status) + (info.empty()?"\r\n":""));
    RE_BOLD(0);
    if (!info.empty()) RE_ADD(" \""+info+"\"\r\n");
    SendMessage(hwnd , WM_VSCROLL , SB_BOTTOM , 0);
    if (SendMessage(hwnd , EM_GETFIRSTVISIBLELINE , 0 , 0) == SendMessage(hwnd , EM_GETLINECOUNT , 0 , 0)-1) SendMessage(hwnd , EM_SCROLL , SB_PAGEUP , 0);

}


#undef  RE_PREPARE
#undef  RE_ALIGNMENT
#undef  RE_BOLD
#undef  RE_COLOR
#undef  RE_ADD
#undef  RE_ADDLINE
#undef  RE_SETEXFONT


void cMsgControlRich::contextMenu(int x , int y) {
    HMENU mainMenu = LoadMenu(Ctrl->hDll() , MAKEINTRESOURCE(IDM_MSG));
    HMENU menu = GetSubMenu(mainMenu , 0);
   // jedziem
    int ret = TrackPopupMenu(menu,TPM_NONOTIFY | TPM_RETURNCMD ,
                       x,y,0,this->hwnd , 0);
    switch (ret) {
       case ID_MSG_COPY:
           SendMessage(hwnd , WM_COPY , 0 , 0);
           break;
       case ID_MSG_SELECTALL:
           SendMessage(hwnd , EM_SETSEL , 0 , 0x7FFFFFFF);
           break;
       case ID_MSG_CLEAR:
           SetWindowText(hwnd , "");
           break;
    }
    // robim

    DestroyMenu(mainMenu);
}


int CALLBACK cMsgControlRich::WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
//   static char str_buff2 [MAX_STRBUFF];
   cMsgControlRich * MC;
   MC = (cMsgControlRich*)GetProp(hwnd , "cMsgControl*");
   if (!MC) return 0;
   static int mouseState = 0;
   switch (message)
   {
        case WM_CONTEXTMENU: {
            MC->contextMenu(LOWORD(lParam) , HIWORD(lParam));
            break;}
		case WM_DESTROY: {
			delete MC;
			SetProp(hwnd , "cMsgControl*" , 0);
			MC = 0;
			}
		case WM_LBUTTONDOWN:
			mouseState = 1;
			break;
		case WM_MOUSEMOVE:
			if (mouseState == 1) {
				mouseState = 2;
			}
			break;
		case WM_LBUTTONUP:
			if (mouseState == 2 && GETINT(CFG_UIMSGVIEW_COPY)) {
				mouseState = 0;
				int start , end;
				SendMessage(hwnd , EM_GETSEL , (WPARAM)&start , (LPARAM)&end);
				if (start == end) break;
	            SendMessage(hwnd , WM_COPY , 0 , 0);
		        SendMessage(hwnd , EM_SETSEL , -1 , 0);
				MessageBeep(MB_OK);
			} else mouseState = 0;
			break;

   }
   if (!MC) return 0;
   return CallWindowProc(MC->_oldwndproc , hwnd , message , wParam , lParam); 
}


// -------------------------------------------------------------


int ActionMsgEditMenuProc(sUIActionNotify_base * anBase) {
    sUIActionNotify_2params * an = static_cast<sUIActionNotify_2params*>(anBase);
	sUIAction actSend = sUIAction(IMIG_MSGWND, UI::ACT::msg_ctrlsend, an->act.cnt);
	unsigned int cntId = an->act.cnt;
    switch (an->act.id) {
		case UI::ACT::msg_ctrlsend_menu_copy:  case UI::ACT::msg_ctrlsend_menu_cut:
			if (an->code == ACTN_CREATE) {
				UI::Notify::_getSelection gs;
				gs.act = actSend;
				UIActionCall(&gs);
				UIActionSetStatus(an->act, (gs._start==gs._end) ? -1 : 0 , ACTS_DISABLED);
			} else if (an->code == ACTN_ACTION) {
				SendMessage((HWND)UIActionHandle(actSend), 
					(an->act.id == UI::ACT::msg_ctrlsend_menu_copy ? WM_COPY : WM_CUT), 0, 0);
			}
			break;
		case UI::ACT::msg_ctrlsend_menu_delete: 
			if (an->code == ACTN_ACTION) {
				UI::Notify::_getSelection gs;
				gs.act = actSend;
				UIActionCall(&gs);
				if (gs._start == gs._end) {
					gs.code = UI::Notify::setSelection;
					gs._start = 0;
					gs._end = 0x7FFFFFFF;
					UIActionCall(&gs);
				}
				UI::Notify::_insertText it;
				it.act = actSend;
				it._isHtml = false;
				it._position = -1;
				it._text = "";
				UIActionCall(&it);
			}
			break;
		case UI::ACT::msg_ctrlsend_menu_select: 
			if (an->code == ACTN_ACTION) {
				UI::Notify::_getSelection gs;
				gs.act = actSend;
				gs.code = UI::Notify::setSelection;
				gs._start = 0;
				gs._end = 0x7FFFFFFF;
				UIActionCall(&gs);
			}
			break;
		case UI::ACT::msg_ctrlsend_menu_undo:  case UI::ACT::msg_ctrlsend_menu_redo:
			if (an->code == ACTN_CREATE) {
				int bits = SendMessage((HWND)UIActionHandle(actSend), 
					(an->act.id == UI::ACT::msg_ctrlsend_menu_undo ? EM_CANUNDO : EM_CANREDO), 0, 0) ? 0 : -1;
				UIActionSetStatus(an->act, bits , an->act.id == UI::ACT::msg_ctrlsend_menu_undo ? ACTS_DISABLED : ACTS_HIDDEN);
			} else if (an->code == ACTN_ACTION) {
				SendMessage((HWND)UIActionHandle(actSend), 
					(an->act.id == UI::ACT::msg_ctrlsend_menu_undo ? EM_UNDO : EM_REDO), 0, 0);
			}
			break;
		case UI::ACT::msg_ctrlsend_menu_paste:
		case UI::ACT::msg_ctrlsend_menu_paste_nf:
		case UI::ACT::msg_ctrlsend_menu_paste_qt: {
			bool formatting = Act(actSend).call(UI::Notify::supportsFormatting, 0, 0);
//			formatting = true;
			actSend.cnt = cntId;
			HWND wndSend = (HWND)Act(actSend).handle;
			bool canPaste = SendMessage(wndSend, EM_CANPASTE, CF_TEXT, 0);
			int cfRTF = RegisterClipboardFormat(CF_RTF);
			bool pasteRTF = canPaste && SendMessage(wndSend, EM_CANPASTE, cfRTF, 0);
			if (an->code == ACTN_CREATE) {
				// wszystkie za³atwiamy za jednym razem...
				UIActionSetStatus(sUIAction(an->act.parent, UI::ACT::msg_ctrlsend_menu_paste, cntId)
					, ((canPaste)?0:ACTS_DISABLED) | ((!formatting && pasteRTF)?ACTS_HIDDEN:0) ,  ACTS_DISABLED | ACTS_HIDDEN);
				UIActionSetStatus(sUIAction(an->act.parent, UI::ACT::msg_ctrlsend_menu_paste_nf, cntId)
					, ((pasteRTF)?0:ACTS_HIDDEN) ,  ACTS_HIDDEN);
				UIActionSetStatus(sUIAction(an->act.parent, UI::ACT::msg_ctrlsend_menu_paste_qt, cntId)
					, ((canPaste)?0:ACTS_HIDDEN) ,  ACTS_HIDDEN);
			} else if (an->code == ACTN_ACTION) {
				switch (an->act.id) {
				case UI::ACT::msg_ctrlsend_menu_paste:
					SendMessage(wndSend, EM_PASTESPECIAL, pasteRTF ? cfRTF : CF_TEXT, 0);
					
					break;
				case UI::ACT::msg_ctrlsend_menu_paste_nf:
					SendMessage(wndSend, EM_PASTESPECIAL, CF_TEXT, 0);
					break;
				case UI::ACT::msg_ctrlsend_menu_paste_qt:
					if (OpenClipboard(0)) {
						HANDLE clipHandle = (char*) GetClipboardData(CF_TEXT);
						if (clipHandle) {
							const char * clip = (const char *) GlobalLock(clipHandle);
							if (clip) {
								CStdString text = " \"<i>" + string(clip) + "</i>\" ";
								GlobalUnlock(clipHandle);
								UI::Notify::_insertText it;
								it.act = actSend;
								it._isHtml = true;
								it._position = -1;
								it._text = text;
								UIActionCall(&it);
							}
						}
						CloseClipboard();
					}
					break;
				}
			}
			break;
			}
		case UI::ACT::msg_ctrlsend_menu_hist_previous:  case UI::ACT::msg_ctrlsend_menu_hist_next:
			if (an->code == ACTN_CREATE) {
				UIActionSetStatus(an->act, Cnt[an->act.cnt].haveMsgHistory(an->act.id == UI::ACT::msg_ctrlsend_menu_hist_next) ? 0 : -1 , ACTS_HIDDEN);
			} else if (an->code == ACTN_ACTION) {
				Cnt[an->act.cnt].useMsgHistory(an->act.id == UI::ACT::msg_ctrlsend_menu_hist_next);
			}
			break;
		case UI::ACT::msg_ctrlsend_menu_hist_clear:
			if (an->code == ACTN_CREATE) {
				UIActionSetStatus(an->act, (Cnt[an->act.cnt].msgHistory.size() > 1) ? 0 : -1 , ACTS_HIDDEN);
			} else if (an->code == ACTN_ACTION) {
				Cnt[an->act.cnt].clearMsgHistory();
			}
			break;
		
	}
/*
			const int msg_ctrlsend_menu_paste = 262;
			const int msg_ctrlsend_menu_paste_nf = 263;
			const int msg_ctrlsend_menu_paste_qt = 264;
			
			const int msg_ctrlsend_menu_hist_previous = 270;
			const int msg_ctrlsend_menu_hist_next = 271;
			const int msg_ctrlsend_menu_hist_clear = 272;
*/
	return 0;
}
