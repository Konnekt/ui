//---------------------------------------------------------------------------
//http://appmsg.gadu-gadu.pl/appsvc/appmsg.asp?fmnumber=3501332

//#pragma link "MEMMGR.LIB"


#include "stdafx.h"
#pragma hdrstop
#define IN_UIMAIN_CPP

#include "resource_ui.h"
#include "resource.h"

#include "ui_main.h"

#include "ui_actions.h"
#include "ui_cntlist.h"
#include "ui_dialogs.h"
#include "ui_msgcontrol.h"
#include "ui_ui.h"
#include "ui_window.h"
#include "checkComposing.h"
#include "dlgShowBits.h"
#include "konnekt/lib.h"

#define DOCS_DIR "doc\\"
#define DOCS_STEPBYSTEP "szybka pomoc.html"

// zmienne

#include <konnekt/core_message.h>
int  IMsgOpen(Message*m);
class UIMessageHandler : public MessageHandler {
public:
  UIMessageHandler(enMessageQueue queue) : MessageHandler(queue) { };

public:
  bool handlingMessage(enMessageQueue queue, Message* msg);
  tMsgResult handleMessage(Message* msg, enMessageQueue queue, enPluginPriority priority);
};

bool UIMessageHandler::handlingMessage(enMessageQueue queue, Message* msg) {
  return queue & _queue;
}

tMsgResult UIMessageHandler::handleMessage(Message* msg, enMessageQueue queue, 
  enPluginPriority priority) {

    if (queue & mqReceive) {
      if (msg->getOneFlag(Message::flagMenuByUI)) {
        if (!msg->getAction().id) {
          msg->setAction(sUIAction(msg->getType() & Message::typeMask_NotOnList ? IMIG_EVENT : IMIG_CNT, IMIA_EVENT_OPENANYMESSAGE));
        }
        if (!msg->getNotify()) {
          msg->setNotify(UIIcon(IT_MESSAGE, 0, Message::flagMenuByUI, 0));
        }
      }

      if (!msg->getOneFlag(Message::flagHandledByUI) || msg->getOneFlag(Message::flagSend)) {
        return (tMsgResult) 0;
      }

      sUIAction act;

      if (msg->getType() == Message::typeMessage) {
        act = sUIAction(IMIG_CNT, IMIA_CNT_MSGOPEN);

      } else if (msg->getType() == Message::typeServerEvent) {
        act = sUIAction(IMIG_EVENT, IMIA_EVENT_SERVER);

      } else if (msg->getType() == Message::typeUrl) {
        act = sUIAction(IMIG_EVENT, IMIA_EVENT_URL);

      } else if (msg->getType() == Message::typeQuickEvent) {
        ICMessage(IMI_MSG_OPEN, (int) msg);
        return Message::resultDelete;

      } else {
        return (tMsgResult) 0;
      }

      msg->setAction(act);

      if (!msg->getNotify()) {
        msg->setNotify(UIIcon(IT_MESSAGE, 0, msg->getType(), 0));
      }

      return Message::resultOk;

    } else if (queue == mqOpen) {
      switch (msg->getType()) {
        case Message::typeMessage:
        case Message::typeQuickEvent:
          return (tMsgResult)ICMessage(IMI_MSG_OPEN, (int) msg);

        case Message::typeServerEvent:
          ServerEventDialogNext();
        case Message::typeUrl:
          return (tMsgResult) 0;
      }
      return (tMsgResult) 0;

    }
    return (tMsgResult)0;
}





int trayBitCount = isComctl(6,0)?32:4;
int trayStatus = IDI_TRAY;

HWND hwndList = 0;
HWND hwndGroups = 0;
HWND hwndDock = 0;
HWND hwndSearch = 0;
HWND hwndMsgEvent = 0;
HWND hwndHistory = 0;
HWND hwndDbgAct = 0;

HFONT font = 0;
HFONT fontB = 0;
HFONT fontBig = 0;
HFONT fontNormal = 0;
HFONT fontSmall = 0;
HFONT fontSmallB = 0;
HFONT fontLabel = 0;
HFONT fontMain = 0;
HFONT fontDing = 0;
HFONT fontTip = 0;
HFONT fontTipB = 0;
HFONT fontMsgSend = 0;
int fontHeight = 0;

LOGFONTEX fontMenu;
LOGFONTEX fontMenuDefault;
LOGFONTEX fontMenuActive;
LOGFONTEX fontMenuDefaultActive;


MenuType menuType = menuTypeNormal;

bool cursorInList = false;
HMENU hDestroyMenu=0;

HANDLE timerLong = 0;
DWORD threadLong = 0;
int longReference=0;

HANDLE timerAnim=0;
UINT timerHide = 0;
UINT timerAway = 0;

bool timerAnimOn = false;
bool canQuit = true;

int listSelNet=-1;
int listSelScroll=0;
string listSelUID = "";
string listGroup = "";

bool wmSize=false;

string trayStatusDescr="";
sUIAction trayAction = NOACTION;
int trayNotify = 0;
int trayCnt = 0;
//int trayNet = NET_GG;
int trayMsgID = 0;
CStdString trayInfo;

int docked = 0; // 1 - z hide'em , 2 - na sztywno
int docked_h = 200;
bool docked_right = false;

int lastUIdata [4] = {0,0,0,0}; // ostatnie - pozycja i aktywne okienka ...
bool away = false;

bool longOnly = false;

