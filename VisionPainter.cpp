#include "VisionPainter.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QRect>

VisionPainter::VisionPainter(QObject *parent)
    : QObject{parent}
{}

void VisionPainter::draw(QImage &drawImg,const std::vector<Track> &tracks)
{
    if(drawImg.isNull()) return;

    QPainter painter(&drawImg);
    painter.setRenderHint(QPainter::Antialiasing);      //抗锯齿

    for(const auto &track : tracks) {
        if(track.lostFrameCount > 0)
            continue;

        float x1 = track.box.x1;
        float y1 = track.box.y1;
        float x2 = track.box.x2;
        float y2 = track.box.y2;

        //绘制绿色边框
        painter.setPen(QPen(Qt::green,2));
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
        //绘制ID标签
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial",10,QFont::Bold));
        QString label = QString("ID:%1").arg(track.trackId);

        QRect textRect = painter.boundingRect(QRect(x1, y1 - 20, 100, 20),label).toRect();
        painter.fillRect(textRect,Qt::black);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }
    emit paintReady(drawImg);
}

void VisionPainter::onTrackReady(const std::vector<Track> &rawTracks)
{
    if(!m_isRunning || m_currentImg.isNull())
        return;

    draw(m_currentImg,rawTracks);
}

QImage VisionPainter::convertMatToQImage(const cv::Mat &mat)
{
    if(mat.empty())
        return {};
    QImage img(mat.data,
               mat.cols,
               mat.rows,
               mat.step,
               QImage::Format_RGB888);
    return img.copy();
}

void VisionPainter::receiveFrame(const cv::Mat &matForDraw)
{
    if(!matForDraw.empty())
        m_currentImg = convertMatToQImage(matForDraw);
}

VisionPainter::~VisionPainter()
{}