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
#include <QMessageBox>
#include <QApplication>

#include "micmonitor.h"

namespace MicMonitor {
    void pw_registry_destroy(pw_registry* ctx) {
        pw_proxy_destroy(reinterpret_cast<pw_proxy*>(ctx));
    }

    void registry_global_remove(void* data, uint32_t id) {
        if(auto* ctx = static_cast<PipeWire*>(data)) {
            ctx->onRegistryGlobalRemove(id);
        }
    }

    void registry_global(void* data, uint32_t id, uint32_t permissions, const char* type, uint32_t version, const spa_dict *props) {
        if(auto* ctx = static_cast<PipeWire*>(data)) {
            ctx->onRegistryGlobal(id, permissions, type, version, props);
        }
    }

    const struct pw_registry_events registryEvents = {
        .version = PW_VERSION_REGISTRY_EVENTS,
        .global = registry_global,
        .global_remove = registry_global_remove,
    };

    class PwLockGuard {
        pw_thread_loop* ctx;
    public:
        PwLockGuard(pw_thread_loop* ptr) : ctx(ptr) {
            pw_thread_loop_lock(ctx);
        }

        ~PwLockGuard() {
            pw_thread_loop_unlock(ctx);
        }
    };

    PipeWire::PipeWire(QObject* parent) : QObject(parent) {
        pw_init(nullptr, nullptr);

        threadLoop.reset(pw_thread_loop_new("micmonitor-loop", nullptr));
        if(!threadLoop) {
            throw std::runtime_error("pw_thread_loop is null");
        }

        context.reset(pw_context_new(pw_thread_loop_get_loop(threadLoop.get()), nullptr, 0));
        if(! context) {
            throw std::runtime_error("pw_context is null");
        }

        if(pw_thread_loop_start(threadLoop.get()) < 0) {
            throw std::runtime_error("pw_thread_loop_start failed");
        }

        listenLoopStart();
    }

    PipeWire::~PipeWire() {
        pw_thread_loop_stop(threadLoop.get());
        {
            const PwLockGuard guard(threadLoop.get());
            spa_hook_remove(&registryListener);
            registry.reset();
            core.reset();
            context.reset();
        }
        threadLoop.reset(); 
        pw_deinit();
    }

    bool PipeWire::isValid() const {
        return !! context;
    }

    void PipeWire::listenLoopStart() {
        const PwLockGuard guard(threadLoop.get());
        core.reset(pw_context_connect(context.get(), nullptr, 0));

        if(! core) {
            throw std::runtime_error("pipewire service not found");
        }

        registry.reset(pw_core_get_registry(core.get(), PW_VERSION_REGISTRY, 0));

        if(! registry) {
            throw std::runtime_error("pw_registry failed");
        }

        spa_zero(registryListener);
        pw_registry_add_listener(registry.get(), & registryListener, & registryEvents, this);
    }

    void PipeWire::onRegistryGlobalRemove(uint32_t id) {
        emit micListenOff(id);
    }

    void PipeWire::onRegistryGlobal(uint32_t id, uint32_t permissions, const char* type, uint32_t version, const spa_dict* props) {
        if(std::string_view{type} != PW_TYPE_INTERFACE_Node) return;

        if(props == nullptr) {
            return;
        }

        const char* media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);

        if(media_class == nullptr) {
            return;
        }

        if(std::string_view{media_class} != "Stream/Input/Audio") return;

        const char* app_name = spa_dict_lookup(props, PW_KEY_APP_NAME);
        const char* app_binary = spa_dict_lookup(props, PW_KEY_APP_PROCESS_BINARY);
        const char* app_pid = spa_dict_lookup(props, PW_KEY_APP_PROCESS_ID);

        emit micListenOn(id, (app_name ? app_name : ""), (app_binary ? app_binary : ""), (app_pid ? app_pid : ""));
    }

    Applet::Applet(QWidget *parent) : QWidget(parent) {
        actionSettings = new QAction("Settings", this);
        actionAbout = new QAction("About", this);
        actionExit = new QAction("Exit", this);

        auto version = QString("%1 version: %2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());

        mic = std::make_unique<PipeWire>();

        connect(mic.get(), &PipeWire::micListenOn, this, &Applet::listenAppAdded, Qt::QueuedConnection);
        connect(mic.get(), &PipeWire::micListenOff, this, &Applet::listenAppRemoved, Qt::QueuedConnection);

        QMenu* menu = new QMenu(this);
        menu->addAction(actionSettings);
        menu->addSeparator();
        menu->addAction(actionAbout);
        menu->addSeparator();
        menu->addAction(actionExit);

        pixon = QPixmap(QString(":/icons/mic_on"));
        pixof = QPixmap(QString(":/icons/mic_off"));

        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(pixof);
        trayIcon->setToolTip(version);
        trayIcon->setContextMenu(menu);

        //connect(actionSettings, SIGNAL(triggered()), this, SLOT(show()));
        connect(actionExit, &QAction::triggered, this, &Applet::closeProgram);
        connect(actionAbout, &QAction::triggered, this, &Applet::aboutProgram);
        connect(trayIcon, &QSystemTrayIcon::activated, this, &Applet::iconActivated);

        trayIcon->show();
    }

    void Applet::closeProgram() {
        QCoreApplication::quit();
    }

    void Applet::aboutProgram() {
        auto github = QString("https://github.com/AndreyBarmaley/QMicMonitor");
        QMessageBox aboutBox(this);
        aboutBox.setWindowTitle("About MicMonitor");
        aboutBox.setText(QString("<h3>%1 version: %2</h3>").
            arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));
        aboutBox.setInformativeText(QString("<p>Source code: <a href='%1'>%2</a></p>"
            "<p>Copyright © 2026 by Andrey Afletdinov <public.irkutsk@gmail.com></p>").arg(github).arg("GitHub"));
        aboutBox.setStandardButtons(QMessageBox::Ok);
        aboutBox.exec();
    }

    void Applet::iconActivated(QSystemTrayIcon::ActivationReason reason) {
        if(reason == QSystemTrayIcon::Trigger) {
        }
    }

    void Applet::listenAppAdded(uint32_t id, const QString& name, const QString& path, const QString& pid) {
        apps.insert(id, AppInfo{name, path, pid});
        trayIcon->setIcon(pixon);

        auto tooltip = QString("%1 (%2, %3)").arg(name).arg(pid).arg(path);
        trayIcon->setToolTip(tooltip);
    }

    void Applet::listenAppRemoved(uint32_t id) {
        apps.remove(id);

        if(apps.isEmpty()) {
            trayIcon->setIcon(pixof);

            auto tooltip = QString("%1 version: %2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
            trayIcon->setToolTip(tooltip);
        }
    }
}
