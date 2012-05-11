/****************************************************************************
 * NxWidgets/nxwm/include/cfullscreenwindow.hxx
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX, NxWidgets, nor the names of its contributors
 *    me be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_CFULLSCREENWINDOW_NXX
#define __INCLUDE_CFULLSCREENWINDOW_NXX

/****************************************************************************
 * Included Files
 ****************************************************************************/
 
#include <nuttx/config.h>

#include "cnxwindow.hxx"

#include "iapplicationwindow.hxx"

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Implementation Classes
 ****************************************************************************/

#if defined(__cplusplus)

namespace NxWM
{
  /**
   * This class represents a generic window.  This bland window is used,
   * for example, to support full screen displays.
   */

  class CFullScreenWindow : public IApplicationWindow,
                            private NXWidgets::CWindowEventHandler
  {
  protected:
    NXWidgets::CNxWindow *m_window; /**< The generic window used by the application */

    /**
     * Handle an NX window mouse input event.
     *
     * @param e The event data.
     */

#ifdef CONFIG_NX_MOUSE
    void handleMouseEvent(void);
#endif

    /**
     * Handle a NX window keyboard input event.
     */

#ifdef CONFIG_NX_KBD
    void handleKeyboardEvent(void);
#endif

  public:

    /**
     * CFullScreenWindow Constructor
     *
     * @param window.  The raw window to be used by this full screen application.
     */

    CFullScreenWindow(NXWidgets::CNxWindow *window);

    /**
     * CFullScreenWindow Destructor
     */

    ~CFullScreenWindow(void);

    /**
     * Initialize window.  Window initialization is separate from
     * object instantiation so that failures can be reported.
     *
     * @return True if the window was successfully initialized.
     */

    bool open(void);

    /**
     * Re-draw the application window
     */

    void redraw(void);

    /**
     * The application window is hidden (either it is minimized or it is
     * maximized, but not at the top of the hierarchy)
     */

    void hide(void);

    /**
     * Recover the contained NXTK window instance
     *
     * @return.  The window used by this application
     */

    NXWidgets::INxWindow *getWindow(void) const;

    /**
     * Set the window label
     *
     * @param appname.  The name of the application to place on the window
     */

    void setWindowLabel(NXWidgets::CNxString &appname);

    /**
     * Register to receive callbacks when toolbar icons are selected
     */

    void registerCallbacks(IApplicationCallback *callback);

    /**
     * Simulate a mouse click on the minimize icon.  This inline method is only
     * used during automated testing of NxWM.
     */

#if defined(CONFIG_NXWM_UNITTEST) && !defined(CONFIG_NXWM_TOUCHSCREEN)
    void clickMinimizeIcon(int index);
#endif

    /**
     * Simulate a mouse click on the stop applicaiton icon.  This inline method is only
     * used during automated testing of NxWM.
     */

#if defined(CONFIG_NXWM_UNITTEST) && !defined(CONFIG_NXWM_TOUCHSCREEN)
    void clickStopIcon(int index);
#endif
  };
}

#endif // __cplusplus

#endif // __INCLUDE_CFULLSCREENWINDOW_NXX