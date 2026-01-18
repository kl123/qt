/********************************************************************************
** Form generated from reading UI file 'monitorconfig.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MONITORCONFIG_H
#define UI_MONITORCONFIG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_MonitorConfig
{
public:

    void setupUi(QDialog *MonitorConfig)
    {
        if (MonitorConfig->objectName().isEmpty())
            MonitorConfig->setObjectName(QString::fromUtf8("MonitorConfig"));
        MonitorConfig->resize(400, 469);

        retranslateUi(MonitorConfig);

        QMetaObject::connectSlotsByName(MonitorConfig);
    } // setupUi

    void retranslateUi(QDialog *MonitorConfig)
    {
        MonitorConfig->setWindowTitle(QCoreApplication::translate("MonitorConfig", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MonitorConfig: public Ui_MonitorConfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MONITORCONFIG_H
