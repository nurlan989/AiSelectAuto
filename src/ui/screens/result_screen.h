#ifndef RESULT_SCREEN_H
#define RESULT_SCREEN_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <vector>

#include "ai/types/detection.h"

/**
 * @brief Widget for displaying AI detection results in textual and visual form
 *
 * ResultScreen is responsible for presenting inference output produced by
 * the AI system (Detection results) to the user.
 *
 * It provides both:
 * - textual representation of detections (labels, confidence, etc.)
 * - graphical overlay rendering via Qt paint system
 *
 * The class does not perform inference itself. It only consumes precomputed
 * detection results and visualizes them.
 *
 * Used as a secondary results view in AISelectCar MVP to inspect model output.
 */
class ResultScreen : public QWidget {
    Q_OBJECT

public:
    explicit ResultScreen(QWidget* parent = nullptr);
    ~ResultScreen() override;

    void setDetections(const std::vector<Detection>& detections);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawDetections(QPainter& painter);

private:
    QLabel* m_resultList = nullptr;
    std::vector<Detection> detections;
};

#endif // RESULT_SCREEN_H