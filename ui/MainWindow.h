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
    QPushButton *cleanButton;
    QTextEdit *sqlOutput;
    QSqlQueryModel *model;

    QSqlDatabase db;
    QString current;
    QMap<QString, QString> conditions;
    QMap<QString, QString> joinedTables;

    void showFilterMenu(int column);
    void showTreeContextMenu(const QPoint &pos);
};


#endif //MAINWINDOW_H
