/*

    KONNEKT UI

    Deklaracje klas, definicje, zmienne/funkcje globalne

*/


#pragma once
#ifndef UIMAINH
#define UIMAINH


// ------------------------------------------------------------------------------
// Wymagane include'y

#ifdef IN_UIMAIN_CPP
    #define UIEXTERN
#else
    #define UIEXTERN extern
    #define MAKE_CTRL_EXTERN
#endif

#include "konnekt/plug_export.h"
#include "plug_sharedex.h"
#include "konnekt/ui.h"
#include "konnekt/plug_func.h"       // chyba nie jest tu potrzebny
#include "konnekt/ui_message_controls.h"       // chyba nie jest tu potrzebny

namespace Konnekt {
};

using namespace std;
using namespace Konnekt;
#include "include\func.h"
#include "include\time64.h"
#include "resource_ui.h"
#include "include\garbage.h"
#include "include\critical_section.h"

// ------------------------------------------------------------------------------
// DEFINE'y
#define __DEBUG

// NIE ZMIENIAÆ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                #define HIST_XOR_KEY "\x40\x13\xf8\xb2\x84\x23\x04\xae\x6f\x3d"
                                #define UI_CERT "\x10\x20 a\x60\xf0\x15\xae_uiw98"
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define MYWM_NOTIFYICON WM_USER+1086
#define MYWM_NOTIFYAPPBAR WM_USER+1087
#define MYWM_DLGLONGSET WM_USER+1088



#define EMSG_UI "UI.DLL - %s - "

#define MSGTITLE_CHAR '"'

#define MAINALPHA 191

#define BIG_STRING MAX_STRING

#define ANIM_INTERVAL GETINT(CFG_UIANIM)  //500
#define AWAY_INTERVAL 2000
#define TIMEOUT_INTERVAL 5000

#define DOCK_SPACE 80

#define MIN_WIDTH  100
#define MIN_HEIGHT 200

#define TIMER_LISTTIP 111

#define MYWM_REFRESH WM_USER+18090
#define MYWM_CANAPPLY WM_USER+18091
#define MYWM_SETICON WM_USER+18092

#define FILE_ENTRYHISTORY "ehst.dtb"

#define PROPParent "UIAParent"

// ------------------------------------------------------------------------------
// STRUKTURY


class CRITICAL_SECTION_WRAPER {
    private:
        //CRITICAL_SECTION section;
        cCriticalSection_WM section;
    public:
/*        void enter() {EnterCriticalSection(&section);}
        void leave(){LeaveCriticalSection(&section);}
        CRITICAL_SECTION_WRAPER() {InitializeCriticalSection(&section);}
        ~CRITICAL_SECTION_WRAPER() {DeleteCriticalSection(&section);}
        */
        void enter() {section.lock();}
        void leave() {section.unlock();}
		CRITICAL_SECTION_WRAPER () : section(WMProcess) {
        }
};



struct str_critical {
    CRITICAL_SECTION_WRAPER action;

};

// ------------------------------------------------------------------------------
// Zmienne globalne


#ifdef GARBAGE_H
    UIEXTERN cGarbageCollection Garbage;
#endif

UIEXTERN HWND hwndTop;
UIEXTERN HWND hwndMain;
UIEXTERN HWND hwndUp;
UIEXTERN HWND hwndDown;
UIEXTERN HWND hwndBar;
extern HWND hwndList;
extern HWND hwndGroups;
extern HWND hwndDock;
UIEXTERN HWND hwndConfig , hwndConfigTree;
extern HWND hwndSearch;
extern HWND hwndMsgEvent;
extern HWND hwndHistory;
extern HWND hwndDbgAct;

extern HINSTANCE hDll;

UIEXTERN bool isComCtl6;

extern bool cursorInList;
//bool isOpened=false;
UIEXTERN WNDPROC oldListControlProc;

UIEXTERN HMENU hmenuMain;
UIEXTERN HMENU hmenuStatus;
UIEXTERN HMENU hmenuCnt;
UIEXTERN HMENU hmenuTray;

extern HMENU hDestroyMenu;

extern HANDLE timerLong;
extern DWORD threadLong;
extern int longReference;

extern HANDLE timerAnim;
extern UINT timerHide;
extern UINT timerAway;

extern bool timerAnimOn;
extern bool canQuit;

extern int trayBitCount;

UIEXTERN HINSTANCE instRE;

extern HFONT font;
extern HFONT fontB;
extern HFONT fontBig;
extern HFONT fontNormal;
extern HFONT fontSmall;
extern HFONT fontSmallB;
extern HFONT fontLabel;
extern HFONT fontDing;
extern HFONT fontTip;
extern HFONT fontTipB;
extern HFONT fontMsgSend;
extern int fontHeight;

extern LOGFONTEX fontMenu;
extern LOGFONTEX fontMenuDefault;
extern LOGFONTEX fontMenuActive;

extern int listSelNet;
extern int listSelScroll;
extern string listSelUID;

extern string listGroup;


extern bool wmSize;

UIEXTERN bool autostart; // czy zostal wystartowany automatycznie 

extern int trayStatus;
extern string trayStatusDescr;
extern sUIAction trayAction;
extern int trayNotify;
extern int trayCnt;
//extern int trayNet;
extern int trayMsgID;
extern CStdString trayInfo;

extern int docked; // 1 - z hide'em , 2 - na sztywno
extern int docked_h;
extern bool docked_right;
UIEXTERN RECT docked_rect;

enum MenuType {
	menuTypeNormal = 0,
	menuTypeSmall = 1
};

extern MenuType menuType;

extern int lastUIdata [4]; // ostatnie - pozycja i aktywne okienka ...
extern bool away;

extern bool longOnly;

UIEXTERN CStdString profileDir;

UIEXTERN cCtrlEx * CtrlEx;

UIEXTERN str_critical critical; // Critical Sections


// -----------------------------------------------------------------------------
// FUNKCJE GLOBALNE

int CNotify(int pos);
int IPrepare();
UISet();
void UISetAlpha();

int CALLBACK InfoDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int CALLBACK MsgDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

int ActionProc(sUIActionNotify_base * anBase);
int ActionCntProc(sUIActionNotify_base * anBase);
int ActionCfgProc(sUIActionNotify_base * anBase);
int ActionProfileProc(sUIActionNotify_base * anBase);
int ActionMsgEditMenuProc(sUIActionNotify_base * anBase);

int IACntDel(int cnt);

void DebugIcons();


// -----------------------------------------------------------------------------
// ID akcji


#endif