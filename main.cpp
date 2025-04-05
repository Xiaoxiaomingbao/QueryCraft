#include <QApplication>

#include "ui/ConnectionWindow.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    ConnectionWindow connWin;
    connWin.setWindowTitle("连接数据库");
    connWin.resize(400, 300);
    connWin.show();

    QObject::connect(&connWin, &ConnectionWindow::connectionSuccess, [=](const QSqlDatabase& database, const QString& schemaName) {
        auto *mainWin = new MainWindow;
        mainWin->resize(800, 600);
        mainWin->setDatabase(database, schemaName);
        mainWin->show();
    });

    return QApplication::exec();
}