HINSTANCE hDll;
// ---------------------------------------------------------------------------
int WINAPI DllMain(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
    switch (reason) {
		case DLL_THREAD_ATTACH: if (Ctrl) Ctrl->onThreadStart();break;
		case DLL_THREAD_DETACH: if (Ctrl) Ctrl->onThreadEnd(); TLS.detach(); break;
        case DLL_PROCESS_ATTACH: hDll = hinst;break;
    }

    return 1;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int Init() {

  setlocale( LC_ALL, "Polish" );
  instRE = LoadLibrary("Riched20.dll");
  CoInitializeEx(0 , COINIT_APARTMENTTHREADED);

  font=GetLogFont("Tahoma" , 13 , 0);//GetStockObject(ANSI_VAR_FONT);
  fontB=GetLogFont("Tahoma" , 13 , 1);//GetStockObject(ANSI_VAR_FONT);
  fontNormal=GetLogFont("Tahoma" , 13 , 0);//GetStockObject(ANSI_VAR_FONT);
  fontSmall=GetLogFont("Tahoma" , 12 , 0);//GetStockObject(ANSI_VAR_FONT);
  fontSmallB=GetLogFont("Tahoma" , 12 , 1);//GetStockObject(ANSI_VAR_FONT);
  fontLabel=GetLogFont("Tahoma" , 11 , 2);//GetStockObject(ANSI_VAR_FONT);
  fontBig=GetLogFont("Tahoma" , 16 , 1);//GetStockObject(ANSI_VAR_FONT);
  fontDing=GetLogFont("Wingdings" , 18 , 0);
  isComCtl6 = ICMessage(IMC_ISWINXP)!=0;
  return 1;
}
void deInit() {
    delete cntTip;
    cntTip = 0;
    delete cntListWnd;
    cntListWnd = 0;
}

#define CFGSETCOL(i,d) sc.id=i;sc.def=(int)d;IMessage(IMC_CFG_SETCOL,0,0,(int)&sc)
#define CNTSETCOL(i,d) sc.id=i;sc.def=(int)d;IMessage(IMC_CNT_SETCOL,0,0,(int)&sc)


int ISetCols() {

//  char * c = 0;
//  c[0]=1;

	SetColumn(DTCFG , CFG_SHOWBITS, DT_CT_INT , 0 , "UI/showBits");
	SetColumn(DTCFG , CFG_SHOWBITS_BYHAND, DT_CT_INT , 0 , "UI/showBits/byHand");

	SetColumn(DTCFG , CFG_UIAUTOSTART , DT_CT_INT , 0 , "UI/autoStart");
	SetColumn(DTCFG , CFG_UIAUTOSTARTAPPLIESTO , DT_CT_INT , UI::CFG::autostartLast , "UI/autoStart/appliesTo");
	SetColumn(DTCFG , CFG_UIAUTOSTARTASKFORPROFILE , DT_CT_INT , 0 , "UI/autoStart/askForProfile");
	SetColumn(DTCFG , CFG_AUTOAWAY, DT_CT_INT , 300 , "UI/autoAway");
	SetColumn(DTCFG , CFG_UISPLASHSCREEN , DT_CT_INT , 1 , "UI/splashScreen");
	SetColumn(DTCFG , CFG_UIPLUGINSGROUP , DT_CT_INT , IMIG_PLUGINS , "UI/pluginsGroup"); // numer grupy. Gdy 0xFFFF jest == 15 to 0x1000 daje toolbar a 0x0 menu
	SetColumn(DTCFG , CFG_UIANIM , DT_CT_INT , 500 , "UI/animate");

	SetColumn(DTCFG , CFG_UIHIDDENSTART , DT_CT_INT , 0 , "UI/wnd/hiddenStart");
	SetColumn(DTCFG , CFG_UIAUTOHIDE , DT_CT_INT , 0 , "UI/wnd/autoHide");
	SetColumn(DTCFG , CFG_UIHIDDENAUTOSTART , DT_CT_INT , 1 , "UI/wnd/hiddenAutostart");
	SetColumn(DTCFG , CFG_UISNAP , DT_CT_INT , 1 , "UI/wnd/snap");
	SetColumn(DTCFG , CFG_UIONTOP , DT_CT_INT , 0 , "UI/wnd/onTop");
	SetColumn(DTCFG , CFG_UIPOS_X , DT_CT_INT , 0 , "UI/wnd/pos/X");
	SetColumn(DTCFG , CFG_UIPOS_Y , DT_CT_INT , 0 , "UI/wnd/pos/Y");
	SetColumn(DTCFG , CFG_UIPOS_W , DT_CT_INT , 0 , "UI/wnd/pos/W");
	SetColumn(DTCFG , CFG_UIPOS_H , DT_CT_INT , 0 , "UI/wnd/pos/H");
	SetColumn(DTCFG , CFG_UIMAINTITLE , DT_CT_PCHAR | DT_CF_CXOR , "KONNEKT {version}{ [profile]}" , "UI/wnd/wndTitle");
	SetColumn(DTCFG , CFG_UIMAINALPHA , DT_CT_INT , 75 , "UI/wnd/alpha");
	SetColumn(DTCFG , CFG_UIUSEMAINALPHA , DT_CT_INT , 0, "UI/wnd/alphaUse");
	SetColumn(DTCFG , CFG_UIMAINTASKBAR , DT_CT_INT , 0 , "UI/wnd/onTaskBar");

	SetColumn(DTCFG , CFG_UIMENUTYPE , DT_CT_INT , menuTypeNormal , "UI/menu/type");

	SetColumn(DTCFG , CFG_UICANDOCK , DT_CT_INT , 1 , "UI/dock");
	SetColumn(DTCFG , CFG_UIDOCKSPACE ,  DT_CT_INT , 20 , "UI/dock/space");
	SetColumn(DTCFG , CFG_UIDOCKEDALPHA ,  DT_CT_INT , 70 , "UI/dock/alpha");
	SetColumn(DTCFG , CFG_UIDOCKWAIT ,  DT_CT_INT , 250 , "UI/dock/wait");
	SetColumn(DTCFG , CFG_UIPOS_DOCKED , DT_CT_INT , 0 , "UI/dock/pos");
	SetColumn(DTCFG , CFG_UIDOCKHIDE , DT_CT_INT , 1 , "UI/dock/hide");
	SetColumn(DTCFG , CFG_UIDOCKSTRETCH , DT_CT_INT , 1 , "UI/dock/stretch");

	SetColumn(DTCFG , CFG_UITRAY , DT_CT_INT , 1 , "UI/tray");
	SetColumn(DTCFG , CFG_UICNTONLINENOTIFY , DT_CT_INT , 1 , "UI/tray/cntOnlineNotify");
	SetColumn(DTCFG , CFG_UISTATUSINTRAY , DT_CT_INT , -1 , "UI/tray/showStatus");
	SetColumn(DTCFG , CFG_UIONECLICKTRAY, DT_CT_INT , 1 , "UI/tray/oneClickOpen");
	SetColumn(DTCFG , CFG_UITRAYTITLE , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/tray/title");
	SetColumn(DTCFG , CFG_UITRAYTITLEEVENT , DT_CT_PCHAR | DT_CF_CXOR , "<b>{eventTitle}</b>{ \"eventMsg\"}{ [eventDisplay]}" , "UI/tray/titleOnEvent");
	SetColumn(DTCFG , CFG_UITRAYTOOLTIPSHOWSTATUS , DT_CT_INT , 1 , "UI/tray/tooltip/showStatus");

	SetColumn(DTCFG , CFG_UIMSGFLASH , DT_CT_INT , 1 , "UI/msg/flash");
	SetColumn(DTCFG , CFG_UIMSGPOPUP , DT_CT_INT , 0 , "UI/msg/popup");
	SetColumn(DTCFG , CFG_UIMSGFROMOTHER , DT_CT_INT , 1 , "UI/msg/fromOther");
	SetColumn(DTCFG , CFG_UIMSGHOLDNOTIFY, DT_CT_INT , 1 , "UI/msg/holdNotify");
	SetColumn(DTCFG , CFG_UIMSGBYENTER , DT_CT_INT , 1 , "UI/msg/sendByEnter");
	SetColumn(DTCFG , CFG_UIMESSAGEHISTORY , DT_CT_INT , 1 , "UI/msg/MRU");
	SetColumn(DTCFG , CFG_UIMSGSHOWSTATUS , DT_CT_INT , 1 , "UI/msg/showStatus");
	SetColumn(DTCFG , CFG_UIMSGVIEW_COPY , DT_CT_INT , 0 , "UI/msg/copyBySelection");
	SetColumn(DTCFG , CFG_ACK_SHOWQUEUED , DT_CT_INT , 0 , "UI/msg/ack/queued");
	SetColumn(DTCFG , CFG_ACK_SHOWFAILED , DT_CT_INT , 1 , "UI/msg/ack/failed");
	SetColumn(DTCFG , CFG_UIMSGTITLE , DT_CT_PCHAR | DT_CF_CXOR , "{display }{[status] }{\"info\"}" , "UI/msg/wndTitle");
	SetColumn(DTCFG , CFG_UIMSGFROMOTHERREPLY , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/msg/nol/reply");
	SetColumn(DTCFG , CFG_UIMSGFROMOTHERPASS , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/msg/nol/pass");
//	SetColumn(DTCFG , CFG_UISORTBYMSG , DT_CT_INT , 0 , "UI/sortByMsg");


//  sc.id = CFG_FRIENDSONLY;
//  IMessage(IMC_CFG_SETCOL,0,0,(int)&sc);

//	SetColumn(DTCFG , CFG_UIMSGFONTSIZE , DT_CT_INT , 10 , "UI/msgFontSize");
	SetColumn(DTCFG , CFG_UIINFOTIP , DT_CT_INT , 1 , "UI/tip");
	SetColumn(DTCFG , CFG_UITIPDELAY, DT_CT_INT , 250 , "UI/tip/delay");
	SetColumn(DTCFG , CFG_UILASTACTIVITY, DT_CT_INT , 1 , "UI/tip/showLastActivity");
	SetColumn(DTCFG , CFG_UISHOWIP ,  DT_CT_INT , 1 , "UI/tip/showIP");
	SetColumn(DTCFG , CFG_UICNTTIPDOCKED ,  DT_CT_INT , 0 , "UI/tip/docked");
	SetColumn(DTCFG , CFG_UICNTTIPICONS , DT_CT_INT , 1 , "UI/tip/showIcons");
	SetColumn(DTCFG , CFG_UICNTTIPHEADER , DT_CT_INT , 1 , "UI/tip/showHeader");
	SetColumn(DTCFG , CFG_UICNTTIPINFO , DT_CT_PCHAR , "" , "UI/tip/infoFormat");
	SetColumn(DTCFG , CFG_UICNTTIP_SHOWDEFAULT, DT_CT_INT, 1 , "UI/tip/showDefault");

	SetColumn(DTCFG , CFG_UIREMEMBERMSGWND, DT_CT_INT , 1 , "UI/msg/dontClearWnd");
	SetColumn(DTCFG , CFG_UIEDITONLIST, DT_CT_INT , 0 , "UI/list/editNames");
	SetColumn(DTCFG , CFG_UISORTACTIVE, DT_CT_INT , 1 , "UI/list/sortActive");
	SetColumn(DTCFG , CFG_UIONLINEBOLD, DT_CT_INT , 1 , "UI/list/onlineBold");
	SetColumn(DTCFG , CFG_UICL_ICONSON ,  DT_CT_INT , 1 , "UI/list/icons/On");
	SetColumn(DTCFG , CFG_UICL_ICONSOFFSET ,  DT_CT_INT , 1 , "UI/list/icons/Offset");
	SetColumn(DTCFG , CFG_UICL_ICONSONRIGHT ,  DT_CT_INT , 0 , "UI/list/icons/OnRight");
	SetColumn(DTCFG , CFG_UICL_ICONS ,  DT_CT_INT , 0x15 , "UI/list/icons"); /* 010101b */
	SetColumn(DTCFG , CFG_UICL_INFO ,  DT_CT_INT , 1 , "UI/list/statusInfo"); 
	SetColumn(DTCFG , CFG_UICL_INFO_FORMAT ,  DT_CT_PCHAR , "{info}" , "UI/list/statusInfo/Format"); 
	SetColumn(DTCFG , CFG_UIAUTOADDCNT , DT_CT_INT , 0 , "UI/list/autoAddCnt");
	SetColumn(DTCFG , CFG_UIANIMCNT , DT_CT_INT , 1 , "UI/list/animate");
	SetColumn(DTCFG , CFG_UISAMECNTSTATUS , DT_CT_INT , 0 , "UI/list/sameStatusIconType");
	SetColumn(DTCFG , CFG_UICNTONLINEONLIST , DT_CT_INT , 1 , "UI/list/markStatusChange");
	SetColumn(DTCFG , CFG_UILIST_FLATSB ,  DT_CT_INT , 0 , "UI/list/flatScrollbar");
	SetColumn(DTCFG , CFG_UICL_BUFFEREDPAINT , DT_CT_INT , 0 , "UI/list/doubleBuffer");
	SetColumn(DTCFG , CFG_UIFILTERS , DT_CT_PCHAR , 0 , "UI/list/filters");  
	SetColumn(DTCFG , CFG_UIFILTERACTION , DT_CT_INT , Konnekt::UI::CFG::gtypeHidden , "UI/list/filters/actionType");
	SetColumn(DTCFG , CFG_UICL_DISPLAY_FORMAT , DT_CT_PCHAR , 0 , "UI/list/displayFormat");


	SetColumn(DTCFG , CFG_UICNT_CREATEDISPLAY_FORMAT , DT_CT_PCHAR , "" , "UI/cnt/createDisplayFormat");


	SetColumn(DTCFG , CFG_UIGRPTABS_MULTILINE , DT_CT_INT ,  1 , "UI/groups/multiline");
	SetColumn(DTCFG , CFG_UIGRPTABS_VERTICAL , DT_CT_INT ,  0 , "UI/groups/vertical");
	SetColumn(DTCFG , CFG_UIALLGROUPSDESC , DT_CT_PCHAR | DT_CF_CXOR , "*" , "UI/groups/allDescription");
	SetColumn(DTCFG , CFG_UIALLGROUPS_NOGROUP , DT_CT_INT , 0 , "UI/groups/allNoGroup");

	SetColumn(DTCFG , CFG_DESCR_AWAY , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/descr/away");
  	SetColumn(DTCFG , CFG_DESCR_AAWAY , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/descr/autoAway");
  	SetColumn(DTCFG , CFG_DESCR_DND , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/descr/DND");
  	SetColumn(DTCFG , CFG_DESCR_HIDDEN , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/descr/hidden");
	SetColumn(DTCFG , CFG_DESCR_ONLINE , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/descr/online");
	SetColumn(DTCFG , CFG_DESCR_OFFLINE , DT_CT_PCHAR | DT_CF_CXOR , "" , "UI/descr/offline");
	SetColumn(DTCFG , CFG_DESCR_USEMANUAL, DT_CT_INT , 0 , "UI/descr/useManual");
	SetColumn(DTCFG , CFG_DESCR_CLEARMANUAL, DT_CT_INT , 0 , "UI/descr/clearManual");



	SetColumn(DTCFG , CFG_ISAUTOAWAY , DT_CT_INT | DT_CF_NOSAVE , 0);
	
	

//  CFGSETCOL(CFG_DESCR,0);

  // Czcionki
	SetColumn(DTCFG , CFG_UIF_MSG,DT_CT_PCHAR ,  "Tahoma;238;-9;;0x0;0xeeeeee;" , "UI/font/msg");
	SetColumn(DTCFG , CFG_UIF_MSGCNT,DT_CT_PCHAR , "Tahoma;238;-9;;0x804000;0xeeeeee;" , "UI/font/msgContact");
	SetColumn(DTCFG , CFG_UIF_MSGRCV,DT_CT_PCHAR , "Tahoma;238;-12;b;0x532900;0xeeeeee;" , "UI/font/msgReceived");
	SetColumn(DTCFG , CFG_UIF_MSGSEND,DT_CT_PCHAR , "Tahoma;238;-12;;0x532900;0xeeeeee;" , "UI/font/msgSend");
	SetColumn(DTCFG , CFG_UIF_MSGINFO,DT_CT_PCHAR , "Tahoma;238;-11;b;0xff;0xeeeeee;" , "UI/font/msgInfo");
	SetColumn(DTCFG , CFG_UIF_MSGMARK,DT_CT_PCHAR , "Tahoma;238;-15;b;0x0080;0xeeeeee;" , "UI/font/msgMark");
	SetColumn(DTCFG , CFG_UIF_CLIST,DT_CT_PCHAR , "Tahoma;238;-11;;0x462300;0xeeeeee;" , "UI/font/list");
	SetColumn(DTCFG , CFG_UIF_CLISTSEL,DT_CT_PCHAR , "Tahoma;238;-11;b;0xffffff;0xa2877d;" , "UI/font/listSelection");
	SetColumn(DTCFG , CFG_UIF_CLISTINFO,DT_CT_PCHAR , "Tahoma;238;-9;;0xAA6A00;0xeeeeee;" , "UI/font/listInfo");

	SetColumn(DTCFG , CFG_UICNTTIPFONT , DT_CT_PCHAR , "Tahoma;238;-9;;0xffffff;0x804000;" , "UI/font/tip");

	SetColumn(DTCFG , CFG_UIF_MENU , DT_CT_PCHAR , "Tahoma;238;-11;;0x0;0xffffff;" , "UI/menu/font");
	SetColumn(DTCFG , CFG_UIF_MENUDEFAULT , DT_CT_PCHAR , "Tahoma;238;-11;b;0x0;0xffffff;" , "UI/menu/fontDefault");
	SetColumn(DTCFG , CFG_UIF_MENUACTIVE , DT_CT_PCHAR , "Tahoma;238;-11;b;0xffffff;0x804000;" , "UI/font/fontActive");

	SetColumn(DTCNT , Contact::colStatusIcon , DT_CT_INT , 0 , "UI/status/icon");
  return 1;
}


void IUpdate(int ver) {
	if (!ver) return;
	if (ver < VERSION_TO_NUM(0, 1, 21, 33)) {
		if (GETINT(CFG_UIMAINALPHA) < 0) {
			SETINT(CFG_UIMAINALPHA, -GETINT(CFG_UIMAINALPHA));
			SETINT(CFG_UIUSEMAINALPHA, 0);
		} else {
			SETINT(CFG_UIUSEMAINALPHA, 1);
		}
	}
    if (ver < VERSION_TO_NUM(0,1,9,137)) {
        SETSTR(CFG_UIMAINTITLE , "KONNEKT {version}{ [profile]}");
    } 
    if (ver < VERSION_TO_NUM(0,1,12,139)) {
        SETSTR(CFG_UITRAYTITLEEVENT , "{eventTitle}{ \"eventMsg\"}{ [eventDisplay]}");
    }
    if (ver < VERSION_TO_NUM(0,1,13,141)) {
        SETSTR(CFG_UIF_MSG,"Tahoma;238;-9;;0x0;0xeeeeee;");
        SETSTR(CFG_UIF_MSGCNT,"Tahoma;238;-9;;0x804000;0xeeeeee;");
        SETSTR(CFG_UIF_MSGRCV,"Tahoma;238;-12;b;0x532900;0xeeeeee;");
        SETSTR(CFG_UIF_MSGSEND,"Tahoma;238;-12;;0x532900;0xeeeeee;");
        SETSTR(CFG_UIF_MSGINFO,"Tahoma;238;-11;b;0xff;0xeeeeee;");
        SETSTR(CFG_UIF_CLIST,"Tahoma;238;-11;;0x462300;0xeeeeee;");
        SETSTR(CFG_UIF_CLISTSEL,"Tahoma;238;-11;b;0xffffff;0xa2877d;");
    }
    if (ver == VERSION_TO_NUM(0,1,15,148)) {
        MessageBox(0 , "W wersji UI 0.1.15.148 znajdowa³ siê b³¹d który móg³ uszkodziæ pliki historii.\r\nAby je naprawiæ od razu po uruchomieniu otwórz historiê i w³¹cz kompaktowanie!!!" , "Konnekt - ostrze¿enie" , MB_OK|MB_ICONWARNING|MB_APPLMODAL);
    }
    if (ver < VERSION_TO_NUM(0,1,16,0)) {
		if (GETINT(CFG_UIANIM)) SETINT(CFG_UIANIM , 500);
	}
}

// *********************************************************
int IPrepare() {
/*
//  timerAnim = CreateWaitableTimer(&sa,0,0);
//  DWORD size = MAX_STRBUFF;
//  void * p;
//  InternetQueryOption(0,INTERNET_OPTION_PROXY, str_buff , &size);
//  ErrorMsg();
//  INTERNET_PROXY_INFO * ipi = (INTERNET_PROXY_INFO *)str_buff;

  string proxy = ipi->lpszProxy;
  string bypass = ipi->lpszProxyBypass;
  GlobalFree((void*)ipi->lpszProxy);
  GlobalFree((void*)ipi->lpszProxyBypass);

  InternetQueryOption(0,INTERNET_OPTION_PROXY_USERNAME, 0 , &size);

  int re = InternetQueryOption(0,INTERNET_OPTION_PROXY_USERNAME, str_buff , &size);
  string password = str_buff;
//  INTERNET_OPTION_PROXY
//INTERNET_OPTION_PROXY_PASSWORD
//INTERNET_OPTION_PROXY_USERNAME
*/
  profileDir = (char*)ICMessage(IMC_PROFILEDIR);
//  profileDir += "\\";


  // SPLASH
  bool splashFirst = ICMessage(IMC_ISNEWPROFILE) || ICMessage(IMC_ISNEWVERSION);
  if (splashFirst || GETINT(CFG_UISPLASHSCREEN)) {
    cSplash * splash = new cSplash;
    splash->show((HBITMAP)LoadImage(Ctrl->hDll() , MAKEINTRESOURCE(IDB_SPLASH) , IMAGE_BITMAP , 0 , 0 , 0) 
        , splashFirst?0:3000);
  }


  timerLong = CreateWaitableTimer(0,0,0);

  hInst=(HINSTANCE)IMessage(IMC_GETINSTANCE);

  autostart = ICMessage(IMC_ARGC) > 1 && !stricmp((char*)ICMessage(IMC_ARGV,1) , "/autostart");
  docked = -GETINT(CFG_UIPOS_DOCKED);
  if (docked < 0) docked_h = GETINT(CFG_UIPOS_H);
  RECT rc;
  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
  docked_right = rc.right - GETINT(CFG_UIPOS_X) - GETINT(CFG_UIPOS_W)
               < GETINT(CFG_UIPOS_X) - rc.left;

  if (isComctl(6,0)) SETINT(CFG_UIGRPTABS_VERTICAL , 0);

  registerClasses();
  createWindows();
  registerActions();
  createTray();
  fillGroups();
  Cnt.imPrepare();
  cntList.refresh(true);
  Act[IMIG_MAINWND].ghandle = (void*)hwndMain;
  Act[0].ghandle = (void*)hwndTop;
  fontHeight = GetFontHeight(hwndTop, font);


  // Skonczyl
  return 1;
}

void UISetAlpha() {
	int alpha = GETINT(CFG_UIMAINALPHA);
	SetAlpha(hwndMain , (!GETINT(CFG_UIUSEMAINALPHA) || alpha >= 100) ? 255 : (char)ceil(alpha * 2.55));
}

int UISet() {
    IMLOG("UISet");

	menuType = (MenuType) GETINT(CFG_UIMENUTYPE);

    // Fonty
    if (fontTip) DeleteObject(fontTip);
    if (fontTipB) DeleteObject(fontTipB);
    if (fontMsgSend) DeleteObject(fontMsgSend);
    LOGFONTEX lfe = StrToLogFont(GETSTR(CFG_UICNTTIPFONT));
    fontTip = CreateFontIndirect(&lfe.lf);
    lfe.lf.lfWeight = FW_BOLD;
    fontTipB = CreateFontIndirect(&lfe.lf);
    lfe = StrToLogFont(GETSTR(CFG_UIF_MSGSEND));
    fontMsgSend = CreateFontIndirect(&lfe.lf);

	fontMenu = StrToLogFont(GETSTR(CFG_UIF_MENU));
	fontMenuDefault = StrToLogFont(GETSTR(CFG_UIF_MENUDEFAULT));
	fontMenuActive = StrToLogFont(GETSTR(CFG_UIF_MENUACTIVE));
	fontMenuDefaultActive = fontMenuActive;
	fontMenuDefaultActive.lf.lfWeight = (fontMenuDefault.isBold() || fontMenuActive.isBold()) ? FW_BOLD : FW_NORMAL;

  string title = formatTitle((char*)GETSTR(CFG_UIMAINTITLE), 0 , FT_WINDOW);
  SetWindowText(hwndMain , title.c_str());
  SetWindowLong(hwndGroups , GWL_STYLE , (GetWindowLong(hwndGroups , GWL_STYLE) & ~(TCS_VERTICAL | TCS_MULTILINE))
      | (GETINT(CFG_UIGRPTABS_VERTICAL)?TCS_VERTICAL:0)
      | (GETINT(CFG_UIGRPTABS_MULTILINE)?TCS_MULTILINE:0));
  ListView_SetExtendedListViewStyleEx(cntListWnd->_hwnd , LVS_EX_FLATSB , (GETINT(CFG_UILIST_FLATSB)?LVS_EX_FLATSB:0));
  
  SetWindowPos(hwndGroups , 0 , 0 , 0 , 0 , 0 , SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

  AutoAway();
  if (!GETINT(CFG_UISTATUSINTRAY)) {
          trayStatus = IDI_TRAY;
          if (!trayNotify) changeTray(trayStatus , (char*)-1);
  } else {
/*	  int inTray = GETINT(CFG_UISTATUSINTRAY);
	  if (inTray == 1) {
		  trayNet = IMessageDirect(IM_PLUG_NET , Act[IMIG_STATUS][0].owner);
	  } else {
		  if (inTray > 0) {
			  // sprawdzamy czy sieæ istnieje...
			  if (!IMessage(IM_PLUG_NETNAME, inTray, imtNetUID, 0, 0)) {
				  inTray = -1;
			  } else {

			  }
		  }
	  }	 */
  }
  SetWindowPos(hwndMain , GETINT(CFG_UIONTOP)?HWND_TOPMOST:HWND_NOTOPMOST , 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
  // GETINT(CFG_UIONTOP)
  dockEnd();
  if (docked) {
    int nd = GETINT(CFG_UICANDOCK)?(GETINT(CFG_UIDOCKHIDE)?1:2):0;
    if (nd != docked) {
      docked = -10 - abs(docked);
      dockEnd();
      docked = -nd;
      dockEnd();
    }
  }
  cntList.cfgSet();

  Cnt.cfgSet();
  cntList.refill();
  fillGroups();
  onSizeMain(0,0);
  UISetAlpha();

  SetWindowLong(hwndMain , GWL_EXSTYLE , GetWindowLong(hwndMain , GWL_EXSTYLE) & (~WS_EX_APPWINDOW) | ( GETINT(CFG_UIMAINTASKBAR) ? WS_EX_APPWINDOW : 0 ) );
//  SetWindowLong(hwndTop , GWL_EXSTYLE , GetWindowLong(hwndTop , GWL_EXSTYLE) & (~WS_EX_NOACTIVATE) | ( GETINT(CFG_UIMAINTASKBAR) ? WS_EX_NOACTIVATE : 0 ) );
  
  return 1;
}

void TestBreakDown() {
	char * e = 0;
	e[0] = 'b';
	e[0] = 'a';
	e[0] = 'o';
}

int IMsgOpen(Message * m);

iMessageHandler* rhandler;
iMessageHandler* ohandler;
int IStart() {            
//	TestBreakDown();
    registerActionsAfter();
    IMLOG("> autostart = %d" , autostart);
    Act.Mod.load();
  makeActions();
  UISet();
  startWindows();
  if (cntTip->docked) cntTip->show(-1);
 // MessageHandler::registerHandler(
//  GetWindowText(GetWindow(hwndMain,GW_CHILD) , str_buff , MAX_STRBUFF);
  IMessage(&iMessageHandler::IM(iMessageHandler::IM::imcRegisterMessageHandler,
    rhandler = new UIMessageHandler(iMessageHandler::mqReceive), priorityCore));
  IMessage(&iMessageHandler::IM(iMessageHandler::IM::imcRegisterMessageHandler,
    ohandler = new UIMessageHandler(iMessageHandler::mqOpen), priorityLowest));

  return 1;
}

int IEnd() {
    // Najpierw trzeba zatrzymaæ wszystko co siê "rusza"
    IMessage(&iMessageHandler::IM(iMessageHandler::IM::imcUnregisterMessageHandler,
     rhandler, priorityCore));
    IMessage(&iMessageHandler::IM(iMessageHandler::IM::imcUnregisterMessageHandler,
      ohandler, priorityLowest));

    if (timerLong) CloseHandle(timerLong);
    timerLong = 0;
    if (timerAnim) KillTimer(hwndMain , 2);

  IMLOG("- Shuting down UI");
  unregisterActions();
  if (hwndConfig) {
	  IMLOG("-   Destroy hwndConfig");
	  DestroyWindow(hwndConfig);
  }
  hwndConfig = 0;
  if (hwndHistory) {
	  IMLOG("-   Destroy hwndHistory");
	  DestroyWindow(hwndHistory);
  }
  IMLOG("-   closing CNT");
  hwndHistory = 0;
  Cnt.closeAll();
  RECT rc;
  GetWindowRect(hwndMain , &rc);
  SETINT(CFG_UIPOS_X , rc.left);
  SETINT(CFG_UIPOS_Y , rc.top);
  SETINT(CFG_UIPOS_W , rc.right - rc.left);
  SETINT(CFG_UIPOS_H , docked>0?docked_h : rc.bottom - rc.top);
  SETINT(CFG_UIPOS_DOCKED , docked>=0?docked : 0);
  if (docked > 0) docked = -10 - docked;
  
  IMLOG("-   dockEnd");
  dockEnd();
  IMLOG("-   destroyTray");
  destroyTray();
  IMLOG("-   destroyAct");
  destroyActions();
  IMLOG("-   destroyWindows");
  destroyWindows();
  // AutoStart
/*  HKEY hKey=HKEY_CURRENT_USER;
  if (!RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
    if (GETINT(CFG_UIAUTOSTART)) {
       RegSetSZ(hKey,resStr(IDS_APPNAME,hInst),_sprintf("\"%s\" /autostart",GETSTR(CFG_APPFILE)));
    }
    else
       RegDeleteValue(hKey , resStr(IDS_APPNAME,hInst));
    RegCloseKey(hKey);
  }
*/
  FreeLibrary(instRE);
  IMLOG("-   Mod.save");
  Act.Mod.save();
  IMLOG("! UI shut down");
  return 1;
}



int IMsgOpen(Message * m) {
  int pos = ICMessage(IMC_FINDCONTACT , m->getNet() , (int)m->getFromUid().a_str());
  if (pos < 0) return Message::resultDelete;
  if (!Cnt[pos].hwndMsg || !IsWindowVisible(Cnt[pos].hwndMsg))
      if (GETINT(CFG_UIMSGPOPUP))
        {Cnt[pos].MsgWndOpen(0 , true);}
      else
        return 0;

//  IMLOG("ACTIVE msg %x foc %x frg %x act %x" , Cnt[pos].hwndMsg , GetFocus() , GetForegroundWindow() , GetActiveWindow() , );
  int ret = Message::resultDelete;
   if (GetForegroundWindow()!=Cnt[pos].hwndMsg) {
    if (GETINT(CFG_UIMSGHOLDNOTIFY) && m->getType() == Message::typeMessage) {
      m->setOneFlag(Message::flagOpened, true);
      SetProp(Cnt[pos].hwndMsg , "MsgWaiting" , (void*)1);
      ret = Message::resultUpdate;
      SendMessage(Cnt[pos].hwndMsg , MYWM_SETICON , IDI_WND_MSG_NEW , 0);
	  //
    }
    if (GETINT(CFG_UIMSGFLASH)) {
      FLASHWINFO fwi;
      memset(&fwi , 0 , sizeof(fwi));
      fwi.cbSize=sizeof(fwi);
      fwi.hwnd=Cnt[pos].hwndMsg;
      fwi.dwFlags = FLASHW_ALL /*| FLASHW_TIMERNOFG*/ | FLASHW_TIMER;
      fwi.uCount = GETINT(CFG_UIMSGFLASH);
      fwi.dwTimeout = 0;
      FlashWindowEx(&fwi);
      SetProp(Cnt[pos].hwndMsg , "WindowFlashing" , (HANDLE)1);
    }
  }

  switch (m->getType()) {
    case Message::typeMessage:
      {
        int session = (int)GetProp(Cnt[pos].hwndMsg , "MsgSession");
        if (!session) SetProp(Cnt[pos].hwndMsg , "MsgSession" , (void*)1);
      }
    /* pass through */  
    case Message::typeQuickEvent:
//      st = (sTime64*)&m->time;
        Cnt[pos]._msgControl->msgInsert(m);
      return ret;
  }
  //SetProp(Cnt[pos].hwndMsg , "MsgSession" , (void*)1);
  return 0;
}


int CStatus (tPluginId sender , int status , const char * descr){
    if (!hwndMain) return 0;
    if (!descr) descr=""; 

    int inTray = GETINT(CFG_UISTATUSINTRAY);
	if (inTray > 1 && !IMessage(IM_PLUG_NETNAME, (Net::tNet)inTray, imtProtocol, 0, 0)) {
		// sprawdzamy czy sieæ istnieje...
		inTray = -1;
	}

	int net = IMessageDirect(IM_PLUG_NET , sender);
    int ico = UIIcon(4 , net , status , 0);
	static int lastStatus = -1;
    for (int i = 0 ; i<Act[IMIG_STATUS].size() ; i++) {
      if (Act[IMIG_STATUS][i].owner == sender) {
//        IMLOG("STATUS %d %d %x %d" , i , status , UIIcon(4 , IMessageDirect(IM_PLUG_NET , sender) , status , 0) , Ico.find(UIIcon(4 , IMessageDirect(IM_PLUG_NET , sender) , status , 0)));
        ActionStatus(Act[IMIG_STATUS][i].act() , -1 , 0 , ico);
        if (inTray == 1 && i == 0) {
			inTray = net;
		}// else {trayStatusDescr="";}
        Animate();
        break;
      }
    }
    if ((inTray == -1 && status != lastStatus) || (inTray > 1 && net == inTray)) {
		lastStatus = status;
        trayStatus = ico;
        trayStatusDescr = descr;

        // Zmieniamy format glownego okna
        SetWindowText(hwndMain , formatTitle((char*)GETSTR(CFG_UIMAINTITLE), 0 , FT_WINDOW));

        if (!trayNotify) {
			changeTray(ico , (char*)-1);
			trayAction=NOACTION;
			trayCnt = 0;
        }
	}// else {trayStatusDescr="";}

   return 1;
}

bool CSetNotify(int pos , int * pNotify=0 , sUIAction * pAction=0 , unsigned int * msgID=0) {
    pos = Ctrl->DTgetID(DTCNT , pos);
    if (pos == -1 || !hwndMain || !Cnt.exists(pos)) return false;
    bool animate = false;
    int notify = GETCNTI(pos , Contact::colNotify);
    sUIAction action = NOACTION;
	if (!Cnt.exists(pos)) return false; // Cos tu nie gra!!!
    if (notify == NOTIFY_AUTO) {
        MessageNotify mn;
        if (Cnt[pos].user) {mn.net = 0; mn.setUid("");}
        else {
            mn.net = GETCNTI(pos , Contact::colNet);
            mn.setUid(GETCNTC(pos , Contact::colUid));
        }
        ICMessage(MessageNotify::IM::imcMessageNotify,(int)&mn);
        notify = mn.notify;
        action = mn.action;
        if (msgID) *msgID = mn.id;
    } else {action = sUIAction(GETCNTI(pos , Contact::colActParent),GETCNTI(pos , Contact::colActId));
        if (msgID) *msgID = GETCNTI(pos , Contact::colNotifyMsg);
    }
    if (Cnt[pos].notify != notify) {
        Cnt[pos].notify = notify;
        cntList.refresh(false);
        animate = true;
    } 
    if (memcmp(&Cnt[pos].action , &action , 8)) {
        Cnt[pos].action = Act.exists(action)?action:NOACTION;
        animate = true;
    }
    if (pNotify) *pNotify = notify;
    if (pAction) *pAction = action;
    return animate;
}

int CNotify(int pos) {
   if (pos == -1 || !hwndMain) return 0;
   if (pos != NOTIFY_AUTO) return CSetNotify(pos)?1:0;
   int nTrayNotify = 0;
   sUIAction nTrayAction = NOACTION;
   int nTrayMsgID = 0;
   unsigned int maxID = 0;
   int animate=0;
   int cntPos = -1;
   for (int i=0; i<Cnt.size(); i++) {
       int notify;
       sUIAction action;
       unsigned int msgID = 0;
       if (CSetNotify(i , &notify , &action , &msgID)) animate ++;
       unsigned int lastMsg = GETCNTI(i , Contact::colLastMsg);
       if (notify && lastMsg>=maxID) {
          nTrayNotify = notify;
          nTrayAction = action;
          nTrayMsgID = msgID;
          maxID = lastMsg;
          cntPos = i;
       }
   }
   cntPos = Ctrl->DTgetID(DTCNT , cntPos);
   if (memcmp(&nTrayAction , &trayAction , 8) || nTrayNotify != trayNotify || trayCnt!=cntPos) {
       trayAction = nTrayAction;
       trayNotify = nTrayNotify;
       trayMsgID = nTrayMsgID;
       trayCnt = cntPos;
       if (trayNotify) {
           string notifyStr = formatTitle((char*)GETSTR(CFG_UITRAYTITLEEVENT), trayCnt , FT_TRAYEVENT);
           changeTray(trayNotify , trayNotify?(char*)notifyStr.c_str() : 0);
       } else {       
           changeTray(trayStatus , (char*)-1);
           trayNotify = 0;
           trayCnt = 0;
       }
   }
   if (animate) Animate();

   //refreshList(findinList(&Cnt[pos]) , false);
   return 0;
}

int CNewNotify(sNOTIFY * n) {
   n->cnt = Ctrl->DTgetPos(DTCNT , n->cnt);
   if (n->cnt == -1 || n->cnt == 0) {
     IMLOG("- Nowy trayNotify = %d" , trayNotify);
     if (trayNotify && !(n->flag & NOTIFY_IMPORTANT)) return 0;
     changeTray(n->notify , n->info);
     trayNotify = n->notify;
     trayCnt = 0;
     trayAction = n->action;
     trayMsgID = 0;
   } else {
     int pos = n->cnt;
     if (pos < 0) return 0;
     if (Cnt[pos].notify && !(n->flag & NOTIFY_IMPORTANT)) return 0;
     Cnt[pos].notify = n->notify;
     Cnt[pos].action = n->action;
   }
   Animate();
   return 1;
}

int ICfgChanged() {
   IMLOG("- Zmiana w konfiguracji");
   UISet();
   cntList.refresh(true);
   return 1;
}

int ILstGetSelPos(int pos) {
   if (pos > 1000 || pos >= SendMessage(cntListWnd->_hwnd , LVM_GETSELECTEDCOUNT , 0 , 0)) return -1;
   int buff [1001];
   int r = ListView_GetSelItems(cntListWnd->_hwnd , min(1000 , pos+1) , buff);
   if (r == LB_ERR) return -1;
   sUICnt * c = (sUICnt*) ListView_GetItemData(cntListWnd->_hwnd , buff[pos]);
   if (!c) return -1;
   return c->ID;
}

int IACntDel(int cnt) {
       int buff [500];
       int c = 1;
       if (cnt<0) {
         if (!ICMessage(IMI_CONFIRM , (int)"Usun¹æ zaznaczone kontakty?")) return 0;
         c = ListView_GetSelItems(hwndList , 500 , buff);
         for (int i=c-1; i>=0; i--)
           ICMessage(IMC_CNT_REMOVE,((sUICnt*)ListView_GetItemData(hwndList , buff[i]))->ID);
         ICMessage(IMI_REFRESH_LST);
       } else {
         buff[0] = cntListWnd->find(&Cnt[cnt]);
         ICMessage(IMC_CNT_REMOVE,cnt,1);
       }
       if (c>0 && buff[0]>-1) ListView_SetCurSel(hwndList , buff[0]);
       return 1;
}

// -----------------------



//---------------------------------------------------------------------------
VOID CALLBACK APCProc(ULONG_PTR param) {
//  IMLOG("APCPROC %d [%x]" , GetCurrentThreadId() , param);
}

int __stdcall MessageBoxThreadProc(void * param) {
	return MessageBoxIndirect((MSGBOXPARAMS*)param);
}

//---------------------------------------------------------------------------


IMPARAM __stdcall IMessageProc(sIMessage_base * msgBase) {
 sIMessage_2params * msg = static_cast<sIMessage_2params*>(msgBase);

 int a , pos , r;
 sDIALOG * sd;

 char * ch;
 switch (msg->id) {
    // INITIALIZATION
    case IM_PLUG_NET:        return Net::none;
    case IM_PLUG_TYPE:       return imtAll & ~(imtMsgUI | imtNetSearch | imtNetUID);
    case IM_PLUG_VERSION:    return (int)"";
    case IM_PLUG_SDKVERSION: return KONNEKT_SDK_V;
    case IM_PLUG_SIG:        return (int)"UI";
    case IM_PLUG_CORE_V:     return (int)"W98";
    case IM_PLUG_UI_V:       return 0;
    case IM_PLUG_NAME:       return (int)"Konnekt User Interface";
    case IM_PLUG_NETNAME:    return (int)"bez sieci";
    case IM_PLUG_INIT:       Plug_Init(msg->p1,msg->p2); CtrlEx = (cCtrlEx*) msg->p1; return Init();
    case IM_PLUG_INITEX:     CtrlEx=(cCtrlEx*)msg->p1; return 1;
    case IM_PLUG_DEINIT:
      Plug_Deinit(msg->p1,msg->p2);
      deInit();
      return 1;

    case IM_PLUG_CERT:       if (!msg->p1 && !msg->p2) return (int)UI_CERT;

    case IM_SETCOLS:         return ISetCols();

    case IM_PLUG_UPDATE:
      IUpdate(msg->p1);
      return 1;

    case IM_UI_PREPARE:      return IPrepare();
    case IM_START:           return IStart();
    case IM_END:             return IEnd();

    case IMI_SET_SHOWBITS:
      ShowBits::openDialog();
      return 0;

    case IM_CNT_ADD:
      Cnt.imAdd(msg->p1);Cnt[msg->p1].hidden = false;
      cntList.refresh(true);

      return 1;

    case IM_CNT_REMOVE:      /*if (msg->p2)*/ 
                               //else ListView_SetSelectionMark(hwndList , LB_ERR);

      Cnt.imRemove(msg->p1 , !msg->p2);
      return 1;

    case IM_CNT_REMOVED:
      if (msg->p2) cntList.refresh(true);
      return 1;

//    case IM_CNT_CHANGED:   msg->p2=0;

    case IM_CNT_COMPOSING:
      if (msg->sender != Ctrl->ID()) {
        Konnekt::UI::Cnt::checkComposing.composing(msg->p1);
      }
      return 0;

    case IM_CNT_COMPOSING_STOP:
      if (msg->sender != Ctrl->ID()) {
        Konnekt::UI::Cnt::checkComposing.stopComposing(msg->p1);
      }
      return 0;

    case IMI_CNT_ISCOMPOSING:
      return Konnekt::UI::Cnt::checkComposing.isComposing(msg->p1);

    case IMI_REFRESH_CNT:
      ISRUNNING();
      IMESSAGE_TS();

      if (!cntList.running()) return 0;
      if (!Cnt.exists(msg->p1)) return 0;

      Cnt[msg->p1].imRefresh();

      if (msg->p2 && Cnt[msg->p1].hwndInfo) InfoDialogLoad(Cnt[msg->p1].hwndInfo , msg->p1);

      cntList.refresh(true);

      return 1;

    case IMI_REFRESH_LST:
      ISRUNNING();
      IMESSAGE_TS();

      if (!cntList.running()) return 0;

      Cnt.imPrepare();

      if (msg->p1) {
        cntList.refill(); 
      } else {
        cntList.refresh(true);
      }

      return 1;

    case IMI_CONFIRM:
    case IMI_ERROR:
    case IMI_WARNING:
    case IMI_INFORM: {

      if (msg->id == IMI_CONFIRM) {
        r = MB_YESNO | MB_ICONQUESTION;
      } else if (msg->id == IMI_ERROR) {
        r = MB_OK | MB_ICONHAND;
      } else if (msg->id == IMI_WARNING) {
        r = MB_OK | MB_ICONEXCLAMATION;
      } else if (msg->id == IMI_INFORM) {
        r = MB_OK | MB_ICONASTERISK;
      }

      //IMESSAGE_TS();
      canQuit = false; 
      sIMessage_msgBox msgBox(msg->id);

      bool disable = false;
      if (msgBase->s_size == sizeof(sIMessage_msgBox)) {
        msgBox = *(static_cast<sIMessage_msgBox*>(msgBase));

        if (!msgBox.title) msgBox.title = "Konnekt";

        GetWindowText((HWND)msgBox.parent , TLS().buff, 2000);
        const char * sums = TLS().buff;

//            if (msgBox.parent && !IsWindowEnabled((HWND)msgBox.parent)) {disable=true;EnableWindow((HWND)msgBox.parent , true);}
      } else {
        msgBox.msg = (const char *)msg->p1;
        msgBox.flags = msg->p2;
        msgBox.title = "Konnekt";
        msgBox.parent = hwndTop;
      }

      MSGBOXPARAMS mbp;
      memset(&mbp , 0 , sizeof(mbp));

      mbp.cbSize = sizeof(mbp);
      mbp.dwStyle = msgBox.flags ? msgBox.flags |(r & 0xF0) : r;
      mbp.hwndOwner = msgBox.flags & MB_TASKMODAL ? 0 :(HWND)msgBox.parent;
      mbp.lpszCaption = msgBox.title;
      mbp.lpszText = msgBox.msg;

      if (!(msgBox.flags & (MB_TASKMODAL | MB_APPLMODAL | MB_SYSTEMMODAL)) && !Ctrl->Is_TUS(0)) {
        IMLOG("- MessageBox own thread");
        r = Ctrl->BeginThreadAndWait("MessageBox", 0 , 0 , (cCtrl::fBeginThread)MessageBoxThreadProc , &mbp);
      } else {
        r = MessageBoxThreadProc(&mbp);
      }

      if (msg->id == IMI_CONFIRM && !msgBox.flags) r = (r == IDYES);

      if (disable) EnableWindow((HWND)msgBox.parent , false);

      canQuit = true;
      return r;
    }

    case IMI_DEBUG:
     #ifdef __DEBUG

     if (!hwndDbgAct) {
       hwndDbgAct = CreateDialogParam(Ctrl->hDll() , MAKEINTRESOURCE(IDD_DBGACTIONS), 
         hwndTop , (DLGPROC)DbgActionsProc , 0);

       IMLOG("IMI_DEBUG %x" , hwndDbgAct);
       ShowWindow(hwndDbgAct, SW_SHOW);
     } else {
       SetForegroundWindow(hwndDbgAct);
     }

     #endif // __DEBUG
     break;

    case IM_UIACTION:  return ActionProc((sUIActionNotify_base*) msg->p1);

    case IMI_DLGSETPASS:
      ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*) msg->p1;

      ((sDIALOG_access*) msg->p1)->flag |= DFLAG_PASS2 | (msg->p2 ? DFLAG_SAVE : 0);

      if (DialogBoxParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_SETPASS), 
        sd->handle ? (HWND)sd->handle : hwndTop, (DLGPROC)PassDialogProc, msg->p1) != IDOK) 
         return 0;

      return (int) 1;

    case IMI_DLGPASS:
      ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*) msg->p1;

      ((sDIALOG_access*) msg->p1)->flag |= (msg->p2 ? DFLAG_SAVE : 0);

      if (DialogBoxParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_PASS), sd->handle ? (HWND)sd->handle : hwndTop, 
        (DLGPROC)PassDialogProc, msg->p1) != IDOK) 
         return 0;

      return 1;

    case IMI_DLGLOGIN:
      ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*) msg->p1;

      ((sDIALOG_access*) msg->p1)->flag |= (msg->p2 ? DFLAG_SAVE : 0);

      if (DialogBoxParam(Ctrl->hDll(), MAKEINTRESOURCE((sd->flag & DFLAG_PASS2) ? IDD_LOGIN : IDD_LOGINPASS2), 
        sd->handle ? (HWND)sd->handle : hwndTop, (DLGPROC)PassDialogProc , msg->p1) != IDOK)
         return 0;

      return 1;

    case IMI_DLGENTER:
      ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*)msg->p1;

      if (DialogBoxParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_ENTER), 
        sd->handle ? (HWND)sd->handle : hwndTop, (DLGPROC) EnterDialogProc, msg->p1) != IDOK) 
         return 0;

      return 1;

    case IMI_DLGBUTTONS:
      ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*) msg->p1;

      return DialogBoxParam(Ctrl->hDll(), 
        MAKEINTRESOURCE(IDD_BUTTONS), sd->handle ? (HWND)sd->handle : hwndTop,
        (DLGPROC) ButtonsDialogProc, msg->p1);

    case IMI_DLGTOKEN:
      ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG_token*) msg->p1;

      return DialogBoxParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_GETTOKEN), 
        sd->handle ? (HWND) sd->handle : hwndTop, (DLGPROC)TokenDialogProc, (LPARAM)msg);

    case IMI_LONGSTART: {
      // ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*) msg->p1;
      a = 1;

      if (sd) {
        sd->handle = 0;
        if (longOnly) sd->flag |= DLONG_NODLG;

        if (!(sd->flag & DLONG_NODLG)) {
          sd->handle = CreateDialogParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_LONGPROC),
            sd->handle ? (HWND) sd->handle : hwndTop , (DLGPROC)LongDialogProc , msg->p1);

          ShowWindow((HWND) sd->handle , SW_SHOW);
        }

        StartLong((sDIALOG_long*) sd);

        if (sd->flag & DLONG_MODAL) {
          if (hwndConfig) EnableWindow(hwndConfig , false);
          EnableWindow(hwndMain , false);
        }
        if (sd->flag & DLONG_SINGLE) longOnly = true;

      } else {
        /*StartLong(0); a = 0;*/
      }

      return a;
    }

    case IMI_LONGEND:
      //ISRUNNING();
      IMESSAGE_TS();

      sd = (sDIALOG*) msg->p1;

      if (sd) {
        if (sd->flag & DLONG_SINGLE) longOnly = false;
        if (sd->handle) DestroyWindow((HWND) sd->handle);
        if (sd->flag & DLONG_MODAL) {
          if (hwndConfig) EnableWindow(hwndConfig , true);
          EnableWindow(hwndMain , true);
        }
//      if (!longOnly && (sd->flag & DLONG_BLOCKING))
        StopLong((sDIALOG_long*)sd);
      }// else if (!longOnly) StopLong();

      return 1;

    case IMI_LONGSET:
      sd = (sDIALOG*) msg->p1;

      if (sd && IsWindow((HWND) sd->handle)) {
        SendMessage((HWND) sd->handle , MYWM_DLGLONGSET , msg->p2 , msg->p1);
      }

      return 1;

    case IMI_CNT_DETAILS:
      ISRUNNING();
      IMESSAGE_TS();

      a = msg->p1;
      if (a<0) return 0;

