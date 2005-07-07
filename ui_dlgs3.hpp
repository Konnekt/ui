int CALLBACK PassDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   sDIALOG_access * dlg;
   static char str_buff2 [MAX_STRBUFF];
   bool login;
   switch (message)
   {
        case WM_INITDIALOG:
                SetWindowPos(hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE);
                SetProp(hwnd , "DLG" , (void*)lParam);
                dlg = (sDIALOG_access*)lParam;
                SetWindowText(hwnd , dlg->title);
                SetDlgItemText(hwnd , IDC_STATIC , dlg->info);
                login = GetDlgItem(hwnd , IDC_LOGIN)!=0;
                if (login) {SetDlgItemText(hwnd , IDC_LOGIN , dlg->login);}
                SetDlgItemText(hwnd , IDC_PASS1 , dlg->pass);
                SetFocus(GetDlgItem(hwnd , login?IDC_LOGIN:IDC_PASS1));
                ShowWindow(GetDlgItem(hwnd , IDC_PASS2) , dlg->flag & DFLAG_PASS2 ? SW_SHOW : SW_HIDE);
//                SetDlgItemText(hwnd , IDC_STATIC , dlg->info);
                CheckDlgButton(hwnd , IDC_CHECK , dlg->save?BST_CHECKED:BST_UNCHECKED);
                ShowWindow(GetDlgItem(hwnd , IDC_CHECK) , dlg->flag & DFLAG_SAVE ? SW_SHOW : SW_HIDE);
                break;
        case WM_CLOSE:
                EndDialog(hwnd, IDCANCEL);
                break;
        case WM_NCDESTROY:
                dlg=(sDIALOG_access*)GetProp(hwnd , "DLG");
                RemoveProp(hwnd , "DLG");
                break;
        case WM_COMMAND:
            dlg=(sDIALOG_access*)GetProp(hwnd , "DLG");
            switch (LOWORD(wParam))
            {
                case IDOK:
                    login = GetDlgItem(hwnd , IDC_LOGIN)!=0;
                    GetDlgItemText(hwnd , IDC_PASS1 , TLS().buff , MAX_STRING);
                    if (login) {
                        GetDlgItemText(hwnd , IDC_LOGIN , TLS().buff2 , MAX_STRING);
                        dlg->login = TLS().buff2;
                    }
					if (GetDlgItem(hwnd , IDC_PASS2) && IsWindowVisible(GetDlgItem(hwnd , IDC_PASS2))) {
					  GetDlgItemText(hwnd , IDC_PASS2 , TLS().str.GetBuffer(MAX_STRBUFF) , MAX_STRBUFF);
					  TLS().str.ReleaseBuffer();
                      if (TLS().str != TLS().buff) {IMessage(IMI_WARNING , 0 , 0 , (int)"Zrobi³eœ b³¹d przy potwierdzaniu has³a! \nWpisz w oba pola TO SAMO has³o!");
                                                       break;}
                    }
                    dlg->pass = TLS().buff;
                    dlg->save = IsDlgButtonChecked(hwnd , IDC_CHECK)==BST_CHECKED;
                    // Fall through.

                case IDCANCEL:
                    EndDialog(hwnd, wParam);
                    return TRUE;
            }

   }
   return 0;
}



void EnterDialogLoadHistory(sDIALOG_enter * dlg) {
    sMRU MRU;
    HWND hwnd = (HWND) GetDlgItem((HWND) dlg->handle , IDC_COMBO);
    string name = "dlg_" + string(dlg->id);
    MRU.name = name.c_str();
    MRU.buffSize = 255;
    MRU.count = 10;
    MRU.values = new char * [10];
    int i;
    for (i = 0; i<10; i++) 
        MRU.values[i] = new char [255];

    MRU.count = Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_GET , &MRU));

    for (i=0; i<MRU.count; i++)
        SendMessage(hwnd , CB_ADDSTRING , 0 , (LPARAM)MRU.values[i]);


    for (i = 0; i<10; i++) 
        delete MRU.values[i];
    delete [] MRU.values;
}
void EnterDialogSaveHistory(sDIALOG_enter * dlg, char * actual) {
  if (!*dlg->id) return;
  if (!*actual) return;
  sMRU MRU;
  MRU.count = 10;
  MRU.flags = MRU_SET_LOADFIRST | MRU_GET_USETEMP;
  MRU.current = actual;
  string name = "dlg_" + string(dlg->id);
  MRU.name = name.c_str();
  Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_SET , &MRU));

}

