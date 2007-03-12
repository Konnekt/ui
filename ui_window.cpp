/*
  Konnekt UI

  Obs³uga listy kontaktów.
*/


#include "stdafx.h"
#pragma hdrstop
#include "ui_main.h"

#include "include\win_listview.h"
#include "include\win_registry.h"

#include "ui_actions.h"
#include "ui_cntlist.h"
#include "ui_window.h"
#include "ui_ui.h"


int registerClasses() {
   INITCOMMONCONTROLSEX icex;
   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC  = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES | ICC_DATE_CLASSES | ICC_WIN95_CLASSES;
   InitCommonControlsEx(&icex);

   OleInitialize(0);

  // Rejestrowanie klasy glownego okna
     WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style		= 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon		= LoadIconEx(Ctrl->hDll(), "app",32);
//	wcex.hIconSm		= LoadIconEx(hDll, MAKEINTRESOURCE(IDI_WND_MAIN),16);
//        wcex.hIcon = 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszMenuName	= "";
	wcex.lpszClassName	= "UImain";
//	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
        wcex.hIconSm = 0 ;
	RegisterClassEx(&wcex);
   // klada UItop
	wcex.lpfnWndProc	= (WNDPROC)TopProc;
    CStdString uiTopClass = "UItop." + CStdString((char*)ICMessage(IMC_SESSION_ID));
	wcex.lpszClassName	= uiTopClass;
	RegisterClassEx(&wcex);

   // Klasa ListTip
        wcex.lpfnWndProc        = (WNDPROC)ListTipClassProc;
        wcex.lpszClassName      = "LISTTIP";
#define CS_DROPSHADOW 0x00020000
        wcex.style              = isComctl(6 , 0) * CS_DROPSHADOW;
        wcex.hIcon              = 0;
        wcex.hIconSm            = 0;
	RegisterClassEx(&wcex);
   // Klasa dokowania
        wcex.lpfnWndProc        = (WNDPROC)DockClassProc;
        wcex.lpszClassName      = "UIdock";
	RegisterClassEx(&wcex);
   // Klasa DIALOG_buttons
   // Klasa dla Config'a
        wcex.lpfnWndProc        = (WNDPROC)PropClassProc;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName      = "PROP";
		wcex.style = 0 /*CS_PARENTDC*/;
	RegisterClassEx(&wcex);
        return 1;

}
#define WS_EX_COMPOSITED        0x02000000L
void destroyWindows(){
      //DestroyWindow(hwndTop);
      if (hwndConfig) DestroyWindow(hwndConfig);
      if (hwndSearch) DestroyWindow(hwndSearch);
      if (hwndMsgEvent) DestroyWindow(hwndMsgEvent);
//      DestroyWindow(hwndMain);
      // Trzeba wywalic okno toolbara, zeby zostaly zapisane ustawienia
      DestroyWindow((HWND)Act[IMIG_BAR].ghandle);
      hwndTop = hwndMain = hwndUp = hwndList = hwndDown =
      hwndBar = hwndDock = hwndConfig = hwndConfigTree = hwndSearch =
      hwndMsgEvent = 0;
      delete cntTip;
      cntTip = 0;
      delete cntListWnd;
      cntListWnd = 0;

}

int createWindows() {

//   test();


   RECT rc;
//   GetClientRect(GetDesktopWindow() , &rc);
   SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
   //   hwndParent = 0;
   hwndTop = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW , "UItop." + CStdString((char*)ICMessage(IMC_SESSION_ID)), "Konnekt" /*("KONNEKT." + CStdString((char*)ICMessage(IMC_SESSION_ID)))*/,WS_VISIBLE | WS_POPUP/* | WS_SYSMENU*/ , -10, -10, 5, 5, NULL, NULL, hInst, NULL);
   ShowWindow(hwndTop , SW_SHOWNOACTIVATE);
   bool userPos = GETINT(CFG_UIPOS_W)>0;

   RECT nr = Rect(userPos?GETINT(CFG_UIPOS_X):rc.right-150,
      userPos?GETINT(CFG_UIPOS_Y):rc.bottom-400,
      userPos?GETINT(CFG_UIPOS_W):240,
      userPos?GETINT(CFG_UIPOS_H):410
      ,true
   );
   snapIt(&nr);
   hwndMain = CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW /*| WS_EX_COMPOSITED*///| WS_EX_TOPMOST //| 
      ,"UImain", "",
      WS_OVERLAPPED|WS_SYSMENU|WS_CAPTION|
      WS_THICKFRAME | WS_CLIPCHILDREN
      ,
      nr.left,nr.top,nr.right-nr.left,nr.bottom - nr.top,
      hwndTop, NULL, hInst, NULL);

//   IMLOG("W H %d %d" , GETINT(CFG_UIPOS_W) , GETINT(CFG_UIPOS_H));

   GetClientRect(hwndMain , &rc);

// Upper Group
   hwndUp = CreateWindowEx(0 , "STATIC", "", /*WS_VISIBLE |*/ WS_CHILD | SS_BLACKFRAME ,
      0, 0, rc.right, 20 , hwndMain, NULL, hInst, NULL);
//   SetWindowLong(hwndUp,GWL_WNDPROC, (LONG) WndProc);
// List
   cntListWnd = new cCntListWindow(&cntList , 0 , 22 , rc.right , rc.bottom-55 , hwndMain , (HMENU)IDC_LIST); 
   cntTip = new cCntListTip(hwndMain , GETINT(CFG_UICNTTIPDOCKED)&&GETINT(CFG_UIINFOTIP));
   cntListWnd->tip = cntTip;

   hwndList = cntListWnd->_hwnd;


//    ListView_SetToolTips(hwndList , hwndListTip);    

//  GroupsTab
   hwndGroups = CreateWindowEx(0,WC_TABCONTROL,"",
   WS_VISIBLE | WS_CHILD |
   TCS_HOTTRACK 
   | (GETINT(CFG_UIGRPTABS_VERTICAL)?TCS_VERTICAL:0)
   , 0 , 22 , 10 , 20 , hwndMain , (HMENU)IDC_TAB , hInst , NULL );
   SendMessage(hwndGroups , WM_SETFONT , (WPARAM)fontSmall , 1);

// LowerGroup
//   hwndDown = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_BLACKFRAME,
//      0, rc.bottom-30, rc.right, 30, hwndMain, NULL, hInst, NULL);
//   SetWindowLong(hwndDown,GWL_WNDPROC, (LONG) WndProc);

