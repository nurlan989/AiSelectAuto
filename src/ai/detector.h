#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "types/detection.h" // ← используем твою структуру

nlohmann::json detect_damage(const std::string& image_path);