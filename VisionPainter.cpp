#include "VisionPainter.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QRect>
#include <QDebug>

VisionPainter::VisionPainter(QObject *parent)
    : QObject{parent}
{}

void VisionPainter::draw(const QImage &img,const std::vector<DetectionBox> &boxes)
{
    if(img.isNull()) return;

    QImage drawImg = img.copy();

    QPainter painter(&drawImg);
    painter.setRenderHint(QPainter::Antialiasing);

    for(const auto &box : boxes) {
        painter.setPen(QPen(Qt::green,2));
        painter.drawRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial",10,QFont::Bold));
        QString label = QString("ID:%1 | %2").arg(box.classId).arg(box.score);
        QRect textRect = painter.boundingRect(QRect(box.x1, box.y1 - 20, 100, 20),label).toRect();
        painter.fillRect(textRect,Qt::black);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }
    emit paintReady(drawImg);
}

void VisionPainter::onPostProcessReady(const std::vector<DetectionBox> &boxes)
{
    if(!m_isRunning)
        return;

    draw(m_drawImg,boxes);
}

void VisionPainter::ReceiveFrame(const QImage &img)
{
    if(!img.isNull())
        m_drawImg = img;
}