//   hwndBStatus = CreateWindow("BUTTON", "Status", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
//      0, rc.bottom-30, rc.right, 30, hwndMain, (HMENU)IDB_STATUS, hInst, NULL);

   tooltip.create(0, hInst);

   return 1;
}


int startWindows() {
    onSizeMain(0,0);
    if (autostart) 
        dockHide(!GETINT(CFG_UIHIDDENAUTOSTART));
    else 
        dockHide(!GETINT(CFG_UIHIDDENSTART));
   return 1;
}

/*
  Format:
  {...} - pomiedzy {} znajduje sie wartosc.
  {[...} zamknie wynik w [] (dozwolone równie¿ s¹ '(' '{' ' ' '"' ''')

  {profile} - nazwa profilu
  {version} - wersja konnekta
  {UID}  - UID kontaktu
  {status}  - status kontaktu
  {info} - opis statusu
  {display} - "wyœwietl jako" kontaktu
  {group} - grupa kontaktu
  {IP}
  Dla tray'a
  {eventDisplay} - kontakt dowi¹zany do zdarzenia
  {eventTitle} - nazwa zdarzenia

  %
*/

map <string , string> formatMap;

// -----------------------------------------
void createTray(int ico) {
   NOTIFYICONDATA tnid;
   memset(&tnid , 0 , sizeof(NOTIFYICONDATA));
   tnid.cbSize = 88;//sizeof(NOTIFYICONDATA);
   tnid.hWnd = hwndMain;
   tnid.uID = ID_TRAY;
   tnid.uFlags = NIF_MESSAGE | NIF_ICON/* | NIF_TIP*/;
   tnid.uCallbackMessage = MYWM_NOTIFYICON;
   tnid.hIcon = Ico[ico].ico_? Ico[ico].ico_ : Ico[ico].getIcon();
/*   if (GETINT(CFG_UITRAYTOOLTIP) == 0) {
	   strcpy(tnid.szTip , "Konnekt");
	   tnid.uFlags |= NIF_TIP;
   }*/
   int log = 0;
   while (!Shell_NotifyIcon(NIM_ADD, &tnid)) {
     IMLOG("! TrayIcon NOT created");
	 log++;
	 SleepEx(250 , true);
	 if (log > 120) {ICMessage(IMI_ERROR , (int)"Ikonka w tray'u nie mog³a zostaæ utworzona!");  break;}
   }
   tooltip.addTarget(hwndTop, ToolTipX::TargetPtr(new TargetActionTaskbar(ID_TRAY)));
}


void changeTray(int ico , char * txt) {
	if (!Ctrl->isRunning()) return;
   NOTIFYICONDATA tnid;
   memset(&tnid , 0 , sizeof(NOTIFYICONDATA));
   tnid.cbSize = 88;//sizeof(NOTIFYICONDATA);
   tnid.hWnd = hwndMain;
   tnid.uID = ID_TRAY;
   tnid.uFlags = NIF_ICON;
   if (txt == (char*)-1) {
	   trayInfo = formatTitle((char*)GETSTR(CFG_UITRAYTITLE), 0 , FT_TRAY);
   } else if (txt) {
	   trayInfo = SAFECHAR(txt);
   }


/*   if (GETINT(CFG_UITRAYTOOLTIP) == 0 && txt) {
	   tnid.uFlags |= NIF_TIP;
	   strncpy(tnid.szTip , trayInfo , 127);
	   tnid.szTip[63]=0;
   }*/
   
   tnid.hIcon = Ico[ico].ico_? Ico[ico].ico_ : Ico[ico].getIcon();
   int log = 0;
   while (!Shell_NotifyIcon(NIM_MODIFY, &tnid)) {
     IMLOG("! TrayIcon modify error");
	 log++;
	 SleepEx(500 , true);
	 if (log > 10) {break;}
   }

}

void destroyTray() {
   NOTIFYICONDATA tnid;
   tnid.cbSize = sizeof(NOTIFYICONDATA);
   tnid.hWnd = hwndMain;
   tnid.uID = ID_TRAY;
   Shell_NotifyIcon(NIM_DELETE, &tnid);

}

// ----------------------------------------
int getAnim(int id) {
//  IMLOG("GETANIM %x" , id);
  if (!id) return 0;
  if (ISICON_BLANK(id)) {id&=~IT_BLANK_MASK;} // Wylaczamy blanka
  else {
    if (!Ico.find(id+1)) { // Nie ma nastepnej 'klatki'
        if ((id & UIIcon(0,0,0,-1)) == 0) { // to jest klatka zerowa
            id|=IT_BLANK_MASK; 
        }
        else { 
            id&=UIIcon(-1,-1,-1,0); // ustawiamy 0 klatke
        }
    } else id++;
  }
//  IMLOG("GOTANIM %x" , id);
  return id;
}

