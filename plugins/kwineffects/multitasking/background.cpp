#include "background.h"
#include "constants.h"
#include "kwineffects.h"

#include <QtDBus>
#include <QGSettings>
#include <QScreen>

#define DBUS_APPEARANCE_SERVICE "com.deepin.daemon.Appearance"
#define DBUS_APPEARANCE_OBJ "/com/deepin/daemon/Appearance"
#define DBUS_APPEARANCE_INTF "com.deepin.daemon.Appearance"

#define DBUS_DEEPIN_WM_SERVICE "com.deepin.wm"
#define DBUS_DEEPIN_WM_OBJ "/com/deepin/wm"
#define DBUS_DEEPIN_WM_INTF "com.deepin.wm"

const char fallback_background_name[] = "file:///usr/share/backgrounds/default_background.jpg";

Q_GLOBAL_STATIC_WITH_ARGS(QGSettings, _gs_dde_appearance, ("com.deepin.dde.appearance"))

#define GsettingsBackgroundUri "backgroundUris"

BackgroundManager& BackgroundManager::instance()
{
    static BackgroundManager* _self = nullptr;
    if (!_self) {
        _self = new BackgroundManager();
    }

    return *_self;
}

BackgroundManager::BackgroundManager()
    :QObject()
{
    m_defaultNewDesktopURI = QLatin1String(fallback_background_name);
    onGsettingsDDEAppearanceChanged(GsettingsBackgroundUri);

    connect(_gs_dde_appearance, &QGSettings::changed, this, &BackgroundManager::onGsettingsDDEAppearanceChanged);
    //Refreshes the thumbnails in the multitasking view
    //Temporary circumvention program exits unexpectedly
    //QDBusConnection::sessionBus().connect(DBUS_DEEPIN_WM_SERVICE, DBUS_DEEPIN_WM_OBJ, DBUS_DEEPIN_WM_INTF, "WorkspaceBackgroundChanged", this, SIGNAL(desktopWallpaperChanged(int)));

    emit defaultBackgroundURIChanged();
}

static QString toRealPath(const QString& path)
{
    QString res = path;

    QFileInfo fi(res);
    if (fi.isSymLink()) {
        res = fi.symLinkTarget();
    }

    return res;
}

QPixmap BackgroundManager::getBackground(int workspace, QString screenName, const QSize& size)
{
    QString uri = QLatin1String(fallback_background_name);
    QString strBackgroundPath = QString("%1%2").arg(workspace).arg(screenName);

    if (workspace <= 0) {
        //fallback to first workspace
        workspace = 1;
    }

    QDBusInterface wm(DBUS_DEEPIN_WM_SERVICE, DBUS_DEEPIN_WM_OBJ, DBUS_DEEPIN_WM_INTF);
    QDBusReply<QString> reply = wm.call( "GetWorkspaceBackgroundForMonitor", workspace, screenName);
    if (!reply.value().isEmpty()) {
        uri = reply.value();
    }

    if (uri.startsWith("file:///")) {
        uri.remove("file://");
    }

    uri = toRealPath(uri);

    if (m_cachedPixmaps.contains(uri  + strBackgroundPath)) {
        auto& p = m_cachedPixmaps[uri + strBackgroundPath];
        if (p.first != size) {
            p.first = size;
            p.second = p.second.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        }
        return p.second;
    }

    QPixmap pm;
    if (!pm.load(uri)) {
        uri = toRealPath(QString::fromUtf8(fallback_background_name).remove("file://"));
        pm.load(uri);
    }

    pm = pm.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    m_cachedPixmaps[uri + strBackgroundPath] = qMakePair(size, pm);
    return pm;
}

void BackgroundManager::onGsettingsDDEAppearanceChanged(const QString &key)
{
    //FIXME: no signal received sometimes during append desktop, why?
    qCDebug(BLUR_CAT) << "---------- " << __func__ << key;
    if (key == GsettingsBackgroundUri) {
        m_cachedUris = _gs_dde_appearance->get(GsettingsBackgroundUri).toStringList();
        emit wallpapersChanged();
    }
}

void BackgroundManager::desktopAboutToRemoved(int d)
{
    QDBusInterface wm(DBUS_DEEPIN_WM_SERVICE, DBUS_DEEPIN_WM_OBJ, DBUS_DEEPIN_WM_INTF);

    for(int i = 0; i < m_screenNamelst.count(); i++) {
        QString monitorName = m_screenNamelst.at(i);

        for(int i = d; i <  m_desktopCount; i++) {
            QDBusReply<QString> getReply = wm.call( "GetWorkspaceBackgroundForMonitor", i + 1, monitorName);
            QDBusReply<QString> setReply = wm.call( "SetWorkspaceBackgroundForMonitor", i, monitorName, getReply.value());
        }

    }
}

