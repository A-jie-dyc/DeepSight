#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <Qtqml>
#include "VideoCapture.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<CameraCapture>("DeepSight", 1, 0, "VideoCapture");
    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("DeepSight", "Main");

    return QCoreApplication::exec();
}
