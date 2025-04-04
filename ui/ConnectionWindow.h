#ifndef CONNECTIONWINDOW_H
#define CONNECTIONWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ConnectionWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit ConnectionWindow(QWidget *parent = nullptr);
    ~ConnectionWindow() override;

private:
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLineEdit *userLineEdit;
    QLineEdit *passwordLineEdit;
    QLineEdit *schemaLineEdit;
    QLabel *statusLabel;

private slots:
    void connectToDatabase();
    void disconnectDatabase();

private:
    QSqlDatabase db;
};


#endif //CONNECTIONWINDOW_H
