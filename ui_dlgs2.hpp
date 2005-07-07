

// *****************

// *********************************************************



int CALLBACK AboutDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   string str;
   CHARFORMAT cf;
   PARAFORMAT pf;
   HWND item;
   NMHDR * nmhdr;
   switch (message)
   {
        case WM_INITDIALOG: {
                SendMessage(hwnd , WM_SETICON , ICON_SMALL , (LPARAM)Ico[IDI_WND_MAIN].ico);
                SendMessage(hwnd , WM_SETICON , ICON_BIG , (LPARAM)Ico[IDI_WND_MAIN].ico_);
                SendDlgItemMessage(hwnd , IDC_ICON , STM_SETICON , (LPARAM)Ico[IDI_WND_MAIN].ico_ , 0);
                ICMessage(IMC_VERSION , (int)TLS().buff);
                str = TLS().buff;
                GetDlgItemText(hwnd , IDC_STATIC , TLS().buff , MAX_STRBUFF);
                SetDlgItemText(hwnd , IDC_STATIC , _sprintf(TLS().buff , str.c_str()));
                item = GetDlgItem(hwnd , IDC_EDIT);

                SendMessage(item , EM_SETBKGNDCOLOR , 0 , (LPARAM)GetSysColor(COLOR_BTNFACE));
                SendMessage(item , EM_AUTOURLDETECT , 1 , 0);
                SendMessage(item , EM_SETEVENTMASK  , 0 , ENM_SELCHANGE | ENM_LINK);
                //  Wypelnianie informacji
                FILEVERSIONINFO vi(true);
                FileVersionInfo((char*)ICMessage(IMC_ARGV) , 0 , &vi);
                pf.cbSize = sizeof(pf);
                cf.cbSize = sizeof(cf);
#define RE_ALIGNMENT(al) pf.dwMask = PFM_ALIGNMENT;pf.wAlignment = al;SendMessage(item , EM_SETPARAFORMAT , 0 , (LPARAM)&pf)
#define RE_BOLD(bo) cf.dwMask = CFM_BOLD;cf.dwEffects = bo?CFM_BOLD:0;SendMessage(item , EM_SETCHARFORMAT , SCF_SELECTION , (LPARAM)&cf)
#define RE_ADD(txt) SendMessage(item , EM_REPLACESEL , 0 , (LPARAM)(string(txt).c_str()))
#define RE_ADDLINE(txt) RE_ADD(string(txt) + '\n')
                RE_ALIGNMENT(PFA_LEFT);
                RE_BOLD(0);
                if (vi.CompanyName!="") RE_ADDLINE(vi.CompanyName);
                if (vi.LegalCopyright!="") RE_ADDLINE(vi.LegalCopyright);
                if (vi.LegalTrademarks!="") RE_ADDLINE(vi.LegalTrademarks);
                if (vi.Comments!="") RE_ADDLINE(vi.Comments);
                if (vi.URL!="") RE_ADDLINE("URL : " + vi.URL);

                RE_ALIGNMENT(PFA_CENTER);
                RE_BOLD(1);
                RE_ADDLINE("\nWtyczki\n");
                RE_ALIGNMENT(PFA_LEFT);
                int i , c;
                c = ICMessage(IMC_PLUG_COUNT);
                for (i = 0 ; i < c ; i++) {
                  FileVersionInfo((char*)ICMessage(IMC_PLUG_FILE , i) , 0 , &vi);
                  RE_BOLD(1);
                  CStdString fileName = (char*)ICMessage(IMC_PLUG_FILE , i);

                  RE_ADD(fileName.substr(fileName.find('\\')+1));
                  RE_ADD("   ");
                  RE_ADDLINE(vi.FileDescription);
                  RE_BOLD(0);
                  RE_ADDLINE(vi.FileVersion);
                  if (vi.CompanyName!="") RE_ADDLINE(vi.CompanyName);
                  if (vi.LegalCopyright!="") RE_ADDLINE(vi.LegalCopyright);
                  if (vi.LegalTrademarks!="") RE_ADDLINE(vi.LegalTrademarks);
                  if (vi.Comments!="") RE_ADDLINE(vi.Comments);
                  if (vi.URL!="") RE_ADDLINE("URL : " + vi.URL);
                  RE_ADD("\n");
                }
                SendMessage(item , EM_SETSEL , 0 , 0);
                SetFocus(item);
                HideCaret(item);

#undef  RE_ALIGNMENT
#undef  RE_BOLD
#undef  RE_ADD
#undef  RE_ADDLINE

                break;}
        case WM_CLOSE:
                //EndDialog(hwnd , IDOK);
				DestroyWindow(hwnd);
                break;
        case WM_COMMAND:
            if (HIWORD(wParam)!=BN_CLICKED) return 0;
            switch (LOWORD(wParam))
            {
                case IDOK:
					DestroyWindow(hwnd);
    //                EndDialog(hwnd , IDOK);
                    return TRUE;
            }
            break;
        case WM_NOTIFY:
                nmhdr=(NMHDR *)lParam;
                switch (nmhdr->code) {
                   case EN_SELCHANGE:
//                      SendMessage(nmhdr->hwndFrom , EM_HIDESELECTION , 1 , 0);
                      HideCaret(nmhdr->hwndFrom);
                      break;
                   case EN_LINK:
                      ENLINK* enl;
                      enl = (ENLINK*)lParam;
                      if (enl->msg == WM_LBUTTONUP) {
                        TEXTRANGE tr;
                        tr.chrg = enl->chrg;
                        tr.lpstrText = TLS().buff;
                        SendMessage(nmhdr->hwndFrom , EM_GETTEXTRANGE  , 0 , (LPARAM)&tr);
                        if (*TLS().buff)
                            ShellExecute(0 , "open" , TLS().buff , "" , "" , SW_SHOW);

                      }  
                      break;
                }
                break;

   }
   return 0;
}

