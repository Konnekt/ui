/**
  *  Action Dispatcher class
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

#ifndef __ACTIONDISPATCHER_H__
#define __ACTIONDISPATCHER_H__

#include "EventDispatcher.h"
#include "IMEvent.h"

namespace Konnekt {
  class ActionDispatcher : public EventDispatcher<class ActionEvent> {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(ActionDispatcher, EventDispatcher<ActionEvent>, Version(0,1,0,0));

  protected:
    class NotSubclassedException : public ExceptionString {
    public:
      NotSubclassedException() : ExceptionString("Action is not subclassed") { }
    };

  public:
    class SubclassInfo {
    public:
      friend class ActionDispatcher;

    protected:
      /**
       * Constructs a new SubclassInfo.
       *
       * @param int   action id
       * @param int   parent action id
       * @param bool  automatically forwards action to it's parent if set to true
       */
      SubclassInfo(int id, int parent, bool auto_forward):
        _id(id), _parent(parent), _prev_owner(pluginNotFound), _auto_forward(auto_forward), _registered(false) { }

      /**
       * Constructs an empty instance of SubclassInfo.
       */
      SubclassInfo(): _id(0), _parent(0), _prev_owner(pluginNotFound), _auto_forward(false), _registered(false) { }

    public:
      /**
       * Returns action ID
       *
       * @return int
       */
      inline int getID() const {
        return _id;
      }

      /**
       * Returns parent action ID
       *
       * @return int
       */
      inline int getParent() const {
        return _parent;
      }

      /**
       * Returns actual owner plugin ID
       *
       * @return int
       */
      inline unsigned int getOwner() const {
        int owner = Ctrl->ICMessage(IMI_ACTION_GETOWNER, (int) &sUIAction(getParent(), getID()));
        if (!owner) {
          owner = Ctrl->ICMessage(IMC_PLUG_ID, 0);
        }
        return owner;
      }

      /**
       * Returns previous owner plugin ID
       *
       * @return int
       */
      inline tPluginId getPrevOwner() const {
        return _prev_owner;
      }

      /**
       * Sets previous owner plugin ID
       *
       * @param int
       */
      inline void setPrevOwner(unsigned int prev_owner) {
        _prev_owner = (tPluginId) prev_owner;
      }

      /**
       * Returns true if action should be forwarded to it's previous owner
       *
       * @return bool
       */
      inline bool isRegistered() const {
        return _registered;
      }

      /**
       * Returns true if action should be forwarded to it's previous owner
       *
       * @return bool
       */
      inline bool autoForward() const {
        return _auto_forward;
      }

      /**
       * Sets auto forward option
       *
       * @param bool
       */
      inline void setAutoForward(bool auto_forward) {
        _auto_forward = auto_forward;
      }

      /**
       * Subclass action in UI.
       */
      inline void subclass() {
        if (isRegistered()) {
          // throw new ExceptionString("Already subclassed");
          return;
        }
        if (!getParent()) {
          _parent = Ctrl->ICMessage(IMI_ACTION_FINDPARENT, (int) &sUIAction(0, getID()));
        }
        sUIActionInfo nfo(getParent(), getID());
        nfo.mask = UIAIM_ALL;
        nfo.txt = new char[100];
        nfo.txtSize = 99;
        UIActionGet(nfo);

        // obtain actual action owner
        int owner = getOwner();
        // throw exception if action belongs to the current plugin
        if (owner == Ctrl->ID()) {
          // throw new ExceptionString("No need to subclass");
          return;
        }
        // update action info
        setPrevOwner(owner);

        // log for debug purposes
        Ctrl->log(logMisc, "ActionDispatcher::SubclassInfo", "subclass", "name = %s, id = %i, parent = %i, owner = %i",
          nfo.txt, getID(), getParent(), owner);

        // remove old action and register it as ours afterwards
        Ctrl->ICMessage(IMI_ACTION_REMOVE, (int) &nfo.act);
        Ctrl->ICMessage(IMI_ACTION, (int) &nfo);

        // mark action as registered
        setRegistered(true);

        // delete buffer
        delete [] nfo.txt;
      }

    protected:
      /**
       * Marks action as registered in Core
       *
       * @param bool
       */
      inline void setRegistered(bool registered) {
        _registered = registered;
      }

    protected:
      unsigned int _parent;
      unsigned int _id;
      tPluginId _prev_owner;
      bool _auto_forward;
      bool _registered;
    };

  public:
    typedef deque<SubclassInfo> tSubclassed;

  public:
    /**
     * Returns true if the given action is subclassed
     *
     * @param int action id
     * @param int parent action id
     *
     * @return bool
     */
    inline bool isSublassed(int id, int parent) const {
      for (tSubclassed::const_iterator it = _subclassed.begin(); it != _subclassed.end(); it++) {
        if (it->getID() == id && it->getParent() == parent) return true;
      }
      return false;
    }

    /**
     * Returns reference of SubclassInfo
     *
     * @param int action id
     * @param int parent action id
     *
     * @return SubclassInfo&
     */
    inline SubclassInfo& getSublassInfo(int id, int parent) {
      for (tSubclassed::iterator it = _subclassed.begin(); it != _subclassed.end(); it++) {
        if (it->getID() == id && it->getParent() == parent) return *it;
      }
      throw NotSubclassedException();
    }

    /**
     * Subclasses the given action
     *
     * @param int action id
     * @param int parent action id
     * @param bool true if given action should be forwarded to previous owner after execution
     * @param bool true if registration of the given action should be deferred
     */
    inline void subclass(int id, int parent, bool auto_forward = false, bool defer = true) {
      if (isSublassed(id, parent)) {
        return;
      }
      SubclassInfo info(id, parent, auto_forward);

      if (!defer) {
        info.subclass();
      }
      _subclassed.push_back(info);
    }

    inline void subclass(int id, bool auto_forward = false, bool defer = true) {
      subclass(id, 0, auto_forward, defer);
    }

    /**
     * Dispatches incoming Action IMessage
     *
     * @param sIMessage_base*
     *
     * @return oEvent
     */
    oEvent dispatch(sIMessage_base* msgBase);

    /**
     * Subclasses all unprocessed/deferred actions
     *
     * @param IMEvent&
     */
    inline void doSubclass(IMEvent& ev) {
      for (tSubclassed::iterator it = _subclassed.begin(); it != _subclassed.end(); it++) {
        it->subclass();
      }
    }

  protected:
    tSubclassed _subclassed;
  };
}

#endif // __ACTIONDISPATCHER_H__