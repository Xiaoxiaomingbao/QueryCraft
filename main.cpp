#include <QApplication>

#include "ui/ConnectionWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    ConnectionWindow window;
    window.setWindowTitle("连接数据库");
    window.resize(400, 300);
    window.show();

    return QApplication::exec();
}
