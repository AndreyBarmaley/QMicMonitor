/***************************************************************************
 *   Copyright © 2026 by Andrey Afletdinov <public.irkutsk@gmail.com>      *
 *                                                                         *
 *   Part of the QMicMonitor                                               *
 *   https://github.com/AndreyBarmaley/QMicMonitor                         *
 *                                                                         *
 *   MIT License                                                           *
 *                                                                         *
 ***************************************************************************/

#include <QIcon>
#include <QDebug>
#include <QApplication>

#include "micmonitor.h"
//#include "ui_settings.h"


MicMonitor::MicMonitor(QWidget *parent) : QWidget(parent) { //, ui(new Ui::Settings) {
    actionSettings = new QAction("Settings", this);
    actionExit = new QAction("Exit", this);

    auto version = QString("%1 version: %2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
    //auto github = QString("https://github.com/AndreyBarmaley/qxkb5");

    //ui->setupUi(this);
    //ui->tabWidget->setCurrentIndex(0);
    //ui->aboutInfo->setText(QString("<center><b>%1</b></center><br><br>"
    //                               "<p>Source code: <a href='%2'>%2</a></p>"
    //                               "<p>Copyright © 2026 by Andrey Afletdinov <public.irkutsk@gmail.com></p>").arg(version).arg(github));

    QMenu* menu = new QMenu(this);
    menu->addAction(actionSettings);
    menu->addSeparator();
    menu->addAction(actionExit);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QPixmap(QString(":/icons/mic_off")));
    trayIcon->setToolTip(version);
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    //connect(actionSettings, SIGNAL(triggered()), this, SLOT(show()));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(exitProgram()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

MicMonitor::~MicMonitor() {
    // delete ui;
}

void MicMonitor::exitProgram(void) {
    hide();
    close();
}

void MicMonitor::showEvent(QShowEvent* event) {
    actionSettings->setDisabled(true);
}

void MicMonitor::hideEvent(QHideEvent* event) {
    actionSettings->setEnabled(true);
}

void MicMonitor::closeEvent(QCloseEvent* event) {
    if(isVisible()) {
        hide();
        event->ignore();
    } else {
        QCoreApplication::quit();
    }
}

void MicMonitor::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    if(reason == QSystemTrayIcon::Trigger) {
        //xcb->switchXkbLayout();
    }
}