void SearchDialogUID(HWND hwnd) {
    GetDlgItemText(hwnd , IDE_UID , TLS().buff , MAX_STRBUFF);
    int state = !*TLS().buff;
    if (IsWindowEnabled(GetDlgItem(hwnd , IDE_NAME))==state) return;
    EnableWindow(GetDlgItem(hwnd , IDE_NAME) , state);
    EnableWindow(GetDlgItem(hwnd , IDE_SURNAME) , state);
    EnableWindow(GetDlgItem(hwnd , IDE_NICK) , state);
    EnableWindow(GetDlgItem(hwnd , IDE_CITY) , state);
    EnableWindow(GetDlgItem(hwnd , IDE_EMAIL) , state);
    EnableWindow(GetDlgItem(hwnd , IDE_GENDER) , state);
    EnableWindow(GetDlgItem(hwnd , IDE_BORN) , state);
}


void SearchDialogChange(HWND hwnd) {
    SetProp(hwnd , "STARTFROM" , 0);
    SetWindowText(GetDlgItem(hwnd , IDOK) , "Szukaj");
}


void SearchDialogColumns(HWND hwnd) {
    ListView_SetExtendedListViewStyle(hwnd , LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP/* | LVS_EX_TRACKSELECT*/);
    EnableWindow(hwnd , 0);
    LVCOLUMN lvc;
    struct {
      char * txt; int cx;
    } Columns [] = {{"UID" , 70}
                   ,{"Pseudonim" , 70}
                   ,{"Imiê i Nazwisko" , 100}
                   ,{"Kontakt" , 70}
                   ,{"Miasto"  , 50}
                   ,{"P³eæ / Wiek" , 50}
                   ,{"Inne" , 70}
                   };
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    for (int i = 0; i < 7; i++)
	{
        lvc.iSubItem = i;
        lvc.pszText = Columns[i].txt;
        lvc.cx = Columns[i].cx;
        lvc.fmt = LVCFMT_LEFT;
        if (ListView_InsertColumn(hwnd, i, &lvc) == -1)
            break;
    }

}

