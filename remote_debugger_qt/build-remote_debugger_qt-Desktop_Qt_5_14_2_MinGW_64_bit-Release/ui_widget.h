/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *serialLayout;
    QLabel *portLabel;
    QComboBox *portComboBox;
    QPushButton *refreshButton;
    QPushButton *connectButton;
    QHBoxLayout *controlLayout;
    QPushButton *powerOnButton;
    QPushButton *powerOffButton;
    QPushButton *resetButton;
    QHBoxLayout *firmwareLayout;
    QLabel *firmwareLabel;
    QLineEdit *pathEdit;
    QPushButton *browseButton;
    QHBoxLayout *otaOptionsLayout;
    QLabel *versionLabel;
    QSpinBox *versionSpinBox;
    QSpacerItem *otaSpacer;
    QPushButton *otaButton;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(620, 265);
        Widget->setMinimumSize(QSize(620, 265));
        verticalLayout = new QVBoxLayout(Widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        serialLayout = new QHBoxLayout();
        serialLayout->setObjectName(QString::fromUtf8("serialLayout"));
        portLabel = new QLabel(Widget);
        portLabel->setObjectName(QString::fromUtf8("portLabel"));

        serialLayout->addWidget(portLabel);

        portComboBox = new QComboBox(Widget);
        portComboBox->setObjectName(QString::fromUtf8("portComboBox"));

        serialLayout->addWidget(portComboBox);

        refreshButton = new QPushButton(Widget);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));

        serialLayout->addWidget(refreshButton);

        connectButton = new QPushButton(Widget);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));

        serialLayout->addWidget(connectButton);


        verticalLayout->addLayout(serialLayout);

        controlLayout = new QHBoxLayout();
        controlLayout->setObjectName(QString::fromUtf8("controlLayout"));
        powerOnButton = new QPushButton(Widget);
        powerOnButton->setObjectName(QString::fromUtf8("powerOnButton"));

        controlLayout->addWidget(powerOnButton);

        powerOffButton = new QPushButton(Widget);
        powerOffButton->setObjectName(QString::fromUtf8("powerOffButton"));

        controlLayout->addWidget(powerOffButton);

        resetButton = new QPushButton(Widget);
        resetButton->setObjectName(QString::fromUtf8("resetButton"));

        controlLayout->addWidget(resetButton);


        verticalLayout->addLayout(controlLayout);

        firmwareLayout = new QHBoxLayout();
        firmwareLayout->setObjectName(QString::fromUtf8("firmwareLayout"));
        firmwareLabel = new QLabel(Widget);
        firmwareLabel->setObjectName(QString::fromUtf8("firmwareLabel"));

        firmwareLayout->addWidget(firmwareLabel);

        pathEdit = new QLineEdit(Widget);
        pathEdit->setObjectName(QString::fromUtf8("pathEdit"));

        firmwareLayout->addWidget(pathEdit);

        browseButton = new QPushButton(Widget);
        browseButton->setObjectName(QString::fromUtf8("browseButton"));

        firmwareLayout->addWidget(browseButton);


        verticalLayout->addLayout(firmwareLayout);

        otaOptionsLayout = new QHBoxLayout();
        otaOptionsLayout->setObjectName(QString::fromUtf8("otaOptionsLayout"));
        versionLabel = new QLabel(Widget);
        versionLabel->setObjectName(QString::fromUtf8("versionLabel"));

        otaOptionsLayout->addWidget(versionLabel);

        versionSpinBox = new QSpinBox(Widget);
        versionSpinBox->setObjectName(QString::fromUtf8("versionSpinBox"));
        versionSpinBox->setMinimum(1);
        versionSpinBox->setMaximum(2147483647);
        versionSpinBox->setValue(1);

        otaOptionsLayout->addWidget(versionSpinBox);

        otaSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        otaOptionsLayout->addItem(otaSpacer);

        otaButton = new QPushButton(Widget);
        otaButton->setObjectName(QString::fromUtf8("otaButton"));
        otaButton->setMinimumSize(QSize(120, 0));

        otaOptionsLayout->addWidget(otaButton);


        verticalLayout->addLayout(otaOptionsLayout);

        progressBar = new QProgressBar(Widget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);

        verticalLayout->addWidget(progressBar);

        statusLabel = new QLabel(Widget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setMinimumSize(QSize(0, 28));
        statusLabel->setWordWrap(true);

        verticalLayout->addWidget(statusLabel);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Remote Debugger", nullptr));
        portLabel->setText(QCoreApplication::translate("Widget", "\350\256\276\345\244\207\344\270\262\345\217\243", nullptr));
        refreshButton->setText(QCoreApplication::translate("Widget", "\345\210\267\346\226\260", nullptr));
        connectButton->setText(QCoreApplication::translate("Widget", "\350\277\236\346\216\245", nullptr));
        powerOnButton->setText(QCoreApplication::translate("Widget", "\344\270\212\347\224\265", nullptr));
        powerOffButton->setText(QCoreApplication::translate("Widget", "\346\226\255\347\224\265", nullptr));
        resetButton->setText(QCoreApplication::translate("Widget", "\345\244\215\344\275\215", nullptr));
        firmwareLabel->setText(QCoreApplication::translate("Widget", "\345\233\272\344\273\266\346\226\207\344\273\266", nullptr));
        pathEdit->setPlaceholderText(QCoreApplication::translate("Widget", "\351\200\211\346\213\251\351\223\276\346\216\245\345\210\2600x08001400\347\232\204bin\346\226\207\344\273\266", nullptr));
        browseButton->setText(QCoreApplication::translate("Widget", "\346\265\217\350\247\210...", nullptr));
        versionLabel->setText(QCoreApplication::translate("Widget", "\345\233\272\344\273\266\347\211\210\346\234\254", nullptr));
        otaButton->setText(QCoreApplication::translate("Widget", "\345\274\200\345\247\213OTA", nullptr));
        statusLabel->setText(QCoreApplication::translate("Widget", "\345\207\206\345\244\207\345\260\261\347\273\252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