//    IMLOG("hwnd %x" , Cnt[a].hwndInfo);

      Cnt[a].InfoWndOpen();

      return 1;

    case IMI_CNT_DETAILS_SUMMARIZE:
      if (!Cnt.exists(msg->p1)) 
        return 0;

      InfoDialogSummary(msg->p1 , true);

      return 0;

    case IMI_CNT_SEARCH_FOUND:
      if (!hwndSearch) return 0;

      SearchDialogAdd(hwndSearch , (sCNTSEARCH*) msg->p1);

      return 1;

/*
    case Message::IM::imReceiveMessage:

              ISRUNNING();      
              Message * m;
              m = (Message*)msg->p1;
              if (m->getFlags() & Message::flagHandledByUI) {
				  if (!m->getAction().id && !m->getAction().id)
            m->setAction(sUIAction( m->getType() & Message::typeMask_NotOnList ? IMIG_EVENT : IMIG_CNT , IMIA_EVENT_OPENANYMESSAGE ));
				  if (!m->getNotify())
            m->setNotify( UIIcon(IT_MESSAGE , 0 , Message::typeEvent , 0));
			  }
        if (!(m->getFlags() & Message::flagHandledByUI)) return 0;
        if (m->getFlags() & Message::flagSend) return 0;
              switch (m->getType()) {
                 case Message::typeMessage:
//                   if (m->flag & MF_SEND) {} else
                   {
                     m->setAction(sUIAction(IMIG_CNT , IMIA_CNT_MSGOPEN));
                     if (!m->getNotify())
                         m->setNotify(UIIcon(5,0,Message::typeMessage,0));
                   }
                   return Message::resultOk;
                 case Message::typeServerEvent:
                   m->setAction(sUIAction(IMIG_EVENT , IMIA_EVENT_SERVER));
                   if (!m->getNotify())
                     m->setNotify(UIIcon(5,0,Message::typeServerEvent,0));
                   return Message::resultOk;
                 case Message::typeUrl:
                   m->setAction (sUIAction(IMIG_EVENT , IMIA_EVENT_URL));
                   if (!m->getNotify()) m->setNotify(UIIcon(5,0,Message::typeUrl,0));
                   return Message::resultOk;
                 case Message::typeQuickEvent:
                   ICMessage(IMI_MSG_OPEN , msg->p1);
                   return Message::resultDelete; //quickMessage
              }
              return 0;
              */

    case IMI_CONFIG:
      ISRUNNING();
      IMESSAGE_TS();

      if (!hwndMain) return 0;

      if (!hwndConfig) {
        hwndConfig = CreateDialog(Ctrl->hDll(), MAKEINTRESOURCE(IDD_CONFIG), hwndTop,
          (DLGPROC)ConfigDialogProc);

        ShowWindow(hwndConfig, SW_SHOW);
      } else {
        SetForegroundWindow(hwndConfig);
      }

      ConfigDialogOpen(msg->p1);

      return 1;

    case IMI_CONFIGOPENED:
      ISRUNNING();

      if (!msg->p1) return (int) hwndConfig;

      return (int)GetProp(hwndConfig , "configProp")==msg->p1;

    case IMI_ACTION: case IMI_GROUP: {
      ISRUNNING();
      IMESSAGE_TS();

      sUIActionInfo * ai = (sUIActionInfo*) msg->p1;

      if (!Act.exists(ai->act.parent)) {
        CtrlEx->PlugOut(msg->sender , _sprintf("Grupa %d dla akcji %d nie istnieje!" , ai->act.parent , ai->act.id) , 1);
      }
      if (ai->act.id && Act.exists(ai->act)) {
        CtrlEx->PlugOut(msg->sender , _sprintf("Akcja %d , %d ju¿ istnieje!" , ai->act.parent , ai->act.id) , 1);
      }

      if (msg->id == IMI_GROUP)
        ai->status |= ACTS_GROUP;
      if (ai->status & ACTS_GROUP) {
        if (Act.exists(ai->act.id)) {
          CtrlEx->PlugOut(msg->sender , _sprintf("Grupa %d ju¿ istnieje!" , ai->act.id) , 1);
        }
      }
      Act.insert(ai , msg->sender);

//          if (ai->status & ACTS_GROUP)
//            Act[ai->act.id];

      return 1;
    }

    case IMI_ACTION_CALL:
      ISRUNNING();

      if (!Act.exists(((sUIActionNotify_base*)msg->p1)->act)) return 0;

      return IMessageDirect(IM_UIACTION , Act(((sUIActionNotify_base*)msg->p1)->act).owner , msg->p1);

    case IMI_ACTION_SETCNT:
      ISRUNNING();
      IMESSAGE_TS();

      if (!Act.exists(((sUIActionNotify_base*) msg->p1)->act)) return 0;

      return Act(((sUIActionNotify_base* )msg->p1)->act).setCnt(((sUIActionNotify_base*) msg->p1)->act.cnt , msg->p2!=0);

    case IMI_ACTION_GET: {
      ISRUNNING();
      IMESSAGE_TS();

      sUIActionInfo * ai = (sUIActionInfo *)msg->p1;
      if (!Act.exists(ai->act)) return 0;

      Act(ai->act).setCnt(ai->act.cnt);
      Act(ai->act).getInfo(ai);

      return 1;
    }

    case IMI_ACTION_SET: {
      ISRUNNING();
      IMESSAGE_TS();

      sUIActionInfo * ai = (sUIActionInfo *)msg->p1;

      if (!Act.exists(ai->act)) return 0;

      return ActionSet(*ai);
    }

    case IMI_ACTION_GETVALUE: {
      ISRUNNING();
      IMESSAGE_TS();

      sUIActionInfo * ai = (sUIActionInfo*) msg->p1;
      if (!Act.exists(ai->act.parent, ai->act.id)) return 0;

      Act(ai->act).setCnt(ai->act.cnt);
      String value = getActionValue(Act(ai->act) , ai->mask & UIAIM_VALUE_CONVERT);

      if (ai->txt) {
        strncpy(ai->txt, value.a_str(), ai->txtSize);
        return (int)ai->txt;

      } else {
        strncpy(TLS().buff, value.a_str(), MAX_STRING);
        return (int)TLS().buff;
      }
    }

    case IMI_ACTION_SETVALUE: {
      ISRUNNING();
      IMESSAGE_TS();

      sUIActionInfo * ai = (sUIActionInfo*) msg->p1;
      if (!Act.exists(ai->act.parent, ai->act.id)) return 0;

      Act(ai->act).setCnt(ai->act.cnt);
      String value = setActionValue(Act(ai->act), ai->txt, ai->mask & UIAIM_VALUE_CONVERT);

      strncpy(TLS().buff, value.a_str(), MAX_STRING);

      return (int)TLS().buff;
    }

    case IMI_GROUP_ACTIONSCOUNT:
      ISRUNNING();

      if (!Act.exists(((sUIAction*) msg->p1)->id)) return 0;

      return Act[((sUIAction*) msg->p1)->id].size(); 

    case IMI_ACTION_EXISTS:
      ISRUNNING();

      return Act.exists(*(sUIAction*) msg->p1);

    case IMI_ACTION_ISGROUP:
      ISRUNNING();

      return Act.exists(msg->p1);

    case IMI_ACTION_GETID:
      ISRUNNING();

      if (!Act.exists(msg->p1) || msg->p2<0 || msg->p2>=Act[msg->p1].size()) return -1;

      return Act[msg->p1][msg->p2].id;

    case IMI_ACTION_GETINDEX:
      ISRUNNING();

      if (!msg->p1 || !Act.exists(*(sUIAction*) msg->p1)) return 0;

      return Act(*(sUIAction*)msg->p1).index;

    case IMI_ACTION_REMOVE: {
      ISRUNNING();
      IMESSAGE_TS();

      sUIAction * uia = (sUIAction*) msg->p1;

      if (!Act.exists(*uia)) return 0;

      return Act[uia->parent].remove(Act[uia->parent].find(uia->id));
    }

    case IMI_ACTION_GETTYPE:
      ISRUNNING();

      if (!Act.exists(*(sUIAction*) msg->p1)) return 0;

      return Act(*((sUIAction*)msg->p1)).type;

    case IMI_GROUP_GETHANDLE:
      ISRUNNING();
      IMESSAGE_TS();

      if (!Act.exists(((sUIAction*) msg->p1)->id)) break;

      Act[((sUIAction*) msg->p1)->id].setCnt(((sUIAction*) msg->p1)->cnt);

      return (int)Act[((sUIAction*) msg->p1)->id].ghandle;

    case IMI_ACTION_FINDPARENT: {
      ISRUNNING();

      if (!Act.exists(((sUIAction*) msg->p1)->parent)) break;

      sUIAction * act = (sUIAction*) msg->p1;
      cUIAction * found = Act[act->parent].findAction(act->id);

      if (msg->p2) act->parent = found ? found->parent : 0;

      return found ? found->parent : 0;
    }

    case IMI_ACTION_GETOWNER:
      ISRUNNING();

      if (!Act.exists(*((sUIAction*) msg->p1))) {
        break; // b³¹d
      }

      return (int)Act(*(sUIAction*) msg->p1).owner;

    case IMI_ACTION_MAKEPOPUPMENU: {
      sIMessage_UIMakePopup * mp = static_cast<sIMessage_UIMakePopup *>(msgBase);

      if (!Act.exists(mp->_action.id))  break; // b³¹d

      Act[mp->_action.id].setCnt(mp->_action.cnt , true);
      mp->_uFlags |= TPM_RETURNCMD;

      if (!mp->_hWnd) mp->_hWnd = hwndMain;

      UIPopupMenu(Act[mp->_action.id] , mp->_uFlags , mp->_x , mp->_y , mp->_nReserved, 
        (HWND) mp->_hWnd, (CONST RECT*) mp->_prcRect , mp->_startFrom);

      return 1;
    }

    case IMI_GETPLUGINSGROUP: {
      return GETINT(CFG_UIPLUGINSGROUP) & 0xFFFF;
    }

    case IMI_GROUP_MAKECFG: {
      ISRUNNING();
      IMESSAGE_TS();

      if (!Act.exists(((sUIAction*) msg->p1)->id)) return 0;

      cUIGroup & g = Act[((sUIAction*)msg->p1)->id];

      if (!getCfgHandle(g)) return 0;
      if (!g.ghandle) makeCfgProp(g);

      return (int)g.ghandle;
    }

    case IMI_ICONRES:
      ISRUNNING();

      return Ico.iconRes(msg->p1,msg->p2 , (HINSTANCE)ICMessage(IMC_PLUGID_HANDLE , msg->sender));

    case IMI_ICONREGISTER:
      ISRUNNING();

      if (Ico.iconRegister((sUIIconRegister*)msg->p1 , (HINSTANCE)ICMessage(IMC_PLUGID_HANDLE , msg->sender))) {
        return ((sUIIconRegister*)msg->p1)->ID;
      }

      return 0;

    case IMI_ICONREGISTERLIST:
      ISRUNNING();

      return Ico.iconRegisterList((sUIIconRegisterList*)msg->p1);

    case IMI_ICONEXISTS:
      ISRUNNING();

      return Ico.iconExists(msg->p1 , (IML_enum)msg->p2);

    case IMI_GETICONINDEX:
      ISRUNNING();

      return Ico.getIconIndex(msg->p1 , (IML_enum)msg->p2);

    case IMI_GETICONLIST:
      ISRUNNING();

      return (int)Ico.getImageList((IML_enum)msg->p1);

    case IMI_ICONUNREGISTER:
      ISRUNNING();

      return Ico.iconUnregister(msg->p1 , (IML_enum)msg->p2);

    case IMI_ICONGET:
      ISRUNNING();

      return (int)Ico.iconGet(msg->p1 , (IML_enum)msg->p2);

    /*
    case Message::IM::imOpenMessage:
      ISRUNNING();

      m = (Message*)msg->p1;

     switch (m->getType()) {
       case Message::typeMessage:
       case Message::typeQuickEvent:
         return ICMessage(IMI_MSG_OPEN , msg->p1);
       case Message::typeServerEvent:
         ServerEventDialogNext();
         return 0;
       case Message::typeUrl:
         return 0;
     }

     return 0;
    */

    case IMI_MSG_OPEN:
      ISRUNNING();
      IMESSAGE_TS();

      return IMsgOpen((Message*)msg->p1);

    case IMI_MSG_NOTINLIST: {
      ISRUNNING();
      IMESSAGE_TS();

      Message *m = (Message*)msg->p1;

      if (m->getType() != Message::typeMessage) return 0;

      if (GETINT(CFG_UIMSGFROMOTHER) 
        || (*GETSTR(CFG_UIMSGFROMOTHERPASS) && !strcmp(GETSTR(CFG_UIMSGFROMOTHERPASS) , m->getBody().a_str()))
         ) {
           pos = ICMessage(IMC_CNT_ADD , (int)m->getNet() , (int)m->getFromUid().a_str());
           SETCNTI(pos , Contact::colStatus , ST_NOTINLIST , CNTM_FLAG);
           ICMessage(IMC_CNT_CHANGED , pos);
           return GETINT(CFG_UIMSGFROMOTHER)!=0;
         }
         if (m->getType()==Message::typeMessage && *GETSTR(CFG_UIMSGFROMOTHERREPLY) && !(m->getFlags() & Message::flagAutomated)) {
           // Odsyla wiadomosc
           Message msg;
           msg.setNet(m->getNet());
           msg.setType(Message::typeMessage);
           msg.setToUid(m->getFromUid());
           msg.setBody(GETSTR(CFG_UIMSGFROMOTHERREPLY));
           msg.setTime(Time64(true));
           msg.setFlags(Message::flagSend | Message::flagAutomated);

           MessageSelect ms;
           ms.id = ICMessage(Message::IM::imcNewMessage, (int)&msg);
           if (ms.id) ICMessage(MessageSelect::IM::imcMessageQueue, (int)&ms);
         }
       }
       return 0;

    case IMI_MSG_WINDOWSTATE:
      if (!Cnt.exists(msg->p1)) return 0;
      if (!Cnt[msg->p1].hwndMsg || !IsWindowVisible(Cnt[msg->p1].hwndMsg)) return 0;
      if (IsIconic(Cnt[msg->p1].hwndMsg)) return -2;

      return (GetForegroundWindow() != Cnt[msg->p1].hwndMsg) ? -1 : 1;

    case IMI_MSG_EDITCTRL_WNDPROC:
      return (int) EditMsgControlProc;

    case IM_STATUSCHANGE: {
      ISRUNNING();

      sIMessage_StatusChange sc (msgBase);

      return CStatus(sc.plugID , sc.status!=-1?sc.status:IMessageDirect(IM_GET_STATUS , sc.plugID), 
        sc.info?sc.info:(const char *)IMessageDirect(IM_GET_STATUSINFO , sc.plugID));
    }

    case IMI_NOTIFY:
      ISRUNNING();

      return CNotify(msg->p1);

    case IMI_NEWNOTIFY:
      ISRUNNING();

      return CNewNotify((sNOTIFY*)msg->p1);

    case IM_CFG_CHANGED:
      ISRUNNING();

      ICfgChanged();

      return 1;

    case IM_GRP_CHANGED:
      ISRUNNING();

      fillGroups();

