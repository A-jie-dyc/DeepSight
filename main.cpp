#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Controller.h"
#include "FrameDisplay.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<DetectionBox>();
    qRegisterMetaType<Track>();
    qRegisterMetaType<PreprocessParams>();
    qRegisterMetaType<std::vector<DetectionBox>>();
    qRegisterMetaType<std::vector<Track>>();

    Controller *controller = new Controller(&app);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("Controller",controller);

    engine.loadFromModule("DeepSight", "Main");

    // QML加载完成后，将paintReady中继信号连接到FrameDisplay
    QObject *root = engine.rootObjects().value(0);
    if (root) {
        FrameDisplay *display = root->findChild<FrameDisplay*>("videoScreen");
        if (display) {
            QObject::connect(controller, &Controller::frameDeliver,
                             display, &FrameDisplay::updateFrame);
        }
    }

    return QCoreApplication::exec();
}
