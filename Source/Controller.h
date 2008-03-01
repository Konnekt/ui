/**
  *  Kronos Controller class
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2002-2008 Stamina
  *  @copyright    Copyright (c) 2008 Sijawusz Pur Rahnama
  *  @link         svn://konnekt.info/kronos/
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy: sija $
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

#pragma once

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "Kronos.h"

#include "PluginController.h"
#include "Config.h"

namespace Kronos {
  class Controller : public PluginController<Controller> {
  public:
    friend class PluginController<Controller>;

  protected:
    Controller();

  protected:
    void _initialize(IMEvent& ev);
    void _prepareUI(IMEvent& ev);
    void _addMessage(IMEvent& ev);

  protected:
  };
}

#endif // __CONTROLLER_H__