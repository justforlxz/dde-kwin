#include "imageprovider.h"
#include "multitasking_model.h"

ImageProvider::ImageProvider(QQmlImageProviderBase::ImageType type, QQmlImageProviderBase::Flags flags)
    : QQuickImageProvider(type, flags)
{

}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);

    bool ok = false;
    qulonglong winId = id.toULongLong(&ok);
    if (!ok) {
        return QPixmap();
    }

    EffectWindow *window = effects->findWindow(winId);
    if (window) {
        return window->icon().pixmap(QSize(Constants::ICON_SIZE, Constants::ICON_SIZE));
    }

    return QPixmap();
}
