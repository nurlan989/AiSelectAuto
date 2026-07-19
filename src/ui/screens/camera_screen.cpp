#include "camera_screen.h"

#include <QPainter>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFont>
#include <QFontMetrics>

// ── Цвета по классу ────────────────────────────────────────────────────────
static QColor classColor(int classId) {
    static const QList<QColor> colors = {
        QColor(255,  80,  80),  // 0 — красный
        QColor(255, 165,   0),  // 1 — оранжевый
        QColor( 80, 200,  80),  // 2 — зелёный
        QColor( 80, 160, 255),  // 3 — синий
        QColor(180,  80, 255),  // 4 — фиолетовый
    };
    return colors[classId % colors.size()];
}

CameraScreen::CameraScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUI();

    connect(&AIService::instance(), &AIService::detectionsReady,
            this, &CameraScreen::onDetectionsReady);

    setAcceptDrops(true);
}

void CameraScreen::setupUI()
{
    // Окно
    setWindowTitle("AIAutoPicker — Car Damage Detection");
    setMinimumSize(900, 680);
    setStyleSheet("background-color: #0f0f13;");

    // ── Главный layout ──
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32, 32, 32, 32);
    root->setSpacing(20);

    // ── Заголовок ──
    auto* title = new QLabel("🚗  AI Car Damage Inspector", this);
    title->setStyleSheet(
        "color: #ffffff;"
        "font-size: 22px;"
        "font-weight: bold;"
        "font-family: 'SF Pro Display', 'Helvetica Neue', Arial, sans-serif;"
    );
    title->setAlignment(Qt::AlignLeft);
    root->addWidget(title);

    auto* subtitle = new QLabel("Upload a photo to detect scratches, dents and cracks", this);
    subtitle->setStyleSheet(
        "color: #888888;"
        "font-size: 13px;"
        "font-family: 'SF Pro Text', Arial, sans-serif;"
    );
    root->addWidget(subtitle);

    // ── Drop zone (imageLabel) ──
    m_imageLabel = new QLabel(this);
    m_imageLabel->setMinimumHeight(420);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet(
        "background-color: #1a1a24;"
        "border: 2px dashed #333355;"
        "border-radius: 16px;"
        "color: #444466;"
        "font-size: 15px;"
        "font-family: Arial, sans-serif;"
    );
    m_imageLabel->setText("Drop photo here\nor click  \"Select Photo\"");
    root->addWidget(m_imageLabel, 1);

    // ── Кнопки ──
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    m_selectButton = new QPushButton("📁  Select Photo", this);
    m_selectButton->setFixedHeight(44);
    m_selectButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #2a2a3d;"
        "  color: #ccccff;"
        "  border: 1px solid #444466;"
        "  border-radius: 10px;"
        "  font-size: 14px;"
        "  padding: 0 24px;"
        "}"
        "QPushButton:hover { background-color: #33334d; }"
        "QPushButton:pressed { background-color: #1e1e2e; }"
    );

    m_analyzeButton = new QPushButton("🔍  Analyze Damage", this);
    m_analyzeButton->setFixedHeight(44);
    m_analyzeButton->setEnabled(false);
    m_analyzeButton->setStyleSheet(
        "QPushButton:enabled {"
        "  background-color: #4a4aff;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 10px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 0 32px;"
        "}"
        "QPushButton:hover:enabled { background-color: #5a5aff; }"
        "QPushButton:pressed:enabled { background-color: #3a3aee; }"
        "QPushButton:disabled {"
        "  background-color: #2a2a3d;"
        "  color: #555566;"
        "  border: none;"
        "  border-radius: 10px;"
        "  font-size: 14px;"
        "  padding: 0 32px;"
        "}"
    );

    btnRow->addWidget(m_selectButton);
    btnRow->addStretch();
    btnRow->addWidget(m_analyzeButton);
    root->addLayout(btnRow);

    // ── Статус ──
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet(
        "color: #666688;"
        "font-size: 12px;"
        "font-family: 'SF Mono', Menlo, monospace;"
    );
    m_statusLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_statusLabel);

    // ── Коннекты ──
    connect(m_selectButton,  &QPushButton::clicked, this, &CameraScreen::onSelectPhoto);
    connect(m_analyzeButton, &QPushButton::clicked, this, [this]() {
        if (!m_currentImage.isNull()) {
            m_detections.clear();
            m_statusLabel->setText("⏳  Analyzing...");
            m_analyzeButton->setEnabled(false);
            AIService::instance().processImage(m_currentImage);
        }
    });
}

// ── Выбор файла ────────────────────────────────────────────────────────────
void CameraScreen::onSelectPhoto()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select Car Photo",
        QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.webp)"
    );

    if (path.isEmpty()) return;

    m_currentImage = QImage(path);
    if (m_currentImage.isNull()) return;

    m_detections.clear();
    m_analyzeButton->setEnabled(true);
    m_statusLabel->setText("📷  Photo loaded — press Analyze");

    // ИСПРАВЛЕНО: показываем фото через setPixmap
    updateImageLabel();
}

// ── Drag & Drop ────────────────────────────────────────────────────────────
void CameraScreen::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void CameraScreen::dropEvent(QDropEvent* e)
{
    auto urls = e->mimeData()->urls();
    if (urls.isEmpty()) return;

    QString path = urls.first().toLocalFile();
    m_currentImage = QImage(path);
    if (m_currentImage.isNull()) return;

    m_detections.clear();
    m_analyzeButton->setEnabled(true);
    m_statusLabel->setText("📷  Photo loaded — press Analyze");
    update();
}

