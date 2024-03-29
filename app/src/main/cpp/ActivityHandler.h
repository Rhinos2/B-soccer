#ifndef _SIM_ACTIVITYHANDLER_H_
#define _SIM_ACTIVITYHANDLER_H_

#include "Types.h"

class ActivityHandler {
protected:
    virtual ~ActivityHandler() {};

    virtual status onActivate() = 0;
    virtual void onDeactivate() = 0;
    virtual status onStep() = 0;

    virtual void onStart() {};
    virtual void onResume() {};
    virtual void onPause() {};
    virtual void onStop() {};
    virtual void onDestroy() {};

    virtual void onSaveInstanceState(void** pData, size_t* pSize) {};
    virtual void onConfigurationChanged() {};
    virtual void onLowMemory() {};

    virtual void onCreateWindow() {};
    virtual void onDestroyWindow() {};
    virtual void onGainFocus() {};
    virtual void onLostFocus() {};
};
#endif
