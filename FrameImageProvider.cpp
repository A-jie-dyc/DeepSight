#include "FrameImageProvider.h"
#include <QDebug>

FrameImageProvider::FrameImageProvider(QObject *parent)
    : QQuickImageProvider(QQuickImageProvider::Image)
{}

QImage FrameImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);
    if(m_showImg.isNull())
        return QImage(640,640,QImage::Format_RGB888);

    if(size)
        *size = m_showImg.size();
    return m_showImg;
}

void FrameImageProvider::onPaintReady(const QImage &img)
{
    m_showImg = img;
    emit imageUpdate();
}