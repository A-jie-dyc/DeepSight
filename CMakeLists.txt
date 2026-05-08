cmake_minimum_required(VERSION 3.16)

project(DeepSight VERSION 0.1 LANGUAGES CXX)

# 强制使用c++17版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QML代码提示辅助功能
set(QT_QML_GENERATE_QMLLS_INI ON CACHE INTERNAL "")

find_package(Qt6 REQUIRED COMPONENTS Quick Multimedia)

qt_standard_project_setup(REQUIRES 6.10)

# OpenCV
set(OpenCV_DIR "D:/OpenCV/opencv/build")
find_package(OpenCV REQUIRED)

# ONNX Runtime
set(ONNX_RUNTIME_ROOT "D:/ONNX_Runtime/onnxruntime-win-x64-1.25.1")

include_directories(${ONNX_RUNTIME_ROOT}/include)   # 自动导入头文件
link_directories(${ONNX_RUNTIME_ROOT}/lib)          # 自动导入库文件

qt_add_executable(appDeepSight
    main.cpp
)

qt_add_qml_module(appDeepSight
    URI DeepSight
    QML_FILES
        Main.qml
        SOURCES CameraCapture.h CameraCapture.cpp
)

set_target_properties(appDeepSight PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appDeepSight
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Multimedia
        ${OpenCV_LIBS}  # OpenCV
        onnxruntime     # ONNX Runtime
)

install(TARGETS appDeepSight
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
