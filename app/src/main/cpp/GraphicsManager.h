#ifndef _SIM_GRAPHICSMANAGER_H_
#define _SIM_GRAPHICSMANAGER_H_


#include "Types.h"
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>


class GraphicsManager {
public:
    GraphicsManager(android_app* pApplication);
    ~GraphicsManager();

    int32_t getRenderWidth() { return mRenderWidth; }
    int32_t getRenderHeight() { return mRenderHeight; }

    status start();
    void stop();
    status SwapBuffers();


private:
    GraphicsManager(const GraphicsManager&);
    void operator=(const GraphicsManager&);

    android_app* mApplication;

    GLint mRenderWidth;
    GLint mRenderHeight;

    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;


};
#endif
