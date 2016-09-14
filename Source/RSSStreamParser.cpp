#include "RSSStreamParser.h"

RSSStreamParser::RSSStreamParser(QObject *parent) : QObject(parent)
{

}
QStandardItem* RSSStreamParser::readRSSChannelDataToModel(QXmlStreamReader& reader)
{
    QStandardItem* parentItem = nullptr;
    while(!reader.atEnd() && reader.readNextStartElement())
    {
        if(reader.name() == QLatin1Literal("title"))
        {
            parentItem = new QStandardItem(reader.readElementText());
        }
        else if(reader.name() == QLatin1Literal("item"))
            readRSSItemDataToModel(reader, parentItem);
        else
            reader.skipCurrentElement();
    }

    return parentItem;
}

// Example RSS entry from Wikipedia
//<item>
// <title>Example entry</title>
// <description>Here is some text containing an interesting description.</description>
// <link>http://www.example.com/blog/post/1</link>
// <guid isPermaLink="true">7bd204c6-1655-4c27-aeee-53f933c5395f</guid>
// <pubDate>Sun, 06 Sep 2009 16:20:00 +0000</pubDate>
//</item>
void RSSStreamParser::readRSSItemDataToModel(QXmlStreamReader& reader, QStandardItem* parentItem)
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
