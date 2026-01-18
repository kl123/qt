/****************************************************************************
** Meta object code from reading C++ file 'fuzhu.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../Monitor/fuzhu.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fuzhu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_fuzhu_t {
    QByteArrayData data[16];
    char stringdata0[203];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_fuzhu_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_fuzhu_t qt_meta_stringdata_fuzhu = {
    {
QT_MOC_LITERAL(0, 0, 5), // "fuzhu"
QT_MOC_LITERAL(1, 6, 19), // "areaParamsConfirmed"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 12), // "detectStartX"
QT_MOC_LITERAL(4, 40, 11), // "detectWidth"
QT_MOC_LITERAL(5, 52, 12), // "detectStartY"
QT_MOC_LITERAL(6, 65, 12), // "detectHeight"
QT_MOC_LITERAL(7, 78, 10), // "cardStartX"
QT_MOC_LITERAL(8, 89, 9), // "cardWidth"
QT_MOC_LITERAL(9, 99, 10), // "cardStartY"
QT_MOC_LITERAL(10, 110, 10), // "cardHeight"
QT_MOC_LITERAL(11, 121, 20), // "captureScreenAndShow"
QT_MOC_LITERAL(12, 142, 20), // "onSliderValueChanged"
QT_MOC_LITERAL(13, 163, 8), // "cv::Mat&"
QT_MOC_LITERAL(14, 172, 7), // "drawMat"
QT_MOC_LITERAL(15, 180, 22) // "onConfirmButtonClicked"

    },
    "fuzhu\0areaParamsConfirmed\0\0detectStartX\0"
    "detectWidth\0detectStartY\0detectHeight\0"
    "cardStartX\0cardWidth\0cardStartY\0"
    "cardHeight\0captureScreenAndShow\0"
    "onSliderValueChanged\0cv::Mat&\0drawMat\0"
    "onConfirmButtonClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_fuzhu[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    8,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   51,    2, 0x08 /* Private */,
      12,    1,   52,    2, 0x08 /* Private */,
      15,    0,   55,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,    3,    4,    5,    6,    7,    8,    9,   10,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,

       0        // eod
};

void fuzhu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<fuzhu *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->areaParamsConfirmed((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6])),(*reinterpret_cast< int(*)>(_a[7])),(*reinterpret_cast< int(*)>(_a[8]))); break;
        case 1: _t->captureScreenAndShow(); break;
        case 2: _t->onSliderValueChanged((*reinterpret_cast< cv::Mat(*)>(_a[1]))); break;
        case 3: _t->onConfirmButtonClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (fuzhu::*)(int , int , int , int , int , int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&fuzhu::areaParamsConfirmed)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject fuzhu::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_fuzhu.data,
    qt_meta_data_fuzhu,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *fuzhu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *fuzhu::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_fuzhu.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int fuzhu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void fuzhu::areaParamsConfirmed(int _t1, int _t2, int _t3, int _t4, int _t5, int _t6, int _t7, int _t8)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t8))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
