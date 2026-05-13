#ifndef AIANALYSIS_H
#define AIANALYSIS_H

#include <QObject>
#include <onnxruntime_cxx_api.h>
#include "CommonDef.h"

class AIAnalysis : public QObject
{
    Q_OBJECT
public:
    explicit AIAnalysis(QObject *parent = nullptr);
    ~AIAnalysis();

public slots:
    void initModel();
    void onAIInputReady(const AIDataInput &input);

signals:
    void modelReady();
    void AIOutputReady(const std::vector<float> &output);

private:
    bool infer(const AIDataInput &input);

    Ort::Env m_env = nullptr;                           //ONNX环境配置
    std::unique_ptr<Ort::Session> m_session;            //模型
    std::string m_inputName;                  //输入节点
    std::string m_outputName;                 //输出节点
};

#endif // AIANALYSIS_H
