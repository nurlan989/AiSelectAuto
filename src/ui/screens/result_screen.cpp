#include "ui/screens/result_screen.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>

ResultScreen::ResultScreen(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #0f0f13;");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(16);

    auto* title = new QLabel("Detection Results", this);
    title->setStyleSheet(
        "color: #ffffff; font-size: 18px; font-weight: bold;");
    layout->addWidget(title);

    m_resultList = new QLabel(this);
    m_resultList->setStyleSheet(
        "color: #aaaacc;"
        "font-size: 13px;"
        "font-family: 'SF Mono', Menlo, monospace;"
        "background: #1a1a24;"
        "border-radius: 10px;"
        "padding: 16px;"
    );
    m_resultList->setWordWrap(true);
    m_resultList->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(m_resultList, 1);
}

ResultScreen::~ResultScreen() {}

void ResultScreen::setDetections(const std::vector<Detection>& newDetections)
{
    detections = newDetections;

    if (detections.empty()) {
        m_resultList->setText("✅  No damage detected");
        return;
    }

    QString text;
    for (size_t i = 0; i < detections.size(); ++i) {
        const auto& d = detections[i];
        text += QString("▸  [%1]  %2  —  confidence: %3%\n")
                    .arg(i + 1)
                    .arg(d.label)
                    .arg((int)(d.confidence * 100));
    }

    m_resultList->setText(text.trimmed());
    update();
}

void ResultScreen::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);
}

void ResultScreen::drawDetections(QPainter&) {}