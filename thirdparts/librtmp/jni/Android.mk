# This is the Android makefile for librtmp
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	amf.c \
	hashswf.c \
	log.c \
	parseurl.c \
	rtmp.c

#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
#LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_CFLAGS += -DNO_CRYPTO

LOCAL_MODULE := librtmp
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
