#include "include\Richhtml.h"



// Wraper do nowego sposobu wyœwietlania wiadomoœci
class cMsgControl {
    public:
    HWND hwnd;
    sUIAction act;

     cMsgControl(sUIAction act , int x , int y , int w , int h , HWND parent , HMENU id);
    ~cMsgControl();

    void clear();
    void lock();
    void unlock();
//    __inline int parentMessageProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam);

    void msgInsert(cMessage * m , const char * display=0 , bool scroll=true);
    void statusInsert(int status , CStdString info);
};



// Kontrolka wyœwietal¹ca wiadomoœci.
class cMsgControlRich {
    public:
    HWND hwnd;

    cMsgControlRich(int x , int y , int w , int h , HWND parent , HMENU id);
    ~cMsgControlRich();

    void clear();
    void lock();
    void unlock();
    int parentMessageProc(HWND hwnd , UINT message, WPARAM wParam, LPARAM lParam);

    void msgInsert(cMessage * m , const char * display=0 , bool scroll=true);
	static void SetStyleCB(const CStdString & token , const CStdString & styleClass , cRichEditFormat::cSetStyle & ss);

	void statusInsert(int status , CStdString info);
	void actionProcess(sUIActionNotify_base * anBase);
	void insertHTML(CStdString body , int defColor);
    
    static int CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

    void contextMenu(int x , int y);

    private:
    WNDPROC _oldwndproc;
    int _lock;


};
