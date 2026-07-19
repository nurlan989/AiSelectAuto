#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

#include "ai/types/detection.h"

/**
 * @brief Non-Maximum Suppression (NMS) post-processing stage for object detection
 *
 * NMS is used to filter duplicate and highly overlapping bounding boxes
 * produced by the object detection model.
 *
 * It keeps the most confident detection and removes redundant ones
 * based on Intersection over Union (IoU) thresholding.
 *
 * This class is part of the post-processing pipeline between raw model
 * output and final UI-ready Detection results.
 */
class NMS {
public:

    static std::vector<Detection> apply(
        const std::vector<Detection>& detections,
        float iouThreshold
    );
};