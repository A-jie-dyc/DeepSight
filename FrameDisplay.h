#ifndef FRAMEDISPLAY_H
#define FRAMEDISPLAY_H

#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>
#include <QImage>
#include <QMutex>
#include <QMouseEvent>
#include "CommonDef.h"

class FrameDisplay : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool drawMode READ drawMode WRITE setDrawMode NOTIFY drawModeChanged FINAL)
public:
    explicit FrameDisplay(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    bool drawMode() const { return m_drawMode; };
    void setDrawMode(bool mode);

public slots:
    void updateFrame(const QImage &image);

signals:
    void drawModeChanged();
    void countLineUpdated(const Line &line);
    void errorOccurred(ErrorDef::ErrorType type, const QString &msg);

protected:
    //重写鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    //qml坐标->视频帧原始坐标
    QPointF screenToFrameCoord(const QPointF &qmlPos);
    void calculateScaleParams();                //外部加锁
    void updateScaleParams();                   //内部用

    QImage m_image;
    QMutex m_mutex;
    bool m_drawMode = false;
    QPointF m_startPoint = QPointF(-1, -1);
    QPointF m_endPoint = QPointF(-1, -1);
    bool m_drawing = false;
    //缓存缩放参数
    qreal m_scale =1.0;
    int m_offsetX = 0;
    int m_offsetY = 0;
    QSize m_lastImageSize;
};

#endif // FRAMEDISPLAY_H