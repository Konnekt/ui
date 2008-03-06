#include "stdafx.h"
#include <Konnekt/core_tables.h>

using namespace Konnekt::Tables;

#define MSGH_SESSION 100
#define MSGH_DISPLAY 101

const char historyDir[] = "%KonnektProfile%\\history";

class HistoryTable : public SharedObject<iSharedObject> {
private:
  typedef SharedPtr<HistoryTable> oHistoryTable;

public:
  static HistoryTable* getInstance();

public:
  HistoryTable() : _initialized(false) { }

public:
  void init();
  void deinit();

public:
  void load(const StringRef& dir, const StringRef& name);
  void unload();

public:
  oTable& getTable() {
    return _historyTable;
  }

private:
  oTable _historyTable;
  bool _initialized;

  static oHistoryTable _instance;
};

