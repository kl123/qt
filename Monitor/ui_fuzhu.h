/********************************************************************************
** Form generated from reading UI file 'fuzhu.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FUZHU_H
#define UI_FUZHU_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_fuzhu
{
public:
    QMenuBar *menubar;
    QWidget *centralwidget;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *fuzhu)
    {
        if (fuzhu->objectName().isEmpty())
            fuzhu->setObjectName(QString::fromUtf8("fuzhu"));
        fuzhu->resize(800, 600);
        menubar = new QMenuBar(fuzhu);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        fuzhu->setMenuBar(menubar);
        centralwidget = new QWidget(fuzhu);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        fuzhu->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(fuzhu);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        fuzhu->setStatusBar(statusbar);

        retranslateUi(fuzhu);

        QMetaObject::connectSlotsByName(fuzhu);
    } // setupUi

    void retranslateUi(QMainWindow *fuzhu)
    {
        fuzhu->setWindowTitle(QCoreApplication::translate("fuzhu", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class fuzhu: public Ui_fuzhu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FUZHU_H
