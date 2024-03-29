#include "EventLoop.h"
#include "Log.h"
#include "GameApp.h"


EventLoop::EventLoop(android_app* pApplication, GameApp& pActivityHandler, InputHandler& pInputHandler):
        mApplication(pApplication),
        mActivityHandler(pActivityHandler),
        mEnabled(false), mQuit(false),
        mInputHandler(pInputHandler)
{
    mApplication->userData = this;
    mApplication->onAppCmd = callback_appEvent;     //void (*onAppCmd)(struct android_app* app, int32_t cmd);
    mApplication->onInputEvent = callback_input;  //int32_t (*onInputEvent)(struct android_app* app, AInputEvent* event);
}

void EventLoop::run() {
    int32_t result; int32_t events;
    android_poll_source* source;

    while (true) {
        // Event processing loop.
        while ((result = ALooper_pollAll(mEnabled ? 0 : -1, NULL, &events, (void**) &source)) >= 0) {
            if (source != NULL) {
                source->process(mApplication, source);
            }
            // Application is getting destroyed.
            if (mApplication->destroyRequested) {
                return;
            }
        }
        // Steps the application.
        if ((mEnabled) && (!mQuit)) {
            if (mActivityHandler.onStep() != STATUS_OK) {
                mQuit = true;
                ANativeActivity_finish(mApplication->activity);
            }
        }
    }
}

void EventLoop::activate() {
    // Enables activity only if a window is available.
    if ((!mEnabled) && (mApplication->window != NULL)) {
        mQuit = false; mEnabled = true;
        if (mActivityHandler.onActivate() != STATUS_OK) {
            mQuit = true;
            deactivate();
            ANativeActivity_finish(mApplication->activity);
        }
    }
    return;
}

void EventLoop::deactivate() {
    if (mEnabled) {
        mActivityHandler.onDeactivate();
        mEnabled = false;
    }
}

void EventLoop::callback_appEvent(android_app* pApplication, int32_t pCommand) {
    EventLoop& eventLoop = *(EventLoop*) pApplication->userData;
    eventLoop.processAppEvent(pCommand);
}

void EventLoop::processAppEvent(int32_t pCommand) {
    switch (pCommand) {
    case APP_CMD_CONFIG_CHANGED:
        mActivityHandler.onConfigurationChanged();
        break;
    case APP_CMD_INIT_WINDOW:
        mActivityHandler.onCreateWindow();
        break;
    case APP_CMD_DESTROY:
        mActivityHandler.onDestroy();
        break;
    case APP_CMD_GAINED_FOCUS:
        activate();
        mActivityHandler.onGainFocus();
        break;
    case APP_CMD_LOST_FOCUS:
        mActivityHandler.onLostFocus();
        deactivate();
        break;
    case APP_CMD_LOW_MEMORY:
        mActivityHandler.onLowMemory();
        break;
    case APP_CMD_PAUSE:
        mActivityHandler.onPause();
        deactivate();
        break;
    case APP_CMD_RESUME:
        mActivityHandler.onResume();
        break;
    case APP_CMD_SAVE_STATE:
        mActivityHandler.onSaveInstanceState(
            &mApplication->savedState, &mApplication->savedStateSize);
        break;
    case APP_CMD_START:
        mActivityHandler.onStart();
        break;
    case APP_CMD_STOP:
        mActivityHandler.onStop();
        break;
    case APP_CMD_TERM_WINDOW:
        mActivityHandler.onDestroyWindow();
        deactivate();
        break;
    default:
        break;
    }
}
int32_t EventLoop::callback_input(android_app* pApplication, AInputEvent* pEvent) {
    EventLoop& eventLoop = *(EventLoop*) pApplication->userData;
    return eventLoop.processInputEvent(pEvent);
}

int32_t EventLoop::processInputEvent(AInputEvent* pEvent) {
    int32_t eventType = AInputEvent_getType(pEvent);
    switch (eventType) {
        case AINPUT_EVENT_TYPE_MOTION:
            return 0;
    }
}