#include "stdafx.h"
#include "ui_main.h"
#include "res\ui\resource_ui.h"
#include "res\ui\resource.h"
#include "dlgShowBits.h"
#include "ui_actions.h"

using namespace Konnekt::ShowBits;

Dialog * openedDialog = 0;

Dialog * Dialog::getDialogFromWindow(HWND hwnd) {
	return reinterpret_cast<Dialog *>(GetProp(hwnd, "Dialog*"));
}
void Dialog::initDialog(HWND hwnd) {
	/*TODO: ...wgranie grafiki... */
	this->showBits = GETINT(CFG_SHOWBITS);
	this->showBitsByHand = GETINT(CFG_SHOWBITS_BYHAND);
	this->hwnd = hwnd;
	this->isExpanded = true;
	if (this->isModal) {
		ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
	}
	HWND item;
	item = GetDlgItem(hwnd, IDC_SLIDER);
	SendMessage(item, TBM_SETRANGE, 0, MAKELONG(1,5));
	item = GetDlgItem(hwnd, IDC_SLIDERTIPS);
	SendMessage(item, TBM_SETRANGE, 0, MAKELONG(8,11));
	item = GetDlgItem(hwnd, IDC_SLIDERINFO);
	SendMessage(item, TBM_SETRANGE, 0, MAKELONG(12,15));
	item = GetDlgItem(hwnd, IDC_SLIDERWIZARDS);
	SendMessage(item, TBM_SETRANGE, 0, MAKELONG(16,19));
	cRichEditHtml::Init(GetDlgItem(hwnd, IDC_INFO));

	item = GetDlgItem(hwnd, IDC_IMAGE);
	int imageType;
	HANDLE image = Ico.getImageFromURL("file://%KonnektData%/img/ui_showbits.png", imageType);
	if (imageType == IMAGE_BITMAP) {
		BlendBitmapWithColor((HBITMAP)image, GetSysColor(COLOR_BTNFACE));
	}
	SetWindowLong(item, GWL_STYLE, (GetWindowLong(item, GWL_STYLE) & ~(SS_ICON | SS_BITMAP)) | (imageType==IMAGE_BITMAP ? SS_BITMAP : SS_ICON));
	SendMessage(item, STM_SETIMAGE, imageType, (LPARAM)image);

	this->updateLevel((enLevel) (showBits & levelFull));
	this->updateBits(-1, 0, false);
	this->expand(false);
}