// ── Результаты ─────────────────────────────────────────────────────────────
void CameraScreen::onDetectionsReady(const std::vector<Detection>& detections)
{
    m_detections = detections;
    m_analyzeButton->setEnabled(true);

    if (detections.empty()) {
        m_statusLabel->setText("✅  No damage detected");
    } else {
        m_statusLabel->setText(
            QString("⚠️  Found %1 damage area(s)").arg(detections.size()));
    }

    // ИСПРАВЛЕНО: рисуем bbox прямо на pixmap
    renderDetectionsOnImage();
}


// ── Paint ──────────────────────────────────────────────────────────────────
void CameraScreen::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void CameraScreen::drawDetections(QPainter& painter,
                                   int offsetX, int offsetY,
                                   float scaleX, float scaleY)
{
    QFont font("SF Pro Text", 10, QFont::Bold);
    painter.setFont(font);

    for (const auto& d : m_detections) {

        QColor color = classColor(d.classId);

        int rx = offsetX + (int)(d.box.x * scaleX);
        int ry = offsetY + (int)(d.box.y * scaleY);
        int rw = (int)(d.box.width  * scaleX);
        int rh = (int)(d.box.height * scaleY);

        QRect rect(rx, ry, rw, rh);

        // Полупрозрачная заливка
        painter.fillRect(rect, QColor(color.red(), color.green(), color.blue(), 30));

        // Рамка
        painter.setPen(QPen(color, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect);

        // Углы (декоративные)
        int c = 10;
        painter.setPen(QPen(color, 3));
        painter.drawLine(rect.topLeft(),     rect.topLeft()     + QPoint(c, 0));
        painter.drawLine(rect.topLeft(),     rect.topLeft()     + QPoint(0, c));
        painter.drawLine(rect.topRight(),    rect.topRight()    + QPoint(-c, 0));
        painter.drawLine(rect.topRight(),    rect.topRight()    + QPoint(0,  c));
        painter.drawLine(rect.bottomLeft(),  rect.bottomLeft()  + QPoint(c,  0));
        painter.drawLine(rect.bottomLeft(),  rect.bottomLeft()  + QPoint(0, -c));
        painter.drawLine(rect.bottomRight(), rect.bottomRight() + QPoint(-c, 0));
        painter.drawLine(rect.bottomRight(), rect.bottomRight() + QPoint(0, -c));

        // Label badge
        QString text = QString("%1  %2%")
            .arg(d.label)
            .arg((int)(d.confidence * 100));

        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(text);
        textRect.setWidth(textRect.width() + 16);
        textRect.setHeight(textRect.height() + 8);
        textRect.moveBottomLeft(QPoint(rx, ry - 4));

        // Badge background
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(textRect, 5, 5);

        // Badge text
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void CameraScreen::updateImageLabel()
{
    if (m_currentImage.isNull()) return;

    QPixmap pixmap = QPixmap::fromImage(m_currentImage);
    m_imageLabel->setPixmap(
        pixmap.scaled(
            m_imageLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}

void CameraScreen::renderDetectionsOnImage()
{
    if (m_currentImage.isNull()) return;

    // Берём оригинальное фото
    QImage result = m_currentImage.convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&result);

    QFont font("Arial", 12, QFont::Bold);
    painter.setFont(font);

    static const QList<QColor> colors = {
        QColor(255,  80,  80), QColor(255, 165,   0),
        QColor( 80, 200,  80), QColor( 80, 160, 255),
        QColor(180,  80, 255), QColor(255, 220,   0),
        QColor(  0, 200, 200), QColor(255, 100, 180),
    };

    for (const auto& d : m_detections) {

        QColor color = colors[d.classId % colors.size()];
        QRect rect(d.box.x, d.box.y, d.box.width, d.box.height);

        // Полупрозрачная заливка
        painter.fillRect(rect, QColor(color.red(), color.green(), color.blue(), 40));

        // Рамка
        painter.setPen(QPen(color, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect);

        // Углы
        int c = 14;
        painter.setPen(QPen(color, 4));
        painter.drawLine(rect.topLeft(),     rect.topLeft()     + QPoint(c,  0));
        painter.drawLine(rect.topLeft(),     rect.topLeft()     + QPoint(0,  c));
        painter.drawLine(rect.topRight(),    rect.topRight()    + QPoint(-c, 0));
        painter.drawLine(rect.topRight(),    rect.topRight()    + QPoint(0,  c));
        painter.drawLine(rect.bottomLeft(),  rect.bottomLeft()  + QPoint(c,  0));
        painter.drawLine(rect.bottomLeft(),  rect.bottomLeft()  + QPoint(0, -c));
        painter.drawLine(rect.bottomRight(), rect.bottomRight() + QPoint(-c, 0));
        painter.drawLine(rect.bottomRight(), rect.bottomRight() + QPoint(0, -c));

        // Badge
        QString text = QString("%1  %2%")
            .arg(d.label)
            .arg((int)(d.confidence * 100));

        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(text);
        textRect.setWidth(textRect.width() + 16);
        textRect.setHeight(textRect.height() + 8);
        textRect.moveBottomLeft(QPoint(rect.x(), rect.y() - 4));

        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(textRect, 5, 5);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }

    painter.end();

    // Показываем результат в label
    m_imageLabel->setPixmap(
        QPixmap::fromImage(result).scaled(
            m_imageLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}