void SearchDialogSearch(HWND hwnd) {
  char * buff = TLS().buff;
  sCNTSEARCH src;
  src.handle = hwnd;
  src.status = IsDlgButtonChecked(hwnd , IDC_CHECK);
  src.gender = SendDlgItemMessage(hwnd , IDE_GENDER , CB_GETCURSEL , 0 , 0);
  GetDlgItemText(hwnd , IDE_UID , src.uid , sizeof(src.uid));
  GetDlgItemText(hwnd , IDE_NAME , src.name , sizeof(src.name));
  GetDlgItemText(hwnd , IDE_SURNAME , src.surname , sizeof(src.surname));
  GetDlgItemText(hwnd , IDE_NICK , src.nick , sizeof(src.nick));
  GetDlgItemText(hwnd , IDE_CITY , src.city , sizeof(src.city));
  GetDlgItemText(hwnd , IDE_EMAIL , src.email , sizeof(src.email));
  if (!strchr(src.email , '@')) {strcpy(src.phone , src.email); src.email[0]=0;}

// Obliczanie wieku

  GetDlgItemText(hwnd , IDE_BORN , buff , MAX_STRBUFF);
  removeChar(buff , ' ');
  SYSTEMTIME st;
  GetLocalTime(&st);
  int year = st.wYear;
  src.born_min = src.born_max = 0;
  int pos;
  int age1 , age2;
  if (*buff == '<' && (age1=atoi(buff+1))!=0) {src.born_min = year - age1; src.born_max = year;}
  else
  if (*buff == '>' && (age1=atoi(buff+1))!=0) {src.born_min = 1900; src.born_max = year - age1;}
  else if ((pos=(int)strchr(buff , '-')-(int)buff) > 0) {
		age1=atoi(buff);
		age2=atoi(buff+pos+1);
		if (age1>0 && age2>0 && age2>age1) {
			src.born_min = year - age2; src.born_max = year - age1;
		}
  } else if ((age1=atoi(buff))!=0) {
	  src.born_min = src.born_max = year - age1;
  }   
  // sprawdzanie sieci
  COMBOBOXEXITEM cbi;
  cbi.iItem = SendDlgItemMessage(hwnd , IDE_NET , CB_GETCURSEL , 0 , 0);
  cbi.mask = CBEIF_LPARAM;
  SendDlgItemMessage(hwnd , IDE_NET , CBEM_GETITEM , 0 , (LPARAM)&cbi);

// Wysylanie zapytania
  ListView_DeleteAllItems(GetDlgItem(hwnd , IDC_LIST));
  if (!(*src.uid || *src.name || *src.surname || *src.nick || *src.city || *src.email || *src.phone || src.gender || src.born_min || src.born_max)) {
     MessageBox(hwnd , "Musisz podaæ kryteria wyszukiwania!" , "Szukaj kontaktów" , MB_ICONERROR);
     return;}
  src.start = (int)GetProp(hwnd , "STARTFROM");
  EnableWindow(hwnd , false);
  EnableWindow(GetDlgItem(hwnd , IDC_LIST) , 1);
  EnableWindow(GetDlgItem(hwnd , IDYES) , 0);
  EnableWindow(GetDlgItem(hwnd , IDOK) , 0);
  src.net = cbi.lParam;
  src.handle = hwnd;
  IMessage(IM_CNT_SEARCH , cbi.lParam , IMT_PROTOCOL , (int)&src);
  EnableWindow(hwnd , true);
  SetForegroundWindow(hwnd);
  SetActiveWindow(hwnd);

  EnableWindow(GetDlgItem(hwnd , IDOK) , 1);
}

void SearchDialogAdd(HWND hwndDlg , sCNTSEARCH * res) {
  EnableWindow(GetDlgItem(hwndDlg , IDYES) , 1);
  HWND hwnd = GetDlgItem(hwndDlg , IDC_LIST);
  LVITEM lvi;
  lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
  sCNTSEARCH * lParam = new sCNTSEARCH;
  memcpy(lParam , res , sizeof(*lParam));
  lvi.lParam = (LPARAM)lParam;
  lvi.pszText = res->uid;
  lvi.iItem=0xFFFF;
  lvi.iSubItem=0;
  lvi.iImage = Ico[UIIcon(IT_STATUS , res->net , res->status , 0)].index[0];
  int pos = ListView_InsertItem(hwnd, &lvi);
  lvi.mask = LVIF_TEXT;
  lvi.iItem=pos;
  // wszystkie inne
  lvi.iSubItem=1;
  lvi.pszText=res->nick;
  ListView_SetItem(hwnd, &lvi);
  lvi.iSubItem=2;
  lvi.pszText=(char*)_sprintf("%s %s" , res->name , res->surname);
  ListView_SetItem(hwnd, &lvi);
  lvi.iSubItem=3;
  bool both = *res->email&&*res->phone;
  lvi.pszText=(char*)_sprintf("%s%s%s" , *res->email?res->email:res->phone , both?" / ":"" , both?res->phone:"");
  ListView_SetItem(hwnd, &lvi);
  lvi.iSubItem=4;
  lvi.pszText=res->city;
  ListView_SetItem(hwnd, &lvi);
  lvi.iSubItem=5;
  
  SYSTEMTIME st;
  GetLocalTime(&st);
  int year = st.wYear;
  strcpy(TLS().buff , inttoch(year - res->born_min));
  lvi.pszText=(char*)_sprintf("%s%s" , res->gender==1?"K ":res->gender==2?"M ":"" , res->born_min?TLS().buff:"");
  ListView_SetItem(hwnd, &lvi);
  lvi.iSubItem=6;
  lvi.pszText=res->other;
  ListView_SetItem(hwnd, &lvi);
  // sort
  SendMessage(hwnd , MYWM_REFRESH , 0 , 0);
  if (res->start) {
      SetProp(hwndDlg , "STARTFROM" , (HANDLE)res->start);
      SetWindowText(GetDlgItem(hwndDlg , IDOK) , "Dalej >>>");
  }
}

