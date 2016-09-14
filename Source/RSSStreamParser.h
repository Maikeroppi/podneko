#ifndef RSSSTREAMPARSER_H
#define RSSSTREAMPARSER_H

#include <QObject>
#include <QStandardItemModel>
#include <QXmlStreamReader>

class RSSStreamParser : public QObject
{
    Q_OBJECT
public:
    explicit RSSStreamParser(QObject *parent = 0);

    static const int kTitleColumn = 0;
    static const int kDescriptionColumn = 1;
    static const int kLinkColumn = 2;
    static const int kGuidColumn = 3;
    static const int kPubDataColumn = 4;

    QStandardItem* readRSSChannelDataToModel(QXmlStreamReader& reader);
    void readRSSItemDataToModel(QXmlStreamReader& reader, QStandardItem* parentItem);

signals:

public slots:
};

#endif // RSSSTREAMPARSER_H