/*         ch = (char *) GETSTR(CFG_CURGROUP);
         if (*ch && !ICMessage(IMC_GRP_FIND , (int)ch)) {
            SETSTR(CFG_CURGROUP , "");
            cntList.refill();
         }*/

      onSizeMain(0,0);
      selectGroups();
      Cnt.imPrepare();

      return 1;

    case IM_IGN_CHANGED:
      ICMessage(IMI_REFRESH_LST);

      return 1;

    case IMI_LST_SELCOUNT:
      ISRUNNING();

      return SendMessage(hwndList , LVM_GETSELECTEDCOUNT , 0 , 0);

    case IMI_LST_GETSELPOS:
      ISRUNNING();

      return ILstGetSelPos(msg->p1);

    case IMI_CNT_ACTIVITY:
      ISRUNNING();

      if (!Cnt.exists(msg->p1)) return 0;

      Ctrl->DTsetInt64(DTCNT , msg->p1 , Contact::colLastActivity , _time64(0));
//    Cnt[msg->p1].activity=time(0);
      Cnt[msg->p1].active = true;
      Cnt[msg->p1].ApplyFilters();

      return 1;

    case IMI_CNT_DEACTIVATE:
      ISRUNNING();

      if (!Cnt.exists(msg->p1)) return 0;

      Cnt[msg->p1].active = false;