void SearchDialogAddCnt(HWND hwndDlg , int item) {
  HWND hwnd = GetDlgItem(hwndDlg , IDC_LIST);

  LVITEM lvi;
  lvi.mask = LVIF_PARAM;
  lvi.iItem = item;
  lvi.iSubItem = 0;
  ListView_GetItem(hwnd , &lvi);
  sCNTSEARCH* cnt = (sCNTSEARCH*)lvi.lParam;

  if (ICMessage(IMC_CNT_FIND , cnt->net , (int)cnt->uid) >=0) return;

  int pos = ICMessage(IMC_CNT_ADD , cnt->net , (int)cnt->uid);

  SETCNTC (pos , CNT_NAME , cnt->name);
  SETCNTC (pos , CNT_SURNAME , cnt->surname);
  SETCNTC (pos , CNT_NICK , cnt->nick);
  SETCNTC (pos , CNT_DISPLAY , *cnt->nick?cnt->nick:*cnt->surname?cnt->surname:cnt->uid);
  SETCNTC (pos , CNT_EMAIL , cnt->email);
  SETCNTC (pos , CNT_CELLPHONE , cnt->phone);
  SETCNTC (pos , CNT_CITY , cnt->city);
  SETCNTI (pos , CNT_BORN , (WORD)cnt->born_min >> 16);
  SETCNTC (pos , CNT_GROUP , GETSTR(CFG_CURGROUP));
//  IMLOG("%s - rok = %d" , *cnt->nick?cnt->nick:*cnt->surname?cnt->surname:cnt->uid , (WORD)cnt->born_min << 16);
  SETCNTI (pos , CNT_GENDER , cnt->gender);
  ICMessage(IMC_CNT_CHANGED , pos);
}

int CALLBACK ListViewCompare(LPARAM p1, LPARAM p2,LPARAM lParam) {
  __int64 big;
  big = *((__int64*)lParam);
  HWND hwnd = (HWND)(big >> 32);
  int sort = (__int32)big;
  int dir = sort>0?1:-1;
  sort = abs(sort)-1;
  char * buff = TLS().buff;
  char * buff2 = TLS().buff2;
  ListView_GetItemText(hwnd , p1 , sort , buff , MAX_STRING2);
  ListView_GetItemText(hwnd , p2 , sort , buff2 , MAX_STRING2);
  if (!min(*buff , *buff2)) return *buff?-1:1;
  return dir * stricmp(buff , buff2);
}

