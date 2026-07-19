#pragma once

#include <memory>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <onnxruntime_cxx_api.h>

#include "ai/types/inference_result.h"

/**
 * @brief Low-level inference execution engine for ONNX models
 *
 * InferenceEngine is responsible for executing deep learning models
 * using ONNX Runtime and producing raw model outputs.
 *
 * It operates at the lowest level of the AI pipeline and provides:
 * - model loading and session initialization
 * - execution of forward pass on prepared input tensors
 * - retrieval of raw output tensors from the model
 *
 * This component is independent of preprocessing, postprocessing,
 * and UI layers. It only focuses on model execution.
 */
class InferenceEngine {
public:
    InferenceEngine();

    bool loadModel(const std::string& modelPath);

    // ИЗМЕНЕНИЕ: принимает готовый float tensor + shape
    InferenceResult run(
        const std::vector<float>& tensor,
        const std::vector<int64_t>& shape
    );

private:
    Ort::Env            m_env;
    Ort::SessionOptions m_sessionOptions;
    std::unique_ptr<Ort::Session> m_session;

    // ДОБАВЛЕНО: имена input/output из самой модели
    std::string m_inputName;
    std::string m_outputName;
};