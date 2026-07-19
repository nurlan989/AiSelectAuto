#pragma once

#include <QImage>
#include <vector>
#include <memory>

#include "ai/types/frame.h"
#include "ai/types/detection.h"
#include "ai/engine/inference_engine.h"
#include "ai/pipeline/preprocessor.h"
#include "ai/pipeline/post_processor.h"
#include "ai/config/detector_model_config.h"

/**
 * @brief Core orchestration component of the AI inference pipeline
 *
 * AIModelRunner is responsible for executing the full inference workflow:
 * from input frame preprocessing to final detection post-processing.
 *
 * It coordinates the following stages:
 * - preprocessing (image normalization, resizing, format conversion)
 * - model inference (via InferenceEngine abstraction)
 * - post-processing (decoding, NMS, filtering)
 *
 * This class acts as the central pipeline executor and hides all
 * low-level ML and CV implementation details from higher-level services.
 *
 * The output of this pipeline is a list of final Detection objects,
 * ready for UI consumption.
 */
class AIModelRunner
{
public:
    AIModelRunner();

    bool loadModel(const std::string &modelPath, const std::vector<std::string> &labels);
    std::vector<Detection> run(const Frame &frame);
    void setConfThreshold(float threshold);

private:
    std::shared_ptr<InferenceEngine> engine;
    Preprocessor preprocessor;
    PostProcessor postprocessor;
    DetectorModelConfig config;
};