//VOID CALLBACK timerAnimProc(void * lpArg,DWORD dwLow,DWORD dwHigh) {
VOID CALLBACK timerAnimProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime) {
	bool anim = false;
	// status
	for (int i=0; i<Act[IMIG_STATUS].size();i++) {
		//   IMLOG("____STATUS %x , %x == %x" ,Act[IMIG_STATUS][i].icoId , Act[IMIG_STATUS][i].icoId & (UIIcon(-1,0,-1,0)), UIIcon(4,0,ST_CONNECTING,0));
		if ((Act[IMIG_STATUS][i].p1 & (UIIcon(0x7,0,-1,0))) == UIIcon(4,0,ST_CONNECTING,0))
		{
			ActionStatus(Act[IMIG_STATUS][i].act(),-1,0,getAnim(Act[IMIG_STATUS][i].p1));
			anim = true;
		}
	}
	// msgButton

	static int lastMsgNotify = 0;
	if (Cnt[0].notify || lastMsgNotify != Cnt[0].notify) {
		if (!Cnt[0].notify) {
			ActionStatus(Act[IMIG_EVENT_PARENT](IMIG_EVENT).act(),ACTS_HIDDEN,0, IDI_TB_MAIN);
		} else {
			if (lastMsgNotify != Cnt[0].notify)
				ActionStatus(Act[IMIG_EVENT_PARENT](IMIG_EVENT).act(),0,0,Cnt[0].notify);
			else
				ActionStatus(Act[IMIG_EVENT_PARENT](IMIG_EVENT).act(),-1,0,getAnim(Act[IMIG_EVENT_PARENT](IMIG_EVENT).p1));
		}
		anim = true;

	}
	lastMsgNotify = Cnt[0].notify;

	// IMLOG("anim");
	// Tray
	static int lastTray = 0;
	static int loopTray = 0;
	if ((trayNotify & UIIcon(-1,-1,-1,0)) != (lastTray & UIIcon(-1,-1,-1,0)) || !trayNotify) loopTray=0;
	else loopTray ++;
	//IMLOG("ANIM  %x != %x  Loop = %d" , trayNotify & UIIcon(-1,-1,-1,0) , lastTray & UIIcon(-1,-1,-1,0) , loopTray);
	if (!trayNotify) {
		if ((trayStatus & (UIIcon(0x7,0,-1,0))) == UIIcon(4,0,ST_CONNECTING,0))
		{
			anim = true;
			trayStatus = getAnim(trayStatus);
			changeTray(trayStatus , (char*)-1);
		}
	} else {
		if ((((trayNotify /*& UIIcon(0,0,-1,0)*/)) == IDI_NEW_USER) && loopTray > (4000 / GETINT(CFG_UIANIM)))
		{ //IMLOG("ANIMLOOP_FINISH!");
			trayNotify = lastTray = loopTray = 0;
			trayAction = NOACTION;
			changeTray(trayStatus , (char*)-1);
			//           ICMessage(IMI_NOTIFY);
		}
		else
		{
			anim = true;
			trayNotify = getAnim(trayNotify);
			changeTray(trayNotify);
		}

	}
	lastTray = trayNotify;
	// kontakty
	if (GETINT(CFG_UIANIMCNT)) {
		bool redraw = false;
		for (int i =1 ; i<Cnt.size(); i++) {
			if (!(Cnt[i].ex & CNT_EX_NEW) && !Cnt[i].notify) continue;
			//       redraw = true;
			sUICnt * test = &Cnt[i]; 
			int a = cntListWnd->find(&Cnt[i]);
			anim = true;
			if (Cnt[i].notify) {
				Cnt[i].notify = getAnim(Cnt[i].notify);
			}

			if (Cnt[i].ex & CNT_EX_NEW) {
				Cnt[i].ex++;
				if ((Cnt[i].ex & CNT_EX_NEW) > CNT_NEW_TIME) {
					Cnt[i].ex &= ~CNT_EX_NEW;
					ICMessage(IMI_NOTIFY , i);
					anim = true;
				} else if (!anim && ((Cnt[i].ex & CNT_EX_NEW)>2))
					a=-1; 
			}

			if (a<0) continue;
			ListView_RedrawItems(hwndList , a , a);
		}
		//   if (redraw)
		//     RedrawWindow(hwndList,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE);
	}
	if (!anim) {
		//CancelWaitableTimer(timerAnim);
		KillTimer(hwnd , idEvent); timerAnim = 0;

		timerAnimOn=false;
	}

}


void Animate() {
 LARGE_INTEGER    lDueTime;
 lDueTime.QuadPart = 0;
 int interval = GETINT(CFG_UIANIM);
 if (!timerAnim && interval)
   timerAnim = (void*)SetTimer(hwndMain , 2 , max(interval , 10) , (TIMERPROC)timerAnimProc);
// SetWaitableTimer(timerAnim , &lDueTime , ANIM_INTERVAL ,timerAnimProc, 0 , 0);
 timerAnimOn=true;
}
//-------------------------------------
VOID CALLBACK timerHideProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime) {
   if (docked==2 || !IsWindowVisible(hwndMain) || !(docked == 1 || GETINT(CFG_UIAUTOHIDE))) {
     KillTimer(hwnd , idEvent);
     timerHide = 0;
   }

   bool hide = false;
   static int hideCount = 0;
   hideCount ++;
   //IMLOG("HIDE %x - %x , %x" , hwndMain , GetActiveWindow() , GetForegroundWindow());
/*   if (!GetActiveWindow()
       || (GetActiveWindow() != GetForegroundWindow())) hide = true;
       */
   if (GetActiveWindow() != hwndMain/* || docked==1*/) hide=true;
   RECT rc;
   POINT pt;
   GetWindowRect(hwndMain , &rc);GetCursorPos(&pt);
   if (docked==1) {rc.left-=GETINT(CFG_UIDOCKSPACE); rc.right+=GETINT(CFG_UIDOCKSPACE);
      rc.top-=1000;rc.bottom+=1000;
   }
   if (hide && (PtInRect(&rc , pt) || GetCapture())) hide = false;
   if (hide && ((docked==1 && hideCount>=1)||(hideCount >= GETINT(CFG_UIAUTOHIDE)))) {
     dockHide(false);
     KillTimer(hwnd , idEvent);
     timerHide = 0;
     hideCount = 0;
   }
   if (!hide) hideCount=0;
}

VOID CALLBACK timerAwayProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
VOID CALLBACK timerAwayProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime) {
   POINT pt;
   static int time = 0;
   Cnt.checkActivity();
   GetCursorPos(&pt);
   int input = GetLastInputTime();

   typedef DWORD  (__stdcall * GETACTIVECONSOLESESSIONID)();
   static GETACTIVECONSOLESESSIONID __WTSGetActiveConsoleSessionId = (GETACTIVECONSOLESESSIONID)GetProcAddress(GetModuleHandle("kernel32.dll") , "WTSGetActiveConsoleSessionId");
   typedef BOOL (__stdcall * PROCESSIDTOSESSIONID)(DWORD,DWORD*);
   static PROCESSIDTOSESSIONID __ProcessIdToSessionId = (PROCESSIDTOSESSIONID)GetProcAddress(GetModuleHandle("kernel32.dll") , "ProcessIdToSessionId");

   bool ownSession = true; // czy jesteœmy w dobrej sesji
   if (__WTSGetActiveConsoleSessionId && __ProcessIdToSessionId) {
	   DWORD sessID;
	   __ProcessIdToSessionId(GetCurrentProcessId() , &sessID);
	   ownSession = (sessID == __WTSGetActiveConsoleSessionId());
   }

   int UIdata [4] = {
          MAKELONG(pt.x , pt.y),
          /*(int)GetForegroundWindow()*/0,
          /*(int)GetTopWindow(GetDesktopWindow())*/0,
          input
          };
   if (ownSession && memcmp(lastUIdata , UIdata , sizeof(lastUIdata))) {
     time = 0;
     if (away) {
       IMessage(IM_BACK , NET_BC , IMT_ALL);
       SETINT(CFG_ISAUTOAWAY , 0);
       away = false;
       IMLOG("AUTO-BACK");
     }
   } else {
     if (!away) time += AWAY_INTERVAL;
     if (!away && (!ownSession || (time >= GETINT(CFG_AUTOAWAY)*1000)))
        {
          IMessage(IM_AWAY , NET_BC , IMT_ALL);
          SETINT(CFG_ISAUTOAWAY , 1);
          away = true;
          IMLOG("AUTO-AWAY ownSession=%d" , ownSession);
        }
   }

//   IMLOG("Away %d" , time);

   memcpy(lastUIdata , UIdata , sizeof(lastUIdata));
}