int CALLBACK EnterDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   sDIALOG_enter * dlg;
   static char str_buff2 [MAX_STRBUFF];
   switch (message)
   {
        case WM_INITDIALOG:
                SetWindowPos(hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE);
                SetProp(hwnd , "DLG" , (void*)lParam);
                dlg = (sDIALOG_enter*)lParam;
                dlg->handle = hwnd;
                SetWindowText(hwnd , dlg->title);
                SetDlgItemText(hwnd , IDC_STATIC , dlg->info);
                if (dlg->id && *dlg->id) {
                  ShowWindow(GetDlgItem(hwnd , IDC_COMBO) , SW_SHOW);
                  ShowWindow(GetDlgItem(hwnd , IDC_EDIT) , SW_HIDE);
                  EnterDialogLoadHistory(dlg);
                  SetDlgItemText(hwnd , IDC_COMBO , dlg->value);
                  GetDlgItemText(hwnd , IDC_COMBO , str_buff2 , MAX_STRING);
                  SendMessage(GetDlgItem(hwnd , IDC_COMBO) , CB_LIMITTEXT , dlg->maxLength , 0);
                  SetFocus(GetDlgItem(hwnd , IDC_COMBO));
                }
                else {
                  SetDlgItemText(hwnd , IDC_EDIT , dlg->value);
                  SendDlgItemMessage(hwnd , IDC_EDIT, EM_SETSEL , 0 , -1);
                  SendMessage(GetDlgItem(hwnd , IDC_EDIT) , EM_LIMITTEXT , dlg->maxLength , 0);
                  SetFocus(GetDlgItem(hwnd , IDC_EDIT));
                }
                break;
        case WM_CLOSE:
                EndDialog(hwnd, IDCANCEL);
                break;
        case WM_NCDESTROY:
                dlg=(sDIALOG_enter*)GetProp(hwnd , "DLG");
				if (dlg)
					dlg->handle = 0;
                RemoveProp(hwnd , "DLG");
                break;
        case WM_COMMAND:
            dlg=(sDIALOG_enter*)GetProp(hwnd , "DLG");
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (*dlg->id) {
                      GetDlgItemText(hwnd , IDC_COMBO , str_buff2 , MAX_STRBUFF);
                      EnterDialogSaveHistory(dlg , str_buff2);
                    } else
                      GetDlgItemText(hwnd , IDC_EDIT , str_buff2 , MAX_STRBUFF);
                    dlg->value = str_buff2;
                    // Fall through.

                case IDCANCEL:
                    EndDialog(hwnd, wParam);
                    return TRUE;
            }

   }
   return 0;
}
// *********************************************

