/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[22];
    char stringdata0[297];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 17), // "onStartMonitoring"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 16), // "onStopMonitoring"
QT_MOC_LITERAL(4, 47, 15), // "onCaptureScreen"
QT_MOC_LITERAL(5, 63, 10), // "openConfig"
QT_MOC_LITERAL(6, 74, 23), // "detectAndCaptureChanges"
QT_MOC_LITERAL(7, 98, 7), // "cv::Mat"
QT_MOC_LITERAL(8, 106, 12), // "currentFrame"
QT_MOC_LITERAL(9, 119, 11), // "onTestAlert"
QT_MOC_LITERAL(10, 131, 13), // "onViewHistory"
QT_MOC_LITERAL(11, 145, 18), // "onContinuousDetect"
QT_MOC_LITERAL(12, 164, 19), // "onContinuousCapture"
QT_MOC_LITERAL(13, 184, 17), // "stopAllMonitoring"
QT_MOC_LITERAL(14, 202, 5), // "mode1"
QT_MOC_LITERAL(15, 208, 5), // "mode2"
QT_MOC_LITERAL(16, 214, 6), // "newOCR"
QT_MOC_LITERAL(17, 221, 20), // "onAreaParamsReceived"
QT_MOC_LITERAL(18, 242, 13), // "startXPercent"
QT_MOC_LITERAL(19, 256, 12), // "widthPercent"
QT_MOC_LITERAL(20, 269, 13), // "startYPercent"
QT_MOC_LITERAL(21, 283, 13) // "heightPercent"

    },
    "MainWindow\0onStartMonitoring\0\0"
    "onStopMonitoring\0onCaptureScreen\0"
    "openConfig\0detectAndCaptureChanges\0"
    "cv::Mat\0currentFrame\0onTestAlert\0"
    "onViewHistory\0onContinuousDetect\0"
    "onContinuousCapture\0stopAllMonitoring\0"
    "mode1\0mode2\0newOCR\0onAreaParamsReceived\0"
    "startXPercent\0widthPercent\0startYPercent\0"
    "heightPercent"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x08 /* Private */,
       3,    0,   85,    2, 0x08 /* Private */,
       4,    0,   86,    2, 0x08 /* Private */,
       5,    0,   87,    2, 0x08 /* Private */,
       6,    1,   88,    2, 0x08 /* Private */,
       9,    0,   91,    2, 0x08 /* Private */,
      10,    0,   92,    2, 0x08 /* Private */,
      11,    0,   93,    2, 0x08 /* Private */,
      12,    0,   94,    2, 0x08 /* Private */,
      13,    0,   95,    2, 0x08 /* Private */,
      14,    0,   96,    2, 0x08 /* Private */,
      15,    0,   97,    2, 0x08 /* Private */,
      16,    0,   98,    2, 0x08 /* Private */,
      17,    4,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    0x80000000 | 7, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   18,   19,   20,   21,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onStartMonitoring(); break;
        case 1: _t->onStopMonitoring(); break;
        case 2: _t->onCaptureScreen(); break;
        case 3: _t->openConfig(); break;
        case 4: { cv::Mat _r = _t->detectAndCaptureChanges((*reinterpret_cast< const cv::Mat(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< cv::Mat*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->onTestAlert(); break;
        case 6: _t->onViewHistory(); break;
        case 7: _t->onContinuousDetect(); break;
        case 8: _t->onContinuousCapture(); break;
        case 9: _t->stopAllMonitoring(); break;
        case 10: _t->mode1(); break;
        case 11: _t->mode2(); break;
        case 12: _t->newOCR(); break;
        case 13: _t->onAreaParamsReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
