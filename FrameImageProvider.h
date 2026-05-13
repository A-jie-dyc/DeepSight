#ifndef FRAMEIMAGEPROVIDER_H
#define FRAMEIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QImage>

class FrameImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit FrameImageProvider(QObject *parent = nullptr);
    //QML请求图像时自动调用(纯虚函数)
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

public slots:
    void onPaintReady(const QImage &drawImg);

signals:
    void imageUpdate();

private:
    QImage m_showImg;
};

#endif // FRAMEIMAGEPROVIDER_H
