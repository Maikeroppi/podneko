#include "PodnekoMainWindow.h"
#include "ui_PodnekoMainWindow.h"

#include <QFile>
#include <QMap>
#include <QXmlStreamReader>
#include <QTextStream>


static const int kTitleColumn = 0;
static const int kDescriptionColumn = 1;
static const int kLinkColumn = 2;
static const int kGuidColumn = 3;
static const int kPubDataColumn = 4;

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
    initHeaderItem(kTitleColumn,        "Title");
    initHeaderItem(kDescriptionColumn,  "Description");
    initHeaderItem(kLinkColumn,         "Link");
    initHeaderItem(kGuidColumn,         "GUID");
    initHeaderItem(kPubDataColumn,      "Publication Date");
    ui->feedTree->setModel(m_feedModel);

    setWindowIcon(QIcon(":/podneko.png"));

    connect(ui->grabFeed, &QPushButton::clicked, [=]()
    {
        QUrl feedUrl(ui->feedUrl->text());
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
            showStatusMessage(QString("Feed URL is invalid: %1").arg(ui->feedUrl->text()));
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
                    readRSSChannelDataToModel(xmlStream, parentIndex);
                }
            }
        }

        ui->feedTree->resizeColumnToContents(kTitleColumn);
    }
    else
        showStatusMessage("Invalid reply connected to handleReply!");
}

void PodnekoMainWindow::readRSSChannelDataToModel(QXmlStreamReader& reader, QModelIndex& createdIndex)
{
    QStandardItem* parentItem = nullptr;
    while(!reader.atEnd() && reader.readNextStartElement())
    {
        if(reader.name() == QLatin1Literal("title"))
        {
            parentItem = new QStandardItem(reader.readElementText());
            m_feedModel->appendRow(parentItem);
            createdIndex = m_feedModel->indexFromItem(parentItem);
        }
        else if(reader.name() == QLatin1Literal("item"))
            readRSSItemDataToModel(reader, parentItem);
        else
            reader.skipCurrentElement();
    }
}

// Example RSS entry from Wikipedia
//<item>
// <title>Example entry</title>
// <description>Here is some text containing an interesting description.</description>
// <link>http://www.example.com/blog/post/1</link>
// <guid isPermaLink="true">7bd204c6-1655-4c27-aeee-53f933c5395f</guid>
// <pubDate>Sun, 06 Sep 2009 16:20:00 +0000</pubDate>
//</item>
void PodnekoMainWindow::readRSSItemDataToModel(QXmlStreamReader& reader, QStandardItem* parentItem)
{
    static QMap<QString, int> s_nameToColumn =
    {
        {"title", kTitleColumn},
        {"description", kDescriptionColumn},
        {"link", kLinkColumn},
        {"guid", kGuidColumn},
        {"pubDate", kPubDataColumn}
    };
    static int s_expectedColumns = s_nameToColumn.size();
    bool parsingItem = true;

    QList<QStandardItem*> items;
    for(int i = 0; i < s_expectedColumns; ++i)
    {
        items.push_back(new QStandardItem());
    }

    QString name;
    while(!reader.atEnd() && parsingItem)
    {
        auto tokenType = reader.readNext();
        auto name = reader.name().toString();
        if(tokenType == QXmlStreamReader::StartElement && s_nameToColumn.contains(name))
        {
            int columnPos = s_nameToColumn.value(name);
            items[columnPos]->setText(reader.readElementText());
        }
        else if(tokenType == QXmlStreamReader::EndElement && name == QLatin1Literal("item"))
            parsingItem = false;
    }

    if(parentItem)
        parentItem->appendRow(items);
    else
        qDeleteAll(items);
}