void Dialog::updateLevel(enLevel level) {
	if (level == 0) level = levelBeginner;
	// ustawiamy level...
	this->showBits &= ~levelFull;
	this->showBits |= level;
	int bits = 0;
	int mask = showTooltips | showInfo | showWizards;
	CStdString info, name;
	switch(level) {
		case levelBeginner: 
			name = "Pocz¹tkuj¹cy";
			info = "idealny na pocz¹tek. Wiele dodatkowych informacji i tylko"
				" najbardziej potrzebne ustawienia, pozwol¹ przyjemniej" 
				" oswoiæ siê z Konnekt'em..."
				;
			bits = showTooltipsBeginner | showInfoBeginner | showWizardsBeginner;
			break;
		case levelIntermediate: 
			name = "Prosty";
			info = "je¿eli nie lubisz byæ traktowany jak \"pocz¹tkuj¹cy\", a lubisz prost¹"
				" obs³ugê - ten poziom jest dla Ciebie."
				;
			bits = showTooltipsBeginner | showInfoBeginner | showWizardsNormal;
			break;
		case levelNormal: 
			name = "Normalny";
			info = "dostêpne wszystkie przydatne opcje, garœæ podpowiedzi,"
				" trochê kreatorów, to to co <i>normalny</i> u¿ytkownik lubi najbardziej."
				;
			bits = showTooltipsNormal | showInfoNormal | showWizardsNormal;
			break;
		case levelAdvanced: 
			name = "Zaawansowany";
			info = "poziom dla zaawansowanych u¿ytkowników. Wszystkie opcje dostêpne,"
				" plus najbardziej przydatne u³atwienia i podpowiedzi.";
			bits = showTooltipsAdvanced | showInfoAdvanced | showWizardsAdvanced;
			break;
		case levelPro: 
			name = "Pro";
			info = "uwa¿asz, ¿e Konnekt'a masz w ma³ym palcu? Absolutnie"
				" wszystkie opcje dostêpne, oprócz"
				" (zbêdnych oczywiœcie ;)) kreatorów i podpowiedzi."
				;
			bits = 0;
			break;
	}
	cRichEditHtml::SetHTML(GetDlgItem(this->hwnd, IDC_INFO), "<b>" +name+ "</b>  -  "+info+"");
	this->updateBits(mask, bits, false);
}
void Dialog::updateBits(int changed, int bits, bool byHand) {
	if (changed != -1 || bits) {
		if (byHand) {
			this->showBits &= ~changed;
			this->showBits |= bits & changed;
			this->showBitsByHand |= changed;
		} else {
			// czyœcimy niepotrzebne bity...
			this->showBits &= this->showBitsByHand | ~changed;
			// ustawiamy nowe...
			this->showBits |= bits & ~this->showBitsByHand & changed;
		}
	}
	this->updateBits(changed, IDC_SLIDER, showLevel);
	this->updateBits(changed, IDC_SLIDERTIPS, showTooltips);
	this->updateBits(changed, IDC_SLIDERINFO, showInfo);
	this->updateBits(changed, IDC_SLIDERWIZARDS, showWizards);
}
void Dialog::updateBits(int changed, int ctrlId, enShowBits mask) {
	if (changed & mask) {
		SendMessage(GetDlgItem(hwnd, ctrlId), TBM_SETPOS, 1, this->bitsToPos(mask));
	}
}
void Dialog::expand(bool expand) {
	if (this->isExpanded == expand) return;
	this->isExpanded = expand;
	RECT rc;
	HWND more = GetDlgItem(this->hwnd, IDC_MORE);
	GetClientRect(more, &rc);
	RECT wrc;
	GetWindowRect(this->hwnd, &wrc);
	SetWindowPos(this->hwnd, 0, 0, 0, wrc.right - wrc.left, wrc.bottom - wrc.top + (expand ? rc.bottom : -rc.bottom), SWP_NOMOVE|SWP_NOZORDER);
	ShowWindow(more, (expand ? SW_SHOW : SW_HIDE));
	CheckDlgButton(this->hwnd, IDB_MORE, expand);
}

void Dialog::sliderNotify(HWND slider) {
	if (!slider) return;
	int pos = SendMessage(slider, TBM_GETPOS, 0, 0);
	int id = GetWindowLong(slider, GWL_ID);
	if (id == IDC_SLIDER) {
		this->updateLevel( (enLevel) this->posToBits(showLevel, pos) );
	} else {
		enShowBits mask = (enShowBits)0;
		switch (id) {
			case IDC_SLIDERTIPS:
				mask = showTooltips;
				break;
			case IDC_SLIDERINFO:
				mask = showInfo;
				break;
			case IDC_SLIDERWIZARDS:
				mask = showWizards;
				break;
		}
		if (mask)
			this->updateBits( mask, this->posToBits(mask, pos), true);
	}
}
void Dialog::pressedCancel() {
	if (this->isModal) {
		this->pressedOk();
	} else {
		DestroyWindow(this->hwnd);
	}
}
void Dialog::pressedOk() {
	bool changed = GETINT(CFG_SHOWBITS) != this->showBits;
	SETINT(CFG_SHOWBITS, this->showBits);
	SETINT(CFG_SHOWBITS_BYHAND, this->showBitsByHand);
	if (this->isModal) {
		EndDialog(this->hwnd, IDOK);
	} else {
		DestroyWindow(this->hwnd);
        if (changed && MessageBox(0 , 
            "¯eby zobaczyæ efekt, trzeba ponownie uruchomiæ program.\r\Zrobiæ to teraz?" , "Konnekt" , MB_ICONWARNING | MB_YESNO)==IDYES)
            ICMessage(IMC_RESTART);
	}
}
void Dialog::pressedReset() {
	this->showBitsByHand = false;
	this->updateLevel((enLevel) (this->showBits & showLevel));
}


