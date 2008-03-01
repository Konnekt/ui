/**
  *  Kronos header file
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

#ifndef __KRONOS_H__
#define __KRONOS_H__

namespace Kronos {
  const unsigned int net = 6;
  const char sig[] = "KRONOS";

  namespace api {
    const unsigned int api = net * 1000 + IM_USER;

  }

  namespace im {
    const unsigned int im = net * 1000 + IM_USER + 100;

  }

  namespace ui {
    const unsigned int ui = net * 1000;

  }

  namespace ico {
    const unsigned int ico = net * 1000 + 100;

    const unsigned int main = ico + 1;
 
  }

  namespace act {
    const unsigned int act = net * 1000 + 200;

    const unsigned int recv_notify = act + 1;
  }

  // dynamiczne akcje
  namespace dynAct {
    const unsigned int dynAct = net * 1000 + 300;

  }

  namespace cfg {
    const unsigned int cfg = net * 1000 + 500;

    const unsigned int dragndrop = cfg + 1;
  }
}

#endif // __KRONOS_H__