int CALLBACK ButtonsDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   sDIALOG_choose * dlg;
   static char str_buff2 [MAX_STRBUFF];
   switch (message)
   {
        case WM_INITDIALOG: {
                IMLOG("- Buttons");
//                SetWindowPos(hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE);
                SetProp(hwnd , "DLG" , (void*)lParam);
                dlg = (sDIALOG_choose*)lParam;
                SetWindowText(hwnd , dlg->title);
                SetDlgItemText(hwnd , IDC_STATIC , dlg->info);
                char * items = strdup(dlg->items);
                char * item = items;
                char * found;
                RECT rc,rc2;
                GetClientRect(hwnd , &rc);
                GetClientRect(GetDlgItem(hwnd,IDC_STATIC) , &rc2);
                int w=dlg->width?dlg->width:120;
                int h=24;
                int x=(rc.right/2) - (w/2);
                int y=rc2.bottom + 16;
                int i=1;
                while (item && *item) {
                  found = strchr(item , '\n');
                  if (found) *found=0;
                  if (*item) {
                    SendMessage(
                      CreateWindowEx(0 , "BUTTON" , item ,
                             WS_VISIBLE|WS_CHILD|WS_TABSTOP|(dlg->def==i?BS_DEFPUSHBUTTON:BS_PUSHBUTTON)
                             , x , y , w , h , hwnd , (HMENU)(100+i) , hInst , 0)
                      , WM_SETFONT , (WPARAM)fontNormal , 1);
                    y+=h+2;
                    i++;
                  }
                  item = found?found+1:0;
                }
                if (dlg->flag & DFLAG_CANCEL) {
                    y+=6;
                    SendMessage(
                      CreateWindowEx(0 , "BUTTON" , "Anuluj" ,
                             WS_VISIBLE|WS_CHILD|WS_TABSTOP|(dlg->def==0?BS_DEFPUSHBUTTON:BS_PUSHBUTTON)
                             , x , y , w , h , hwnd , (HMENU)(IDCANCEL) , hInst , 0)
                      , WM_SETFONT , (WPARAM)fontNormal , 1);
                    y+=h+2;
                }
                rc=Rect(0 , 0 , rc.right , y + 4);
                AdjustWindowRect(&rc , GetWindowLong(hwnd , GWL_STYLE) , 0);
                OffsetRect(&rc , -rc.left , -rc.top);
                GetClientRect(GetDesktopWindow() , &rc2);
                SetWindowPos(hwnd , HWND_TOP , rc2.right/2-rc.right/2 , rc2.bottom/2-rc.bottom/2 , rc.right , rc.bottom , 0);
                free(items);
                //                SetFocus(GetDlgItem(hwnd , IDC_EDIT));
                break;}
        case WM_CLOSE:
                EndDialog(hwnd, 0);
                break;
        case WM_NCDESTROY:
                dlg=(sDIALOG_choose*)GetProp(hwnd , "DLG");
                RemoveProp(hwnd , "DLG");
                break;
        case WM_COMMAND:
            dlg=(sDIALOG_choose*)GetProp(hwnd , "DLG");
            switch (LOWORD(wParam))
            {
//                case IDOK:
                    //GetDlgItemText(hwnd , IDC_EDIT , str_buff2 , MAX_STRBUFF);
//                    return TRUE;
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
                default:
                    if (wParam>100) EndDialog(hwnd , wParam - 100);
            }

   }
   return 0;
}


int CALLBACK TokenDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	sDIALOG_token * tk;
	switch (message) {
        case WM_INITDIALOG: {
			sIMessage_2params * msg = (sIMessage_2params *)lParam;
			tk = (sDIALOG_token *) msg->p1;
			SetProp(hwnd , "sDIALOG_token*" , (HANDLE)tk);
			SetDlgItemText(hwnd , IDC_INFO , tk->info);
			SetDlgItemText(hwnd , IDC_EDIT1 , tk->token);
			SetWindowText(hwnd , tk->title);
			int imageType;
			HANDLE bmp = cUIIcons::getImageFromURL(tk->imageURL , imageType , (HINSTANCE)ICMessage(IMC_PLUGID_HANDLE , msg->sender));
			IMDEBUG(DBG_DUMP , "getImageFromURL(%s) = %x" , tk->imageURL , bmp);
			if (bmp) {
				SendDlgItemMessage(hwnd , IDC_TOKEN , STM_SETIMAGE , imageType , (LPARAM)bmp);
				RECT rc;
				BITMAP bi;
				GetObject(bmp , sizeof(BITMAP),&bi);
				GetWindowRect(hwnd , &rc);
				IMDEBUG(DBG_DUMP , "Token size - w=%d h=%d" , bi.bmWidth , bi.bmHeight);
				SetWindowPos(hwnd , 0 , 0 , 0 , rc.right - rc.left , rc.bottom - rc.top + bi.bmHeight , SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hwnd , IDC_TOKEN) , 0 , 0 , 0 , rc.right - rc.left - 28 , bi.bmHeight , SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
			}
            SetFocus(GetDlgItem(hwnd , IDC_EDIT1));
            break;}
        case WM_CLOSE:
			EndDialog(hwnd , 0);
			return 0;
        case WM_DESTROY:
            DeleteObject((HGDIOBJ)SendDlgItemMessage(hwnd , IDC_TOKEN , STM_GETIMAGE , IMAGE_BITMAP , 0));
            return 0;
    case WM_COMMAND:
		tk = (sDIALOG_token*) GetProp(hwnd , "sDIALOG_token*");
        switch(HIWORD(wParam)) {
        case BN_CLICKED:{
            switch (LOWORD(wParam)) {
			case IDOK: {
				tk->token = (char*)Ctrl->GetTempBuffer(500);
				GetDlgItemText(hwnd , IDC_EDIT1 , tk->token , 500);  
				EndDialog(hwnd , IDOK);
				return 0;}
            }
            break;}
        }
        break;
    }
    return 0;//DefWindowProc(hwnd , message , wParam , lParam);
}



