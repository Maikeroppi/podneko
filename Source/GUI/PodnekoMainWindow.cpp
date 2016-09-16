#include "PodnekoMainWindow.h"
#include "ui_PodnekoMainWindow.h"

#include <QFile>
#include <QMap>
#include <QXmlStreamReader>
#include <QTextStream>

static QMap<QString, int> s_nameToColumn =
{
    {"title",       PodnekoMainWindow::kTitleColumn},
    {"description", PodnekoMainWindow::kDescriptionColumn},
    {"link",        PodnekoMainWindow::kLinkColumn},
    {"guid",        PodnekoMainWindow::kGuidColumn},
    {"pubDate",     PodnekoMainWindow::kPubDataColumn}
};
static int s_expectedColumns = s_nameToColumn.size();



PodnekoMainWindow::PodnekoMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PodnekoMainWindow)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_feedModel(new QStandardItemModel(this))
    , m_parser(new RssStreamParser(this))
{
    ui->setupUi(this);

    // Set default header data for model
    ui->feedTree->setModel(m_feedModel);

    setWindowIcon(QIcon(":/podneko.png"));
    setHorizontalHeader();

    // Setup slots to handle model update when parsing
    QObject::connect(m_parser, &RssStreamParser::channelFound, [=](const QMap<QString,QString>& channelInfo)
    {
        QList<QStandardItem*> items;

        m_activeChannelItem = new QStandardItem();
        m_activeChannelItem->setText(channelInfo.value("title"));
        items.push_back(m_activeChannelItem);
        items.push_back(new QStandardItem());
        items.push_back(new QStandardItem(channelInfo.value("link")));

        m_feedModel->appendRow(items);
    });

    QObject::connect(m_parser, &RssStreamParser::itemFound, [=](const QMap<QString, QString>& itemData)
    {
        QList<QStandardItem*> items;
        for(int i = 0; i < s_expectedColumns; ++i)
            items.push_back(new QStandardItem);

        for(const auto& keyName : s_nameToColumn.keys())
        {
            auto item = items[s_nameToColumn.value(keyName)];
            item->setText(itemData.value(keyName));
        }

        if(m_activeChannelItem)
            m_activeChannelItem->appendRow(items);
        else
            qDeleteAll(items);
    });

    QObject::connect(m_parser, &RssStreamParser::parseFinished, [=]()
    {
        m_activeChannelItem = nullptr;
    });


    connect(ui->grabFeed, &QPushButton::clicked, this, &PodnekoMainWindow::grabFeedData);
}

void PodnekoMainWindow::setHorizontalHeader()
{
    m_feedModel->setHorizontalHeaderLabels({"Title", "Description", "Podcast URL", "GUID", "Publication Date"});
}

PodnekoMainWindow::~PodnekoMainWindow()
{
    delete ui;
}

void PodnekoMainWindow::grabFeedData()
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

        // Parser will emit signals with necessary data
        m_parser->parseStreamData(replyAsBytes);
    }
    else
        showStatusMessage("Invalid reply connected to handleReply!");
}
