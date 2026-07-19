#ifndef AI_SERVICE_H
#define AI_SERVICE_H

#include <QObject>
#include <QImage>
#include <vector>

#include "ai/types/detection.h"
#include "ai/runner/ai_model_runner.h"
#include "ai/engine/inference_engine.h"

/**
 * @brief High-level service for AI inference pipeline integration with UI
 *
 * AIService acts as a bridge between the Qt UI layer and the underlying
 * inference pipeline.
 *
 * Responsibilities:
 * - receives input frames (QImage) from UI components
 * - delegates inference execution to AIModelRunner / InferenceEngine
 * - manages inference lifecycle (initialization, execution)
 * - emits structured detection results back to UI via Qt signals
 *
 * This class does NOT contain model logic itself.
 * It only orchestrates inference execution and result delivery.
 *
 * Implemented as a singleton to provide a shared inference context
 * across the application.
 */
class AIService : public QObject
{
    Q_OBJECT

public:
    static AIService &instance();

    bool init();

    void processImage(const QImage &image);

signals:
    void detectionsReady(const std::vector<Detection> &detections);

private:
    explicit AIService(QObject *parent = nullptr);

    AIModelRunner m_damageRunner;
};

#endif // AI_SERVICE_H
