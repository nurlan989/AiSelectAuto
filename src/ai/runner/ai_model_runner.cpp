#include "ai_model_runner.h"

#include <iostream>

AIModelRunner::AIModelRunner()
{
    engine = std::make_shared<InferenceEngine>();
}

bool AIModelRunner::loadModel(
    const std::string &modelPath,
    const std::vector<std::string> &labels)
{

    config.setLabels(labels);
    return engine->loadModel(modelPath);
}

std::vector<Detection> AIModelRunner::run(
    const Frame &frame)
{

    std::cout
        << "[Runner] run called"
        << std::endl;

    auto prep =
        preprocessor.process(
            frame.mat,
            config.inputSize().width,
            config.inputSize().height);

    auto output =
        engine->run(
            prep.tensor,
            prep.shape);

    std::cout
        << "[Runner] engine run finished"
        << std::endl;

    auto detections =
        postprocessor.decode(
            output.data.data(),
            output.shape,
            prep.scaleX,
            prep.scaleY,
            config);

    detections =
        postprocessor.applyNMS(
            detections,
            config.nmsThreshold());

    std::cout
        << "[Runner] detections: "
        << detections.size()
        << std::endl;

    for (const auto &d : detections)
    {
        std::cout
            << "[DET] "
            << d.label.toStdString()
            << " conf="
            << d.confidence
            << " x="
            << d.box.x
            << " y="
            << d.box.y
            << " w="
            << d.box.width
            << " h="
            << d.box.height
            << std::endl;
    }

    return detections;
}

void AIModelRunner::setConfThreshold(
    float threshold)
{
    config.setConfThreshold(threshold);
}