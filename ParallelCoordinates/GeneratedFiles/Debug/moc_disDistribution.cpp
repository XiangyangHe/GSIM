/****************************************************************************
** Meta object code from reading C++ file 'disDistribution.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../disDistribution.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'disDistribution.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DisDistribution_t {
    QByteArrayData data[7];
    char stringdata0[68];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DisDistribution_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DisDistribution_t qt_meta_stringdata_DisDistribution = {
    {
QT_MOC_LITERAL(0, 0, 15), // "DisDistribution"
QT_MOC_LITERAL(1, 16, 16), // "sendCurrentValue"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 1), // "v"
QT_MOC_LITERAL(4, 36, 15), // "getDistribution"
QT_MOC_LITERAL(5, 52, 13), // "vector<float>"
QT_MOC_LITERAL(6, 66, 1) // "d"

    },
    "DisDistribution\0sendCurrentValue\0\0v\0"
    "getDistribution\0vector<float>\0d"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DisDistribution[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   27,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Float,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

void DisDistribution::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DisDistribution *_t = static_cast<DisDistribution *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sendCurrentValue((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->getDistribution((*reinterpret_cast< vector<float>(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DisDistribution::*_t)(float );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DisDistribution::sendCurrentValue)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject DisDistribution::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DisDistribution.data,
      qt_meta_data_DisDistribution,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DisDistribution::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisDistribution::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DisDistribution.stringdata0))
        return static_cast<void*>(const_cast< DisDistribution*>(this));
    return QWidget::qt_metacast(_clname);
}

int DisDistribution::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DisDistribution::sendCurrentValue(float _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
