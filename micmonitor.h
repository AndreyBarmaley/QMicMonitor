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
#include <QPixmap>
#include <QWidget>
#include <QAction>
#include <QString>
#include <QCloseEvent>
#include <QSystemTrayIcon>

#include <pipewire/pipewire.h>
#include <memory>

namespace MicMonitor {
    void pw_registry_destroy(pw_registry* ctx);

    class PipeWire : public QObject {
        Q_OBJECT

        std::unique_ptr<pw_thread_loop, void(*)(pw_thread_loop*)> threadLoop = { nullptr, pw_thread_loop_destroy };
        std::unique_ptr<pw_context, void(*)(pw_context*)> context = { nullptr, pw_context_destroy };
        std::unique_ptr<pw_core, int(*)(pw_core*)> core = { nullptr, pw_core_disconnect };
        std::unique_ptr<pw_registry, void(*)(pw_registry*)> registry = { nullptr, pw_registry_destroy };

        struct spa_hook registryListener;

      protected:
        void listenLoopStart();

      public:
        PipeWire(QObject* parent = nullptr);
        ~PipeWire();

        bool isValid() const;

        void onRegistryGlobal(uint32_t id, uint32_t permissions, const char* type, uint32_t version, const spa_dict* props);
        void onRegistryGlobalRemove(uint32_t id);
        void disconnectNode(uint32_t);

      signals:
        void micListenOn(uint32_t id, const QString& app, const QString& path, const QString& pid);
        void micListenOff(uint32_t id);
    };

    class Applet : public QWidget {
        Q_OBJECT

        QSystemTrayIcon* trayIcon = nullptr;
        QAction* actionHideUnused = nullptr;
        QAction* actionAbout = nullptr;
        QAction* actionExit = nullptr;
        QMenu* sectionApps = nullptr;

        QPixmap pixon;
        QPixmap pixof;

        std::unique_ptr<PipeWire> mic;

      public:
        explicit Applet(QWidget* parent = nullptr);
        ~Applet() = default;

      private slots:
        void closeProgram();
        void aboutProgram();
        void appActionUnchecked();
        void iconHideUnused(bool);
        void iconActivated(QSystemTrayIcon::ActivationReason reason);
        void listenAppAdded(uint32_t id, const QString& app, const QString& path, const QString& pid);
        void listenAppRemoved(uint32_t id);
    };
}

#endif // MICMONITOR_H
