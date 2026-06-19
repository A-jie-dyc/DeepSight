#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Controller.h"
#include "FrameDisplay.h"
#include "CommonDef.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<DetectionBox>();
    qRegisterMetaType<Track>();
    qRegisterMetaType<PreprocessParams>();
    qRegisterMetaType<std::vector<DetectionBox>>();
    qRegisterMetaType<std::vector<Track>>();
    qRegisterMetaType<Line>();
    qRegisterMetaType<ErrorDef::ErrorType>();
    qRegisterMetaType<ErrorDef::ErrorLevel>();
    qRegisterMetaType<ErrorInfo>();

    Controller *controller = new Controller(&app);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    //注册全局上下文属性
    engine.rootContext()->setContextProperty("Controller",controller);

    //加载主模块
    engine.loadFromModule("DeepSight", "Main");

    //QML加载完成后，将paintReady中继信号连接到FrameDisplay
    QObject *root = engine.rootObjects().value(0);
    if (root) {
        FrameDisplay *display = root->findChild<FrameDisplay*>("videoScreen");
        if (display) {
            //预览帧传递
            QObject::connect(controller, &Controller::frameDeliver,
                             display, &FrameDisplay::updateFrame);
            //统计线回传
            QObject::connect(display, &FrameDisplay::countLineUpdated,
                             controller, &Controller::countLineChanged);
            //错误信号
            QObject::connect(display, &FrameDisplay::errorOccurred,
                             controller, &Controller::handleError);
        }
    }

    qmlRegisterUncreatableMetaObject(
        ErrorDef::staticMetaObject,
        "DeepSight",
        1,0,
        "ErrorDef",
        "Cannot create instance of ErrorDef"
        );

    return QCoreApplication::exec();
}
