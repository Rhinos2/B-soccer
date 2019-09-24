#ifndef _ASSET_READER_H_
#define _ASSET_READER_H_

#include "Types.h"
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>

class AssetReader {
public:
    AssetReader(android_app* pApplication){
        mAssetManager = pApplication->activity->assetManager;
        aBuffer = NULL;
        strcpy(aFileName, "EMPTY");
    }
    GLshort readAssetFile(GLchar* filename);

   ~AssetReader(){
       if(aBuffer != NULL){
           delete[] aBuffer;
           aBuffer = NULL;
       }
   }
    GLchar* getaBuffer() {
        return &aBuffer[0];
    }
private:
    GLchar	aFileName[128];
    GLchar* aBuffer;
    AAssetManager* mAssetManager;
};

#endif
