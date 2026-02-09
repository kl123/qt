win32 {
    TOOLCHAIN_BIN =
    exists(D:/StudyApp/QT/qt_5.14.2/Tools/mingw730_64/bin/g++.exe) {
        TOOLCHAIN_BIN = D:/StudyApp/QT/qt_5.14.2/Tools/mingw730_64/bin
    } else: exists(D:/Qt/5.14.2/Tools/mingw730_64/bin/g++.exe) {
        TOOLCHAIN_BIN = D:/Qt/5.14.2/Tools/mingw730_64/bin
    }
    !isEmpty(TOOLCHAIN_BIN) {
        QMAKE_CXX = $$TOOLCHAIN_BIN/g++.exe
        QMAKE_CC = $$TOOLCHAIN_BIN/gcc.exe
        QMAKE_LINK = $$QMAKE_CXX
        QMAKE_AR = $$TOOLCHAIN_BIN/ar.exe cqs
        QMAKE_OBJCOPY = $$TOOLCHAIN_BIN/objcopy.exe
        QMAKE_STRIP = $$TOOLCHAIN_BIN/strip.exe
    }
}

QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += _Frees_ptr_opt_=

SOURCES += \
    fuzhu.cpp \
    image.cpp \
    login.cpp \
    main.cpp \
    mainwindow.cpp \
    monitorconfig.cpp \
    ocrhelper.cpp \
    userauth.cpp \
    third_party/RapidOcrOnnx/src/AngleNet.cpp \
    third_party/RapidOcrOnnx/src/CrnnNet.cpp \
    third_party/RapidOcrOnnx/src/DbNet.cpp \
    third_party/RapidOcrOnnx/src/OcrLite.cpp \
    third_party/RapidOcrOnnx/src/OcrLiteImpl.cpp \
    third_party/RapidOcrOnnx/src/OcrResultUtils.cpp \
    third_party/RapidOcrOnnx/src/OcrUtils.cpp \
    third_party/RapidOcrOnnx/src/clipper.cpp

HEADERS += \
    fuzhu.h \
    image.h \
    login.h \
    mainwindow.h \
    monitorconfig.h \
    ocrhelper.h \
    userauth.h

FORMS += \
    fuzhu.ui \
    login.ui \
    mainwindow.ui \
    monitorconfig.ui

QT += multimedia

# === OpenCV 设置 ===
OPENCV_DIR = $$PWD/asset/openCV-minGW

# 添加头文件路径
INCLUDEPATH += $$PWD
INCLUDEPATH += $$OPENCV_DIR/include

# ✅ 链接 MinGW 的 .a 静态库（注意名字要匹配！）
QMAKE_LIBS += $$OPENCV_DIR/libopencv_world450.dll.a

# === RapidOCR(Onnx) 设置 ===
RAPIDOCR_DIR = $$PWD/third_party/RapidOcrOnnx
ONNXRUNTIME_DIR = $$PWD/asset/onnxruntime-win-x64-1.15.1

INCLUDEPATH += $$RAPIDOCR_DIR/include
INCLUDEPATH += $$ONNXRUNTIME_DIR/include

# ✅ 自动复制 DLL 到输出目录（Windows）
win32 {
    OPENCV_BIN = $$OPENCV_DIR
    CONFIG(debug, debug|release) {
        TARGET_DIR = $$OUT_PWD/debug
    } else {
        TARGET_DIR = $$OUT_PWD/release
    }
    isEmpty(TARGET_DIR): TARGET_DIR = $$OUT_PWD

    OPENCV_DLLS = libopencv_world450.dll
    ONNXRUNTIME_BIN = $$ONNXRUNTIME_DIR/lib
    ONNXRUNTIME_DLLS = onnxruntime.dll onnxruntime_providers_shared.dll

    for(dll, OPENCV_DLLS) {
        dll_source = $$shell_path($$OPENCV_BIN/$$dll)
        dll_dest = $$shell_path($$TARGET_DIR/$$dll)
        QMAKE_POST_LINK += $$QMAKE_COPY \"$$dll_source\" \"$$dll_dest\" $$escape_expand(\\n\\t)
    }

    for(dll, ONNXRUNTIME_DLLS) {
        dll_source = $$shell_path($$ONNXRUNTIME_BIN/$$dll)
        dll_dest = $$shell_path($$TARGET_DIR/$$dll)
        QMAKE_POST_LINK += $$QMAKE_COPY \"$$dll_source\" \"$$dll_dest\" $$escape_expand(\\n\\t)
    }
}

RESOURCES += \
    audio.qrc

win32 {
    RC_ICONS = monitor.ico
}

win32 {
    LIBS += -lwinmm
}

win32:contains(QMAKE_CXX, g++) {
    QMAKE_LINK = $$QMAKE_CXX
    QMAKE_CXXFLAGS += -fexceptions
}

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
