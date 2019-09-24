LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#LS_CPP=$(subst $(1)/,,$(wildcard $(1)/*.cpp))
LOCAL_MODULE    := droidsoccer
#LOCAL_SRC_FILES := $(call LS_CPP,$(LOCAL_PATH))
LOCAL_CFLAGS   := -O3 -mno-thumb
LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)

SRC_FILES := $(wildcard $(LOCAL_PATH)/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/ttf/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/nvtristrip/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/common/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/game/*.cpp)

SRC_FILES := $(SRC_FILES:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES := $(SRC_FILES)
LOCAL_LDLIBS    := -landroid -llog -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)


