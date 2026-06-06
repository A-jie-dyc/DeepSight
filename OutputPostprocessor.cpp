#include "OutputPostprocessor.h"
#include <opencv2/dnn.hpp>

OutputPostprocessor::OutputPostprocessor(QObject *parent)
    : QObject{parent}
{}

void OutputPostprocessor::onOutputReady(const std::vector<float> &output, const PreprocessParams &params)
{
    if(!m_isRunning)
        return;

    postProcess(output.data(), params);
    emit postProcessReady(m_detBoxes);
}

void OutputPostprocessor::postProcess(const float *output, const PreprocessParams &params)
{
    m_detBoxes.clear();

    for(int i = 0; i < BOX_COUNT; i++) {
        float x = output[0 * BOX_COUNT + i];
        float y = output[1 * BOX_COUNT + i];
        float w = output[2 * BOX_COUNT + i];
        float h = output[3 * BOX_COUNT + i];

        float maxScore = 0.0f;
        int classId = 0;
        for(int j = 4; j < OUT_DIM; j++) {
            float currentScore = output[j * BOX_COUNT + i];
            if(currentScore > maxScore) {
                maxScore = currentScore;
                classId = j - 4;
            }
        }
        if(maxScore > CONF_THRESHOLD && classId == 0) {
            DetectionBox box;
            box.x1 = x - w / 2;
            box.y1 = y - h / 2;
            box.x2 = x + w / 2;
            box.y2 = y + h / 2;
            box.score = maxScore;
            box.classId = classId;

            m_detBoxes.emplace_back(box);
        }
    }

    if(!m_detBoxes.empty()) {
        std::vector<cv::Rect> boxes;
        std::vector<float> scores;
        std::vector<DetectionBox> finalBoxes;
        boxes.reserve(m_detBoxes.size());
        scores.reserve(m_detBoxes.size());

        //提取坐标和置信度
        for(const auto &box : m_detBoxes) {
            boxes.emplace_back(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
            scores.push_back(box.score);
        }
        //调用opencv的NMS函数去重
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, scores, CONF_THRESHOLD, NMS_THRESHOLD, indices);

        finalBoxes.reserve(indices.size());
        for(int idx : indices)
            finalBoxes.emplace_back(m_detBoxes[idx]);

        m_detBoxes = std::move(finalBoxes);
    }

    // 将检测框从 letterbox 640x640 空间映射回原始图像空间
    for (auto &box : m_detBoxes) {
        box.x1 = (box.x1 - params.padLeft) / params.scale;
        box.y1 = (box.y1 - params.padTop)  / params.scale;
        box.x2 = (box.x2 - params.padLeft) / params.scale;
        box.y2 = (box.y2 - params.padTop)  / params.scale;
    }
}