void AutoHide() {
 if (!timerHide && (GETINT(CFG_UIAUTOHIDE) || docked==1))
   timerHide = SetTimer(hwndMain , 1 , HIDE_INTERVAL , (TIMERPROC)timerHideProc);
}

void AutoAway() {
 if (GETINT(CFG_AUTOAWAY)) {
   if (!timerAway)
     timerAway = SetTimer(0 , 0 , AWAY_INTERVAL , (TIMERPROC)timerAwayProc);
 } else if (timerAway) KillTimer(0 , timerAway);
}

//-------------------------------------

void onSizeMain(int w , int h) {
  if (!hwndMain) return;
  HDWP hDwp;
  RECT rc;
  if (!w && !h) {
    GetClientRect(hwndMain , &rc);
    w = rc.right;
    h = rc.bottom;
  }
  SendMessage(hwndList , WM_SETREDRAW , 0,0);
  int bh = SendMessage(hwndBar , RB_GETBARHEIGHT , 0 , 0);
  WINDOWINFO wi;wi.cbSize=sizeof(WINDOWINFO);
  hDwp=BeginDeferWindowPos(4);
  hDwp=DeferWindowPos(hDwp , hwndUp ,0, 0,0,w,20,SWP_NOZORDER);
  GetWindowInfo(hwndUp , &wi);
  int y=(wi.dwStyle & WS_VISIBLE)? 22 : 0;
  SetRect(&rc, 2, y+2,
     w - 1, h - y - bh );
  hDwp=DeferWindowPos(hDwp , hwndGroups ,0, rc.left,rc.top,rc.right-rc.left,rc.bottom - rc.top,SWP_NOZORDER);
  TabCtrl_AdjustRect(hwndGroups, FALSE, &rc);
  hDwp=DeferWindowPos(hDwp , hwndList ,0, rc.left,rc.top,rc.right-rc.left,rc.bottom - rc.top,SWP_NOZORDER);
  hDwp=DeferWindowPos(hDwp , hwndBar ,0, 0,h-bh,w,bh,SWP_NOZORDER);
//  SendMessage(hwndBar , WM_SIZE , 0 , 0);
  EndDeferWindowPos(hDwp);
//  ListView_SetColumnWidth(hwndList , 0 , w);
  ListView_SetColumnWidth(hwndList , 0 , LVSCW_AUTOSIZE_USEHEADER);
  SendMessage(hwndList , WM_SETREDRAW , 1,0);
  if (hwndMain && cntTip->docked) cntTip->refresh();

}



//-------------------------------------

void dockDesktop(bool show , bool repos) {
  static int Registered = false;
  if (!Registered && docked != 2) return;
  RECT rc;
//  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
  GetWindowRect(GetDesktopWindow() , &rc);
  APPBARDATA abd;
  abd.cbSize = sizeof(abd);
  abd.hWnd = hwndMain;
  if (repos && Registered) {
      GetWindowRect(hwndMain , &abd.rc);
      abd.rc.top = rc.top;
      abd.rc.bottom = rc.bottom;
      if (docked_right)
         abd.rc.right = rc.right;
        else
         abd.rc.left = rc.left;
      abd.uEdge = docked_right?ABE_RIGHT:ABE_LEFT;
      SHAppBarMessage(ABM_QUERYPOS, &abd);
      if (abd.rc.right - abd.rc.left < MIN_WIDTH)
        if (docked_right) abd.rc.left -= MIN_WIDTH - abd.rc.right + abd.rc.left;
                    else  abd.rc.right += MIN_WIDTH - abd.rc.right + abd.rc.left;
      SHAppBarMessage(ABM_SETPOS, &abd);
      docked_rect = abd.rc;
//      if (!EqualRect(&abd.rc , &rc))
//        SetWindowRect(hwndMain , abd.rc);
      return;
  }
  if (show) {
    if (!Registered) {
      abd.uCallbackMessage = MYWM_NOTIFYAPPBAR;
      SHAppBarMessage(ABM_NEW, &abd);
      GetWindowRect(hwndMain , &abd.rc);
      Registered = true;
      dockDesktop(true , true);
    }
  } else {
    if (Registered) {
      SHAppBarMessage(ABM_REMOVE, &abd);
      Registered = false;
      docked_rect = Rect(0,0,0,0);
    }
  }
}

RECT dockRect(bool invalidate) {
  if (invalidate && docked == 2) dockDesktop(true , true);
  RECT rc , ret , wr;
  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
  if (docked == 2 || docked == -12 || docked==-2) UnionRect(&rc , &rc , &docked_rect);
  GetWindowRect(hwndMain , &wr);
  int cx = wr.right - wr.left;
  if (docked == 2) {
    ret = docked_rect;
  } else {
    ret.left = (docked_right?rc.right - cx - GETINT(CFG_UIDOCKSPACE) : (rc.left+GETINT(CFG_UIDOCKSPACE)));
    ret.right = ret.left + cx;
	if (GETINT(CFG_UIDOCKSTRETCH)) {
		ret.top = rc.top;
		ret.bottom = rc.bottom;
	} else {
		ret.top = wr.top;
		ret.bottom = wr.bottom;
	}
  }
  return ret;
}

void dockIt (RECT* wr) {
//  if (wp->flags & SWP_NOMOVE) return;
  int dockEdge = 50;
  RECT rc;
  POINT pt;
  int cx = wr->right - wr->left
     ,cy = wr->bottom - wr->top
     ;
  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
//  IMLOG("BEFORE %d %d" , rc.left , rc.right);
  if (docked == 2 || docked == -12) UnionRect(&rc , &rc , &docked_rect);
//  IMLOG("AFTER %d %d %d" , rc.left , rc.right , GetLastError());
  GetCursorPos(&pt);
  if ((!docked || docked<-10) && (pt.x < rc.left + dockEdge || pt.x > rc.right - dockEdge)) {
    IMLOG("DOCKING");
    docked_h = docked<-10? docked_h : cy;
    docked = GETINT(CFG_UIDOCKHIDE) ? -1 : -2;
    docked_right = pt.x > rc.right - dockEdge;
//    SetWindowPos(hwndMain , HWND_TOPMOST , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE);
  }
  if (docked < -10) {wr->bottom = wr->top + docked_h; return;}
  if (docked) {
    if ((docked_right && pt.x < wr->left) || (!docked_right && pt.x > wr->right)) {
//      IMLOG("UNDOCKING");
      wr->top = pt.y - 5;
      wr->left = pt.x - 40;
      wr->right = wr->left + cx;
      wr->bottom = wr->top + docked_h;
//      SetWindowPos(hwndMain , GETINT(CFG_UIONTOP)? HWND_TOPMOST:HWND_NOTOPMOST , 0,0,0,0,SWP_NOMOVE);
      docked = -10 - abs(docked);
    } else {
      *wr = dockRect(); 
    }
  }
}

