#include "AssetReader.h"
#include <sys/stat.h>


GLshort AssetReader::readAssetFile(GLchar* filename) {
    strcpy(aFileName, filename);
    if(aBuffer != NULL){
        delete[] aBuffer;
        aBuffer = NULL;
    }
    AAsset* file = AAssetManager_open(mAssetManager, aFileName, AASSET_MODE_STREAMING);

    if (file == NULL)
        return -1;
        size_t iSize = (size_t) AAsset_getLength(file);
        aBuffer = new GLchar[iSize+1];

        if(iSize != AAsset_read(file, aBuffer, iSize)){
           AAsset_close(file);
           return -1;
        }
        aBuffer[iSize] = '\0';
        AAsset_close(file);
    return 0;
}


