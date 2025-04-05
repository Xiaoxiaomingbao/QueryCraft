#include <QSplitter>
#include <QVBoxLayout>
#include <QMenu>
#include <QInputDialog>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QString>
#include <QDebug>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto *central = new QWidget(this);
    setCentralWidget(central);

    tree = new QTreeWidget;
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    tree->setHeaderLabel("查询类型");
    auto *single = new QTreeWidgetItem(tree, QStringList("单表查询"));
    auto *join = new QTreeWidgetItem(tree, QStringList("联合查询"));
    tree->addTopLevelItem(single);
    tree->addTopLevelItem(join);
    tree->expandAll();

    table = new QTableView;
    table->verticalHeader()->setVisible(false);

    sqlOutput = new QTextEdit;
    sqlOutput->setReadOnly(true);

    queryButton = new QPushButton("条件查询");
    cleanButton = new QPushButton("清除筛选");
    queryButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    cleanButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(table);
    rightLayout->addWidget(sqlOutput);
    rightLayout->addWidget(queryButton);
    rightLayout->addWidget(cleanButton);

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
        if (item->parent() && (item->parent()->text(0) == "联合查询" || item->parent()->text(0) == "单表查询")) {
            current = item->text(0);
            QString sql;
            if (item->parent()->text(0) == "单表查询") {
                sql = "SELECT * FROM " + current;
            } else {
                sql = joinedTables[current].first;
            }
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
        const QString sql = (joinedTables.contains(current) ? joinedTables[current].first : "SELECT * FROM " + current) + (conditions[current].isEmpty() ? "" : " WHERE " + conditions[current]);
        model->setQuery(sql, db);

        if (model->lastError().isValid()) {
            sqlOutput->setText(sql + "\nError: " + model->lastError().text());
        } else {
            sqlOutput->setText(sql + "\nOK!");
            table->setModel(model);
        }
    });

    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::showFilterMenu);

    connect(cleanButton, &QPushButton::clicked, this, [=]() {
        qInfo("点击按钮：清除筛选");
        conditions[current] = "";
    });

    connect(tree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showTreeContextMenu);
}

