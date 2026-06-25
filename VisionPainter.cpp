#include "VisionPainter.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QRect>
#include <QMutexLocker>
#include <cmath>

VisionPainter::VisionPainter(QObject *parent)
    : QObject{parent}
{}

void VisionPainter::draw(QImage &drawImg,const std::vector<Track> &tracks)
{
    if(drawImg.isNull()) return;

    QPainter painter(&drawImg);
    painter.setRenderHint(QPainter::Antialiasing);      //图形抗锯齿
    painter.setRenderHint(QPainter::TextAntialiasing);  //文字抗锯齿

    //设置标签字体
    QFont labelFont;
    labelFont.setFamily("Arial");
    labelFont.setPixelSize(10);
    labelFont.setBold(true);
    painter.setFont(labelFont);
    QFontMetrics fm = painter.fontMetrics();
    const int padding = 4;

    for(const auto &track : tracks) {
        if(track.lostFrameCount > 0)
            continue;

        float x1 = track.box.x1;
        float y1 = track.box.y1;
        float x2 = track.box.x2;
        float y2 = track.box.y2;

        //绘制边框
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
        //绘制ID标签
        if(track.lostFrameCount == 0) {
            QString label = QString("ID:%1").arg(track.trackId);
            int textW = fm.horizontalAdvance(label);
            int textH = fm.height();

            int bgW = textW + padding * 2;
            int bgH = textH + padding * 2;
            int bgX = static_cast<int>(x1);
            int bgY = static_cast<int>(y1) - bgH;
            if(bgY < 0) {
                bgY = static_cast<int>(y1);
            }
            QRect bgRect(bgX, bgY, bgW, bgH);
            //绘制半透明圆角背景
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 180));
            painter.drawRoundedRect(bgRect, 3, 3);

            painter.setPen(Qt::white);
            painter.drawText(bgRect, Qt::AlignCenter, label);
        }
    }

    //绘制统计线
    if(m_countLine.isValid) {
        painter.setPen(QPen(Qt::green, 3));
        painter.drawLine(m_countLine.start.x, m_countLine.start.y,
                         m_countLine.end.x, m_countLine.end.y);

        //绘制方向箭头
        cv::Point2f midPoint((m_countLine.start.x + m_countLine.end.x) / 2,
                             (m_countLine.start.y + m_countLine.end.y) / 2
        );
        float dx = m_countLine.end.x - m_countLine.start.x;
        float dy = m_countLine.end.y - m_countLine.start.y;
        cv::Point2f normal(-dy, dx);

        float length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        normal.x = normal.x / length * 30 * m_countLine.direction;
        normal.y = normal.y / length * 30 * m_countLine.direction;

        cv::Point2f arrowEnd = midPoint + normal;

        painter.setPen(QPen(Qt::blue, 2));
        painter.drawLine(midPoint.x, midPoint.y, arrowEnd.x, arrowEnd.y);

        //绘制箭头头部
        float arrowSize = 10;
        float angle = std::atan2(normal.y, normal.x);
        painter.drawLine(arrowEnd.x, arrowEnd.y,
                         arrowEnd.x - arrowSize * std::cos(angle - M_PI / 6),
                         arrowEnd.y - arrowSize * std::sin(angle - M_PI / 6)
        );
        painter.drawLine(arrowEnd.x, arrowEnd.y,
                         arrowEnd.x - arrowSize * std::cos(angle + M_PI / 6),
                         arrowEnd.y - arrowSize * std::sin(angle + M_PI / 6)
        );
    }

    emit paintReady(drawImg);
}

void VisionPainter::onTrackReady(uint64_t frameId, const std::vector<Track> &rawTracks)
{
    if(!m_isRunning)
        return;

    QImage targetImage;

    if(m_frameCache.contains(frameId)) {
        targetImage = m_frameCache.take(frameId);
    }else if(!m_frameCache.isEmpty()) {
        qDebug()<<"就近旧帧兜底匹配";
        uint64_t bestId = 0;
        for(auto it = m_frameCache.keyBegin(); it != m_frameCache.keyEnd(); ++it) {
            if(*it <= frameId && *it > bestId)
                bestId = *it;
        }
        if(bestId == 0)
            bestId = *std::min_element(m_frameCache.keyBegin(), m_frameCache.keyEnd());
        targetImage = m_frameCache.take(bestId);
    }else {
        return;
    }
    if(!targetImage.isNull())
        draw(targetImage,rawTracks);
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

void VisionPainter::receiveFrame(uint64_t frameId, const cv::Mat &matForDraw)
{
    if(!m_isRunning || matForDraw.empty())
        return;

    QImage img = convertMatToQImage(matForDraw);

    m_frameCache.insert(frameId, img);

    while(m_frameCache.size() > MAX_CACHE_FRAME) {
        uint64_t oldestId = *std::min_element(m_frameCache.keyBegin(), m_frameCache.keyEnd());
        m_frameCache.remove(oldestId);
    }
}
