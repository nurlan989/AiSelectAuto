#pragma once

#include <QString>
#include <opencv2/opencv.hpp>

/**
 * @brief Represents a single object detection result from the AI model
 *
 * Detection stores the output of the inference pipeline for one object instance.
 *
 * This structure is used across the application as a unified format for:
 * - model output (YOLO / ONNX / OpenCV DNN)
 * - UI visualization (bounding boxes and labels)
 * - post-processing logic (filtering, sorting, aggregation)
 *
 * Coordinates are stored in pixel space relative to the input image.
 */
struct Detection {
    
    cv::Rect box;
    float confidence;
    int classId;
    QString label;
};