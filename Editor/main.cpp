#include <QtGui/QGuiApplication>
#include "EditorApplication.h"

#ifndef RESOURCES_FOLDER
#define RESOURCES_FOLDER "."
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Gsage::EditorApplication editorApplication;
    editorApplication.initialize("../../resources/editorConfig.json");
    editorApplication.resize(1200, 900);
    editorApplication.show();
    editorApplication.raise();

    return app.exec();
}
