#ifndef _SIM_INPUTHANDLER_H_
#define _SIM_INPUTHANDLER_H_

#include <android/input.h>
#include "Log.h"

class InputHandler {
public:
    virtual ~InputHandler() {};

    bool onTouchEvent(AInputEvent* pEvent){

        sim_Log_debug("AMotionEvent_getX=%f", AMotionEvent_getX(pEvent, 0));
        sim_Log_debug("AMotionEvent_getY=%f", AMotionEvent_getY(pEvent, 0));

        return true;
    }
};
#endif
