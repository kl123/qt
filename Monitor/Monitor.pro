QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    fuzhu.cpp \
    home.cpp \
    image.cpp \
    main.cpp \
    mainwindow.cpp \
    monitorconfig.cpp \
    ocrhelper.cpp

HEADERS += \
    fuzhu.h \
    home.h \
    image.h \
    mainwindow.h \
    monitorconfig.h \
    ocrhelper.h

FORMS += \
    fuzhu.ui \
    home.ui \
    mainwindow.ui \
    monitorconfig.ui

QT += multimedia
QT += charts

# === OpenCV 设置 ===
OPENCV_DIR = $$PWD/asset/openCV-minGW

# 添加头文件路径
INCLUDEPATH += $$OPENCV_DIR/include

# ✅ 链接 MinGW 的 .a 静态库（注意名字要匹配！）
QMAKE_LIBS += $$OPENCV_DIR/libopencv_world450.dll.a

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

    for(dll, OPENCV_DLLS) {
        dll_source = $$shell_path($$OPENCV_BIN/$$dll)
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

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
