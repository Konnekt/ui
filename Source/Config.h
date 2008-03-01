/**
  *  Configuration class
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2005-2008 Sijawusz Pur Rahnama
  *  @link         svn://konnekt.info/kaway2/ kAway2 plugin SVN Repo
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy: sija $
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

#pragma once

#ifndef __KONNEKT_CONFIG_H__
#define __KONNEKT_CONFIG_H__

#include "Events/IMessageDispatcher.h"
#include "Events/IMEvent.h"

using namespace Konnekt::Tables;
using namespace Stamina;
using namespace boost;

namespace Konnekt {
  class Config : public SharedObject<iSharedObject>, public signals::trackable {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(Config, iSharedObject, Version(0,3,0,0));

  public:
    class Item {
    public:
      Item(tTable table, tColId col, tRowId row = 0): _table(table), _col(col), _row(row) { }

    public:
      inline operator String () const {
        return to_s();
      }

    public:
      inline Item& operator << (int value) {
        set(value);
        return *this;
      }

      inline Item& operator << (__int64 value) {
        set(value);
        return *this;
      }

      inline Item& operator << (const StringRef& value) {
        set(value);
        return *this;
      }

      inline Item& operator << (const Item& value) {
        switch (value.getType()) {
          case ctypeInt:
            set(value.to_i());
            break;
          case ctypeInt64:
            set(value.to_i64());
            break;
          case ctypeString:
            set(value.to_s());
            break;
        }
        return *this;
      }

    public:
      inline tColId getID() const {
        return _col;
      }

      inline int getTable() const {
        return _table;
      }

      inline tRowId getRow() const {
        return _row;
      }

    public:
      inline tColType getType() const {
        return (tColType) Ctrl->DTgetType(_table, _col);
      }

      inline string getName() const {
        return Ctrl->DTgetName(_table, _col);
      }

    public:
      inline int to_i(tRowId row) const {
        return Ctrl->DTgetInt(_table, row, _col);
      }
      inline int to_i() const {
        return to_i(getRow());
      }

      inline __int64 to_i64(tRowId row) const {
        return Ctrl->DTgetInt64(_table, row, _col);
      }
      inline __int64 to_i64() const {
        return to_i64(getRow());
      }

      inline String to_s(tRowId row) const {
        return Ctrl->DTgetStr(_table, row, _col);
      }
      inline String to_s() const {
        return to_s(getRow());
      }

    public:
      inline bool set(int value, tRowId row) {
        return Ctrl->DTsetInt(_table, row, _col, value);
      }
      inline bool set(int value) {
        return set(value, getRow());
      }

      inline bool set(__int64 value, tRowId row) {
        return Ctrl->DTsetInt64(_table, row, _col, value);
      }
      inline bool set(__int64 value) {
        return set(value, getRow());
      }

      inline bool set(const StringRef& value, tRowId row) {
        return Ctrl->DTsetStr(_table, row, _col, value.a_str());
      }
      inline bool set(const StringRef& value) {
        return set(value, getRow());
      }

    protected:
      tTable _table;
      tColId _col;
      tRowId _row;
    };

  public:
    typedef std::deque<sIMessage_setColumn*> tColumns;

  public:
    /**
     * Creates new instance of Config class with column registration listeners attached.
     *
     * @param IMessageDispatcher& dispatcher object
     */
    inline Config(IMessageDispatcher& dispatcher) {
      attachListeners(dispatcher);
    }
    /**
     * Creates new instance of Config class.
     */
    inline Config() { }

    /**
     * Destroys Config object.
     */
    inline ~Config() { 
      for (tColumns::iterator it = _cols.begin(); it != _cols.end(); it++) {
        delete *it;
      }
    }

  public:
    /**
     * Gets configuration item.
     *
     * @param tTable table id
     * @param tColId column id
     * @param tRowId row id
     *
     * @return Item
     */
    static Item get(tTable table, tColId col, tRowId row) {
      return Item(table, col, row);
    }
    /**
     * Gets configuration item from global configuration table.
     *
     * @param tColId column id
     *
     * @return Item
     */
    static Item get(tColId col) {
      return get(tableConfig, col, 0);
    }
    /**
     * Gets configuration item from contact table.
     *
     * @param tColId column id
     * @param tCntId contact id
     *
     * @return Item
     */
    static Item get(tColId col, tCntId cnt) {
      return get(tableContacts, col, cnt);
    }

  public:
    /**
     * Connects listeners to the IM_SETCOLS IMessage id.
     *
     * @see setColumn
     *
     * @param IMessageDispatcher&
     */
    inline void attachListeners(IMessageDispatcher& dispatcher) {
      dispatcher.connect(IM_SETCOLS, bind(&Config::_registerColumns, this, _1));
    }

    /**
     * Registers column in the given table.
     *
     * @param tTable      Table id
     * @param tColId      Column id
     * @param int         Column type
     * @param const char* Column default value
     * @param const char* Column name
     */
    inline void setColumn(tTable table, tColId id, int type, const char* def, const char* name) {
      _cols.push_back(new sIMessage_setColumn(table, id, type, def, name));
    }
    inline void setColumn(tTable table, tColId id, int type, int def, const char* name) {
      _cols.push_back(new sIMessage_setColumn(table, id, type, def, name));
    }

    /**
     * Sets all registered columns to their's default values.
     *
     * @param tTable Table id to reset
     */
    inline void resetColumns(tTable table) {
      int count = Ctrl->DTgetCount(table);
      if (!count || _cols.empty()) {
        return;
      }
      for (tColumns::iterator it = _cols.begin(); it != _cols.end(); it++) {
        if ((*it)->_table == table) {
          for (int i = 0; i < count; i++) {
            _resetColumn(*it, i);
          }
        }
      }
    }

    /**
     * Sets the given column to it's default value.
     *
     * @param tTable      Table id
     * @param tColId      Column id
     * @param tRowId      Row id
     * @param sUIAction*
     */
    inline void resetColumn(tTable table, tColId id, tRowId row = 0, sUIAction* an = 0) {
      for (tColumns::iterator it = _cols.begin(); it != _cols.end(); it++) {
        if ((*it)->_table == table && (*it)->_id == id) {
          _resetColumn(*it, row, an); break;
        }
      }
    }

  protected:
    inline void _resetColumn(sIMessage_setColumn* it, tRowId row = 0, sUIAction* an = 0) {
      Item item = get(it->_table, it->_id, row);

      String val;
      bool convert = true;

      switch (it->_type) {
        case ctypeInt: {
          item.set(it->_def);
          val = inttostr(it->_def);
          break;
        }
        case ctypeInt64: {
          item.set(*it->_def_p64);
          // val = i64tostr(*it->_def_p64);
          break;
        }
        case ctypeString: {
          item.set(it->_def_ch);
          val = it->_def_ch;
          convert = false;
          break;
        }
      }
      if (an) {
        UIActionCfgSetValue(*an, val.c_str(), convert);
      }
    }

    inline void _registerColumns(IMEvent& ev) {
      for (tColumns::iterator it = _cols.begin(); it != _cols.end(); it++) {
        Ctrl->IMessage(*it);
      }
      ev.setSuccess();
    }

  protected:
    tColumns _cols;
  };

  // smart pointer type
  typedef SharedPtr<Config> oConfig;
}

#endif // __KONNEKT_CONFIG_H__