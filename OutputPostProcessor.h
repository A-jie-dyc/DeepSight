#ifndef OUTPUTPOSTPROCESSOR_H
#define OUTPUTPOSTPROCESSOR_H

#include <QObject>
#include <atomic>
#include "CommonDef.h"

class OutputPostProcessor : public QObject
{
    Q_OBJECT
public:
    explicit OutputPostProcessor(QObject *parent = nullptr);

public slots:
    void onOutputReady(const std::vector<float> &output);
    void setRunning(bool running) { m_isRunning = running; }

signals:
    void postProcessReady(const std::vector<DetectionBox> &boxes);

private:
    void postProcess(const float *output);

    const float m_confThreshold = 0.25f;                //置信度阈值
    const float m_nmsThreshold = 0.5f;                  //NMS阈值
    const int m_outDim = 84;                            //输出维度
    const int m_boxCount = 8400;                        //输出候选框总数

    std::vector<DetectionBox> m_detBoxes;

    std::atomic<bool> m_isRunning = false;
};

#endif // OUTPUTPOSTPROCESSOR_H
