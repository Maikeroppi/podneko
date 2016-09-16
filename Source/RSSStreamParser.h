#ifndef RSSSTREAMPARSER_H
#define RSSSTREAMPARSER_H

#include <QObject>
#include <QStandardItemModel>
#include <QXmlStreamReader>

class RssStreamParser : public QObject
{
    Q_OBJECT
public:
    explicit RssStreamParser(QObject *parent = 0);

    void parseStreamData(const QByteArray& data);

signals:
    void channelFound(const QMap<QString, QString>& channelInfo);
    void itemFound(const QMap<QString, QString>& episodeInfo);
    void parseFinished();

public slots:

protected:
    void parseRssChannelData();
    void parseRssItemData();

    QXmlStreamReader m_streamReader;
};

#endif // RSSSTREAMPARSER_H
