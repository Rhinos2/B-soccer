#include "GraphicsManager.h"
#include "Log.h"



GraphicsManager::GraphicsManager(android_app* pApplication) :
    mApplication(pApplication),
    mDisplay(EGL_NO_DISPLAY), mSurface(EGL_NO_CONTEXT),
    mContext(EGL_NO_SURFACE)
    {
        mRenderWidth = 0;
        mRenderHeight = 0;

    }

GraphicsManager::~GraphicsManager() {

}


status GraphicsManager::start() {
    Log::info("Starting GraphicsManager.");
    EGLint format, numConfigs, errorResult; GLenum status;
    EGLConfig config;
    // Defines display requirements. 16bits mode here.
    const EGLint DISPLAY_ATTRIBS[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 5, EGL_GREEN_SIZE, 6, EGL_RED_SIZE, 5,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    // Request an OpenGL ES 2 context.
    const EGLint CONTEXT_ATTRIBS[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };

    // Retrieves a display connection and initializes it.
    Log::info("Connecting to the display.");
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mDisplay == EGL_NO_DISPLAY) {
        Log::error("Error eglGetDisplay");
        stop();
        return STATUS_KO;
    }
    if (!eglInitialize(mDisplay, NULL, NULL)) {
        Log::error("Error eglGetDisplay.");
        stop();
        return STATUS_KO;
    }

    // Selects the first OpenGL configuration found.
    Log::info("Selecting a display config.");
    if(!eglChooseConfig(mDisplay, DISPLAY_ATTRIBS, &config, 1,
        &numConfigs) || (numConfigs <= 0)) {
        Log::error("Error eglChooseConfig.");
        stop();
        return STATUS_KO;
    }

    // Reconfigures the Android window with the EGL format.
    Log::info("Configuring window format.");
    if (!eglGetConfigAttrib(mDisplay, config,
        EGL_NATIVE_VISUAL_ID, &format)) {
        Log::error("Error eglGetConfigAttrib.");
        stop();
        return STATUS_KO;
    }
    ANativeWindow_setBuffersGeometry(mApplication->window, 0, 0,
        format);

    // Creates the display surface.
    Log::info("Initializing the display.");
    mSurface = eglCreateWindowSurface(mDisplay, config,
        mApplication->window, NULL);
    if (mSurface == EGL_NO_SURFACE){
        Log::error("Error eglCreateWindowSurface.");
        stop();
        return STATUS_KO;
    }
    mContext = eglCreateContext(mDisplay, config, NULL,
        CONTEXT_ATTRIBS);
    if (mContext == EGL_NO_CONTEXT) {
        Log::error("Error eglCreateContext.");
        stop();
        return STATUS_KO;
    }

    // Activates the display surface.
    Log::info("Activating the display.");
    if (!eglMakeCurrent(mDisplay, mSurface, mSurface, mContext)
   || !eglQuerySurface(mDisplay, mSurface, EGL_WIDTH, &mRenderWidth)
   || !eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &mRenderHeight)
   || (mRenderWidth <= 0) || (mRenderHeight <= 0)) {
        Log::error("Error Activating the display.");
        stop();
        return STATUS_KO;
    }

    return STATUS_OK;

}

void GraphicsManager::stop() {

    // Destroys OpenGL context.
    if (mDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
        if (mContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mDisplay, mContext);
            mContext = EGL_NO_CONTEXT;
        }
        if (mSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mDisplay, mSurface);
            mSurface = EGL_NO_SURFACE;
        }
        eglTerminate(mDisplay);
        mDisplay = EGL_NO_DISPLAY;
    }
}

status GraphicsManager::SwapBuffers() {

    if (eglSwapBuffers(mDisplay, mSurface) != EGL_TRUE) {
        Log::error("Error %d swapping buffers.", eglGetError());
        return STATUS_KO;
    } else {
        return STATUS_OK;
    }
}

