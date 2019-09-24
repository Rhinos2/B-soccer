#include "GameApp.h"
#include "EventLoop.h"
#include "Log.h"

void android_main(android_app* pApplication) {

    GameApp(pApplication).run();
}
