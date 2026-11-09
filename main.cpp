/***************************************************************************
 *   Copyright © 2026 by Andrey Afletdinov <public.irkutsk@gmail.com>      *
 *                                                                         *
 *   Part of the QMicMonitor                                               *
 *   https://github.com/AndreyBarmaley/QMicMonitor                         *
 *                                                                         *
 *   MIT License                                                           *
 *                                                                         *
 ***************************************************************************/

#include <QDir>
#include <QDebug>
#include <QLockFile>
#include <QApplication>
#include <QStandardPaths>

#include <exception>
#include "micmonitor.h"
 
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("QMicMonitor");
    QCoreApplication::setApplicationVersion(QString::number(MICMONITOR_VERSION));

    auto localData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(localData);
    auto lockPath = QDir(localData).absoluteFilePath("lock");
    QLockFile lockFile(lockPath);

    if(! lockFile.tryLock(100)) {
        qWarning() << "also running, see lock" << lockPath;
        return 1;
    }

    try {
        MicMonitor widget(nullptr);
        widget.hide();
        return app.exec();
    } catch(const std::exception & err) {
        qWarning() << err.what();
    }

    return -1;
}
