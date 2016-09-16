#include "RSSStreamParser.h"

RssStreamParser::RssStreamParser(QObject *parent) : QObject(parent)
{


}

void RssStreamParser::parseStreamData(const QByteArray& data)
{
    m_streamReader.clear();
    m_streamReader.addData(data);
    while(!m_streamReader.atEnd())
    {
       if(m_streamReader.readNextStartElement())
       {
           auto name = m_streamReader.name();
           if(name == QLatin1Literal("channel"))
           {
               parseRssChannelData();
           }
       }
    }

    emit parseFinished();
}

void RssStreamParser::parseRssChannelData()
{
    QString channelName;
    QMap<QString,QString> channelData;
    while(!m_streamReader.atEnd() && m_streamReader.readNextStartElement())
    {
        auto name = m_streamReader.name();
        if(name == QLatin1Literal("item"))
        {
            parseRssItemData();
        }
        else if(name == QLatin1Literal("title"))
        {
            channelData.insert(name.toString(), m_streamReader.readElementText());
            emit channelFound(channelData);
        }
        else
            m_streamReader.skipCurrentElement();
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
void RssStreamParser::parseRssItemData()
{
    bool parsingItem = true;

    QMap<QString, QString> episodeInfo;

    QString name;
    while(!m_streamReader.atEnd() && parsingItem)
    {
        auto tokenType = m_streamReader.readNext();
        auto name = m_streamReader.name().toString();
        if(tokenType == QXmlStreamReader::StartElement)// && s_nameToColumn.contains(name))
        {
            episodeInfo.insert(name, m_streamReader.readElementText());
        }
        else if(tokenType == QXmlStreamReader::EndElement && name == QLatin1Literal("item"))
        {
            parsingItem = false;
            emit itemFound(episodeInfo);
        }
    }
}
