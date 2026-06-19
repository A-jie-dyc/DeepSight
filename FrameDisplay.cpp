#include "FrameDisplay.h"
#include <QPainter>
#include <QMutexLocker>
#include <QCursor>

FrameDisplay::FrameDisplay(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    //设置离屏FBO渲染
    setRenderTarget(QQuickPaintedItem::Image);
    //快速调整FBO尺寸
    setPerformanceHints(QQuickPaintedItem::FastFBOResizing);
    //启用鼠标左键事件
    setAcceptedMouseButtons(Qt::LeftButton);

    connect(this, &QQuickItem::widthChanged, this, &FrameDisplay::updateScaleParams);
    connect(this, &QQuickItem::heightChanged, this, &FrameDisplay::updateScaleParams);
}

void FrameDisplay::paint(QPainter *painter)
{
    //获取显示区域大小
    QSize itemSize = boundingRect().size().toSize();
    if (itemSize.isEmpty()) return;

    painter->fillRect(boundingRect(), Qt::black);

    QImage img;
    {
        QMutexLocker locker(&m_mutex);
        img = m_image;
    }

    if(!img.isNull()) {
        int scaledW = static_cast<int>(img.width() * m_scale);
        int scaledH = static_cast<int>(img.height() * m_scale);

        //开启平滑缩放
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        //绘制给前端
        painter->drawImage(QRect(m_offsetX, m_offsetY, scaledW, scaledH), img);
    }else if(!m_drawMode) {
        painter->setPen(QColor(160, 160, 160));
        QFont tipFont;
        tipFont.setPixelSize(16);
        painter->setFont(tipFont);
        painter->drawText(boundingRect(), Qt::AlignCenter, QStringLiteral("请打开摄像头或视频文件"));
    }

    //绘制模式
    if (m_drawMode) {
        //绘制临时统计线
        if(m_startPoint.x() >= 0 && m_startPoint.y() >= 0 &&
           m_endPoint.x() >= 0 && m_endPoint.y() >= 0) {
            painter->setPen(QPen(Qt::yellow, 3, Qt::DashLine));
            painter->drawLine(m_startPoint, m_endPoint);
        }

        //绘制操作提示
        painter->setRenderHint(QPainter::TextAntialiasing);
        QFont font;
        font.setFamily("Arial");
        font.setPixelSize(14);
        font.setBold(true);
        painter->setFont(font);
        QString tipText = QStringLiteral("绘制模式：按住左键拖动绘制统计线，点击按钮结束绘制");
        int padding = 8;    //文字内边距
        QRect textRect = painter->fontMetrics().boundingRect(tipText);
        QRect bgRect(10, 10, textRect.width() + padding * 2, textRect.height() + padding * 2);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 0, 160));
        painter->drawRoundedRect(bgRect, 4, 4);

        painter->setPen(Qt::white);
        painter->drawText(bgRect, Qt::AlignCenter, tipText);
    }
}

void FrameDisplay::updateFrame(const QImage &image)
{
    {
        QMutexLocker locker(&m_mutex);
        m_image = image;

        if(image.size() != m_lastImageSize) {
            calculateScaleParams();
        }
    }
    update();
}

//切换绘制模式
void FrameDisplay::setDrawMode(bool mode)
{
    if (m_drawMode == mode)
        return;

    m_drawMode = mode;
    emit drawModeChanged();

    //退出绘制模式，生成最终统计线并发送
    if (!mode) {
        //只有两个点都有效（坐标≥0）才生成统计线
        if (m_startPoint.x() >= 0 && m_startPoint.y() >= 0 &&
            m_endPoint.x() >= 0 && m_endPoint.y() >= 0) {

            //转换为视频原始坐标
            QPointF videoStart = screenToFrameCoord(m_startPoint);
            QPointF videoEnd = screenToFrameCoord(m_endPoint);

            //过滤太短的无效线
            if (QLineF(videoStart, videoEnd).length() < 500) {
                qDebug() << "统计线太短，已取消";
                emit countLineUpdated(Line{});
                emit errorOccurred(ErrorDef::Error_LineInvalid, QString("无效统计线绘制"));
            } else {
                //构造Line结构体
                Line line;
                line.start = cv::Point2f(videoStart.x(), videoStart.y());
                line.end = cv::Point2f(videoEnd.x(), videoEnd.y());
                line.isValid = true;
                line.direction = 1; //默认正向

                // 发送给Controller
                emit countLineUpdated(line);
                qDebug() << "统计线绘制完成："
                         << "起点(" << videoStart.x() << "," << videoStart.y() << ")"
                         << "终点(" << videoEnd.x() << "," << videoEnd.y() << ")";
            }
        }

        //重置临时坐标
        m_startPoint = QPointF(-1, -1);
        m_endPoint = QPointF(-1, -1);
    }

    //更新光标样式
    setCursor(mode ? QCursor(Qt::CrossCursor) : QCursor(Qt::ArrowCursor));
    update();
}

//屏幕坐标转视频原始坐标
QPointF FrameDisplay::screenToFrameCoord(const QPointF &screenPos)
{
    QMutexLocker locker(&m_mutex);
    if (m_image.isNull())
        return QPointF(0, 0);

    QSize imageSize = m_image.size();

    //屏幕坐标 → 原始视频坐标
    qreal videoX = (screenPos.x() - m_offsetX) / m_scale;
    qreal videoY = (screenPos.y() - m_offsetY) / m_scale;

    //边界限制，防止坐标越界
    videoX = qBound(0.0, videoX, (qreal)imageSize.width() - 1);
    videoY = qBound(0.0, videoY, (qreal)imageSize.height() - 1);

    return QPointF(videoX, videoY);
}

void FrameDisplay::calculateScaleParams()
{
    if(m_image.isNull()) {
        m_scale = 1.0;
        m_offsetX = 0;
        m_offsetY = 0;
        m_lastImageSize = QSize();
        return;
    }

    QSize itemSize = boundingRect().size().toSize();
    QSize imageSize = m_image.size();

    if(imageSize.isEmpty() || itemSize.isEmpty()) {
        m_scale = 0;
        m_offsetX = 0;
        m_offsetY = 0;
        return;
    }

    //等比例缩放计算
    qreal scale = qMin(
        static_cast<qreal>(itemSize.width()) / imageSize.width(),
        static_cast<qreal>(itemSize.height()) / imageSize.height()
        );

    int scaledW = static_cast<int>(imageSize.width() * scale);
    int scaledH = static_cast<int>(imageSize.height() * scale);
    int offsetX = (itemSize.width() - scaledW) / 2;
    int offsetY = (itemSize.height() - scaledH) / 2;

    //更新缓存
    m_scale = scale;
    m_offsetX = offsetX;
    m_offsetY = offsetY;
    m_lastImageSize = imageSize;
}

void FrameDisplay::updateScaleParams()
{
    QMutexLocker locker(&m_mutex);
    calculateScaleParams();
}

//鼠标按下：开始绘制
void FrameDisplay::mousePressEvent(QMouseEvent *event)
{
    if (!m_drawMode || event->button() != Qt::LeftButton)
        return;

    m_drawing = true;
    m_startPoint = event->position();
    m_endPoint = event->position();
    update();
}

//鼠标拖动：实时更新临时线
void FrameDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawMode || !m_drawing)
        return;

    m_endPoint = event->position();
    update();
}

//鼠标释放：结束绘制
void FrameDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_drawMode || !m_drawing || event->button() != Qt::LeftButton)
        return;

    m_drawing = false;
    m_endPoint = event->position();
    update();
}
