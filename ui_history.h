int CALLBACK HistoryDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
int hist_add (Message * m , const char * dir , sUICnt * cnt , const char * name , int session);
void hist_start(int cnt=0);
void hist_init();
#define MSGH_SESSION 100
#define MSGH_DISPLAY 101

#define HISTDIR_MSG "messages"
#define HISTDIR "history\\"
#define HISTSETTINGSDIR "data\\" HISTDIR


#ifdef DTABLEH 


#endif


void onSizeHistory(int w , int h);
