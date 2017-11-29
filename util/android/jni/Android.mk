LOCAL_PATH := $(call my-dir)/../../..

include $(CLEAR_VARS)

LOCAL_MODULE    := attract

LOCAL_CPP_FEATURES := exceptions rtti

# HAVE_MEMMOVE hacked in to get expat to build
LOCAL_CFLAGS += -DNO_MOVIE -DNO_SWF -DFE_VERSION_NUM=212 -DFE_VERSION_D='"2.3.0"' -DHAVE_MEMMOVE

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/extlibs/expat \
	$(LOCAL_PATH)/extlibs/sqrat/include \
	$(LOCAL_PATH)/extlibs/squirrel/include \
	$(LOCAL_PATH)/extlibs/miniz

LOCAL_SRC_FILES := \
        src/fe_base.cpp \
        src/fe_file.cpp \
        src/fe_util.cpp \
        src/fe_util_sq.cpp \
        src/fe_util_android.cpp \
        src/fe_cmdline.cpp \
        src/fe_info.cpp \
        src/fe_input.cpp \
        src/fe_romlist.cpp \
        src/fe_settings.cpp \
        src/scraper_xml.cpp \
        src/scraper_general.cpp \
        src/scraper_net.cpp \
        src/fe_config.cpp \
        src/fe_presentable.cpp \
        src/fe_present.cpp \
        src/sprite.cpp \
        src/fe_image.cpp \
        src/fe_sound.cpp \
        src/fe_shader.cpp \
        src/fe_overlay.cpp \
        src/fe_window.cpp \
        src/tp.cpp \
        src/fe_text.cpp \
        src/fe_listbox.cpp \
        src/fe_vm.cpp \
        src/zip.cpp \
        src/main.cpp \
        src/fe_net.cpp \
        extlibs/squirrel/squirrel/sqapi.cpp \
        extlibs/squirrel/squirrel/sqbaselib.cpp \
        extlibs/squirrel/squirrel/sqfuncstate.cpp \
        extlibs/squirrel/squirrel/sqdebug.cpp \
        extlibs/squirrel/squirrel/sqlexer.cpp \
        extlibs/squirrel/squirrel/sqobject.cpp \
        extlibs/squirrel/squirrel/sqcompiler.cpp \
        extlibs/squirrel/squirrel/sqstate.cpp \
        extlibs/squirrel/squirrel/sqtable.cpp \
        extlibs/squirrel/squirrel/sqmem.cpp \
        extlibs/squirrel/squirrel/sqvm.cpp \
        extlibs/squirrel/squirrel/sqclass.cpp \
        extlibs/squirrel/sqstdlib/sqstdblob.cpp \
        extlibs/squirrel/sqstdlib/sqstdio.cpp \
        extlibs/squirrel/sqstdlib/sqstdstream.cpp \
        extlibs/squirrel/sqstdlib/sqstdmath.cpp \
        extlibs/squirrel/sqstdlib/sqstdstring.cpp \
        extlibs/squirrel/sqstdlib/sqstdaux.cpp \
        extlibs/squirrel/sqstdlib/sqstdsystem.cpp \
        extlibs/squirrel/sqstdlib/sqstdrex.cpp \
        extlibs/expat/xmlparse.c \
        extlibs/expat/xmlrole.c \
        extlibs/expat/xmltok.c

LOCAL_SHARED_LIBRARIES := freetype
LOCAL_SHARED_LIBRARIES += openal
LOCAL_SHARED_LIBRARIES += jpeg
LOCAL_SHARED_LIBRARIES += sfml-system
LOCAL_SHARED_LIBRARIES += sfml-window
LOCAL_SHARED_LIBRARIES += sfml-graphics
LOCAL_SHARED_LIBRARIES += sfml-audio
LOCAL_SHARED_LIBRARIES += sfml-network
LOCAL_SHARED_LIBRARIES += sfml-activity
LOCAL_WHOLE_STATIC_LIBRARIES := sfml-main

include $(BUILD_SHARED_LIBRARY)

$(call import-module,sfml)
