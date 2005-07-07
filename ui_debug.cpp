#include "stdafx.h"
#pragma hdrstop
#include "ui_main.h"

#include "include\win_listview.h"

#include "ui_actions.h"

int CALLBACK DebugIconsDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

void DebugIcons() {
	HWND hwnd=CreateDialog(Ctrl->hDll() , MAKEINTRESOURCE(IDD_DEBUGICONS)
			, hwndTop , (DLGPROC)DebugIconsDialogProc);
	ShowWindow(hwnd , SW_SHOW);
}

void DebugIconsAdd(HWND item , int id , sUIIcon * ic , string iml) {
		LVITEM lvi;
		lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
		lvi.iItem=0x7FFFFFFF;
		lvi.iSubItem=0;
		CStdString name = "#";
		name += inttoch(id , 16);
		lvi.pszText=(char*)name.c_str();
		lvi.lParam=(LPARAM)id;
		lvi.iImage=ic->index[0] != -1 ? ic->index[0] : 0;
		int pos = ListView_InsertItem(item, &lvi);
		lvi.mask = LVIF_TEXT;
		lvi.pszText=(char*)iml.c_str();
		lvi.iItem=pos;
		lvi.iSubItem=1;
		ListView_SetItem(item, &lvi);
		return;
}

void DebugIconsFill(HWND hwnd) {
	HWND item = GetDlgItem(hwnd , IDC_LIST);
	ListView_AddColumn(item , "ID" , 100);
	ListView_AddColumn(item , "IML" , 80);

	ListView_SetImageList(item , Ico.iml[0] , LVSIL_SMALL);

	for (Ico_it_t i = Ico.Ico.begin(); i != Ico.Ico.end(); i++) {
		sUIIcon * ic = &i->second;
		if (ic->index[0] != -1) DebugIconsAdd(item , i->first , ic , "16");
		if (ic->index[1] != -1) DebugIconsAdd(item , i->first , ic , "32");
		if (ic->ico) DebugIconsAdd(item , i->first , ic , "ico");
		if (ic->ico_) DebugIconsAdd(item , i->first , ic , "ico2");
		if (ic->bmp) DebugIconsAdd(item , i->first , ic , "bmp");
	}
}
void DebugIconsShow(HWND hwnd , HWND item , int pos) {
	int id = (int)ListView_GetItemData(item , pos);
	CStdString info = "";
	CStdString imlt = "";
	ListView_GetItemText(item , pos , 1 , imlt.GetBuffer(10) , 10);
	imlt.ReleaseBuffer();
	IML_enum iml = imlt==""?IML_16 : imlt=="16"?IML_16: imlt=="32"?IML_32: imlt=="ico"?IML_ICO: imlt=="ico2"?IML_ICO2:imlt=="bmp"?IML_BMP:IML_NONE;
	int w , h;
	sUIIcon * ic = &Ico[id];
	ic->measure(iml , &w , &h);	

	info = "ID = 0x";
	info += inttoch(id , 16 , 8 , true);
	info += "\r\niml=\""+imlt+"\"";
    #define UIIcon(type,net,id,frame) (((frame)&0xF) | (((id)&0xFFFF)<<4) | (((net)&0xFF)<<20) | (((type)&0xF)<<28))
	if ((id)&0xF0000000) {
		info += "\r\ntype=\"";
		info += inttoch(((id)&0xF0000000)>>28);
		info += "\"";
		info += " net=\"";
		info += inttoch(((id)&0xFF00000)>>20);
		info += "\"";
		info += " id=\"";
		info += inttoch(((id)&0xFFFF0)>>4);
		info += "\"";
		info += " frame=\"";
		info += inttoch(((id)&0xF));
		info += "\"";
	}
	info += "\r\nRozmiar: ";
	info += inttoch(w);
	info += "x";
	info += inttoch(h);
	SetDlgItemText(hwnd , IDC_EDIT , info);

	HWND bmp = GetDlgItem(hwnd , IDC_STATIC);
	SetProp(bmp , "icoID" , (HANDLE)id);
	SetProp(bmp , "ico" , (HANDLE)ic);
	SetProp(bmp , "icoIML" , (HANDLE) iml);
	SetProp(bmp , "icoW" , (HANDLE)w);
	SetProp(bmp , "icoH" , (HANDLE)h);
	RepaintWindow(bmp);
}
void DebugIconDrawItem(DRAWITEMSTRUCT * ds) {
	FillRect(ds->hDC , &ds->rcItem , GetSysColorBrush(COLOR_BTNFACE));
	RECT rc;
	GetClientRect(ds->hwndItem , &rc);
	sUIIcon * ic = (sUIIcon*)GetProp(ds->hwndItem , "ico");
	if (!ic) return;
	int w = (int)GetProp(ds->hwndItem , "icoW");
	int h = (int)GetProp(ds->hwndItem , "icoH");
	IML_enum iml = (IML_enum)(int)GetProp(ds->hwndItem , "icoIML");
	if (!w || !h || iml==IML_NONE) return;
	ic->draw(iml , ds->hDC , (rc.right - w) / 2 , (rc.bottom - h) / 2);
}

int CALLBACK DebugIconsDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   switch (message)
   {
        case WM_INITDIALOG:
			DebugIconsFill(hwnd);
            break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DRAWITEM:
			if (wParam == IDC_STATIC)
				DebugIconDrawItem((DRAWITEMSTRUCT*)lParam);
			break;
        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             NMLISTVIEW * nmlv;
             switch (nm->code) {
	             case LVN_ITEMCHANGED:
                     nmlv = (NMLISTVIEW *)lParam;
                     if (!(nmlv->uNewState & LVIS_SELECTED)) break;
					 DebugIconsShow(hwnd , nmlv->hdr.hwndFrom , nmlv->iItem);
					 break;
             }
   }
   return 0;
}
