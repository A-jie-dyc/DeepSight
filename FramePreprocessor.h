#ifndef FRAMEPREPROCESSOR_H
#define FRAMEPREPROCESSOR_H

#include <QObject>
#include <QImage>
#include <atomic>
#include "CommonDef.h"

class FramePreprocessor : public QObject
{
    Q_OBJECT
public:
    explicit FramePreprocessor(QObject *parent = nullptr);

public slots:
    void onFrameReady(const QImage &img);       //接收
    void setRunning(bool running) { m_isRunning = running; }

signals:
    void SendFrame(const QImage &img);
    void AIInputReady(const AIDataInput &input);

private:
    QImage preprocess(const QImage &src);     //预处理

    AIDataInput convertToAIInput(const QImage &img);    //转AI输入数据格式

    std::atomic<bool> m_isRunning = false;
};

#endif // FRAMEPREPROCESSOR_H