void BackgroundManager::desktopSwitchedPosition(int to, int from)
{
    QDBusInterface wm(DBUS_DEEPIN_WM_SERVICE, DBUS_DEEPIN_WM_OBJ, DBUS_DEEPIN_WM_INTF);

    for(int i = 0; i < m_screenNamelst.count(); i++) {
        QString monitorName = m_screenNamelst.at(i);

        QDBusReply<QString> getReply = wm.call( "GetWorkspaceBackgroundForMonitor", from, monitorName);
        QString strFromUri = getReply.value();

        if(from < to) {
            for(int j = from - 1; j < to; j++) {
                int desktopIndex = j + 1; //desktop index
                if( desktopIndex == to) {
                    QDBusReply<QString> setReply = wm.call( "SetWorkspaceBackgroundForMonitor", desktopIndex, monitorName, strFromUri);
                }else {
                    QDBusReply<QString> getReply = wm.call( "GetWorkspaceBackgroundForMonitor", desktopIndex + 1, monitorName);
                    QDBusReply<QString> setReply = wm.call( "SetWorkspaceBackgroundForMonitor", desktopIndex, monitorName, getReply.value());
                }
            }
        }else {
            for(int j = from; j > to - 1; j--) {
                if(j == to) {
                    QDBusReply<QString> setReply = wm.call( "SetWorkspaceBackgroundForMonitor", to, monitorName, strFromUri);
                }else {
                    QDBusReply<QString> getReply = wm.call( "GetWorkspaceBackgroundForMonitor", j - 1, monitorName);
                    QDBusReply<QString> setReply = wm.call( "SetWorkspaceBackgroundForMonitor", j, monitorName, getReply.value());
                }
            }
        }
    }
}

QString BackgroundManager::getDefaultBackgroundURI()
{
    return m_defaultNewDesktopURI;
}

void BackgroundManager::shuffleDefaultBackgroundURI()
{
    if (m_preinstalledWallpapers.size() == 0) {
        QDBusInterface remoteApp(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);
        QDBusReply<QString> reply = remoteApp.call( "List", "background");

        QJsonDocument json = QJsonDocument::fromJson(reply.value().toUtf8());
        QJsonArray arr = json.array();
        if (!arr.isEmpty()) {
            auto p = arr.constBegin();
            while (p != arr.constEnd()) {
                auto o = p->toObject();
                if (!o.value("Id").isUndefined() && !o.value("Deletable").toBool()) {
                    m_preinstalledWallpapers << o.value("Id").toString();
                }
                ++p;
            }
        }
    }

    if (m_preinstalledWallpapers.size() > 0) {
        int id = QRandomGenerator::global()->bounded(m_preinstalledWallpapers.size());
        m_defaultNewDesktopURI = m_preinstalledWallpapers[id];
        emit defaultBackgroundURIChanged();
    }
}

void BackgroundManager::setMonitorName(QList<QString> monitorNamelst)
{
    m_screenNamelst = monitorNamelst;
}

void BackgroundManager::changeWorkSpaceBackground(int workspaceIndex)
{
    QDBusInterface remoteApp(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);
    QDBusReply<QString> reply = remoteApp.call( "List", "background");

    QStringList backgroundAllLst;
    QString lastBackgroundUri;

    QJsonDocument json = QJsonDocument::fromJson(reply.value().toUtf8());
    QJsonArray arr = json.array();
    if (!arr.isEmpty()) {
        auto p = arr.constBegin();
        while (p != arr.constEnd()) {
            auto o = p->toObject();
            if (!o.value("Id").isUndefined() && !o.value("Deletable").toBool()) {
                backgroundAllLst << o.value("Id").toString();
            }
            ++p;
        }
    }

    QDBusInterface wm(DBUS_DEEPIN_WM_SERVICE, DBUS_DEEPIN_WM_OBJ, DBUS_DEEPIN_WM_INTF);

    for(int i = 0; i < m_screenNamelst.count(); i++) {

        QString monitorName = m_screenNamelst.at(i);
        QList<QString> backgroundUri;

        for(int i = 0; i < m_desktopCount; i++) {
            int desktopIndex = i + 1;
            QDBusReply<QString> getReply = wm.call( "GetWorkspaceBackgroundForMonitor", desktopIndex, monitorName);
            backgroundUri.append(getReply.value());
            lastBackgroundUri = getReply.value();
        }
        backgroundUri = backgroundUri.toSet().toList();

        for(int i = 0; i < backgroundUri.count(); i++) {
            QString background = backgroundUri.at(i);
            backgroundAllLst.removeOne(background);
        }

        if(backgroundAllLst.count() <= 0) {
            backgroundAllLst.append(lastBackgroundUri);
        }

        int backgroundIndex = backgroundAllLst.count();

        if(backgroundIndex - 1 != 0){
            backgroundIndex = qrand()%(backgroundAllLst.count() - 1);
        }else {
            backgroundIndex -= 1;
        }
        QDBusReply<QString> setReply = wm.call( "SetWorkspaceBackgroundForMonitor", workspaceIndex, monitorName, backgroundAllLst.at(backgroundIndex));
    }
}
