#ifndef _SIM_EVENTLOOP_H_
#define _SIM_EVENTLOOP_H_

#include "InputHandler.h"
#include <android_native_app_glue.h>

class GameApp;

class EventLoop {
public:
    EventLoop(android_app* pApplication, GameApp& pActivityHandler, InputHandler& pInputHandler);
    void run();

private:
    void activate();
    void deactivate();

    void processAppEvent(int32_t pCommand);
    int32_t processInputEvent(AInputEvent* pEvent);

    static void callback_appEvent(android_app* pApplication, int32_t pCommand);
    static int32_t callback_input(android_app* pApplication, AInputEvent* pEvent);

private:
    android_app* mApplication;
    bool mEnabled;
    bool mQuit;

    GameApp& mActivityHandler;
    InputHandler& mInputHandler;
};
#endif
