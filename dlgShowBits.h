#pragma once

namespace Konnekt { namespace ShowBits {

	void openDialog();

	class Dialog {
	private:
		bool isModal;
		HWND hwnd;
		void initDialog(HWND hwnd);
		int showBitsByHand;
		int showBits;
		bool isExpanded;

		void sliderNotify(HWND slider);
		void pressedCancel();
		void pressedOk();
		void pressedReset();

	public:
		Dialog(bool modal):isModal(modal) {}
		static int CALLBACK dialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
		static Dialog * getDialogFromWindow(HWND hwnd);
		void expand(bool expand);
		void updateLevel(enLevel level);
		void updateBits(int changed, int bits, bool byHand);
		void updateBits(int changed, int ctrlId, enShowBits mask);
		int bitsToPos(enShowBits mask, int bits=-1);
		int posToBits(enShowBits mask, int pos);
	};

}} // namespace