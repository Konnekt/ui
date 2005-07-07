#include "include\fontpicker.h"

#define LISTTIP_HEIGHT  60
#define MSGSENDHISTORY_LIMIT 20

#define CNT_NEW_TIME 15

#define CNT_EX_NEW 0xF

class cMsgHistoryEntry {
public:
	cMsgHistoryEntry(const CStdString & body , const CStdString & ext , int type , int flag)
		:body(body),ext(ext),flag(flag),type(type) {
	}
	CStdString body , ext;
	int flag , type;
};

class sUICnt {
  public:
    unsigned int net , status; // obie wartosc nie zawsze musza byc PRAWDZIWE!!!
    unsigned int ID;
    int notify;
    sUIAction action;
    string display;
    HWND hwndMsg , hwndInfo;
    bool user;
    bool hidden;
	bool onList;
    int ex; // extended... pierwsze 4 bity - NEW
    sUICnt () {hwndMsg=hwndInfo=0; 
        ID = notify=status=net=ex=0; action=NOACTION; user = false; activity=0;active=false;_msgControl=0;
		clearMsgHistory();
        hidden = true;
		onList = false;
    }
    void imRefresh ();
    void close ();
    void MsgGetHandles();
    int MsgUpdate(bool getHandles=true);
    void cfgSet();
    void checkActivity();

    void setStatus(int status , string info);
      
    void MsgSend();
    void MsgWndOpen(bool queue = true , bool autoPopup=false);
    void MsgWndClose();

    void InfoWndOpen(bool newUser=false);
	CStdString getDisplayFormatted();
	int  GetStatusIcon(bool sameStatus);
	void ApplyFilters();

	void useMsgHistory(bool next);
	bool haveMsgHistory(bool next);
	void clearMsgHistory();

    time_t activity;
    bool active;
    class cMsgControl * _msgControl;
    deque <cMsgHistoryEntry> msgHistory; // wpisane poprzednie wiadomoœci...
    int msgHistoryPos;
//    int msgStatus; // aktualny status w okienku wiadomoœci
//    int msgStatus; // aktualny status w okienku wiadomoœci
  private:
  };

class cUICnts {
  public:
  map <unsigned int , sUICnt*> Cnt;
  typedef map <unsigned int , sUICnt*>::iterator Cnt_it_t;
  sUICnt & operator [] (int n);
  void imRemove (int id , bool massive);
  void imAdd (int id , bool hidden=false);
  void imPrepare ();
  void sort();
  ~cUICnts();
  int findBy (int,char);
  int size () {return Cnt.size();};
  int compare(sUICnt * a, sUICnt * b);
  bool exists(int id);
  void closeAll();
  void cfgSet();
  void checkActivity(bool force = false);
};

// --------------
class cCntListWindow {
    public:
        cCntListWindow(class cCntList  * owner , int x , int y , int w , int h , HWND parent , HMENU id);
        ~cCntListWindow();

  static INT_PTR CALLBACK ListControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

        void sort();
        int find(sUICnt * cnt); // podaje pozycjê danego kontaktu na liœcie

        void refresh(bool sort); // Odœwie¿a listê
        void refill(); // Wype³nia na nowo
        void removeCnt(sUICnt * cnt);
        int addCnt(sUICnt * cnt);

        void drawListItem(DRAWITEMSTRUCT * dis);
        void measureListItem(MEASUREITEMSTRUCT * mis);
        void cfgSet();
		void callDefaultAction(); // uruchamia domyœln¹ akcjê dla zaznaczenia
        class cCntListTip * tip;

        WNDPROC _oldwndproc;
        HWND _hwnd;
        class cCntList * parent;
    private:
        CStdString lookupString;
        int lookupLast;

};

#ifdef _VECTOR_

    class cCntList {
        friend class cCntListWindow;
        public:
            void registerWnd(class cCntListWindow * wnd);
            void unregisterWnd(class cCntListWindow * wnd);
            bool running();

            void refresh(bool sort); // Odœwie¿a listê
            void refill(); // Wype³nia na nowo
            void removeCnt(sUICnt * cnt);
            void addCnt(sUICnt * cnt);
            void cfgSet(); // Ustawia sobie Cache po zmianach w konfigu

            cCntList():_hfItem(0),_hfItemSel(0),_hfItemOnline(0) {}
        private:
            vector <cCntListWindow *> _wnd;
            typedef vector <cCntListWindow *>::iterator _wnd_it_t;
            // "cache"
            int _icons; // rozmieszczenie
            /*
               00 - wyl
               10 - lewo (mala)
               01 - prawo (duza)
               Bity:

               Opis 0-1
               Email 2-3
               Telefon 4-5
            */
            bool _offset;
            bool _right;
            bool _iconson;
            bool _samestatus;
			UI::CFG::enCntListInfo _info;
			CStdString _infoFormat;
            LOGFONTEX _lfItem , _lfItemSel, _lfItemInfo;
            HFONT _hfItem , _hfItemSel , _hfItemOnline, _hfItemInfo;
            bool _flickerFree;
			int _infoHeight;
    };

#endif //vector

class cCntListTip {
    public:
        cCntListTip(HWND parent=0 , bool docked = false);
        ~cCntListTip();
        void show(int pos , HWND list=0);
        void refresh(int newPos=-2);
        void paint();
        HWND _hwnd;
        bool docked;
        unsigned int cnt;  // aktualny kontakt
    private:
        void drawItem(HDC hdc , RECT * rc , const char * name , const char * text , unsigned int ico , int maxWidth, bool multiline = false);
        int charHeight; // wysokosc znaku
        int realCharHeight; // wysokosc znaku
        int drawIcon; // wysokosc ikonki
        int toDraw; // liczba el. do narysowania
};


//-------

UIEXTERN cUICnts Cnt;
extern cCntList cntList;
extern cCntListWindow * cntListWnd;
extern cCntListTip * cntTip;


//--------------------------------------------------------------------
// - F-cje

LRESULT CALLBACK	ListTipClassProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ListControlProc (HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

LRESULT CALLBACK ListTipClassProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ListControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

dragdropList(HWND hwnd = 0, UINT message = 0 , WPARAM wParam = 0, LPARAM lParam = 0);


void fillGroups();
void selectGroups();


