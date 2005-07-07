/*

  Konnekt UI

  Nag³ówek list dialogowych

*/



int CALLBACK PropClassProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK InfoDialogProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK MsgDialogProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK EditMsgControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK PassDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK EnterDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK ButtonsDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK LongDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK AboutDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int CALLBACK IgnoreDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK GroupsDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int CALLBACK SearchDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int CALLBACK ServerEventDialogProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK TokenDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);



void onSizeMsg(HWND hwnd , int w , int h , bool bar=1);


void IgnoreDialogShow(bool show = true);
void GroupsDialogShow(bool show = true);
void ServerEventDialogFill();
void ServerEventDialogNext();
void SearchDialogAddCnt(HWND hwndDlg , int item);
void SearchDialogAdd(HWND hwndDlg , sCNTSEARCH * res);
void InfoDialogLoad(HWND hwndDlg , int pos);
void InfoDialogSave(int cntID , bool upload);
void InfoDialogCancel(int cntID , HWND hwnd=0);
void InfoDialogRefresh(int cntID );
void InfoDialogSummary(int cnt , bool local = false);

string InfoGetDisplay(int cntID);
void ConfigDialogOpen (int id);
void MsgCreateWindows(HWND hwndOwner , sUICnt * cnt);;
int StartLong(sDIALOG_long * sd);
void StopLong(sDIALOG_long * sd);

