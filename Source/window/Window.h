#pragma once

#ifndef __WINDOW_H__
#define __WINDOW_H__

  /* class WndMessage */
  class WndMessage {
  public:
    enum enParam {
      tlParam,
      twParam
    };

    typedef unsigned int tMsg;
    typedef LPARAM tParam;
    typedef LRESULT tResult;

  public:
    WndMessage(tMsg msg, tResult wParam, tResult lParam) :
      _msg(msg), _wParam(wParam), _lParam(lParam), _processed(0), _result(0) { }

  public:
    tMsg getID() {
      return _msg;
    }

    template <class T> T getParam(enParam param = tlParam) {
      if (param == twParam) {
        return (T)(_wParam);
      }
      return (T)(_lParam);
    }

    tParam getParam(enParam param = tlParam) {
      return getParam<tParam>(param);
    }

    bool isProcessed() {
      return _processed;
    }

    void setProcessed(bool processed = true) {
      _processed = processed;
    }

    tResult getResult() {
      return _result;
    }

    void setResult(tResult result) {
      setProcessed();
      _result = result;
    }

  private:
    tMsg _msg;
    tParam _wParam;
    tParam _lParam;

    tResult _result;
    bool _processed;
  };

  /* class iControl */
  class iControl : public SharedObject<iSharedObject> {
  public:
    STAMINA_OBJECT_CLASS_VERSION(iControl, SharedObject<iSharedObject>, Version(1,0,0,0));

  public:
    typedef HWND tHwnd;

  public:
    iControl() : _hwnd(0) { }

  public:
    static WndMessage::tResult _windowProc(tHwnd hwnd, WndMessage::tMsg msg, WndMessage::tParam wParam, 
      WndMessage::tParam lParam);

  public:
    tHwnd getHwnd() {
      return _hwnd;
    }
    bool alive() {
      return _hwnd != 0;
    }

  public:
    //some func to control

  protected:
    tHwnd _hwnd;
  };

  typedef SharedPtr<iControl> oControl;

  /* class iWindow */
  class iWindow : public iControl {
  public:
    STAMINA_OBJECT_CLASS_VERSION(iWindow, iControl, Version(1,0,0,0));

  public:
    static iWindow* iWindow::getWindowPtr(tHwnd hwnd) {
      return (iWindow*) GetWindowLongPtr(hwnd, GWL_USERDATA);
    }
    static String getWindowClassName();

  public:
  };

  /* class EventableControl<C, EC> */
  template <class C, typename EC>
  class EventableControl : public C {
  public:
    STAMINA_OBJECT_CLASS_VERSION(EventableControl, C, Version(1,0,0,0));

  private:
    typedef void (__thiscall EC::* fMsgCallback)(WndMessage&);
    typedef map<WndMessage::tMsg, fMsgCallback> tMsgCallbacks;

  public:
    void process(WndMessage& msg) {
      EC* this_ = (EC*) const_cast<EventableControl*>(this);

      tMsgCallbacks::iterator it = _callbacks.find(msg.getID());

      if (it != _callbacks.end()) {
        (this_->*(it->second))(msg);
      }
    }

    void setCallback(WndMessage::tMsg msg, fMsgCallback callback) {
      if (!callback) {
        _callbacks.erase(_callbacks.find(msg));
        return;
      }
      _callbacks[msg] = callback;
    }

    bool hasCallback(WndMessage::tMsg msg) {
      return _callbacks.find(msg) != _callbacks.end();
    }

  private:
    tMsgCallbacks _callbacks;
  };

  typedef SharedPtr<iWindow> oWindow;

  /* class Window<T> */
  template <class T>
  class Window : public EventableControl<iWindow, T> {
  public:
    STAMINA_OBJECT_CLASS_VERSION(Window, EventableControl, Version(1,0,0,0));

  public:
    typedef typename iWindow::tHwnd tHwnd;

  public:
    static void init();
    static void deinit();

  public:
    static oWindow create(const Stamina::StringRef& title, int style, int x, int y, int w, int h, iWindow::tHwnd parent);

  private:
    static WndMessage::tResult _windowProc(tHwnd hwnd, WndMessage::tMsg msg, WndMessage::tParam wParam, 
      WndMessage::tParam lParam);
    static String getWindowClassName() {
      return stringf("Window::<%s>", T::staticClassInfo().getName()).c_str();
    }

    public:
    class __wndclass_initializer__ {
    public:
      __wndclass_initializer__() {
        if (!__wndclass_initialized) {
          Window<T>::init();
          __wndclass_initialized = true;
        }
      }
      ~__wndclass_initializer__() {
        if (__wndclass_initialized) {
          Window<T>::deinit();
          __wndclass_initialized = false;
        }
      }
    } _initializer;

  private:
    static bool __wndclass_initialized;
  };

  template<class T>
  __declspec(selectany) bool Window<T>::__wndclass_initialized = false;

  template <class T> inline void Window<T>::init() {
    WNDCLASSEX wcex = {0};

    String& name = getWindowClassName();

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbWndExtra = sizeof(void*);

    wcex.lpszClassName = name.c_str();
    wcex.lpfnWndProc = (WNDPROC) &Window<T>::_windowProc;

    wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wcex.hCursor = LoadCursor(0, IDC_ARROW);
    wcex.hInstance = GetModuleHandle(0);

    RegisterClassEx(&wcex);
  }

  template <class T> inline void Window<T>::deinit() {
    UnregisterClass(getWindowClassName().c_str(), GetModuleHandle(0));
  }

  template <class T> inline oWindow Window<T>::create(const Stamina::StringRef& title, int style, int x, int y, int w, 
    int h, tHwnd parent) {

      T* t = new T;

      tHwnd hwnd = CreateWindow(getWindowClassName().c_str(), title.c_str(), style, x, y, w, h, parent, (HMENU) 0,
        GetModuleHandle(0), t);

      if (!hwnd) {
        delete t;
        t = 0;
      }

      return t;
  }

  template <class T> inline WndMessage::tResult Window<T>::_windowProc(tHwnd hwnd, WndMessage::tMsg msg, 
    WndMessage::tParam wParam, WndMessage::tParam lParam) {

      T* window = (T*) getWindowPtr(hwnd);

      WndMessage message(msg, wParam, lParam);

      if (msg == WM_NCCREATE) {
        window = (T*) message.getParam<LPCREATESTRUCT>()->lpCreateParams;
        window->hold();

        window->_hwnd = hwnd;

        SetWindowLong(hwnd, GWL_USERDATA, (LONG) window);
      }
      if (window) window->process(message);

      if (msg == WM_NCDESTROY && window) {
        window->_hwnd = 0;
        window->release();

        SetWindowLong(hwnd, GWL_USERDATA, (LONG) 0);
      }

      return message.isProcessed() ? message.getResult() : DefWindowProc(hwnd, msg, wParam, lParam);
  }

#endif // __WINDOW_H__
