#ifndef CAMERA_SCREEN_H
#define CAMERA_SCREEN_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>

#include "ai/services/ai_service.h"
#include "ai/types/detection.h"


/**
 * @brief Widget for image display and AI-based object detection visualization
 *
 * Connects UI layer with AI inference service (AIService).
 * Handles image input, triggers inference, and renders detection results.
 *
 * Used as the main camera screen in AISelectCar MVP.
 */
class CameraScreen : public QWidget {
    Q_OBJECT

public:
    explicit CameraScreen(QWidget* parent = nullptr);
    ~CameraScreen() = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onSelectPhoto();
    void onDetectionsReady(const std::vector<Detection>& detections);

private:
    void setupUI();
    void drawDetections(QPainter& painter,
                        int offsetX, int offsetY,
                        float scaleX, float scaleY);

    void updateImageLabel();
    void renderDetectionsOnImage();

    // UI
    QLabel*      m_imageLabel;
    QPushButton* m_selectButton;
    QPushButton* m_analyzeButton;
    QLabel*      m_statusLabel;

    // Data
    QImage                   m_currentImage;
    std::vector<Detection>   m_detections;
};

#endif // CAMERA_SCREEN_H