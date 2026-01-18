/****************************************************************************
** Meta object code from reading C++ file 'monitorconfig.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../monitorconfig.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'monitorconfig.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MonitorConfig_t {
    QByteArrayData data[13];
    char stringdata0[256];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MonitorConfig_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MonitorConfig_t qt_meta_stringdata_MonitorConfig = {
    {
QT_MOC_LITERAL(0, 0, 13), // "MonitorConfig"
QT_MOC_LITERAL(1, 14, 27), // "on_audioSourceCombo_changed"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 5), // "index"
QT_MOC_LITERAL(4, 49, 28), // "on_selectAudioButton_clicked"
QT_MOC_LITERAL(5, 78, 27), // "on_addKeywordButton_clicked"
QT_MOC_LITERAL(6, 106, 22), // "on_radioCustom_toggled"
QT_MOC_LITERAL(7, 129, 7), // "checked"
QT_MOC_LITERAL(8, 137, 26), // "on_playAudioButton_clicked"
QT_MOC_LITERAL(9, 164, 26), // "on_shutAudioButton_clicked"
QT_MOC_LITERAL(10, 191, 41), // "on_keywordList_customContextM..."
QT_MOC_LITERAL(11, 233, 3), // "pos"
QT_MOC_LITERAL(12, 237, 18) // "onLoopTimerTimeout"

    },
    "MonitorConfig\0on_audioSourceCombo_changed\0"
    "\0index\0on_selectAudioButton_clicked\0"
    "on_addKeywordButton_clicked\0"
    "on_radioCustom_toggled\0checked\0"
    "on_playAudioButton_clicked\0"
    "on_shutAudioButton_clicked\0"
    "on_keywordList_customContextMenuRequested\0"
    "pos\0onLoopTimerTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MonitorConfig[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x08 /* Private */,
       4,    0,   57,    2, 0x08 /* Private */,
       5,    0,   58,    2, 0x08 /* Private */,
       6,    1,   59,    2, 0x08 /* Private */,
       8,    0,   62,    2, 0x08 /* Private */,
       9,    0,   63,    2, 0x08 /* Private */,
      10,    1,   64,    2, 0x08 /* Private */,
      12,    0,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   11,
    QMetaType::Void,

       0        // eod
};

void MonitorConfig::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MonitorConfig *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_audioSourceCombo_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->on_selectAudioButton_clicked(); break;
        case 2: _t->on_addKeywordButton_clicked(); break;
        case 3: _t->on_radioCustom_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->on_playAudioButton_clicked(); break;
        case 5: _t->on_shutAudioButton_clicked(); break;
        case 6: _t->on_keywordList_customContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 7: _t->onLoopTimerTimeout(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MonitorConfig::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_MonitorConfig.data,
    qt_meta_data_MonitorConfig,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MonitorConfig::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MonitorConfig::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MonitorConfig.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MonitorConfig::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
