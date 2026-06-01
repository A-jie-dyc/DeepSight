#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Controller.h"

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
    //注册图像提供者
    engine.addImageProvider("frameProvider",controller->getProvider());

    engine.rootContext()->setContextProperty("Controller",controller);

    engine.loadFromModule("DeepSight", "Main");

    return QCoreApplication::exec();
}