// *********************************************

VOID CALLBACK LongTimerProc(
  void * arg,   // data value
  DWORD dwTimerLowValue,             // timer low value
  DWORD dwTimerHighValue            // timer high value
)
//VOID CALLBACK LongTimerProc(HWND hwnd,UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
 {
/*   if (arg) {
     static char * move [4] = {
       "///" , "|||" , "\\\\\\" , "---"
     };
     static int i = 0;
     SetDlgItemText(arg , IDC_MSG , move[i]);
     i++;
     if (i>=4) i = 0;
   }
*/
   //ProcessMessages(0);
   CtrlEx->WMProcess();
 }

VOID CALLBACK TimeoutTimerProc(void * arg,DWORD dwTimerLowValue,DWORD dwTimerHighValue) {
  sDIALOG_long * sd = (sDIALOG_long *)arg;
  if (!K_CHECK_PTR(sd) || !K_CHECK_PTR(sd->timeoutProc)) {
	  IMDEBUG(DBG_ERROR, "Bad dLong timer pointer %x", sd);
	  return;
  }
  if (sd->timeoutProc(TIMEOUTT_CHECK , sd)) {
    sd->timeoutPassed+=TIMEOUT_INTERVAL;
//    IMLOG("TIMEOUT Passed %d  / %d" , sd->timeoutPassed , sd->timeout);
    if (sd->timeout && sd->timeoutPassed>=sd->timeout) {
      CancelWaitableTimer(sd->timeoutHandle);
      sd->timeoutProc(TIMEOUTT_TIMEOUT , sd);
      return;
    }
  } else {sd->timeoutPassed=0; /*IMLOG("TIMEOUT Cancel");*/}
}

/*void _USERENTRY LongThreadProc (LPVOID lParam) {
  while (longReference>0) {
    IMLOG("threadLONG");
    CtrlEx->WMProcess();
    SleepEx(500 , 1);
  }
  threadLong=0;
}
*/
int StartLong(sDIALOG_long * sd) {
//  HANDLE timer;
//  if (dlg) {
//    timer = CreateWaitableTimer(0,0,0);
//  } else timer = timerLong;
  IMLOG("* Start Long %s" , (sd->flag & DLONG_BLOCKING)? "with non-BLOCKING.timer" : "Dlg w/o timer");
  if (!K_CHECK_PTR(sd)) {
	  IMDEBUG(DBG_ERROR, "Bad dLong pointer %x", sd);
	  return 0;
  }
  LARGE_INTEGER    lDueTime;
  if (sd->timeoutProc) {
    sd->timeoutProc(TIMEOUTT_START , sd);
    sd->timeoutHandle = CreateWaitableTimer(0,0,0);
    sd->timeoutPassed = 0;
    lDueTime.QuadPart = -10000 * TIMEOUT_INTERVAL;
    SetWaitableTimer(sd->timeoutHandle , &lDueTime , TIMEOUT_INTERVAL,TimeoutTimerProc, sd , 0);
  }
  if (sd->flag & DLONG_BLOCKING) {
    longReference++;
    lDueTime.QuadPart = 0;
    SetWaitableTimer(timerLong , &lDueTime , 50 ,LongTimerProc, sd?sd->handle:0 , 0);
    CtrlEx->WMProcess();
//    if (!threadLong) threadLong=_beginthread(LongThreadProc,0,0);
  }
  return 1;
}