int CALLBACK SearchDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   string str;
   NMHDR * nmhdr;
   HWND item;
   int i , b , c;
   static HIMAGELIST himl;
   static sort = 1;
   __int64 big;
   switch (message)
   {
        case WM_INITDIALOG:
                SendMessage(hwnd , WM_SETICON , ICON_SMALL , (LPARAM)Ico[IDI_WND_SEARCH].ico);
                SendMessage(hwnd , WM_SETICON , ICON_BIG , (LPARAM)Ico[IDI_WND_SEARCH].ico_);
                SearchDialogUID(hwnd);
                himl = ListView_SetImageList(GetDlgItem(hwnd , IDC_LIST) , Ico.iml[0] , LVSIL_SMALL);
                // Sieci
                item=GetDlgItem(hwnd , IDE_NET);
                NetFillIn(item , IMT_NETSEARCH);
                // Plec
                item=GetDlgItem(hwnd , IDE_GENDER);
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"p³eæ [wybierz]");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"Kobieta");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"Mê¿czyzna");
                SendMessage(item , CB_SETCURSEL , 0 , 0);
                // Wiek
                item=GetDlgItem(hwnd , IDE_BORN);
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"<14");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"15-18");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"18-25");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"25-30");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"30-40");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)"40-50");
                SendMessage(item , CB_ADDSTRING , 0 , (LPARAM)">50");
                SetWindowText(item , "wiek [wybierz/wpisz]");
                // Kolumny
                SearchDialogColumns(GetDlgItem(hwnd , IDC_LIST));
                break;
        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
        case WM_DESTROY:
                ListView_SetImageList(GetDlgItem(hwnd , IDC_LIST) , himl , LVSIL_SMALL);
                hwndSearch = 0;
                break;
        case WM_COMMAND:
            switch (HIWORD(wParam)) {
            case BN_CLICKED: 
              switch (LOWORD(wParam))
              {
                  case IDCANCEL:
                      DestroyWindow(hwnd);
                      return TRUE;
                  case IDOK:
                      SearchDialogSearch(hwnd);
                      return TRUE;
                  case IDYES:
                      c = ListView_GetItemCount(GetDlgItem(hwnd , IDC_LIST));
                      b = 0;
                      for (i = 0 ; i < c; i++) {
                        if (ListView_GetItemState(GetDlgItem(hwnd , IDC_LIST) , i , LVIS_SELECTED)) {
                          SearchDialogAddCnt(hwnd , i);
                          b++;
                        }
                      }
                      ICMessage(IMC_SAVE_CNT);
                  case IDC_CHECK:
                      SearchDialogChange(hwnd);
                      return TRUE;
                      //ICMessage(IMI_REFRESH_LST);
                      //if (b) EndDialog(hwnd , IDYES);
                      return TRUE;
              }
              break;
            case CBN_SELCHANGE:
                SearchDialogChange(hwnd);
                return 1;
            case EN_CHANGE:
              switch (LOWORD(wParam))
              {

                  case IDE_UID:
                      SearchDialogUID(hwnd);
                  case IDE_NAME:case IDE_SURNAME:case IDE_NICK:case IDE_EMAIL:
                  case IDE_CITY:
                      SearchDialogChange(hwnd);
                      return TRUE;
              }
              break;
            }
            break;
        case MYWM_REFRESH:
            big = (((__int64)GetDlgItem(hwnd,IDC_LIST)) << 32) | sort;
            ListView_SortItemsEx(GetDlgItem(hwnd,IDC_LIST) , ListViewCompare , (LPARAM)&big);
            break;
        case WM_NOTIFY:
                nmhdr=(NMHDR *)lParam;
                NMLISTVIEW * nmlv;
                LVITEM lvi;
                switch (nmhdr->code) {
                   case LVN_COLUMNCLICK:
                     nmlv = (NMLISTVIEW*)lParam;
                     if (sort == nmlv->iSubItem+1) sort = -sort;
                       else sort = nmlv->iSubItem+1;
                     big = ((__int64)GetDlgItem(hwnd,IDC_LIST) << 32) | sort;
                     ListView_SortItemsEx(GetDlgItem(hwnd,IDC_LIST) , ListViewCompare , (LPARAM)&big);
                     break;
                   case LVN_INSERTITEM:
//                     big = (((__int64)GetDlgItem(hwnd,IDC_LIST)) << 32) | sort;
//                     ListView_SortItemsEx(GetDlgItem(hwnd,IDC_LIST) , ListViewCompare , (LPARAM)&big);
                     break;
                   case LVN_DELETEITEM:
                     nmlv = (NMLISTVIEW*)lParam;
                     memset(&lvi , 0 , sizeof(lvi));
                     lvi.mask = LVIF_PARAM;
                     lvi.iItem = nmlv->iItem;
                     lvi.iSubItem = 0;
                     ListView_GetItem(GetDlgItem(hwnd,IDC_LIST) , &lvi);
                     delete (sCNTSEARCH*)lvi.lParam;
                     break;
                }
                break;


   }
   return 0;
}


