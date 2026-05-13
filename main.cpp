#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Controller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Controller *controller = new Controller(&app);

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
