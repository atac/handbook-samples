/********************************************************************************
** Form generated from reading UI file 'sandbox.ui'
**
** Created by: Qt User Interface Compiler version 5.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SANDBOX_H
#define UI_SANDBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SandboxClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *SandboxClass)
    {
        if (SandboxClass->objectName().isEmpty())
            SandboxClass->setObjectName(QStringLiteral("SandboxClass"));
        SandboxClass->resize(600, 400);
        menuBar = new QMenuBar(SandboxClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        SandboxClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(SandboxClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        SandboxClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(SandboxClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        SandboxClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(SandboxClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        SandboxClass->setStatusBar(statusBar);

        retranslateUi(SandboxClass);

        QMetaObject::connectSlotsByName(SandboxClass);
    } // setupUi

    void retranslateUi(QMainWindow *SandboxClass)
    {
        SandboxClass->setWindowTitle(QApplication::translate("SandboxClass", "Sandbox", 0));
    } // retranslateUi

};

namespace Ui {
    class SandboxClass: public Ui_SandboxClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SANDBOX_H