/*
   Pokazuje lub chowa g³ówne okno odpowiednio do rodzaju dokowania...
*/
void dockHide(bool show) {
   if ((IsWindowVisible(hwndMain)!=0) == show) {
       if (show) SetWindowPos(hwndMain , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
       return;}
   if (docked==0) {
      ShowWindow(hwndMain , show?SW_SHOW : SW_HIDE);
      AutoHide();
   } else
   if (docked==1) {
      ShowWindow(hwndDock , show?SW_SHOWNA : SW_SHOWNA);
      ShowWindow(hwndMain , show?SW_SHOWNOACTIVATE : SW_HIDE);
      if (GetActiveWindow()!=hwndMain) SetAlpha(hwndMain , (unsigned char)ceil(GETINT(CFG_UIDOCKEDALPHA)*2.55));
//      if (show) {
//        SetForegroundWindow(hwndMain);
//      }
      AutoHide();
      return;
   } else
   if (docked==2) {
      ShowWindow(hwndMain , show?SW_SHOW : SW_HIDE);
	  dockDesktop(show);
   }
   if (show) {//SetWindowPos(hwndMain , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE|SWP_NOSIZE/*|SWP_NOACTIVATE*/);
       SetForegroundWindow(hwndMain);
//       SetActiveWindow(hwndMain);
   }
}


void dockEnd() {
  RECT rc , wr;
  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
  GetWindowRect(hwndMain , &wr);
  switch (docked) {
    case -11:
         SetWindowPos(hwndMain , GETINT(CFG_UIONTOP)? HWND_TOPMOST:HWND_NOTOPMOST , 0,0,wr.right - wr.left,docked_h,SWP_NOMOVE|SWP_NOACTIVATE);
         DestroyWindow(hwndDock);
         hwndDock = 0;
         docked = 0;
         break;
    case -12:
         SetWindowPos(hwndMain , GETINT(CFG_UIONTOP)? HWND_TOPMOST:HWND_NOTOPMOST , 0,0,wr.right - wr.left,docked_h,SWP_NOMOVE|SWP_NOACTIVATE);
         dockDesktop(false);
         docked = 0;
         break;
    case -1:
         wr = dockRect();
         SetWindowPos(hwndMain , HWND_TOPMOST , wr.left , wr.top , wr.right - wr.left,wr.bottom - wr.top,SWP_NOACTIVATE);
         if (hwndDock) {DestroyWindow(hwndDock);}
         hwndDock = CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_TOPMOST
           ,"UIdock", "",WS_POPUP,
           docked_right?rc.right-1:0,rc.top + DOCK_SPACE,1,rc.bottom - rc.top - 2*DOCK_SPACE,
           hwndTop, NULL, hInst, NULL);
         SetAlpha(hwndDock , 10);
//         }
         docked = 1;
         break;
    case -2:
         wr = dockRect();
         ShowWindow(hwndMain , SW_SHOWNOACTIVATE);
         SetWindowPos(hwndMain , HWND_TOPMOST , wr.left , wr.top , wr.right - wr.left,wr.bottom - wr.top,SWP_NOACTIVATE);
//         SetWindowPos(hwndMain , HWND_TOPMOST , rc.right - (wr.right - wr.left),0,wr.right - wr.left,rc.bottom - rc.top,0);
         docked = 2;
         dockDesktop(true);
         break;

  }
}


void snapIt (RECT* wr) {
  if (docked) return;
//  if (wp->flags & SWP_NOMOVE) return;
  int snapEdge = 10;
  RECT rc;
  int cx = wr->right - wr->left
     ,cy = wr->bottom - wr->top
     ;
//  GetWindowRect(wp->hwnd , &rc);
//  int w = rc.right - rc.left , h = rc.bottom - rc.top;
  POINT pt;
  GetCursorPos(&pt);
  if (pt.x < wr->left || pt.x > wr->right || pt.y < wr->top || pt.y > wr->top + 30) snapEdge = 0;

  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
  if (wr->right + snapEdge > rc.right) wr->left = rc.right - (wr->right - wr->left);
  if (wr->left < rc.left + snapEdge) wr->left = rc.left;

  if (wr->bottom + snapEdge > rc.bottom) wr->top = rc.bottom - (wr->bottom - wr->top);
  if (wr->top < rc.top + snapEdge) wr->top = rc.top;

  wr->right = wr->left + cx;
  wr->bottom = wr->top + cy;
  return;
}
// --------------------------------------

   int strcompare (string a , string b) {
      return stricmp(a.c_str() , b.c_str())>0 ? 0 : 1;
   }

//************************************** HOOK
HHOOK hookShutProc;
   LRESULT CALLBACK HookShutCallWndProc(int nCode, WPARAM wParam,CWPSTRUCT * cwp) {
	   if (cwp->message != WM_ENDSESSION && cwp->message != WM_QUERYENDSESSION) return CallNextHookEx(hookShutProc , nCode , wParam , (LPARAM)cwp);
	   char buff [101];
	   GetWindowText(cwp->hwnd , buff , 100);
	   IMLOG("SHUT - ent - wP=%d wnd=%x txt=%s msg=%x wp=%d lp=%d" , wParam , cwp->hwnd , buff , cwp->message , cwp->wParam , cwp->lParam);
	   return CallNextHookEx(hookShutProc , nCode , wParam , (LPARAM)cwp);
   }
HHOOK hookShutProcRet;
   LRESULT CALLBACK HookShutCallWndProcRet(int nCode, WPARAM wParam,CWPRETSTRUCT * cwp){
	   if (cwp->message != WM_ENDSESSION && cwp->message != WM_QUERYENDSESSION) return CallNextHookEx(hookShutProcRet , nCode , wParam , (LPARAM)cwp);
	   char buff [101];
	   GetWindowText(cwp->hwnd , buff , 100);
	   IMLOG("SHUT - fin - wP=%d wnd=%x txt=%s msg=%x wp=%d lp=%d ret=%d" , wParam , cwp->hwnd , buff , cwp->message , cwp->wParam , cwp->lParam , cwp->lResult);
	   return CallNextHookEx(hookShutProc , nCode , wParam , (LPARAM)cwp);
   }


