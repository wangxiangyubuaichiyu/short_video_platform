/********************************************************************************
** Form generated from reading UI file 'widget_main.ui'
**
** Created by: Qt User Interface Compiler version 6.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_MAIN_H
#define UI_WIDGET_MAIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget_Main
{
public:
    QLabel *lb_vid;

    void setupUi(QWidget *Widget_Main)
    {
        if (Widget_Main->objectName().isEmpty())
            Widget_Main->setObjectName("Widget_Main");
        Widget_Main->resize(800, 600);
        lb_vid = new QLabel(Widget_Main);
        lb_vid->setObjectName("lb_vid");
        lb_vid->setGeometry(QRect(70, 60, 691, 351));

        retranslateUi(Widget_Main);

        QMetaObject::connectSlotsByName(Widget_Main);
    } // setupUi

    void retranslateUi(QWidget *Widget_Main)
    {
        Widget_Main->setWindowTitle(QCoreApplication::translate("Widget_Main", "Widget_Main", nullptr));
        lb_vid->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Widget_Main: public Ui_Widget_Main {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_MAIN_H
