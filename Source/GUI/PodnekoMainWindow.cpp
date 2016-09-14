#include "PodnekoMainWindow.h"
#include "ui_PodnekoMainWindow.h"

#include <QFile>
#include <QMap>
#include <QXmlStreamReader>
#include <QTextStream>

#include "../RSSStreamParser.h"

PodnekoMainWindow::PodnekoMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PodnekoMainWindow)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_feedModel(new QStandardItemModel(this))
{
    ui->setupUi(this);


    auto initHeaderItem = [=](int column, QString headerString)
    {
        auto headerItem = new QStandardItem(headerString);
        m_feedModel->setHorizontalHeaderItem(column, headerItem);
    };

    // Set default header data for model
    initHeaderItem(RSSStreamParser::kTitleColumn,        "Title");
    initHeaderItem(RSSStreamParser::kDescriptionColumn,  "Description");
    initHeaderItem(RSSStreamParser::kLinkColumn,         "Link");
    initHeaderItem(RSSStreamParser::kGuidColumn,         "GUID");
    initHeaderItem(RSSStreamParser::kPubDataColumn,      "Publication Date");
    ui->feedTree->setModel(m_feedModel);

    setWindowIcon(QIcon(":/podneko.png"));

    auto setHorizontalHeader = [=]()
    {
        m_feedModel->setHorizontalHeaderLabels({"Title", "Description", "Podcast URL", "GUID", "Publication Date"});
    };
    setHorizontalHeader();

    connect(ui->grabFeed, &QPushButton::clicked, [=]()
    {
        m_feedModel->clear();
        setHorizontalHeader();

        auto itemCount = ui->feedsList->count();
        for(auto currentRow = 0; currentRow < itemCount; ++currentRow)
        {
            auto* item = ui->feedsList->item(currentRow);
            auto urlText = item->text();
            QUrl feedUrl(urlText);
            if(feedUrl.isValid())
            {
                QNetworkRequest request;
                request.setUrl(feedUrl);
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/rss+xml");
                auto reply = m_networkManager->get(request);
                connect(reply, &QNetworkReply::finished, this, &PodnekoMainWindow::handleReply);
                showStatusMessage(QString("Requesting: %1...").arg(feedUrl.toString()));
            }
            else
                showStatusMessage(QString("Feed URL is invalid: %1").arg(urlText));
        }
    });
}

PodnekoMainWindow::~PodnekoMainWindow()
{
    delete ui;
}

void PodnekoMainWindow::showStatusMessage(const QString& message) const
{
    statusBar()->showMessage(message);
}

void PodnekoMainWindow::handleError(QNetworkReply::NetworkError)
{
    auto reply = qobject_cast<QNetworkReply*>(sender());

    if(reply)
    {
        showStatusMessage(QString("Network error: %1").arg(reply->errorString()));
    }
    else
        showStatusMessage("Invalid reply connected to handleError!");
}

void PodnekoMainWindow::handleReply()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        showStatusMessage("Reply received!");
        auto replyAsBytes = reply->readAll();
        QString totalFeedData = replyAsBytes;

        QFile debugFile("debug.txt");
        if(debugFile.open(QIODevice::WriteOnly))
        {
            QTextStream debugStream(&debugFile);
            debugStream << totalFeedData;
        }

        QXmlStreamReader xmlStream(replyAsBytes);
        RSSStreamParser streamParser;
        QModelIndex parentIndex;

        while(!xmlStream.atEnd())
        {
            if(xmlStream.readNextStartElement())
            {
                auto name = xmlStream.name();
                if(name == QLatin1Literal("channel"))
                {
                    // Reset index to invalid in case something fails to parse
                    parentIndex = QModelIndex();
                    auto item = streamParser.readRSSChannelDataToModel(xmlStream);
                    m_feedModel->appendRow(item);
                }
            }
        }

        ui->feedTree->resizeColumnToContents(RSSStreamParser::kTitleColumn);
    }
    else
        showStatusMessage("Invalid reply connected to handleReply!");
}