// --------------------------------------------

void GroupsDialogShow(bool show) {
  static HWND hwnd = 0;
  if (show) {
    if (!hwnd) {
      hwnd=CreateDialog(Ctrl->hDll() , MAKEINTRESOURCE(IDD_GROUPS)
         , hwndTop , (DLGPROC)GroupsDialogProc);
      ShowWindow(hwnd , SW_SHOW);
    } else SetForegroundWindow(hwnd);
  } else hwnd = 0;
}

void GroupsDialogFill(HWND hwnd) {
   HWND item = GetDlgItem(hwnd , IDC_LIST);
   ListView_DeleteAllItems(item);
   char * grps2 = strdup(GETSTR(CFG_GROUPS)) , * grp = grps2 , * grps = grps2;
   while ((grps = strchr(grps , '\n'))!=0) {
      *grps = 0;
      grps++;
      if (*grp) ListView_AddString(item , grp , Ico[UIIcon(IT_LOGO , NET_NONE , 0 , 0)].index[0]);
      grp = grps;
   }
   free(grps2);
   ListView_SetColumnWidth(item , 0 , LVSCW_AUTOSIZE_USEHEADER);
   
}

int CALLBACK GroupsDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   string str;
   NMHDR * nmhdr;
   HWND item;
   int i ,c;
   char * ch;
   switch (message)
   {
        case WM_INITDIALOG:
//                SendMessage(hwnd , WM_SETICON , ICON_SMALL , (LPARAM)Ico[IDI_WND_SEARCH].ico);
//                SendMessage(hwnd , WM_SETICON , ICON_BIG , (LPARAM)Ico[IDI_WND_SEARCH].ico_);
                ListView_AddColumn(GetDlgItem(hwnd , IDC_LIST) , "Nazwy" , 100);
                SendDlgItemMessage(hwnd , IDC_LIST , LVM_SETEXTENDEDLISTVIEWSTYLE , 0 ,
                      LVS_EX_FULLROWSELECT);
                GroupsDialogFill(hwnd);
                break;
        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
        case WM_DESTROY:
                GroupsDialogShow(false);
                break;
        case WM_COMMAND:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
              switch (LOWORD(wParam))
              {
                  case IDOK:
                      DestroyWindow(hwnd);
                      return TRUE;
                  case IDYES:
                      ch = strdup(_sprintf("grupa %d" , max(1,strcount(GETSTR(CFG_GROUPS),'\n'))));
//                      IMLOG(":: %s , %d" ,GETSTR(CFG_GROUPS) , strcount(GETSTR(CFG_GROUPS),'\n'));
                      ICMessage(IMC_GRP_ADD , (int)ch);
                      GroupsDialogFill(hwnd);
                      // Wlaczamy edycje
                      item = GetDlgItem(hwnd , IDC_LIST);
                      c = ListView_GetItemCount(item);
                      for (i = 0; i<c; i++) {
                          ListView_GetItemText(item , i , 0 , TLS().buff , MAX_STRING);
                          if (!strcmp(TLS().buff , ch)) {
                              SetFocus(item);
                              ListView_EditLabel(item , i);
                              break;
                          }
                      }

                      free(ch);
                      return TRUE;
                  case IDNO:
                      item = GetDlgItem(hwnd , IDC_LIST);
                      c = ListView_GetItemCount(item);
                      for (i = 0; i<c; i++) {
                          if (ListView_GetItemState(item , i , LVIS_SELECTED)) {
                            ListView_GetItemText(item , i , 0 , TLS().buff , MAX_STRING);
                            ICMessage(IMC_GRP_DEL , (int)TLS().buff);
                          }
                      }
                      GroupsDialogFill(hwnd);
                      return TRUE;
                  case IDC_EDIT:  // Edycja
                      item = GetDlgItem(hwnd , IDC_LIST);
                      c = ListView_GetItemCount(item);
                      for (i = 0; i<c; i++) {
                          if (ListView_GetItemState(item , i , LVIS_SELECTED)) {
                              SetFocus(item);
                              ListView_EditLabel(item , i);
                              break;
                          }
                      }
                      return TRUE;
              }
              break;
            }
            break;
        case WM_NOTIFY:
                nmhdr=(NMHDR *)lParam;
                NMLVDISPINFO * nmdi;
                switch (nmhdr->code) {
                  case LVN_ENDLABELEDIT:
                     nmdi = (NMLVDISPINFO*)lParam;
                     ListView_GetItemText(nmdi->hdr.hwndFrom , nmdi->item.iItem , 0 , TLS().buff , MAX_STRING);
                     if (!nmdi->item.pszText || !*nmdi->item.pszText || !*TLS().buff) return false;
                     ICMessage(IMC_GRP_RENAME , (int)TLS().buff , (int)nmdi->item.pszText);
                     GroupsDialogFill(hwnd);
                     return false;
                }
                break;


   }
   return 0;
}



