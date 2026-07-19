#ifndef FRAME_H
#define FRAME_H

#include <opencv2/core.hpp>
#include <QImage>

/**
 * @brief Lightweight wrapper around OpenCV Mat for image/frame handling
 *
 * Frame is a utility data container used to unify image representation
 * across the application pipeline.
 *
 * It serves as a bridge between:
 * - Qt image representation (QImage) used in UI layer
 * - OpenCV matrix representation (cv::Mat) used in CV/AI pipeline
 *
 * The class does not perform processing or inference.
 * It only provides a consistent data format for image transfer between layers.
 */
class Frame {
public:
    cv::Mat mat;

    Frame() = default;
    explicit Frame(const cv::Mat& m) : mat(m) {}

    static Frame fromQImage(const QImage& image);
};

#endif