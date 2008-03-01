/*

  Konnekt UI

  Obs³uga interfejsu.

*/

#include "stdafx.h"
#pragma hdrstop
#include "ui_main.h"
#include "include\dlghtml.h"


//#include <commctrl.h>
//#include <Richedit.h>
//#include <Mshtml.h>
//#include "include\win_listview.h"
#include "include\win_registry.h"


//#include <md5.h>
#include "include\func.h"
#include "include\preg.h"
//#include "include\dbtable.h"
//#include "include\dtablebin.h"

#include "include\time64.h"

#include "resource_ui.h"


#include "ui_actions.h"
#include "ui_cntlist.h"
#include "ui_cntlistfilter.h"
#include "ui_window.h"
#include "ui_dialogs.h"
#include "ui_ui.h"
#include "ui_msgcontrol.h"

#include "ui_act.hpp"
#include "ui_actcfg.hpp"

#pragma hdrstop


CStdString getStatusName(int status) {
    CStdString s;
    switch(status & ST_MASK) {
        case ST_OFFLINE: s = "Niedostêpny";break;
        case ST_ONLINE: s = "Dostêpny";break;
        case ST_AWAY: s = "Zaraz wracam";break;
        case ST_NA: s = "Nie ma mnie";break;
        case ST_AUTOAWAY: s = "Auto Away";break;
        case ST_CHAT: s = "Chêtnie pogadam";break;
        case ST_DND: s = "Nie przeszkadzaæ";break;
		case ST_HIDDEN: s = "Ukryty";break;
    }
    return s;
}

struct sFormatCallback {
	int cntID;
	formatTitle_enum type;
};


std::string __stdcall formatCallback(class cPreg & Preg , sFormatCallback * d) {
	bool matched = true;
    CStdString s("");
	CStdString token = Preg[2];
	bool fromInfoWindow = (d->type == FT_DISPLAYPROPOSAL);
	if (token == "profile") {
		s = SAFECHAR((char*)ICMessage(IMC_GETPROFILE));
	} else if (token == "version") {
	    ICMessage(IMC_VERSION , (int)s.GetBuffer(100));
		s.ReleaseBuffer();
	} else if (token == "UID") {
		s = SAFECHAR((char*)GETCNTC(d->cntID , CNT_UID));
	} else if (token == "status") {
		s = d->cntID?getStatusName(GETCNTI(d->cntID , CNT_STATUS) & ST_MASK):"";
	} else if (token == "info") {
		s = d->cntID?(char*)GETCNTC(d->cntID , CNT_STATUSINFO) : (char*)trayStatusDescr.c_str();
	} else if (token == "display") {
		s = GETCNTC(d->cntID , CNT_DISPLAY);
	} else if (token == "group") {
		s = GETCNTC(d->cntID , CNT_GROUP);
	} else if (token == "IP") {
		s = GETCNTC(d->cntID , CNT_HOST);
	} else if (d->type == FT_TRAYEVENT) {
		if (token == "eventDisplay") {
			s = trayCnt?(char*)GETCNTC(trayCnt , CNT_DISPLAY):"";
		} else if (token == "eventTitle") {
			s = (trayAction.id)?Act(trayAction).txt.substr(0 , Act(trayAction).txt.find(MSGTITLE_CHAR)):"";
		} else if (token == "eventMsg" && trayMsgID) {
			// Musimy pobrac Title!
      s=GetExtParam(Ctrl->DTgetStr(DTMSG , trayMsgID , Message::colExt) , Message::extTitle);
		} else
			matched = false;
	} else
		matched = false;
	if (!matched) { // druga szansa...
		int id = Ctrl->DTgetNameID(DTCNT, token);
		if (id < 0) // nic ju¿ nam nie pomo¿e
			return Preg[0];
		s = CntGetInfoValue(fromInfoWindow, (d->type == FT_TRAYEVENT) ? trayCnt : d->cntID, id);
	}
	if (s.empty())
		return "";
	else
		return Preg[1] + string(s) + Preg[3];
}

CStdString formatTitle(string format , int cntID , formatTitle_enum type) {
	if (!Ctrl->isRunning())
		return "";
	sFormatCallback data;
	data.cntID = cntID;
	data.type = type;

	cPreg Preg;
    Preg.setSubject(format);
    Preg.setPattern("/\\\\n/g");
	Preg.setSubject(Preg.replace("\n"));
    Preg.setPattern("/\\{([^a-z0-9_}]*?)([a-z0-9_\\/]+)([^a-z0-9_}]*?)\\}/mgi");
	return Preg.replace((fPregReplace)formatCallback , 0 , &data);
}


// ------------

int openURLMessage(Message * m) {

  CStdString url = m->getBody().a_str();
  CStdString info = m->getBody().a_str();

  CStdString Width = m->getExtParam("Width").a_str();
  CStdString Height = m->getExtParam("Height").a_str();

  if (url.find('\n')!=url.npos) {
      info = url.substr(0 , url.find_last_of('\n'));
      url.erase(0 , url.find_last_of('\n')+1);
  } else {
      info="";
  }

    info += "\r\nAdres URL zostanie otwarty.";
    int r = MessageBox((HWND)ICMessage(IMI_GROUP_GETHANDLE , (int)&sUIAction(0 , IMIG_MAINWND))
        , info , m->getExtParam("Title").a_str() , MB_ICONINFORMATION|MB_OKCANCEL);
    if (r==IDOK) {
        CStdString open = "javascript:window.open(\"" + url + "\",\"\",\"";
        open += "toolbar=0, "
	        + string("status=0, ")
	        + "menubar=0, "
	        + string("scrollbars=1, ")
            + "resizable=" + string(Height.empty()?"1":"0")
            ;
        if (!Width.empty()) open+=", width=" + Width;
        if (!Height.empty()) open+=", height=" + Height;
        open += "\");";
        ShellExecute(0 , "open" , url , "" , "" , SW_HIDE);
    }

    MessageSelect ms;
    ms.id = m->getId();
    ICMessage(MessageSelect::IM::imcMessageRemove, (int) &ms);

    return Message::resultOk;
}


