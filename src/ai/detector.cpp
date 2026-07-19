#include "detector.h"
#include <opencv2/opencv.hpp>
#include "engine/inference_engine.h"
#include <cmath>
#include <algorithm>

const int INPUT_WIDTH  = 640;
const int INPUT_HEIGHT = 640;
const int INPUT_CHANNELS = 3;

// YOLO параметры
const int NUM_CLASSES = 1;   // только один класс: 'damage'
const int NUM_ANCHORS = 8400;
const float CONF_THRESHOLD = 0.1f;
const float NMS_THRESHOLD = 0.45f;

// Вспомогательная функция: сигмоида
inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

// Non-Maximum Suppression (NMS)
std::vector<Detection> nms(const std::vector<Detection>& detections, float iou_threshold) {
    std::vector<Detection> result;
    std::vector<int> indices(detections.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    // Сортируем по уверенности (по убыванию)
    std::sort(indices.begin(), indices.end(),
        [&](int a, int b) { return detections[a].confidence > detections[b].confidence; });
    
    while (!indices.empty()) {
        int best = indices[0];
        result.push_back(detections[best]);
        indices.erase(indices.begin());
        
        std::vector<int> to_remove;
        for (int i = 0; i < indices.size(); ++i) {
            const auto& d1 = detections[best];
            const auto& d2 = detections[indices[i]];
            
            // Вычисляем IoU
            float inter_area = std::max(0, std::min(d1.box.x + d1.box.width, d2.box.x + d2.box.width) - std::max(d1.box.x, d2.box.x)) *
                               std::max(0, std::min(d1.box.y + d1.box.height, d2.box.y + d2.box.height) - std::max(d1.box.y, d2.box.y));
            float union_area = d1.box.area() + d2.box.area() - inter_area;
            float iou = inter_area / union_area;
            
            if (iou > iou_threshold) {
                to_remove.push_back(indices[i]);
            }
        }
        for (int idx : to_remove) {
            indices.erase(std::remove(indices.begin(), indices.end(), idx), indices.end());
        }
    }
    return result;
}

nlohmann::json detect_damage(const std::string& image_path) {
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        return {{"error", "Cannot read image"}};
    }

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(INPUT_WIDTH, INPUT_HEIGHT));
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);

    std::vector<float> tensor(INPUT_WIDTH * INPUT_HEIGHT * INPUT_CHANNELS);
    for (int y = 0; y < INPUT_HEIGHT; y++) {
        for (int x = 0; x < INPUT_WIDTH; x++) {
            for (int c = 0; c < INPUT_CHANNELS; c++) {
                tensor[(c * INPUT_HEIGHT + y) * INPUT_WIDTH + x] =
                    resized.at<cv::Vec3f>(y, x)[c];
            }
        }
    }

    InferenceEngine engine;
    if (!engine.loadModel("models/modelDamage.onnx")) {
        return {{"error", "Failed to load ONNX model"}};
    }

    auto raw_result = engine.run(tensor, {1, INPUT_CHANNELS, INPUT_HEIGHT, INPUT_WIDTH});

    // Парсинг YOLO-выхода (1x26x8400)
    std::vector<Detection> detections;
    const float* data = raw_result.data.data();
    int num_anchors = raw_result.shape[2];  // 8400

    for (int i = 0; i < num_anchors; ++i) {
        // Формат: [cx, cy, w, h, obj_conf, class1_conf, ...]
        float obj_conf = data[4 * num_anchors + i];  // confidence объектности
        if (obj_conf < CONF_THRESHOLD) continue;

        float cx = data[0 * num_anchors + i];
        float cy = data[1 * num_anchors + i];
        float w  = data[2 * num_anchors + i];
        float h  = data[3 * num_anchors + i];

        // Уверенность для класса (у нас только 1 класс)
        float cls_conf = data[5 * num_anchors + i];
        float confidence = sigmoid(obj_conf) * sigmoid(cls_conf);

        if (confidence < CONF_THRESHOLD) continue;

        // Перевод в пиксельные координаты
        int x = static_cast<int>((cx - w / 2) * INPUT_WIDTH);
        int y = static_cast<int>((cy - h / 2) * INPUT_HEIGHT);
        int width  = static_cast<int>(w * INPUT_WIDTH);
        int height = static_cast<int>(h * INPUT_HEIGHT);

        Detection d;
        d.confidence = confidence;
        d.classId = 0;  // только один класс
        d.label = QString("Damage");
        d.box = cv::Rect(x, y, width, height);
        detections.push_back(d);
    }

    // Применяем NMS
    detections = nms(detections, NMS_THRESHOLD);

    // Превращаем в JSON
    nlohmann::json json_result;
    for (const auto& d : detections) {
        nlohmann::json j;
        j["label"]       = d.label.toStdString();
        j["confidence"]  = d.confidence;
        j["class_id"]    = d.classId;
        j["box"]         = {d.box.x, d.box.y, d.box.width, d.box.height};
        json_result["detections"].push_back(j);
    }

    return json_result;
}