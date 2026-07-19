#ifndef DETECTOR_MODEL_CONFIG_H
#define DETECTOR_MODEL_CONFIG_H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

/**
 * @brief Configuration container for object detection model parameters
 *
 * DetectorModelConfig encapsulates all runtime parameters required
 * for running and post-processing a detection model (e.g. YOLO / ONNX).
 *
 * It provides a single source of truth for:
 * - model input dimensions and tensor shape
 * - confidence and NMS thresholds
 * - class label mapping for detection outputs
 *
 * This class is used by both inference and post-processing stages
 * to ensure consistent behavior across the pipeline.
 */
class DetectorModelConfig {
public:
    DetectorModelConfig();

    // Input size for YOLO (e.g. 640x640)
    cv::Size inputSize() const;

    // ONNX input tensor shape: {1, 3, H, W}
    std::vector<int64_t> inputShape() const;

    // Confidence threshold
    float confidenceThreshold() const;

    // NMS threshold
    float nmsThreshold() const;

    void setLabels(const std::vector<std::string>& labels);
    void setConfThreshold(float t) { confidence_threshold_ = t; }

    // Class labels
    const std::vector<std::string>& labels() const;

private:
    int input_width_;
    int input_height_;

    float confidence_threshold_;
    float nms_threshold_;

    std::vector<std::string> labels_;
};

#endif // DETECTOR_MODEL_CONFIG_H