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
    // 表名 -> 条件 SQL 片段
    QMap<QString, QString> conditions;
    // 联合查询名 -> {SQL 语句, {列名 -> 列的别名}}
    QMap<QString, QPair<QString, QMap<QString, QString>>> joinedTables;

    void showFilterMenu(int column);
    void showTreeContextMenu(const QPoint &pos);
};


#endif //MAINWINDOW_H
