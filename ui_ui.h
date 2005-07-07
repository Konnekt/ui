enum formatTitle_enum {
    FT_WINDOW , FT_TRAY , FT_TRAYEVENT, FT_CONTANCTLIST, FT_DISPLAY, FT_TIP, FT_DISPLAYPROPOSAL
};

CStdString getStatusName(int status);
CStdString formatTitle(string format , int cntID , formatTitle_enum type);
int openURLMessage(cMessage * m);

class cSplash {
public:
    cSplash();
    ~cSplash();
    void show(HBITMAP bitmap , int time);
    void hide();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    static void CALLBACK TimerProc(HWND hwnd,UINT nMsg,UINT nIDEvent,DWORD dwTime);
private:
    bool running;
    HWND wnd;
    //WNDPROC classProc;
    HBITMAP bmp;
};


CStdString getAutostartKey();
CStdString getAutostartValue();
void autostartCleanUp();
