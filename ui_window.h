   
//--------------------------------------------------
// F-cje

/*
  Format:
  {...} - pomiedzy {} znajduje sie wartosc.
  {[...} zamknie wynik w [] (dozwolone równie¿ s¹ '(' '{' ' ' '"' ''')

  {profile} - nazwa profilu
  {version} - wersja konnekta
  {UID}  - UID kontaktu
  {status}  - status kontaktu
  {info} - opis statusu
  {display} - "wyœwietl jako" kontaktu
  {group} - grupa kontaktu
  {IP}
  Dla tray'a
  {eventDisplay} - kontakt dowi¹zany do zdarzenia
  {eventTitle} - nazwa zdarzenia

  %
*/

extern map <string , string> formatMap;


LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	TopProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	DockClassProc(HWND, UINT, WPARAM, LPARAM);

void snapIt (RECT* wr);
void dockHide(bool show);
void dockEnd();
void AutoHide();
int populateList();
void onSizeMain(int w , int h);
int createWindows();
void Animate();
VOID CALLBACK timerAnimProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);

#define   IDB_STATUS   1

int registerClasses();
void destroyWindows();
int createWindows();
int startWindows();

void createTray(int ico=IDI_TRAY);

void changeTray(int ico , char * txt = 0);

void destroyTray();

// ----------------------------------------
int getAnim(int id);

//VOID CALLBACK timerAnimProc(void * lpArg,DWORD dwLow,DWORD dwHigh) {
VOID CALLBACK timerAnimProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);

void Animate();
//-------------------------------------
VOID CALLBACK timerHideProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
VOID CALLBACK timerAwayProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
void AutoHide();
void AutoAway();
void onSizeMain(int w , int h);

void dockDesktop(bool show , bool repos = false);

RECT dockRect(bool invalidate = false);

void dockIt (RECT* wr);


void dockEnd();

void snapIt (RECT* wr);
int strcompare (string a , string b);


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TopProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK DockClassProc(HWND hWnd , UINT message, WPARAM wParam, LPARAM lParam);



#define HIDE_INTERVAL 1000