#include "nms.h"

#include <algorithm>

namespace {

/**
 * @brief Calculates Intersection over Union.
 */
float calculateIoU(const cv::Rect& a, const cv::Rect& b)
{
    int intersectionArea = (a & b).area();

    int unionArea =
        a.area() + b.area() - intersectionArea;

    if (unionArea <= 0)
        return 0.f;

    return static_cast<float>(intersectionArea)
           / static_cast<float>(unionArea);
}

}

std::vector<Detection> NMS::apply(
    const std::vector<Detection>& detections,
    float iouThreshold)
{
    if (detections.empty())
        return {};

    std::vector<Detection> sorted = detections;

    std::sort(
        sorted.begin(),
        sorted.end(),
        [](const Detection& a, const Detection& b) {
            return a.confidence > b.confidence;
        }
    );

    std::vector<bool> removed(sorted.size(), false);

    std::vector<Detection> result;

    for (size_t i = 0; i < sorted.size(); i++) {

        if (removed[i])
            continue;

        result.push_back(sorted[i]);

        for (size_t j = i + 1; j < sorted.size(); j++) {

            if (removed[j])
                continue;

            if (sorted[i].classId != sorted[j].classId)
                continue;

            float iou = calculateIoU(
                sorted[i].box,
                sorted[j].box
            );

            if (iou > iouThreshold)
                removed[j] = true;
        }
    }

    return result;
}
