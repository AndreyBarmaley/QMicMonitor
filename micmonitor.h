/***************************************************************************
 *   Copyright © 2026 by Andrey Afletdinov <public.irkutsk@gmail.com>      *
 *                                                                         *
 *   Part of the QMicMonitor                                               *
 *   https://github.com/AndreyBarmaley/QMicMonitor                         *
 *                                                                         *
 *   MIT License                                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef MICMONITOR_H
#define MICMONITOR_H

#define MICMONITOR_VERSION 20261010

#include <QMenu>
#include <QWidget>
#include <QAction>
#include <QString>
#include <QCloseEvent>
#include <QSystemTrayIcon>

namespace Ui {
    class Settings;
}

class MicMonitor : public QWidget {
    Q_OBJECT

    //Ui::Settings* ui = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QAction* actionSettings = nullptr;
    QAction* actionExit = nullptr;

  public:
    explicit MicMonitor(QWidget *parent = 0);
    ~MicMonitor();

  protected:
    void closeEvent(QCloseEvent*) override;
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

  private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void exitProgram(void);
};

#endif // MICMONITOR_H
