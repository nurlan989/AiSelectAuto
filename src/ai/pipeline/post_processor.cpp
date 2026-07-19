#include "post_processor.h"

#include <iostream>
#include <algorithm>

std::vector<Detection> PostProcessor::decode(
    const float *output,
    const std::vector<int64_t> &shape,
    float scaleX,
    float scaleY,
    const DetectorModelConfig &config)
{
    std::vector<Detection> detections;

    if (shape.size() != 3)
    {
        std::cerr
            << "[PostProcessor] Wrong output dimension"
            << std::endl;

        return detections;
    }

    int channels = shape[1];
    int boxes = shape[2];

    std::cout
        << "[PostProcessor] channels="
        << channels
        << " boxes="
        << boxes
        << std::endl;

    int classes = channels - 4;

    for (int i = 0; i < boxes; i++)
    {

        float cx = output[0 * boxes + i];
        float cy = output[1 * boxes + i];
        float w = output[2 * boxes + i];
        float h = output[3 * boxes + i];

        float confidence = 0.0f;
        int classId = -1;

        for (int c = 0; c < classes; c++)
        {
            float score =
                output[(4 + c) * boxes + i];

            if (score > confidence)
            {
                confidence = score;
                classId = c;
            }
        }

        if (confidence < config.confidenceThreshold())
            continue;

        if (classId < 0 ||
            classId >= config.labels().size())
            continue;

        Detection d;

        d.classId = classId;
        d.confidence = confidence;

        d.label =
            QString::fromStdString(
                config.labels()[classId]);

        float x1 = cx - w / 2.0f;
        float y1 = cy - h / 2.0f;

        d.box.x =
            static_cast<int>(x1 * scaleX);

        d.box.y =
            static_cast<int>(y1 * scaleY);

        d.box.width =
            static_cast<int>(w * scaleX);

        d.box.height =
            static_cast<int>(h * scaleY);

        detections.push_back(d);
    }

    return detections;
}

float PostProcessor::IoU(
    const cv::Rect &a,
    const cv::Rect &b)
{
    int intersection =
        (a & b).area();

    int unionArea =
        a.area() + b.area() - intersection;

    if (unionArea <= 0)
        return 0.0f;

    return static_cast<float>(intersection) /
           static_cast<float>(unionArea);
}

std::vector<Detection> PostProcessor::applyNMS(
    const std::vector<Detection> &detections,
    float threshold)
{
    std::vector<Detection> result;

    std::vector<Detection> sorted =
        detections;

    std::sort(
        sorted.begin(),
        sorted.end(),
        [](const Detection &a,
           const Detection &b)
        {
            return a.confidence > b.confidence;
        });

    std::vector<bool> removed(
        sorted.size(),
        false);

    for (size_t i = 0; i < sorted.size(); i++)
    {

        if (removed[i])
            continue;

        result.push_back(sorted[i]);

        for (size_t j = i + 1;
             j < sorted.size();
             j++)
        {

            if (removed[j])
                continue;

            // только одинаковый класс
            if (sorted[i].classId != sorted[j].classId)
                continue;

            float iou =
                IoU(
                    sorted[i].box,
                    sorted[j].box);

            if (iou > threshold)
            {
                removed[j] = true;
            }
        }
    }

    return result;
}