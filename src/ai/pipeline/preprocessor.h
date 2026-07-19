#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

#include "ai/types/frame.h"

/**
 * @brief Preprocessing stage for preparing images for deep learning inference
 *
 * Preprocessor converts raw input images into a model-ready format
 * suitable for YOLO (or similar object detection networks).
 *
 * Responsibilities include:
 * - resizing input image to model expected dimensions
 * - normalizing pixel values and converting to tensor format
 * - maintaining scale factors for mapping detections back to original image
 *
 * This component is part of the inference pipeline and sits before
 * the neural network execution stage.
 */
class Preprocessor {
public:

    struct Result {
        std::vector<float> tensor;
        std::vector<int64_t> shape;
        float scaleX;
        float scaleY;
    };

    Result process(
        const cv::Mat& image,
        int inputWidth,
        int inputHeight
    );

    Frame run(const Frame& frame, const cv::Size& size);
};