//-------------------------------------
//-------------------------------------
//-------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   static UINT s_uTaskbarRestart;
   RECT rc;
   cUIGroup * pGrp;
   sUICnt * pCnt;
   sUIACTION uiA;
   int a,b;
   MEASUREITEMSTRUCT * mis;
   DRAWITEMSTRUCT * dis;
   POINT pt;
   static HMENU hActiveMenu=0;
   static sUIAction MenuAction;
   switch (message)
   {
//        case WM_DESTROY:
//             IMLOG("WM_DESTROY");
//             PostQuitMessage(0);
//             return 1;
        case WM_CREATE:
            s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
			/* Auto autostart (usuwa po uruchomieniu) */
			if (GETINT(CFG_UIAUTOSTART) == 2) {
				HKEY hKey=HKEY_CURRENT_USER;
				if (!RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
					RegDeleteValue(hKey , getAutostartKey());
					RegCloseKey(hKey);
				}
			}

			break;
        case WM_CLOSE:
//             IMessage(IMC_SHUTDOWN);
//               ShowWindow(hwndMain , SW_HIDE);
               dockHide(false);
//             DestroyWindow(hwndMain);
//             PostQuitMessage(0);
             return 1;
        case WM_QUERYENDSESSION:
//			hookShutProc = SetWindowsHookEx(WH_CALLWNDPROC , (HOOKPROC)HookShutCallWndProc , 0 , GetCurrentThreadId());
//			hookShutProcRet = SetWindowsHookEx(WH_CALLWNDPROCRET , (HOOKPROC)HookShutCallWndProcRet , 0 , GetCurrentThreadId());
            IMLOG("! Received QUERYENDSESSION");
			/* Auto autostart */
			if (GETINT(CFG_UIAUTOSTART) == 2) {
				HKEY hKey=HKEY_CURRENT_USER;
				if (!RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
					RegSetSZ(hKey,getAutostartKey(),getAutostartValue());
					RegCloseKey(hKey);
				}
			}

//             ICMessage(IMC_SHUTDOWN,1);
            return true;
		case WM_ENDSESSION: {
            IMLOG("! Received ENDSESSION");
            ICMessage(IMC_SHUTDOWN,1);
            IMLOG("! IMC_SHUTDOWN w/o Exit");
			return FALSE;}
             
        case WM_COMMAND:
             //MessageBox(NULL , inttoch(LOWORD(wParam)) , "" , 0);
             if (!lParam)
              switch (HIWORD(wParam)) {
               case 1: // Accel
               break;
            } else
              switch(HIWORD(wParam)) {
               case BN_CLICKED: // Control
//                  IMLOG("Clicked w %d l %x parent %d" , LOWORD(wParam) , lParam , GetMenu((HWND)lParam));
                  if ((pGrp=(cUIGroup*)GetProp((HWND)lParam , "ACTION"))) {
//                    a = (int)GetMenu((HWND)lParam);
                    b = pGrp->findIndex(LOWORD(wParam));
                    if (b<0) return 0;
                    (*pGrp)[b].call(ACTN_ACTION,0,0);
                  }
                  /*
                  switch(LOWORD(wParam)) {
                    case IDB_STATUS :
                    break;
                  }
                  */
                  break;
               case LBN_DBLCLK:
                  if (lParam == (long)hwndList) {
                  }
                  break;
             }
             break;
        case WM_INITMENUPOPUP:
             return UIMessageProc(0,hWnd , message , wParam , lParam);
        case WM_ENTERIDLE:
             return UIMessageProc(0,hWnd , message , wParam , lParam);

        case WM_MEASUREITEM:
             mis=(MEASUREITEMSTRUCT *)lParam;
             if (!wParam) { // MENU
                return UIMessageProc(0,hWnd , message , wParam , lParam);
             }
             switch (wParam) {
               case IDC_LIST:
                  if (!mis->itemData || !cntListWnd) return 0;
                  cntListWnd->measureListItem(mis);
                  return 1;
             }
             break;
        case WM_DRAWITEM:
             dis=(DRAWITEMSTRUCT *)lParam;
             if (!wParam) { // MENU
                return UIMessageProc(0,hWnd , message , wParam , lParam);
             }
             switch (wParam) {
               case IDC_LIST:  //  LISTA KONTAKTOW
// ------------------------------------------------------------
                  cntListWnd->drawListItem(dis);
                  return 1;
             }
             break;
        case WM_CONTEXTMENU:
             if (wParam == (unsigned int)hwndList) {
                  pt.x=LOWORD(lParam);pt.y=HIWORD(lParam);
                  ScreenToClient(hwndList , &pt);
                  a = ListView_ItemFromPoint(hwndList , pt);

                  if (a<0) return 0;
                  if (!ListView_GetItemState(hwndList , a , LVIS_SELECTED)) {
                     ListView_Deselect(hwndList);
                     ListView_SetItemState(hwndList , a , LVIS_SELECTED , LVIS_SELECTED);
                     ListView_SetSelectionMark(hwndList , a);
                  }
//                  a=SendMessage(hwndList , LB_GETCURSEL , 0 , 0);
                  if (ListView_GetSelectedCount(hwndList)>1) Act[IMIG_CNT].setCnt(-1);
                    else Act[IMIG_CNT].setCnt(((sUICnt *)ListView_GetItemData(hwndList , a))->ID);
                  UIPopupMenu(Act[IMIG_CNT],TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD,
                       LOWORD(lParam),HIWORD(lParam),0,hWnd , 0);
             }
             break;

        case WM_SIZING:
             clipSize(wParam , (RECT *)lParam , MIN_WIDTH , MIN_HEIGHT);
             return 1;
        case WM_SIZE:
//             IMessage(IMC_LOG , 0,0,(int)"WM_SIZE" , wParam);
             onSizeMain(LOWORD(lParam) , HIWORD(lParam));
             return 1;
        case WM_WINDOWPOSCHANGING: {
//             return 0;
             WINDOWPOS * wp = (WINDOWPOS*)lParam;
             if (docked>0) {
               rc = dockRect();
               wp->x = rc.right - wp->cx;// + (docked_right?-1:1)*GETINT(CFG_UIDOCKSPACE);
               wp->y = rc.top;
//               wp->cx = rc.right - rc.left;
               wp->cy = rc.bottom - rc.top;
             }
             break;}
/*        case WM_WINDOWPOSCHANGED: {

			return 0;
			}*/
/*        case WM_WINDOWPOSCHANGED: {
            WINDOWPOS * wp = (WINDOWPOS*)lParam;
             if (!(wp->flags & SWP_NOZORDER) && wp->hwndInsertAfter!=HWND_TOP &&  GETINT(CFG_UIONTOP)) {
                 IMLOG("SETONTOP");
                 SetWindowPos(hwndMain , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOSENDCHANGING);
             }
            break;}
            */
//        case WM_MOVING:
//             IMLOG("MOVING %d x %d" , ((RECT*)lParam)->left , ((RECT*)lParam)->right);
//             return 0;
        case WM_MOVING:
             if (hWnd != hwndMain) return 0;
             if (GETINT(CFG_UISNAP)) snapIt((RECT*)lParam);
             if (GETINT(CFG_UICANDOCK)) dockIt((RECT*)lParam);
             return 1;
        case WM_LBUTTONUP: case WM_MOUSEMOVE:
             if (dragdropList()) return dragdropList(hWnd , message , lParam , wParam);
               else return 0;
        case WM_DISPLAYCHANGE:
             wParam = SPI_SETWORKAREA;
        case WM_SETTINGCHANGE:
             if (wParam == SPI_SETWORKAREA && docked > 0) {
               rc = dockRect(true);
               IMLOG("- Settings changed");
               MoveWindow(hwndMain , rc.left , rc.top , rc.right - rc.left , rc.bottom - rc.top , true);
               if (hwndDock) {
                 SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
                 MoveWindow(hwndDock , docked_right?rc.right-1:rc.left-1 , rc.top + DOCK_SPACE , 3 , rc.bottom - 2*DOCK_SPACE , false);
               }
             }
             return 1;
        case WM_EXITSIZEMOVE:
             if (GETINT(CFG_UICANDOCK)) dockEnd();
             dockDesktop(true , true);
//             IMLOG("CHANGED %d" , docked);
			 UISetAlpha();
             return 0;
        case WM_ACTIVATE:
             if (docked == 1) {
                 if (wParam) UISetAlpha(); //SetAlpha(hwndMain , 255);
			 }

             return 0;
        case WM_ACTIVATEAPP:
             if (docked == 1 && !wParam) { // tylko chowamy!
                 dockHide(wParam!=0);
             }

//             SetWindowPos(hwndMain , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
             return 0;

        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
//             IMLOG("NOTIFY %x[%x] = %d" , nm->hwndFrom , hwndList , nm->code);
             switch (nm->code) {
               case RBN_LAYOUTCHANGED:
//                   onSizeMain(0,0);
                   return 1;
               case RBN_CHILDSIZE: {
                   NMREBARCHILDSIZE * nmrcs = (NMREBARCHILDSIZE*)lParam;
                   if (nmrcs->uBand) onSizeMain(0,0);
                   return 1;}
               case TBN_DROPDOWN:case RBN_CHEVRONPUSHED:
                   return UIMessageProc(0 , hWnd , message , wParam , lParam);

                case NM_DBLCLK:
                  if (nm->hwndFrom == hwndList) {
					  cntListWnd->callDefaultAction();
                   }
                   break;
       // LISTVIEW
                case LVN_BEGINLABELEDIT:
                   if (!GETINT(CFG_UIEDITONLIST)) return true;
                   NMLVDISPINFO * nmdi;
                   nmdi = (NMLVDISPINFO*)lParam;
                   pCnt = (sUICnt*)ListView_GetItemData(hwndList , nmdi->item.iItem);
                   if (!pCnt) return true;
                   SetWindowText(ListView_GetEditControl(hwndList) , GETCNTC(pCnt->ID , CNT_DISPLAY));
//                   refreshListItem((sUICnt*)ListView_GetItemData(hwndList , nmdi->item.iItem) , nmdi->item.iItem);
                   return false;
                case LVN_ENDLABELEDIT:
                   nmdi = (NMLVDISPINFO*)lParam;
                   pCnt = (sUICnt*)ListView_GetItemData(hwndList , nmdi->item.iItem);
                   if (!pCnt || !nmdi->item.pszText) return false;
                   SETCNTC(pCnt->ID , CNT_DISPLAY , nmdi->item.pszText);
                   ICMessage(IMI_REFRESH_CNT , pCnt->ID);
//                   refreshListItem((sUICnt*)ListView_GetItemData(hwndList , nmdi->item.iItem) , nmdi->item.iItem);
                   return false;
                case LVN_BEGINDRAG:
                   return dragdropList(hWnd , message , wParam , lParam);
                case LVN_ITEMCHANGED : /*case NM_CLICK:*/{
                    NMLISTVIEW * nlv = (NMLISTVIEW*)lParam;

                    if (cntTip->docked 
                        && nlv->uChanged & LVIF_STATE
                        && nlv->uNewState & LVIS_SELECTED) {
                            sUICnt * cnt;
                            if (ListView_GetSelectedCount(nlv->hdr.hwndFrom)>1)
                                cnt = (sUICnt*)ListView_GetItemData(nlv->hdr.hwndFrom , ListView_GetSelectionMark(nlv->hdr.hwndFrom));
                            else                            
                                cnt = (sUICnt*)ListView_GetItemData(nlv->hdr.hwndFrom , nlv->iItem);
                            if (cnt)
                                cntTip->show(cnt->ID , nm->hwndFrom);
                        }
                    return 0;}
       // TABCONTROL
                case TCN_SELCHANGE:
                   if (nm->hwndFrom == hwndGroups) {
                      selectGroups();
                   }
                   return false;
//               default: if (nm->idFrom == IMIG_UIMAIN && nm->code!=NM_NCHITTEST && nm->code != NM_CUSTOMDRAW) IMLOG("N - %x" , nm->code);
             }
             return 0;
        case MYWM_NOTIFYICON:
             //IMLOG("ICON %x - %d", lParam);
             switch (lParam) {
			   case WM_MOUSEMOVE: {
				   static bool antiRecurence = false;
				   if (!antiRecurence) {
					   antiRecurence = true;
					   tooltip.mouseMovement(hwndTop, getScreenPoint(), Point(wParam, -1));
					   antiRecurence = false;
				   }
				   break;}
               case WM_RBUTTONUP :
				   tooltip.hide();
                   GetCursorPos(&pt);
                   UIPopupMenu(Act[IMIG_TRAY] , TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD
                                  ,pt.x
                                  ,pt.y
                                  ,0,hwndMain , 0);
                 break;
               case WM_LBUTTONDOWN :
 			     tooltip.hide();
                 //ShowWindow(hwndMain , SW_SHOW);
                 IMLOG("TRAY FG=%x AC=%x M=%x UNC=%d" ,  GetForegroundWindow() , GetActiveWindow() , hwndMain , IsWindowUncovered(hwndMain));
                 if (docked!=2 && (!trayAction.id || !GETINT(CFG_UIONECLICKTRAY))) {
                   //if (trayNotify) return 0;
                   if (docked!=1 && IsWindowVisible(hwndMain) && !trayNotify && IsWindowUncovered(hwndMain)) {
                      dockHide(false);
                      return 0;
                   }
                   IMLOG("UNHIDE");
                   dockHide(true);
                   SetForegroundWindow(hwndMain);
//                   SetFocus(hwndMain);
//                   SetWindowPos(hwndMain , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE|SWP_NOSIZE);
                   return 0;
                 }
               case WM_LBUTTONDBLCLK : {
  				   tooltip.hide();

//                 SetForegroundWindow(hwndMain);
                 //SetForegroundWindow(hwndMain);
//                 SetFocus(hwndMain);
                 if ((!trayCnt || ICMessage(IMC_CNT_IDEXISTS , trayCnt)) 
                   && trayAction.id && Act.exists(trayAction)) {
                     Act(trayAction).setCnt(trayCnt);
                     Act(trayAction).call(ACTN_ACTION , 0 , 0);
                 }
                 if (trayAction.id || trayNotify) {
                    ICMessage(IMI_NOTIFY , -1);
                 }

                 break;}
             }
             return 0;
        case MYWM_NOTIFYAPPBAR:
//             IMLOG("NOTIFY %d %x" , wParam , lParam);
             break;
        case WM_TIMER:
//             if (wParam == TIMER_LISTTIP) {
//                ListTipShow(-2);
/*                GetCursorPos(&pt);
                ScreenToClient(hwndList , &pt);
                ListTipShow(ListView_ItemFromPoint(hwndList , pt));
*/
//             }
             break;
    default:
        if(message == s_uTaskbarRestart) {
            createTray();
            trayNotify = -1;
            CNotify(NOTIFY_AUTO);
            break;}
  
	    return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


LRESULT CALLBACK TopProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_ENDSESSION:
            IMLOG("! TOP_ENDSESSION");
            ICMessage(IMC_SHUTDOWN,1);
            return FALSE;
        case WM_ENTERIDLE:
             return UIMessageProc(0,hwnd , message , wParam , lParam);
/* Robimy za proxy dla g³ównego okna... */
		case WM_ACTIVATE:
			/* Aktywujemy g³ówne okno tylko gdy jest widoczne... */
			if (LOWORD(wParam) && IsWindowVisible(hwndMain)) {
//                   dockHide(true);
                   SetActiveWindow(hwndMain);
			}
			return 0;
		case WM_KEYUP:
			if (wParam >= VK_SPACE)
				dockHide(true);
			return 0;
		case WM_CLOSE:
			return SendMessage(hwndMain , WM_CLOSE , wParam , lParam);

			/*
		case WM_COPYDATA: { // Wznawianie sesji
			COPYDATASTRUCT * cds = (COPYDATASTRUCT*) lParam;
			if (wParam || cds->dwData < 1 || (cds->cbData/4 != cds->dwData) || !hwndMain)
				return FALSE;
            dockHide(true);
            SetForegroundWindow(hwndMain);
			HGLOBAL * data = (HGLOBAL*)cds->lpData;
			char ** argv = (char**)malloc(cds->cbData);
			IMLOG("- Restoring %d parameters from %x" , cds->dwData , argv);
			// trzeba odblokowaæ GlobalAlloc'i
			for (int i=0; i<cds->dwData; i++) {
				argv[i] = (char*)GlobalLock(data[i]);
				GlobalUnlock(data[i]);
				IMLOG("- arg %x=%s" , argv[i] , argv[i]);
			}
			Ctrl->IMessage(&sIMessage_plugArgs(cds->dwData , (char**)cds->lpData));
			free(argv);
			return TRUE;}
			*/
		default: {
			static int msgRestore = RegisterWindowMessage("KONNEKT.Msg.RestoreData");
			if (message == msgRestore) {
				dockHide(true);
				SetForegroundWindow(hwndMain);
				if (!wParam) 
					return 0;
				HANDLE fileMapping = OpenFileMapping(FILE_MAP_READ , false , "Local\\KONNEKT.File.RestoreData");
				if (!fileMapping)
					return FALSE;
				char * base = (char *)MapViewOfFile(fileMapping , FILE_MAP_READ , 0 , 0 , lParam);
				if (!base)
					return FALSE;
				IMLOG("- Restoring %d arguments from %db" , wParam , lParam);
				char ** argv = new char* [wParam];
				memcpy(argv , base , wParam * sizeof(char*));
				for (unsigned int i=0; i<wParam; i++) {
					IMLOG("    arg %x=%s" , argv[i] , base+(int)argv[i]);
					argv[i]+=(int)base;
				}
				Ctrl->IMessage(&sIMessage_plugArgs(wParam , (char**)argv));
				delete [] argv;
				UnmapViewOfFile(base);
				CloseHandle(fileMapping);
				return TRUE;
			}
		}
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DockClassProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam){
   static bool cursorIn;
   switch (message){  
    case WM_MOUSEMOVE:
        if (!cursorIn) {
            TRACKMOUSEEVENT tme;
            cursorIn = true;
            tme.cbSize = sizeof (tme);
            tme.dwFlags = TME_LEAVE | TME_HOVER;
            tme.hwndTrack = hwnd;
            tme.dwHoverTime = GETINT(CFG_UIDOCKWAIT);
            _TrackMouseEvent(&tme);
        }
        break;
    case WM_MOUSEHOVER:
        dockHide(true);
    case WM_MOUSELEAVE:
        cursorIn = false;
        break;

      case WM_NCHITTEST: {/*
          static bool test = false;
          if (test) break;
          test = true;
          int wait = GETINT(CFG_UIDOCKWAIT);
          int i=0;
          RECT rc;
          GetWindowRect(hwnd , &rc);
          
          while (i<wait) {
              if ()
              SleepEx(10 , TRUE);
              i+=10;
          }
          dockHide(true);
          test = false;  */
          break;
      }

   }
   return DefWindowProc(hwnd, message, wParam, lParam);
}


