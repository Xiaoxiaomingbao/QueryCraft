#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto* window = new QWidget();

    // hello world message box
    QMessageBox::information(window, "消息", "你好，世界！");

    return QApplication::exec();
}
