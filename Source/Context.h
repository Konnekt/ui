/**
  *  Context class
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

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "Events/IMessageDispatcher.h"
#include "Events/ActionDispatcher.h"

#include "Config.h"

namespace Konnekt {
  class Context : public SharedObject<iSharedObject> {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(Context, iSharedObject, Version(0,1,0,0));

  public:
    typedef SharedPtr<Context> oInstance;

  protected:
    /**
     * Creates new Context object.
     */
    inline Context() {
      _config.attachListeners(getIMessageDispatcher());
    }

  public:
    /**
     * Returns instance of Context object.
     *
     * @return Context
     */
    inline static Context* getInstance() {
      if (!_instance.isValid()) {
        _instance = new Context;
      }
      return _instance;
    }

    /**
     * Returns Config class instance.
     *
     * @return Config&
     */
    inline Config& getConfig() {
      return _config;
    }

    /**
     * Returns reference to IMessageDispatcher
     *
     * @return IMessageDispatcher&
     */
    inline IMessageDispatcher& getIMessageDispatcher() {
      return _imessage_dispatcher;
    }

    /**
     * Returns reference to IMActionDispatcher
     *
     * @return IMActionDispatcher&
     */
    inline ActionDispatcher& getActionDispatcher() {
      return _action_dispatcher;
    }

  private:
    static oInstance _instance;

  private:
    IMessageDispatcher _imessage_dispatcher;
    ActionDispatcher _action_dispatcher;
    Config _config;
  };

  __declspec(selectany)
  Context::oInstance Context::_instance = 0;
}

#endif // __CONTEXT_H__