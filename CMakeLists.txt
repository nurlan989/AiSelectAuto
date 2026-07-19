cmake_minimum_required(VERSION 3.16)

project(aiauto_picker LANGUAGES CXX)

# =========================
# APPLE OPENGL/AGL FIX
# =========================
# Apple removed AGL.framework from newer SDKs, but transitive
# find_package(OpenGL) calls from Qt/OpenCV still look for it.
# This disables that lookup before anything else can trigger it.
if(APPLE)
    set(CMAKE_DISABLE_FIND_PACKAGE_AGL TRUE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_MACOSX_RPATH ON)


# =========================
# APPLE ARM64
# =========================

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
endif()


# =========================
# QT6
# =========================

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

qt_standard_project_setup()

# =========================
# STRIP BROKEN AGL FRAMEWORK FROM QT
# =========================
# Homebrew's Qt6::Gui hardcodes "-framework AGL", which no longer
# exists in newer macOS SDKs. Remove it from Qt's link interface.
if(APPLE)
    foreach(_qt_target Qt6::Gui Qt6::Core Qt6::Widgets)
        if(TARGET ${_qt_target})
            get_target_property(_link_libs ${_qt_target} INTERFACE_LINK_LIBRARIES)
            if(_link_libs)
                list(REMOVE_ITEM _link_libs "-framework AGL")
                set_target_properties(${_qt_target} PROPERTIES
                    INTERFACE_LINK_LIBRARIES "${_link_libs}"
                )
            endif()
        endif()
    endforeach()
endif()

# =========================
# OPENCV
# =========================

find_package(OpenCV REQUIRED)



# =========================
# ONNX RUNTIME
# =========================

set(ONNXRUNTIME_LIB
    /opt/homebrew/lib/libonnxruntime.dylib
)



# =========================
# SOURCES
# =========================

set(SOURCES

    src/app/main.cpp

    src/ui/screens/camera_screen.cpp
    src/ui/screens/result_screen.cpp

    src/ai/config/detector_model_config.cpp

    src/ai/services/ai_service.cpp

    src/ai/runner/ai_model_runner.cpp

    src/ai/engine/inference_engine.cpp

    src/ai/pipeline/preprocessor.cpp
    src/ai/pipeline/post_processor.cpp

    src/ai/types/frame.cpp

    src/ai/utils/nms.cpp

    src/ai/detector.cpp
)



set(HEADERS

    src/ui/screens/camera_screen.h
    src/ui/screens/result_screen.h

    src/ai/config/detector_model_config.h

    src/ai/services/ai_service.h

    src/ai/runner/ai_model_runner.h

    src/ai/engine/inference_engine.h

    src/ai/pipeline/preprocessor.h
    src/ai/pipeline/post_processor.h

    src/ai/types/detection.h
    src/ai/types/frame.h

    src/ai/detector.h
)



# =========================
# EXECUTABLE
# =========================

add_executable(aiauto_picker

    ${SOURCES}
    ${HEADERS}

)



# =========================
# MACOS BUNDLE
# =========================

set_target_properties(aiauto_picker PROPERTIES

    MACOSX_BUNDLE TRUE

    MACOSX_BUNDLE_INFO_PLIST
    ${CMAKE_SOURCE_DIR}/Info.plist

)



# =========================
# INCLUDE
# =========================

target_include_directories(aiauto_picker PRIVATE

    src

    /opt/homebrew/include

    /opt/homebrew/include/onnxruntime

    ${OpenCV_INCLUDE_DIRS}

)



# =========================
# LINK
# =========================

target_link_libraries(aiauto_picker PRIVATE

    Qt6::Core
    Qt6::Gui
    Qt6::Widgets

    ${OpenCV_LIBS}

    ${ONNXRUNTIME_LIB}

)



# =========================
# RPATH
# =========================

set_target_properties(aiauto_picker PROPERTIES

    BUILD_WITH_INSTALL_RPATH TRUE

    INSTALL_RPATH
    "/opt/homebrew/lib"

)



# =========================
# MACOS LINK FIX
# =========================

if(APPLE)

    target_link_options(aiauto_picker PRIVATE

        "-Wl,-no_warn_duplicate_libraries"
        "-F/opt/homebrew/lib/dummy_frameworks"

    )

endif()



message(STATUS "")
message(STATUS "===================================")
message(STATUS " AISelectCar BUILD CONFIG")
message(STATUS " Qt6 + OpenCV + ONNX Runtime")
message(STATUS " ARM64 Apple Silicon")
message(STATUS "===================================")
message(STATUS "")