#ifndef OUTPUTPOSTPROCESSOR_H
#define OUTPUTPOSTPROCESSOR_H

#include <QObject>
#include <atomic>
#include "CommonDef.h"

class OutputPostprocessor : public QObject
{
    Q_OBJECT
public:
    explicit OutputPostprocessor(QObject *parent = nullptr);

    void setRunning(bool running) { m_isRunning.store(running); }

public slots:
    void onOutputReady(uint64_t frameId, const std::vector<float> &output, const PreprocessParams &params);

signals:
    void postProcessReady(uint64_t frameId, const std::vector<DetectionBox> &boxes);

private:
    void postProcess(const float *output, const PreprocessParams &params);

    std::vector<DetectionBox> m_detBoxes;
    std::atomic<bool> m_isRunning = false;
};

#endif // OUTPUTPOSTPROCESSOR_H
