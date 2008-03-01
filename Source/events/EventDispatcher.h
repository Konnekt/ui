/**
  *  Generic Event Dispatcher class
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

#ifndef __EVENTDISPATCHER_H__
#define __EVENTDISPATCHER_H__

#include <hash_map>
#include <deque>

#include <boost/signal.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace Stamina;
using namespace boost;

using namespace std;
using namespace stdext;

namespace Konnekt {
  class StopEventNotifyException : public ExceptionString {
  public:
    StopEventNotifyException() : ExceptionString("Event notification was interrupted") { }
  };

  template <class E>
  class EventDispatcher : public SharedObject<iSharedObject> {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(EventDispatcher<E>, iSharedObject, Version(0,1,0,0));

  public:
    typedef deque<signals::connection> tConnections;

  public:
    typedef function<void(E&)> fListener;
    typedef signal<void(E&)> sigListener;

  public:
    /**
     * Structure holding listener and it's connections
     */
    struct sListener {
      tConnections connections;
      sigListener signal;
    };

  public:
    typedef hash_map<int, sListener*> tListeners;

  public:
    /**
     * Destroys EventDispatcher object
     */
    inline ~EventDispatcher() {
      for (tListeners::iterator it = _listeners.begin(); it != _listeners.end(); it++) {
        delete it->second;
      }
    }

  public:
    /**
     * Connects a listener to a given event id.
     *
     * @param int       An event id
     * @param fListener Listener callback
     * @param int       Listener priority
     * @param signals::connect_position
     *
     * @return signals::connection 
     */
    inline signals::connection connect(int id, const fListener& f, int priority = 0, signals::connect_position pos = signals::at_back) {
      if (f.empty()) {
        throw ExceptionString("Empty functor was given.");
      }

      if (_listeners.find(id) == _listeners.end()) {
        _listeners[id] = new sListener;
      }

      signals::connection c = _listeners[id]->signal.connect(priority, f, pos);
      if (!c.connected()) {
        throw ExceptionString("Listener was not connected.");
      }

      _listeners[id]->connections.push_back(c);
      return c;
    }

    /**
     * Disconnects a listener for a given event id.
     * TODO: jak to obsluzyc ?
     *
     * @param int       An event id
     * @param fListener Listener callback
     *
     * @return bool 
     */
    inline bool disconnect(int id, const fListener& f) {
      if (!hasListeners(id)) {
        return false;
      }
      return true;
    }

    /**
     * Notifies all listeners of a given event.
     *
     * @param Event& A Event instance
     *
     * @return Event& The Event instance
     */
    inline E& notify(E& ev) {
      int id = ev.getID();

      if (hasListeners(id)) {
        try {
          _listeners[id]->signal(ev);
        } catch (StopEventNotifyException&) { }
      }
      return ev;
    }

    /**
     * Returns true if the given event id has some listeners.
     *
     * @param  id  The event id
     *
     * @return bool true if some listeners are connected, false otherwise
     */
    inline bool hasListeners(int id) {
      if (_listeners.find(id) == _listeners.end()) {
        return false;
      }
      return !_listeners[id]->signal.empty();
    }

  protected:
    tListeners _listeners;
  };
}

#endif // __EVENTDISPATCHER_H__