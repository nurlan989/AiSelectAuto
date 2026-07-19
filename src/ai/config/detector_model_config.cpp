#include "detector_model_config.h"

/**
 * @brief Default constructor with YOLOv8n standard settings.
 */
DetectorModelConfig::DetectorModelConfig()
    : input_width_(640),
      input_height_(640),
      confidence_threshold_(0.05f),
      nms_threshold_(0.01f)
{
    labels_ = {
        "Front-Windscreen-Damage",  // 0
        "Headlight-Damage",         // 1
        "Major-Rear-Bumper-Dent",   // 2
        "Rear-windscreen-Damage",   // 3
        "RunningBoard-Dent",        // 4
        "Sidemirror-Damage",        // 5
        "Signlight-Damage",         // 6
        "Taillight-Damage",         // 7
        "bonnet-dent",              // 8
        "doorouter-dent",           // 9
        "doorouter-scratch",        // 10
        "fender-dent",              // 11
        "front-bumper-dent",        // 12
        "front-bumper-scratch",     // 13
        "medium-Bodypanel-Dent",    // 14
        "paint-chip",               // 15
        "paint-trace",              // 16
        "pillar-dent",              // 17
        "quaterpanel-dent",         // 18
        "rear-bumper-dent",         // 19
        "rear-bumper-scratch",      // 20
        "roof-dent"                 // 21
    };
}

cv::Size DetectorModelConfig::inputSize() const {
    return cv::Size(input_width_, input_height_);
}

std::vector<int64_t> DetectorModelConfig::inputShape() const {
    return {1, 3, input_height_, input_width_};
}

float DetectorModelConfig::confidenceThreshold() const {
    return confidence_threshold_;
}

float DetectorModelConfig::nmsThreshold() const {
    return nms_threshold_;
}

const std::vector<std::string>& DetectorModelConfig::labels() const {
    return labels_;
}

void DetectorModelConfig::setLabels(const std::vector<std::string>& labels) {
    labels_ = labels;
}