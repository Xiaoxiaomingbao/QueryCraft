#include <QSplitter>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto *central = new QWidget(this);
    setCentralWidget(central);

    tree = new QTreeWidget;
    tree->setHeaderLabel("查询类型");
    auto *single = new QTreeWidgetItem(tree, QStringList("单表查询"));
    auto *join = new QTreeWidgetItem(tree, QStringList("联合查询"));
    tree->addTopLevelItem(single);
    tree->addTopLevelItem(join);
    tree->expandAll();

    table = new QTableView;
    sqlOutput = new QTextEdit;
    sqlOutput->setReadOnly(true);
    queryButton = new QPushButton("条件查询");

    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(table);
    rightLayout->addWidget(sqlOutput);
    rightLayout->addWidget(queryButton);

    auto *rightWidget = new QWidget;
    rightWidget->setLayout(rightLayout);

    auto *splitter = new QSplitter;
    splitter->addWidget(tree);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(1, 1);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(splitter);

    model = new QSqlQueryModel(this);

    connect(tree, &QTreeWidget::itemClicked, this, [=](const QTreeWidgetItem *item) {
        if (item->parent() && item->parent()->text(0) == "单表查询") {
            tableName = item->text(0);
            const QString sql = "SELECT * FROM " + tableName;
            model->setQuery(sql, db);

            if (model->lastError().isValid()) {
                sqlOutput->setText(sql + "\nError: " + model->lastError().text());
            } else {
                sqlOutput->setText(sql + "\nOK!");
                table->setModel(model);
            }
        }
    });

    connect(queryButton, &QPushButton::clicked, this, [=]() {
        qInfo("点击按钮：条件查询");
        const QString sql = "SELECT * FROM " + tableName;
        model->setQuery(sql, db);

        if (model->lastError().isValid()) {
            sqlOutput->setText(sql + "\nError: " + model->lastError().text());
        } else {
            sqlOutput->setText(sql + "\nOK!");
            table->setModel(model);
        }
    });
}

void MainWindow::setDatabase(const QSqlDatabase& database, const QString& schemaName) {
    db = database;

    if (!db.isOpen()) {
        qWarning("Database is not open!");
        return;
    }

    QSqlQuery query(db);
    query.exec("USE " + schemaName + ";");
    QStringList tables;
    if (query.exec("SHOW TABLES;")) {
        while (query.next()) {
            QString tableName = query.value(0).toString();
            qDebug() << "发现表：" << tableName;
            tables.append(tableName);
        }
    } else {
        qDebug() << "查询失败：" << query.lastError().text();
    }

    // 找到“单表查询”节点
    auto *single = tree->topLevelItem(0);
    single->takeChildren();  // 清空旧子项（如果有）

    for (const QString &tableName : tables) {
        auto *tableItem = new QTreeWidgetItem(QStringList(tableName));
        single->addChild(tableItem);
    }

    tree->expandAll();  // 展开树状图
}
