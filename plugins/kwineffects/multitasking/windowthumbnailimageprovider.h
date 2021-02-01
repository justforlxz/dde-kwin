#ifndef WINDOWTHUMBNAILIMAGEPROVIDER_H
#define WINDOWTHUMBNAILIMAGEPROVIDER_H

#include <QQuickImageProvider>

#include "multitasking_model.h"

class WindowThumbnailImageProvider : public QQuickImageProvider
{

public:
    WindowThumbnailImageProvider(QQmlImageProviderBase::ImageType type, QQmlImageProviderBase::Flags flags = Flags());

    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    MultitaskingModel *m_pMultitaskingModel;
};

#endif // WINDOWTHUMBNAILIMAGEPROVIDER_H
