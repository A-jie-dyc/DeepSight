#include "FrameDisplay.h"
#include <QPainter>

FrameDisplay::FrameDisplay(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    //设置离屏FBO渲染
    setRenderTarget(QQuickPaintedItem::Image);
    //快速调整FBO尺寸
    setPerformanceHints(QQuickPaintedItem::FastFBOResizing);
}

void FrameDisplay::paint(QPainter *painter)
{
    QImage img;
    {
        QMutexLocker locker(&m_mutex);
        if (m_image.isNull()) return;
        img = m_image;
    }

    //获取显示区域大小
    QSize itemSize = boundingRect().size().toSize();
    QSize imageSize = img.size();
    if (imageSize.isEmpty()) return;

    //计算比例缩放因子
    qreal scale = qMin((qreal)itemSize.width()  / imageSize.width(),
                       (qreal)itemSize.height() / imageSize.height());
    int scaledW = (int)(imageSize.width()  * scale);
    int scaledH = (int)(imageSize.height() * scale);

    //计算偏移量，实现居中显示
    int offsetX = (itemSize.width()  - scaledW) / 2;
    int offsetY = (itemSize.height() - scaledH) / 2;

    //开启平滑缩放
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    //绘制给前端
    painter->drawImage(QRect(offsetX, offsetY, scaledW, scaledH), img);
}

void FrameDisplay::updateFrame(const QImage &image)
{
    {
        QMutexLocker locker(&m_mutex);
        m_image = image;
    }
    update();
}
