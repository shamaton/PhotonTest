# Generated by VisualGDB

LOCAL_PATH := $(call my-dir)
COCOS2DX_ROOT := $(LOCAL_PATH)/../../../../etc-bin/cocos2dx
PHOTON_SDK_ROOT := $(LOCAL_PATH)/../../../../..

include $(CLEAR_VARS)

$(shell mkdir assets)
$(shell cp -rf ../resources/* assets)

LOCAL_MODULE := demochat
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := ../../src/AppDelegate.cpp ../../src/DemoScene.cpp ../../src/ListController.cpp main.cpp

LOCAL_C_INCLUDES := $(COCOS2DX_ROOT)/cocos2dx $(COCOS2DX_ROOT)/cocos2dx/include $(COCOS2DX_ROOT)/cocos2dx/platform/android $(COCOS2DX_ROOT)/cocos2dx/kazmath/include $(PHOTON_SDK_ROOT) $(LOCAL_PATH)/../../inc

LOCAL_CFLAGS := -DEG_DEBUGGER -D__STDINT_LIMITS -D_EG_ANDROID_PLATFORM
LOCAL_STATIC_LIBRARIES := chat-cpp-static-prebuilt photon-cpp-static-prebuilt common-cpp-static-prebuilt
LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(PHOTON_SDK_ROOT)/Chat-cpp/lib)
$(call import-add-path, $(COCOS2DX_ROOT))
$(call import-add-path, $(COCOS2DX_ROOT)/cocos2dx/platform/third_party/android/prebuilt)

$(call import-module,chat-cpp-prebuilt)
$(call import-module,cocos2dx)
