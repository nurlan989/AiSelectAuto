#include "ai_service.h"

#include <QDebug>
#include <iostream>

AIService::AIService(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[AIService] initialized";
}

AIService &AIService::instance()
{
    static AIService instance;
    return instance;
}

bool AIService::init()
{
    std::vector<std::string> damageLabels =
        {
            "Front-Windscreen-Damage",
            "Headlight-Damage",
            "Major-Rear-Bumper-Dent",
            "Rear-windscreen-Damage",
            "RunningBoard-Dent",
            "Sidemirror-Damage",
            "Signlight-Damage",
            "Taillight-Damage",
            "bonnet-dent",
            "doorouter-dent",
            "doorouter-scratch",
            "fender-dent",
            "front-bumper-dent",
            "front-bumper-scratch",
            "medium-Bodypanel-Dent",
            "paint-chip",
            "paint-trace",
            "pillar-dent",
            "quaterpanel-dent",
            "rear-bumper-dent",
            "rear-bumper-scratch",
            "roof-dent"};

    std::string modelPath =
        "/Users/nurlan/Documents/AI_Project/AISelectCar/models/best.onnx";

    bool loaded =
        m_damageRunner.loadModel(
            modelPath,
            damageLabels);

    if (!loaded)
    {
        std::cerr
            << "[AIService] Model loading failed!"
            << std::endl;

        return false;
    }

    m_damageRunner.setConfThreshold(0.05f);

    std::cout
        << "[AIService] Damage model loaded successfully"
        << std::endl;

    return true;
}

void AIService::processImage(const QImage &image)
{
    qDebug()
        << "[AIService] processImage called";

    if (image.isNull())
    {
        qWarning()
            << "[AIService] Empty image received";

        return;
    }

    Frame frame =
        Frame::fromQImage(image);

    auto detections =
        m_damageRunner.run(frame);

    qDebug()
        << "[AIService] detections:"
        << detections.size();

    emit detectionsReady(detections);
}