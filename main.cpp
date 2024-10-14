#include <QApplication>
#include "moddingtoolsui.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModdingToolsUI window;
    window.show();
    return app.exec();
}