#include "AIAnalysis.h"
#include <QDebug>
#include <cstring>
#include <thread>

AIAnalysis::AIAnalysis(QObject *parent)
    : QObject{parent}
{}

void AIAnalysis::onAIInputReady(uint64_t frameId, const cv::Mat &matForAI, const PreprocessParams &params)
{
    if(!m_isRunning) {
        m_countSeq.fetch_add(1, std::memory_order_release);
        m_totalFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_dropFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_countSeq.fetch_add(1, std::memory_order_release);
        return;
    }

    bool expected = false;
    if(!m_busy.compare_exchange_strong(expected, true)) {
        qDebug()<<"AI忙，丢一帧";
        m_countSeq.fetch_add(1, std::memory_order_release);
        m_totalFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_dropFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_countSeq.fetch_add(1, std::memory_order_release);
        return;
    }
    std::optional<std::vector<float>> output;
    try {
        output = infer(matForAI, params);
    } catch (...) {
        qDebug()<<"AI分析失败,帧号:"<<frameId;
        m_countSeq.fetch_add(1, std::memory_order_release);
        m_totalFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_dropFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_countSeq.fetch_add(1, std::memory_order_release);
        m_busy.store(false);
        return;
    }

    m_busy.store(false);

    if(output.has_value()) {
        emit AIOutputReady(frameId, std::move(output.value()), params);
        m_countSeq.fetch_add(1, std::memory_order_release);
        m_totalFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_inferFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_countSeq.fetch_add(1, std::memory_order_release);
    }else {
        m_countSeq.fetch_add(1, std::memory_order_release);
        m_totalFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_dropFrameCount.fetch_add(1, std::memory_order_relaxed);
        m_countSeq.fetch_add(1, std::memory_order_release);
    }
}

std::optional<std::vector<float>> AIAnalysis::infer(const cv::Mat &mat, const PreprocessParams &params)
{
    if(!m_session || mat.empty() || mat.type() != CV_32F)
        return std::nullopt;

    //直接拷贝
    std::memcpy(m_inputData.data(), mat.ptr<float>(0),
                m_inputData.size() * sizeof(float));

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        m_inputData.data(),
        m_inputData.size(),
        m_inputShape.data(),
        m_inputShape.size()
    );

    const char* inputNames[]  = {m_inputName.c_str()};
    const char* outputNames[] = {m_outputName.c_str()};

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

    return std::vector<float>(outputPtr, outputPtr + outputSize);
}

void AIAnalysis::initModel()
{
    if(m_session)
        return;

    try {
        m_session.reset();
        Ort::SessionOptions session_options;
        //设置推理线程数
        unsigned int cpu_cores = std::thread::hardware_concurrency();
        int best_threads = 4;
        if(cpu_cores > 0) {
            best_threads = std::max(1, (int)(cpu_cores / 2));
            best_threads = std::min(best_threads, 8);
        }
        session_options.SetIntraOpNumThreads(best_threads);
        //设置主模型会话数
        session_options.SetInterOpNumThreads(1);
        //设置模型图优化
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        m_session = std::make_unique<Ort::Session>(getOrtEnv(),MODEL_PATH,session_options);
        //创建临时内存分配器获取输入输出节点
        Ort::AllocatorWithDefaultOptions allocator;
        auto inputName = m_session->GetInputNameAllocated(0,allocator);
        auto outputName = m_session->GetOutputNameAllocated(0,allocator);
        m_inputName = inputName.get();
        m_outputName = outputName.get();

        // 预分配输入数据缓冲区 (1 x 3 x 640 x 640)
        m_inputData.resize(1 * 3 * MODEL_WIDTH * MODEL_HEIGHT);

        qDebug()<<"模型初始化完成";
        emit modelReady();
    } catch (const std::exception &e) {
        qDebug()<<"模型初始化失败："<<e.what();
        emit errorOccurred(ErrorDef::Error_ModelLoad, QString("模型加载失败：") + QString::fromStdString(e.what()));
    }
}

FpsCount AIAnalysis::fetchAndResetAllCount()
{
    uint64_t seq0, seq1;
    uint64_t infer, drop, total;
    do {
        //等待序列号为偶数
        do {
            seq0 = m_countSeq.load(std::memory_order_acquire);
        } while (seq0 & 1);

        //读取计数器
        total = m_totalFrameCount.load(std::memory_order_acquire);
        infer = m_inferFrameCount.load(std::memory_order_acquire);
        drop  = m_dropFrameCount.load(std::memory_order_acquire);

        //再次读取序列号
        seq1 = m_countSeq.load(std::memory_order_acquire);
    } while (seq0 != seq1);     //若中途发生了更新，重试

    FpsCount count;
    count.inferFrame = infer - m_lastInfer;
    count.dropFrame = drop  - m_lastDrop;
    count.totalFrame = total - m_lastTotal;

    m_lastInfer = infer;
    m_lastDrop  = drop;
    m_lastTotal = total;

    return count;
}

void AIAnalysis::resetAllCount()
{
    //确保没有正在进行的更新
    while (m_countSeq.load() & 1);
    m_totalFrameCount.store(0);
    m_inferFrameCount.store(0);
    m_dropFrameCount.store(0);
    m_countSeq.store(m_countSeq.load() + 2);      //保持偶数，并让读取端感知变化（序列号跳变）
    //同时清空读取端的 last 基准值（可以在读取端主动调用一次 fetchAndResetAllCount 忽略结果）
    m_lastInfer = 0;
    m_lastDrop = 0;
    m_lastTotal = 0;
}

Ort::Env &AIAnalysis::getOrtEnv()
{
    static Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "AIModel");
    return env;
}

AIAnalysis::~AIAnalysis()
{
    resetAllCount();
    m_session.reset();
}