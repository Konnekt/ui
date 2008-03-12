#include "stdafx.h"
#include "HistoryTable.h"

HistoryTable::oHistoryTable HistoryTable::_instance;

HistoryTable* HistoryTable::getInstance() {
  if (!_instance.isValid()) _instance = new HistoryTable;

  return _instance;
}

void HistoryTable::init() {
  if (!_initialized) {

    _historyTable = registerTable(Ctrl, "History", optPrivate | optAutoLoad | optAutoSave | optDiscardLoadedColumns);

    _historyTable->setColumn(Message::colId,ctypeInt, "ID");
    _historyTable->setColumn(Message::colNet,  ctypeInt, "Net");
    _historyTable->setColumn(Message::colType, ctypeInt, "Type");
    _historyTable->setColumn(Message::colFromUid, ctypeString|cflagXor, "FromUID");
    _historyTable->setColumn(Message::colToUid, ctypeString|cflagXor, "ToUID");
    _historyTable->setColumn(Message::colBody, ctypeString|cflagXor, "Body");
    _historyTable->setColumn(Message::colExt, ctypeString|cflagXor, "Ext");
    _historyTable->setColumn(Message::colFlag, ctypeInt, "Flag");
    _historyTable->setColumn(Message::colTime, ctypeInt64, "Time");
    _historyTable->setColumn(MSGH_SESSION, ctypeInt, "Session");
    _historyTable->setColumn(MSGH_DISPLAY, ctypeString|cflagXor, "Display");
  }
  _initialized = true;
}

void HistoryTable::deinit() {
  _historyTable.reset();
  _initialized = false;
}

void HistoryTable::load(const StringRef& dir, const StringRef& name) {
  this->lock(lockDefault);

  getTable()->setFilename(name + ".dtb");
  getTable()->setDirectory(String(historyDir) + "\\" + dir);
  getTable()->load();
}

void HistoryTable::unload() {
  getTable()->unloadData();

  this->unlock(lockDefault);
}