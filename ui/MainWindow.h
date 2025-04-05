#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTableView>
#include <QPushButton>
#include <QTextEdit>
#include <QSqlQueryModel>

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setDatabase(const QSqlDatabase& database, const QString& schemaName);

private:
    QTreeWidget *tree;
    QTableView *table;
    QPushButton *queryButton;
    QTextEdit *sqlOutput;

    QSqlDatabase db;
    QString tableName;
    QSqlQueryModel *model;
};


#endif //MAINWINDOW_H
