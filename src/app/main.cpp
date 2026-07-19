#include <QApplication>
#include "ui/screens/camera_screen.h"
#include "ai/services/ai_service.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Загружаем AI модель
    if (!AIService::instance().init()) {
        qWarning() << "AI initialization failed";
    }

    CameraScreen window;
    window.show();

    return app.exec();
}
// #include <iostream>
// #include <nlohmann/json.hpp>
// #include "../ai/detector.h"  // ← теперь используем detector.h

// int main(int argc, char* argv[]) {
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
//         return 1;
//     }

//     std::string image_path = argv[1];
//     nlohmann::json result = detect_damage(image_path);
//     std::cout << result.dump() << std::endl;
//     return 0;
// }