#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QStandardItemModel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadStockData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadStockData()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://openapi.twse.com.tw/v1/exchangeReport/STOCK_DAY_ALL"));
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "網路錯誤：" << reply->errorString();
            return;
        }

        QByteArray jsonData = reply->readAll();
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &jsonError);
        if (doc.isNull()) {
            qDebug() << "JSON 解析錯誤：" << jsonError.errorString();
            return;
        }

        if (!doc.isArray()) {
            qDebug() << "錯誤：JSON 應該是陣列格式！";
            return;
        }

        QJsonArray stockArray = doc.array();
        QStandardItemModel *model = new QStandardItemModel(stockArray.size(), 10, this);
        model->setHorizontalHeaderLabels({"股票代號", "名稱", "開盤價", "最高價", "最低價", "收盤價", "成交量", "成交值", "漲跌幅", "交易筆數"});

        for (int row = 0; row < stockArray.size(); ++row) {
            QJsonObject stock = stockArray[row].toObject();
            model->setItem(row, 0, new QStandardItem(stock["Code"].toString()));         // 股票代號
            model->setItem(row, 1, new QStandardItem(stock["Name"].toString()));         // 名稱
            model->setItem(row, 2, new QStandardItem(stock["OpeningPrice"].toString())); // 開盤價
            model->setItem(row, 3, new QStandardItem(stock["HighestPrice"].toString())); // 最高價
            model->setItem(row, 4, new QStandardItem(stock["LowestPrice"].toString()));  // 最低價
            model->setItem(row, 5, new QStandardItem(stock["ClosingPrice"].toString())); // 收盤價
            model->setItem(row, 6, new QStandardItem(stock["TradeVolume"].toString()));  // 成交量
            model->setItem(row, 7, new QStandardItem(stock["TradeValue"].toString()));   // 成交值
            model->setItem(row, 8, new QStandardItem(stock["Change"].toString()));       // 漲跌幅
            model->setItem(row, 9, new QStandardItem(stock["Transaction"].toString()));   // 交易筆數
        }

        // 設定模型給 TableView
        // Create the proxy model for filtering
        proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(model);
        // Set the proxy model to the table view
        ui->tableView->resizeColumnsToContents();  // 自動調整欄寬
        ui->tableView->setSortingEnabled(true);
        ui->tableView->setModel(proxyModel);

    });
}

void MainWindow::on_pushButton_clicked()
{
    QString filterText = ui->lineEdit->text();
    proxyModel->setFilterKeyColumn(0);
    proxyModel->setFilterFixedString(filterText);
}


void MainWindow::on_pushButton_2_clicked()
{
    QString filterText = ui->lineEdit2->text();
    proxyModel->setFilterKeyColumn(1);
    proxyModel->setFilterFixedString(filterText);
}