//    Cnt[msg->p1].activity=0;
      Cnt[msg->p1].ApplyFilters();

      return 1;

    case IM_CNT_STATUSCHANGE:
         {
           ISRUNNING();
           if (msgBase->s_size < sizeof(sIMessage_StatusChange)) throw  KException_IM("", msgBase);
           sIMessage_StatusChange * sc = static_cast<sIMessage_StatusChange*>(msgBase);
		   if (!Cnt.exists(msg->p1)) return 0;
           if (sc->status != -1) {
/*                if (msg->p2==ST_OFFLINE)
                    Cnt[msg->p1].active=true;
                else Cnt[msg->p1].active=false;*/
                if (GETINT(CFG_UICNTONLINEONLIST)) {
                    Cnt[msg->p1].ex &= ~CNT_EX_NEW;
                    Cnt[msg->p1].ex |= 1;
                    //Cnt[msg->p1].notify = -1;
                    Animate();
        //              ICMessage(IMI_NEWNOTIFY , (int)&sNOTIFY(NOACTION , -1 , msg->p1));
                }
                if (GETINT(CFG_UICNTONLINENOTIFY)
                        && (GETCNTI(msg->p1 , Contact::colStatus)&CNTM_STATUS) == ST_OFFLINE
                        && (msg->p2&CNTM_STATUS) != ST_OFFLINE) {
                          ICMessage(IMI_NEWNOTIFY , (int)&sNOTIFY(NOACTION , IDI_NEW_USER , (char*)_sprintf("%s jest dostêpn%s!",GETCNTC(msg->p1 , Contact::colDisplay),GETCNTI(msg->p1,Contact::colGender) == Contact::genderFemale?"a":"y")));
                }
           }
           Cnt[sc->cntID].setStatus(sc->status , sc->info?sc->info:GETCNTC(sc->cntID , Contact::colStatusInfo));
           return 0;
         }
    case IM_NEW_PROFILE:
        {
			if (ICMessage(IMI_INFORM , (int)"Twój profil zosta³ utworzony.\n\nPozostaje tylko ustawiæ program i go u¿ywaæ :) ...\r\n\r\nCzy chcesz zobaczyæ jak krok po kroku skonfigurowaæ Konnekta?" , MB_APPLMODAL | MB_YESNO) == IDYES) {
                ShellExecute(0 , "open" , DOCS_DIR DOCS_STEPBYSTEP , "" , "" , SW_SHOW);
            }
            //IMessageDirect(IMI_CNT_DETAILS,0,0);
            ICMessage(IMI_CONFIG);
            /*do {
              CtrlEx->WMProcess();
              SleepEx(50,1);
            } while (ICMessage(IMI_CONFIGOPENED , IMIG_CFG_CONNECTION) || ICMessage(IMI_CONFIGOPENED , IMIG_CFG_CONNECTION_PROXY));
			*/

        }
    case IM_CANTQUIT:
      return !canQuit;


	case IM_PLUG_ARGS: {
		/*
		Obs³ugujemy -cnt=sieæ/uid?wiadomoœæ  -msg=wiadomoœæ

		*/
		sIMessage_plugArgs * pa = static_cast<sIMessage_plugArgs*>(msgBase);
		if (pa->argc < 2) return 0;
		RegEx preg;
		if (preg.match("#^(\\d+|\\w*)(?:/|:)(.+?)(?:\\?([^?]+))?$#i" , getArgV(pa->argv+1 , pa->argc-1 , "-cnt" , true , "")) <= 1) return 0;
		CStdString net = preg.getSub(1);
		CStdString uid = preg.getSub(2);
		CStdString msg = preg.getResult() > 3 ? preg.getSub(3) : getArgV(pa->argv+1 , pa->argc-1 , "-msg" , true , "");
		if (uid.empty()) return 0;
		Net::tNet netID;
		if (net.empty() || atoi(net)>0) {
			netID = (Net::tNet) atoi(net);
			if (!*safeChar(IMessage(IM_PLUG_NETNAME , netID , imtNetUID)))
				netID = Net::broadcast;
		} else { // szukamy sieci
			int c = ICMessage(IMC_PLUG_COUNT);
			netID = Net::broadcast;
			for (int i=1; i < c; i++) {
//				int plugID = ICMessage(IMC_PLUGID_POS , i);
				oPlugin plugin = Ctrl->getPlugin((tPluginId)i);
				if (plugin->getType() & imtNetUID
					&& (plugin->getNetName() == net) || plugin->getNetShortName() == net ||  plugin->getUIDName() == net ) {
						netID = plugin->getNet();
					}
			}
		}
		if (netID == Net::broadcast) return 0;
		// szukamy identyfikatora sieci...
		int cntID = ICMessage(IMC_CNT_FIND , netID , (int)uid.c_str());
		if (cntID == -1 && ICMessage(IMI_CONFIRM , (int)("Czy na pewno chcesz dodaæ kontakt sieci " + string(safeChar(IMessage(IM_PLUG_NETNAME , netID , imtNetUID))) + " " + string(safeChar(IMessage(IM_PLUG_UIDNAME , netID , imtNetUID))) + ":"  + string(uid) + "?").c_str())) {
			cntID = ICMessage(IMC_CNT_ADD , netID , (int)uid.c_str());
			SETCNTI(cntID , Contact::colStatus , ST_NOTINLIST , ST_NOTINLIST);
			ICMessage(IMC_CNT_CHANGED , cntID);
			// symulujemy "wiêcej"
			ICMessage(IMI_ACTION_CALL , (int)&sUIActionNotify_2params(sUIAction(IMIG_CNT , IMIA_CNT_INFO , cntID) , ACTN_ACTION , 0 , 0));
		}
		if (cntID == -1) return 0;
		// symulujemy "wyœlij wiadomoœæ"
		ICMessage(IMI_ACTION_CALL , (int)&sUIActionNotify_2params(sUIAction(IMIG_CNT , IMIA_CNT_MSG , cntID) , ACTN_ACTION , 0 , 0));
		if (!msg.empty()) {
			Message m;
			m.setBody( msg.c_str());
			Konnekt::UI::Notify::_insertMsg ni(&m , 0 , true);
			ni.act = sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend , cntID);
			ICMessage(IMI_ACTION_CALL , (int)&ni);
		}
		return 0;
		}
 }
 if (Ctrl) Ctrl->setError(errorNoResult);
 return 0;
}


