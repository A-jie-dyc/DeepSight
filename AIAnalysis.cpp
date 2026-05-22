#include "AIAnalysis.h"
#include <QDebug>

AIAnalysis::AIAnalysis(QObject *parent)
    : QObject{parent}
{}

void AIAnalysis::onAIInputReady(const AIDataInput &input)
{
    if(!m_isRunning)
        return;

    bool success = infer(input);

    if(!success)
        qDebug()<<"AI异常分析失败";
}

bool AIAnalysis::infer(const AIDataInput &input)
{
    if(!m_session || input.normData.empty())
        return false;

    std::vector<int64_t> inputShape = {
        1,
        input.channels,
        input.height,
        input.width
    };
    //创建CPU内存信息
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
    //vector<float>封装成Tensor
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        const_cast<float*>(input.normData.data()),
        input.normData.size(),
        inputShape.data(),
        inputShape.size()
    );

    const char* inputNames[] = {m_inputName.c_str()};
    const char* outputNames[] = {m_outputName.c_str()};

    try {
        Ort::RunOptions runOptions;
        std::vector<Ort::Value> outputTensors = m_session->Run(
            runOptions,
            inputNames,
            &inputTensor,
            1,
            outputNames,
            1
        );

        float *outputPtr = outputTensors[0].GetTensorMutableData<float>();
        size_t outputSize = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
        std::vector<float> output(outputPtr,outputPtr + outputSize);

        emit AIOutputReady(output);
        return true;
    }catch(...) {
        return false;
    }
}

void AIAnalysis::initModel()
{
    try {
        m_session.reset();
        Ort::SessionOptions session_options;
        //设置推理线程数
        session_options.SetIntraOpNumThreads(1);
        //设置模型图优化
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "AIModel");
        m_session = std::make_unique<Ort::Session>(m_env,MODEL_PATH,session_options);
        //创建临时内存分配器获取输入输出节点
        Ort::AllocatorWithDefaultOptions allocator;
        auto inputName = m_session->GetInputNameAllocated(0,allocator);
        auto outputName = m_session->GetOutputNameAllocated(0,allocator);
        m_inputName = inputName.get();
        m_outputName = outputName.get();

        qDebug()<<"模型初始化完成";
        emit modelReady();
    } catch (const std::exception &e) {
        qDebug()<<"模型初始化失败："<<e.what();
    }
}

AIAnalysis::~AIAnalysis()
{
    m_session.reset();
}