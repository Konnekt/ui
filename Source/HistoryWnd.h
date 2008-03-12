#include "window/Window.h"

class HistoryWindow : public Window<HistoryWindow> {
public:
  STAMINA_OBJECT_CLASS_VERSION(HistoryWindow, Window<HistoryWindow>, Version(1,0,0,0));

public:
  HistoryWindow() {

    setCallback(WM_CREATE, &HistoryWindow::_create);
    setCallback(WM_GETMINMAXINFO, &HistoryWindow::_maxmininfo);
    setCallback(WM_SIZE, &HistoryWindow::_size);
    setCallback(WM_NOTIFY, &HistoryWindow::_notify);
    setCallback(WM_COMMAND, &HistoryWindow::_command);
  }

private:
  void _create(WndMessage& m);
  void _destroy(WndMessage& m);
  void _size(WndMessage& m);
  void _maxmininfo(WndMessage& m);
  void _notify(WndMessage& m);
  void _command(WndMessage& m);

private:
  tHwnd _hTree;
  tHwnd _hStatus;
  tHwnd _hTool;
  tHwnd _hProgress;
  tHwnd _hList;
  tHwnd _hView;
  HIMAGELIST _hImageList;
  HIMAGELIST _hImageList32;
};