void MainWindow::setDatabase(const QSqlDatabase& database, const QString& schemaName) {
    db = database;

    if (!db.isOpen()) {
        qWarning("Database is not open!");
        return;
    }

    QSqlQuery query(db);
    query.exec("USE " + schemaName);
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

void MainWindow::showFilterMenu(const int column) {
    QString fieldName = model->headerData(column, Qt::Horizontal).toString();
    const QVariant::Type type = model->data(model->index(0, column)).type();

    if (joinedTables.contains(current)) {
        fieldName = joinedTables[current].second[fieldName];
    }

    QMenu menu;

    const QAction *greaterThan = nullptr;
    const QAction *lessThan = nullptr;
    const QAction *equalTo = nullptr;
    const QAction *like = nullptr;

    bool withQuote = false;

    if (type == QVariant::Int || type == QVariant::UInt ||  type == QVariant::LongLong ||  type == QVariant::ULongLong || type == QVariant::Double) {
        greaterThan = menu.addAction("大于...");
        lessThan = menu.addAction("小于...");
        equalTo = menu.addAction("等于...");
    } else if (type == QVariant::String) {
        like = menu.addAction("包含...");
        equalTo = menu.addAction("等于...");
        withQuote = true;
    } else {
        // 日期类型等暂时不处理
        return;
    }

    const QAction *selected = menu.exec(QCursor::pos());
    if (!selected) return;

    QString op;
    if (selected == greaterThan) op = ">";
    else if (selected == lessThan) op = "<";
    else if (selected == equalTo) op = "=";
    else if (selected == like) op = "LIKE";

    bool ok;
    QString input = QInputDialog::getText(this, "输入筛选条件",
                                          QString("字段 %1 %2 的值：").arg(fieldName, op),
                                          QLineEdit::Normal, "", &ok);
    if (!ok || input.isEmpty()) return;

    if (conditions[current] != "") {
        conditions[current] += " AND ";
    }

    if (op == "LIKE") {
        conditions[current] += QString("%1 LIKE '%%2%'").arg(fieldName, input);
    } else {
        if (withQuote) {
            conditions[current] += QString("%1 %2 '%3'").arg(fieldName, op, input);
        } else {
            conditions[current] += QString("%1 %2 %3").arg(fieldName, op, input);
        }
    }

    qInfo("当前条件：%s", qUtf8Printable(conditions[current]));
}

void MainWindow::showTreeContextMenu(const QPoint &pos) {
    QTreeWidgetItem *join = tree->itemAt(pos);
    if (!join || join->text(0) != "联合查询") return;

    QMenu menu;
    const QAction *innerJoin = menu.addAction("内连接");
    const QAction *leftJoin = menu.addAction("左连接");
    const QAction *rightJoin = menu.addAction("右连接");

    const QAction *selected = menu.exec(tree->viewport()->mapToGlobal(pos));
    if (!selected) return;

    bool ok;
    const QString input = QInputDialog::getText(this, "输入连接条件",
                                          QString("连接条件："),
                                          QLineEdit::Normal, "", &ok);
    if (!ok || input.isEmpty()) return;

    QList<QString> tables;
    const auto *single = tree->topLevelItem(0);
    for (int i = 0; i < single->childCount(); ++i) {
        QString tableName = single->child(i)->text(0);
        int from = 0;
        while (input.length() > from + tableName.length()) {
            const int index = input.indexOf(tableName, from);
            if (index == -1) {
                break;
            }
            if (input.length() > index + tableName.length() && input[index + tableName.length()] == '.') {
                tables.append(tableName);
                break;
            }
            from = index + 1;
        }
    }

    if (tables.size() != 2) {
        qWarning("连接表数量不正确：%s", qUtf8Printable(tables.join(",")));
        return;
    }

    auto *item = new QTreeWidgetItem(QStringList("联合查询" + QString::number(join->childCount() + 1)));

    QString sql = "SELECT ";

    QMap<QString, QList<QString>> columns;

    for (auto & tableName : tables) {
        QSqlRecord rec = db.record(tableName);
        for (int j = 0; j < rec.count(); ++j) {
            columns[tableName].append(rec.fieldName(j));
        }
    }

    for (auto & column : columns[tables[0]]) {
        if (!sql.endsWith("SELECT ")) {
            sql += ", ";
        }
        if (columns[tables[1]].contains(column)) {
            joinedTables[item->text(0)].second[QString("%1_%2").arg(tables[0], column)] = QString("%1.%2").arg(tables[0], column);
            sql += QString("%1.%2 AS %1_%2").arg(tables[0], column);
        } else {
            joinedTables[item->text(0)].second[column] = column;
            sql += column;
        }
    }

    for (auto & column : columns[tables[1]]) {
        if (!sql.endsWith("SELECT ")) {
            sql += ", ";
        }
        if (columns[tables[0]].contains(column)) {
            joinedTables[item->text(0)].second[QString("%1_%2").arg(tables[1], column)] = QString("%1.%2").arg(tables[1], column);
            sql += QString("%1.%2 AS %1_%2").arg(tables[1], column);
        } else {
            joinedTables[item->text(0)].second[column] = column;
            sql += column;
        }
    }

    sql += " FROM ";

    if (selected == innerJoin) {
        sql += QString("%1 INNER JOIN %2 ON %3").arg(tables[0], tables[1], input);
    } else if (selected == leftJoin) {
        sql += QString("%1 LEFT JOIN %2 ON %3").arg(tables[0], tables[1], input);
    } else if (selected == rightJoin) {
        sql += QString("%1 RIGHT JOIN %2 ON %3").arg(tables[0], tables[1], input);
    } else {
        return;
    }

    joinedTables[item->text(0)].first = sql;

    join->addChild(item);
    tree->expandAll();
}
