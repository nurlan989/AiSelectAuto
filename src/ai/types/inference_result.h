#pragma once

#include <vector>

/**
 * @brief Raw output container of the AI inference engine
 *
 * InferenceResult stores the direct output of a neural network model
 * before any post-processing (decoding, NMS, thresholding).
 *
 * It is used as an intermediate representation between:
 * - inference engine (ONNX / TensorRT / OpenCV DNN)
 * - post-processing stage (decoders, NMS, converters to Detection)
 *
 * The structure is intentionally generic to support different model types.
 */
struct InferenceResult
{
    std::vector<float> data;
    std::vector<int64_t> shape; // ДОБАВЛЕНО: нужен для декодера
};