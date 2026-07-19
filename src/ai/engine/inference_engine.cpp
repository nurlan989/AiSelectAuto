#include "ai/engine/inference_engine.h"
#include <iostream>

InferenceEngine::InferenceEngine()
    : m_env(ORT_LOGGING_LEVEL_WARNING, "aiauto_picker")
{
    m_sessionOptions.SetIntraOpNumThreads(1);
    m_sessionOptions.SetGraphOptimizationLevel(
        GraphOptimizationLevel::ORT_ENABLE_ALL);
}

bool InferenceEngine::loadModel(const std::string &modelPath)
{
    if (modelPath.empty())
    {
        std::cerr << "[ONNX] Model path is empty!" << std::endl;
        return false;
    }

    try
    {
        m_session = std::make_unique<Ort::Session>(
            m_env,
            modelPath.c_str(),
            m_sessionOptions);

        // ДОБАВЛЕНО: читаем реальные имена из модели
        Ort::AllocatorWithDefaultOptions allocator;

        auto inputName = m_session->GetInputNameAllocated(0, allocator);
        auto outputName = m_session->GetOutputNameAllocated(0, allocator);

        m_inputName = std::string(inputName.get());
        m_outputName = std::string(outputName.get());

        std::cout << "[ONNX] Input  name: " << m_inputName << std::endl;
        std::cout << "[ONNX] Output name: " << m_outputName << std::endl;
        std::cout << "[ONNX] Session created successfully" << std::endl;

        return true;
    }
    catch (const Ort::Exception &e)
    {
        std::cerr << "[ONNX] ORT ERROR: " << e.what() << std::endl;
        m_session = nullptr;
        return false;
    }
}

InferenceResult InferenceEngine::run(
    const std::vector<float> &tensor,
    const std::vector<int64_t> &shape)
{
    InferenceResult result;

    if (!m_session)
    {
        std::cerr << "[ONNX] Session not initialized!" << std::endl;
        return result;
    }

    // ДОБАВЛЕНО: создаём OrtValue из готового тензора
    auto memInfo = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memInfo,
        const_cast<float *>(tensor.data()),
        tensor.size(),
        shape.data(),
        shape.size());

    const char *inputNames[] = {m_inputName.c_str()};
    const char *outputNames[] = {m_outputName.c_str()};

    // ДОБАВЛЕНО: реальный вызов ONNX сессии
    auto outputs = m_session->Run(
        Ort::RunOptions{nullptr},
        inputNames, &inputTensor, 1,
        outputNames, 1);

    // ДОБАВЛЕНО: достаём shape и данные из output тензора
    auto &outTensor = outputs[0];
    auto outShape = outTensor.GetTensorTypeAndShapeInfo().GetShape();
    const float *data = outTensor.GetTensorData<float>();
    size_t count = outTensor.GetTensorTypeAndShapeInfo().GetElementCount();

    result.data = std::vector<float>(data, data + count);
    result.shape = outShape; // <- добавим поле в InferenceResult

    std::cout << "[ONNX] Output shape: "
              << outShape[0] << "x"
              << outShape[1] << "x"
              << outShape[2] << std::endl;

    return result;
}