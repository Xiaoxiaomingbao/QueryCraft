#include <QSplitter>
#include <QVBoxLayout>
#include <QMenu>
#include <QInputDialog>
#include <QHeaderView>
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
        const QString sql = "SELECT * FROM " + tableName + (condition.isEmpty() ? "" : " WHERE " + condition);
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
        condition = "";
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

void MainWindow::showFilterMenu(const int column) {
    QString fieldName = model->headerData(column, Qt::Horizontal).toString();
    const QVariant::Type type = model->data(model->index(0, column)).type();

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

    if (condition != "") {
        condition += " AND ";
    }

    if (op == "LIKE") {
        condition += QString("%1 LIKE '%%2%'").arg(fieldName, input);
    } else {
        if (withQuote) {
            condition += QString("%1 %2 '%3'").arg(fieldName, op, input);
        } else {
            condition += QString("%1 %2 %3").arg(fieldName, op, input);
        }
    }

    qInfo("当前条件：%s", qUtf8Printable(condition));
}