void StopLong(sDIALOG_long * sd) {
  IMLOG("*Stop Long" , longReference);
//  if (sd->flag & DLONG_BLOCKING) {
    longReference--;
    if (longReference<=0)
      CancelWaitableTimer(timerLong);
//  }    
  if (sd->timeoutProc) {
    CancelWaitableTimer(sd->timeoutHandle);
    CloseHandle(sd->timeoutHandle);
    sd->timeoutProc(TIMEOUTT_END , sd);
  }
}

int CALLBACK LongDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   sDIALOG_long * dlg;
   HWND hwnd2;

   switch (message)
   {
        case WM_INITDIALOG:
                SetWindowPos(hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE);
                dlg = (sDIALOG_long*)lParam;
                dlg->cancel = false;
                dlg->handle = hwnd;
                LongDialogProc(hwnd , MYWM_DLGLONGSET , 0 , lParam);
//                SetTimer(hwnd , 1 , 500 , (TIMERPROC)LongTimerProc);
//                SetProp(hwnd , "DLGTIMER" , timer);
				SendMessage(GetDlgItem(hwnd , IDC_PROGRESS) , PBM_SETRANGE32 , 0 , dlg->progress>0 ? dlg->progress : 100);
				if (dlg->progress > 0) dlg->progress = 0;
                break;
        case WM_CLOSE:
//                KillTimer(hwnd , 1);
//                timer = GetProp(hwnd , "DLGTIMER");
//                CancelWaitableTimer(timer);
//                CloseHandle(timer);
                DestroyWindow(hwnd);
                break;
        case WM_DESTROY:
                RemoveProp(hwnd , "DLG");
//                RemoveProp(hwnd , "DLGTIMER");
                break;
        case MYWM_DLGLONGSET:
                if (!wParam) wParam = -1;
                dlg = (sDIALOG_long*)lParam;
                SetProp(hwnd , "DLG" , dlg);
                if (wParam & DSET_TITLE) SetWindowText(hwnd , dlg->title);
                if (wParam & DSET_INFO ) SetDlgItemText(hwnd , IDC_STATIC , dlg->info);
                if (wParam & DSET_BUTTONS) ShowWindow(GetDlgItem(hwnd , IDCANCEL) , dlg->flag & DLONG_CANCEL ? SW_SHOW:SW_HIDE);

                if (wParam & DSET_ANIM)
                  switch (dlg->flag & 0xFF0) {
                    case DLONG_ASEND:
                      Animate_Open(GetDlgItem(hwnd , IDC_ANIMATE) , MAKEINTRESOURCE(IDA_LONGSEND));
                    case DLONG_ARECV:
                      Animate_Open(GetDlgItem(hwnd , IDC_ANIMATE) , MAKEINTRESOURCE(IDA_LONGRECV));
                    case DLONG_AINET:
                      Animate_Open(GetDlgItem(hwnd , IDC_ANIMATE) , MAKEINTRESOURCE(IDA_LONGINET));
                    case 0:
                      Animate_Open(GetDlgItem(hwnd , IDC_ANIMATE) , MAKEINTRESOURCE(IDA_LONG));
//                      HINSTANCE hlib = LoadLibrary("shell32.dll");
//                      Animate_OpenEx(GetDlgItem(hwnd , IDC_ANIMATE) , hlib , MAKEINTRESOURCE(152));
//                      FreeLibrary(hlib);
                    break;
                  }
                if (wParam & DSET_PROGRESS) {
                  hwnd2 = GetDlgItem(hwnd , IDC_PROGRESS);
                  if (dlg->progress>=0 && !IsWindowVisible(hwnd2)) ShowWindow(hwnd2 , SW_SHOW);
                  else if (dlg->progress<0 && IsWindowVisible(hwnd2)) ShowWindow(hwnd2 , SW_HIDE);
                  SendMessage(hwnd2 , PBM_SETPOS , dlg->progress , 0);
                }
                break;
        case WM_COMMAND:
            dlg = (sDIALOG_long*) GetProp(hwnd , "DLG");
            if (HIWORD(wParam)!=BN_CLICKED) return 0;
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    IMLOG("- Cancel@LongProc pressed");
                    if (!dlg->cancelProc) {dlg->cancel = true; ShowWindow(hwnd , SW_HIDE);}
                    else 
                      if (dlg->cancelProc(dlg))
                          dlg->cancel = true;
                    return TRUE;
            }

   }
   return 0;
}