int Dialog::bitsToPos(enShowBits mask, int bits) {
	if (bits == -1) bits = this->showBits;
	bits &= mask;
	if (!mask) return 0;
	/* zliczamy ile jest bitowych zer na pocz¹tku maski
	przesuwaj¹c maskê w prawo tak d³ugo, a¿ trafimy na jej pierwszy bit
	*/
	int start = 0;
	while (!((mask >> (start)) & 1)) {start++;}
	if (!bits) return start;
	/* zliczamy który bit ustawiony jest jako ostatni
	przesuwaj¹c maskê w lewo tak d³ugo, a¿ przestan¹ siê pokrywaæ
	*/
	int pos = 0;
	while ((mask << (pos)) & bits) {pos++;}
	return start + pos;
}
int Dialog::posToBits(enShowBits mask, int pos) {
	if (!mask) return 0;
	int bits = 0;
	// ustawiamy kolejne bity a¿ dojdziemy do "pozycji"
	while (pos--) {
		bits = (bits << 1) | 1;
	}
	// usuwamy zbêdne bity wzglêdem maski...
	return bits & mask;
}


int CALLBACK ShowBits::Dialog::dialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
			SetProp(hwnd, "Dialog*", (HANDLE)lParam);
			Dialog::getDialogFromWindow(hwnd)->initDialog(hwnd);
			ShowWindow(hwnd, SW_SHOW);
			break;
		case WM_DESTROY: {
			delete Dialog::getDialogFromWindow(hwnd);
			openedDialog = 0;
			HWND item = GetDlgItem(hwnd, IDC_IMAGE);
			int imageType = (GetWindowLong(item, GWL_STYLE) & SS_BITMAP) ? IMAGE_BITMAP : IMAGE_ICON;
			HANDLE image = (HANDLE)SendMessage(item, STM_GETIMAGE, imageType, 0);
			if (imageType == IMAGE_BITMAP)
				DeleteObject((HGDIOBJ)image);
			else
				DestroyIcon((HICON)image);

			break;}
			
        case WM_COMMAND:
	        switch(HIWORD(wParam)) {
				case BN_CLICKED:
					switch(LOWORD(wParam)) {
						case IDB_MORE:
							Dialog::getDialogFromWindow(hwnd)->expand(IsDlgButtonChecked(hwnd, IDB_MORE)!=0);
							break;
						case IDOK:
							Dialog::getDialogFromWindow(hwnd)->pressedOk();
							break;
						case IDCANCEL:
							Dialog::getDialogFromWindow(hwnd)->pressedCancel();
							break;
						case IDB_RESET:
							Dialog::getDialogFromWindow(hwnd)->pressedReset();
							break;
					}
			}
			break;
		case WM_HSCROLL: 
			Dialog::getDialogFromWindow(hwnd)->sliderNotify((HWND)lParam);
			break;
		case WM_CLOSE:
			Dialog::getDialogFromWindow(hwnd)->pressedCancel();
			break;
	}
	return 0;
}

void ShowBits::openDialog() {
	static bool once = true;
	if (!openedDialog && (!once || !GETINT(CFG_SHOWBITS))) {
		openedDialog = new Dialog(once);
		if (once) { // firsttimer niemodalny
			DialogBoxParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_SHOWBITS), 0, Dialog::dialogProc, (LPARAM) openedDialog);
		} else {
			CreateDialogParam(Ctrl->hDll(), MAKEINTRESOURCE(IDD_SHOWBITS), 0, Dialog::dialogProc, (LPARAM) openedDialog);
		}
	}
	once = false;
}