// --------------------------------------------

void IgnoreDialogShow(bool show) {
  static HWND hwnd = 0;
  if (show) {
    if (!hwnd) {
      hwnd=CreateDialog(Ctrl->hDll() , MAKEINTRESOURCE(IDD_IGNORE)
         , hwndTop , (DLGPROC)IgnoreDialogProc);
      ShowWindow(hwnd , SW_SHOW);
    } else SetForegroundWindow(hwnd);
  } else hwnd = 0;
}

void IgnoreDialogFill(HWND hwnd) {
   HWND item = GetDlgItem(hwnd , IDC_LIST);
   ListView_DeleteAllItems(item);
   char * grps2 = strdup(GETSTR(CFG_IGNORE)) , * grp = grps2 , * grps = grps2;
//   IMLOG("IGNORE [%s]" , grps);
   int group = SendDlgItemMessage(hwnd , IDE_NET , CB_GETITEMDATA , SendDlgItemMessage(hwnd , IDE_NET , CB_GETCURSEL , 0 , 0) , 0);
   char * check = strdup (_sprintf("%d@" , group));
   while ((grps = strchr(grps , '\n'))!=0) {
      *grps = 0;
      grps++;
      if (*grp && strstr(grp , check)) {
         grp = strchr(grp , '@')+1;
         int it = ListView_AddString(item , grp , Ico[UIIcon(IT_LOGO , group , 0 , 0)].index[0]);
         int pos;
         if ((pos = ICMessage(IMC_FINDCONTACT , group , (int)grp))>0)
           ListView_SetString(item , it , 1 , (char*)GETCNTC(pos , CNT_DISPLAY));
      }
      grp = grps;
   }
   free(grps2);
   free(check);
   ListView_SetColumnWidth(item , 0 , LVSCW_AUTOSIZE_USEHEADER);


}

void IgnoreDialogCheck(HWND hwnd) {
   GetDlgItemText(hwnd , IDE_UID , TLS().buff , MAX_STRING);
   EnableWindow(GetDlgItem(hwnd , IDYES) , *TLS().buff?TRUE : FALSE);
}

int CALLBACK IgnoreDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   string str;
   NMHDR * nmhdr;
   HWND item;
   int i , c;
   switch (message)
   {
        case WM_INITDIALOG:
                ListView_AddColumn(GetDlgItem(hwnd , IDC_LIST) , "UIN" , 100);
                ListView_AddColumn(GetDlgItem(hwnd , IDC_LIST) , "Opis" , 80);
                SendDlgItemMessage(hwnd , IDC_LIST , LVM_SETEXTENDEDLISTVIEWSTYLE , 0 ,
                      LVS_EX_FULLROWSELECT);
                ListView_SetImageList(GetDlgItem(hwnd , IDC_LIST) , Ico.iml[0] , LVSIL_SMALL);
                NetFillIn(GetDlgItem(hwnd , IDE_NET) , IMT_NETUID);
                IgnoreDialogFill(hwnd);
                IgnoreDialogCheck(hwnd);
                break;
        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
        case WM_DESTROY:
                IgnoreDialogShow(false);
                break;
        case WM_COMMAND:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
              switch (LOWORD(wParam))
              {
                  case IDOK:
                      DestroyWindow(hwnd);
                      return TRUE;
                  case IDYES:
                      int group;
                      group = SendDlgItemMessage(hwnd , IDE_NET , CB_GETITEMDATA , SendDlgItemMessage(hwnd , IDE_NET , CB_GETCURSEL , 0 , 0) , 0);
                      GetDlgItemText(hwnd , IDE_UID , TLS().buff , MAX_STRING);
                      if (!*TLS().buff) return TRUE;
                      ICMessage(IMC_IGN_ADD , group , (int)TLS().buff);
                      IgnoreDialogFill(hwnd);
                      return TRUE;
                  case IDNO:
                      item = GetDlgItem(hwnd , IDC_LIST);
                      group = SendDlgItemMessage(hwnd , IDE_NET , CB_GETITEMDATA , SendDlgItemMessage(hwnd , IDE_NET , CB_GETCURSEL , 0 , 0) , 0);
                      c = ListView_GetItemCount(item);
                      for (i = 0; i<c; i++)
                        if (ListView_GetItemState(item , i , LVIS_SELECTED)) {
                            ListView_GetItemText(item , i , 0 , TLS().buff , MAX_STRING);
                            if (*TLS().buff)
                              ICMessage(IMC_IGN_DEL , group , (int)TLS().buff);
                        }
                      IgnoreDialogFill(hwnd);
                      return TRUE;
              }
              break;
            case EN_CHANGE:
              switch (LOWORD(wParam))
              {
                  case IDE_UID:
                      IgnoreDialogCheck(hwnd);
                      return TRUE;
              }
              break;
            }
			case CBN_SELENDOK:
				IgnoreDialogFill(hwnd);
				break;

            break;
        case WM_NOTIFY:
                nmhdr=(NMHDR *)lParam;
                switch (nmhdr->code) {
                   case LVN_COLUMNCLICK:
                     break;
                }
                break;


   }
   return 0;
}



