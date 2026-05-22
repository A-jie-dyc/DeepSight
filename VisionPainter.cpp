#include "VisionPainter.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QRect>

VisionPainter::VisionPainter(QObject *parent)
    : QObject{parent}
{}

void VisionPainter::draw(const QImage &img,const std::vector<Track> &rawTracks)
{
    if(img.isNull()) return;

    std::vector<Track> tracks = rawTracks;
    QImage drawImg = img.copy();
    QPainter painter(&drawImg);
    painter.setRenderHint(QPainter::Antialiasing);

    for(const auto &track : tracks) {
        if(track.lostFrameCount > 0)
            continue;

        float x1 = track.box.x1;
        float y1 = track.box.y1;
        float x2 = track.box.x2;
        float y2 = track.box.y2;

        painter.setPen(QPen(Qt::green,2));
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial",10,QFont::Bold));
        QString label = QString("ID:%1").arg(track.trackId);
        QRect textRect = painter.boundingRect(QRect(x1, y1 - 20, 100, 20),label).toRect();
        painter.fillRect(textRect,Qt::black);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }
    emit paintReady(drawImg);
}

void VisionPainter::onTrackReady(const std::vector<Track> &tracks)
{
    if(!m_isRunning)
        return;

    draw(m_drawImg,tracks);
}

void VisionPainter::ReceiveFrame(const QImage &img)
{
    if(!img.isNull())
        m_drawImg = img;
}
