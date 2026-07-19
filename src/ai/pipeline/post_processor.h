#pragma once

#include <vector>
#include "ai/types/detection.h"
#include "ai/config/detector_model_config.h"


class PostProcessor
{

public:

    std::vector<Detection> decode(
        const float* output,
        const std::vector<int64_t>& shape,
        float scaleX,
        float scaleY,
        const DetectorModelConfig& config
    );


    std::vector<Detection> applyNMS(
        const std::vector<Detection>& detections,
        float threshold
    );

private:
    float IoU(
        const cv::Rect& a,
        const cv::Rect& b   
    );

};