#include "OutputPostProcessor.h"
#include <opencv2/core.hpp>
#include <QDebug>

OutputPostProcessor::OutputPostProcessor(QObject *parent)
    : QObject{parent}
{}

void OutputPostProcessor::onOutputReady(const std::vector<float> &output)
{
    postProcess(output.data());
    emit postProcessReady(m_detBoxes);
}

void OutputPostProcessor::postProcess(const float *output)
{
    m_detBoxes.clear();

    for(int i = 0; i < m_boxCount; i++) {
        float x = output[0 * m_boxCount + i];
        float y = output[1 * m_boxCount + i];
        float w = output[2 * m_boxCount + i];
        float h = output[3 * m_boxCount + i];

        float maxScore = 0.0f;
        int classId = 0;
        for(int j = 4; j < m_outDim; j++) {
            float currentScore = output[j * m_boxCount + i];
            if(currentScore > maxScore) {
                maxScore = currentScore;
                classId = j - 4;
            }
        }
        if(maxScore > m_confThreshold) {
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

    std::vector<DetectionBox> finalBoxes;
    if(!m_detBoxes.empty()) {
        std::sort(m_detBoxes.begin(),m_detBoxes.end(),[](const DetectionBox &a,const DetectionBox &b) {
            return a.score > b.score;
        });

        std::vector<bool> isSuppressed(m_detBoxes.size(),false);
        for(int i = 0; i < m_detBoxes.size(); i++) {
            if(isSuppressed[i])
                continue;
            finalBoxes.emplace_back(m_detBoxes[i]);
            for(int j = i + 1; j < m_detBoxes.size(); j++) {
                if(isSuppressed[j])
                    continue;

                cv::Rect2f rect_i(m_detBoxes[i].x1,m_detBoxes[i].y1,
                                  m_detBoxes[i].x2 - m_detBoxes[i].x1,
                                  m_detBoxes[i].y2 - m_detBoxes[i].y1);
                cv::Rect2f rect_j(m_detBoxes[j].x1,m_detBoxes[j].y1,
                                  m_detBoxes[j].x2 - m_detBoxes[j].x1,
                                  m_detBoxes[j].y2 - m_detBoxes[j].y1);
                //计算IOU
                cv::Rect2f intersect = rect_i & rect_j;
                float interArea = intersect.area();
                float unionArea = rect_i.area() + rect_j.area() - intersect.area();
                float iou = unionArea > 0 ? interArea / unionArea : 0.0f;
                if(iou > m_nmsThreshold) {
                    isSuppressed[j] = true;
                }
            }
        }
        m_detBoxes = std::move(finalBoxes);
    }
}