void cSplash::show(HBITMAP bitmap , int time) {
    if (wnd) return;
    bmp = bitmap;
    BITMAP b;
    GetObject(bmp , sizeof(BITMAP),&b);
    b.bmWidth;
    b.bmHeight;
    RECT rc;
    GetWindowRect(GetDesktopWindow() , &rc);
    WNDCLASSEX wcex;
    memset(&wcex , 0 , sizeof(WNDCLASSEX));
    wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.hInstance		= hInst;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName	= "cSplash";
    wcex.style = isComctl(6 , 0) * CS_DROPSHADOW;
	RegisterClassEx(&wcex);
    wnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | (isComctl(6 , 0)?WS_EX_LAYERED:0) , "cSplash" , "kaBlam!" 
        , WS_POPUP
        , (rc.right - b.bmWidth)/2 , (rc.bottom - b.bmHeight)/2 , b.bmWidth , b.bmHeight , 0 , 0 , hInst , 0);

    SetProp(wnd , "cSplash*" , (HANDLE)this);
    if (SetAlpha(0,0)) {
        SetAlpha(wnd , 1);
        ShowWindow(wnd , SW_SHOW);
        Ctrl->WMProcess();
        // "pojawiamy"
        for (double i=0; i <= 1.5; i+=0.05) {
            SetAlpha(wnd , (char)ceil(sin(i)*255)+1);
            Sleep(1);
        }
        SetAlpha(wnd , 254);
    } else
        ShowWindow(wnd , SW_SHOW);
    //classProc = (WNDPROC)SetWindowLong(wnd , GWL_WNDPROC , (LONG)WndProc);
    if (time) SetTimer(wnd , 1 , time , 0);
    running=true;
}
void cSplash::hide() {
    if (!wnd || !running) return;
    running=false;
    KillTimer(wnd , 1);
    if (SetAlpha(0,0)) {
        SetAlpha(wnd , 254);
        Ctrl->WMProcess();
        // "chowamy"
        for (double i=0; i <= 1.5; i+=0.05) {
            SetAlpha(wnd , (char)ceil(cos(i)*255)+1);
            Ctrl->WMProcess();
            Sleep(1);
        }
    }
    DestroyWindow(wnd);
    wnd = 0;
    if (bmp) DeleteObject(bmp);
    bmp = 0;
    delete this;
}
cSplash::cSplash() {
    wnd=0;
    bmp=0;
    running=false;
}
cSplash::~cSplash() {
    if (wnd) hide();
}

LRESULT CALLBACK cSplash::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    cSplash* splash = (cSplash*)GetProp(hwnd , "cSplash*");
    switch (message) {
        case WM_PAINT:{
            PAINTSTRUCT ps; 
            HDC hdc = BeginPaint(hwnd, &ps); 
            DrawBitmap(hdc , splash->bmp , 0 , 0 , false);
            EndPaint(hwnd, &ps); 
            return 0;}
        case WM_TIMER: KillTimer(hwnd , wParam);splash->hide(); return 1;
        case WM_ERASEBKGND:
            return 1;
        case WM_LBUTTONUP:
            splash->hide();
            return 1;
    }
    return DefWindowProc(hwnd , message , wParam , lParam);
}

CStdString getAutostartKey() {
	CStdString key = resStr(IDS_APPNAME,hInst);
	UI::CFG::enApplyAutostart aa = (UI::CFG::enApplyAutostart) GETINT(CFG_UIAUTOSTARTAPPLIESTO);
	if (aa != UI::CFG::autostartLast)
		key += "_" + MD5Hex( (char*)ICMessage(IMC_RESTORECURDIR) ).substr(0,8);
	if (aa == UI::CFG::autostartCurrentProfile)
		key += "_" + string((char*)ICMessage(IMC_GETPROFILE));
	return key;
}
CStdString getAutostartValue() {
	CStdString value = GETSTR(CFG_APPFILE);
	value = "\"" + value + "\" /autostart";
	if (GETINT(CFG_UIAUTOSTARTASKFORPROFILE)) {
		value += " -profile=?";
	} else if (GETINT(CFG_UIAUTOSTARTAPPLIESTO) == UI::CFG::autostartCurrentProfile) {
		value += " -profile=" + string((char*)ICMessage(IMC_GETPROFILE));
	}
	return value;
}
void autostartCleanUp() {
	if (!IMessage(&sIMessage_msgBox(IMI_CONFIRM, "Czy na pewno chcesz usun¹æ ustawienia autostartu dla wszystkich kopii i profili Konnekta?"))) return;
	HKEY hKey=HKEY_CURRENT_USER;
	CStdString appName = resStr(IDS_APPNAME,hInst);
	if (!RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
		CStdString name;
		int i = 0;
		DWORD size = 255;
		int r;
		FILETIME lastWrite;
		while ((r = RegEnumValue(hKey, i, name.GetBuffer(size+1), &size, 0, 0, 0, 0)) == ERROR_SUCCESS) {
			name.ReleaseBuffer(size);
			size = 255;
			if (name.find(appName) == 0) {
				IMLOG("- Usuwam wpis autostartu - %s", name.c_str());
				RegDeleteValue(hKey , name);
			}
			i++;
		}
		RegCloseKey(hKey);
	}

}