// --------------------------------------

void ServerEventDialogNext() {
   if (!hwndMsgEvent) return;
   sMESSAGEWAITING mw;
   mw.net = -1;
   mw.uid = 0;
   mw.type = MT_SERVEREVENT;
   EnableWindow(GetDlgItem(hwndMsgEvent , IDYES) , ICMessage(IMC_MESSAGEWAITING , (int)&mw , 0)>0);
}
void ServerEventDialogFill() {
   if (!hwndMsgEvent) return;
   cMessage m;
   sMESSAGEPOP mp;
   mp.net = 0;
   mp.uid = "";
   mp.type = MT_SERVEREVENT;
   if (!ICMessage(IMC_MESSAGEGET , (int)&mp , (int)&m)) return;
   SetDlgItemText(hwndMsgEvent , IDC_MSG , m.body);
   ICMessage(IMC_MESSAGEREMOVE , (int)&mp);
   ServerEventDialogNext();
   ICMessage(IMC_MESSAGEQUEUE , (int)&sMESSAGESELECT(0 , "" , MT_SERVEREVENT , 0 , MF_SEND));
}

int CALLBACK ServerEventDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
//   static char str_buff2 [MAX_STRBUFF];
   switch (message)
   {
        case WM_INITDIALOG:
                SendDlgItemMessage(hwnd , IDC_MSG , WM_SETFONT , (LPARAM)fontBig , 1);
                SendDlgItemMessage(hwnd , IDC_MSG , EM_SETBKGNDCOLOR , 0 , (LPARAM)GetSysColor(COLOR_BTNFACE));
                SendDlgItemMessage(hwnd , IDC_MSG , EM_AUTOURLDETECT , 1 , 0);
                SendDlgItemMessage(hwnd , IDC_MSG , EM_SETEVENTMASK  , 0 , /*ENM_SELCHANGE |*/ ENM_LINK);

                hwndMsgEvent = hwnd;
                ServerEventDialogFill();
                break;
        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
        case WM_NCDESTROY:
                hwndMsgEvent = 0;
                break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                   DestroyWindow(hwnd);
                   break;
                case IDYES:
                   ServerEventDialogFill();
            }
            return 1;

        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             switch (nm->code) {
               case EN_LINK:
                    ENLINK* enl;
                    enl = (ENLINK*)lParam;
                    if (enl->msg == WM_LBUTTONUP) {
                      TEXTRANGE tr;
                      tr.chrg = enl->chrg;
                      tr.lpstrText = TLS().buff;
                      SendMessage(nm->hwndFrom , EM_GETTEXTRANGE  , 0 , (LPARAM)&tr);
                      if (*TLS().buff)
                          ShellExecute(0 , "open" , TLS().buff , "" , "" , SW_SHOW);
                     }
                    break;
             }
             return 0;
   }
   return 0;
}


