#ifndef FRAMEPREPROCESSOR_H
#define FRAMEPREPROCESSOR_H

#include <QObject>
#include <QImage>
#include "CommonDef.h"

class FramePreprocessor : public QObject
{
    Q_OBJECT
public:
    explicit FramePreprocessor(QObject *parent = nullptr);

public slots:
    void onFrameReady(const QImage &img);       //接收

signals:
    void AIInputReady(const AIDataInput &input);

private:
    QImage preprocess(const QImage &src);     //预处理

    AIDataInput convertToAIInput(const QImage &img);    //转AI输入数据格式
};

#endif // FRAMEPREPROCESSOR_H
