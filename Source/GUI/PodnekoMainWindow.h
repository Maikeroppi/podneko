#ifndef PODNEKOMAINWINDOW_H
#define PODNEKOMAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardItemModel>

#include "../RSSStreamParser.h"

namespace Ui {
class PodnekoMainWindow;
}

class PodnekoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PodnekoMainWindow(QWidget *parent = 0);
    ~PodnekoMainWindow();

    static const int kTitleColumn = 0;
    static const int kDescriptionColumn = 1;
    static const int kLinkColumn = 2;
    static const int kGuidColumn = 3;
    static const int kPubDataColumn = 4;



public slots:
    void grabFeedData();
    void showStatusMessage(const QString& message) const;
    void handleError(QNetworkReply::NetworkError);
    void handleReply();

private:
    void setHorizontalHeader();

    Ui::PodnekoMainWindow *ui;
    QNetworkAccessManager* m_networkManager;
    QStandardItemModel* m_feedModel;
    RssStreamParser* m_parser;
    QStandardItem* m_activeChannelItem;
};

#endif // PODNEKOMAINWINDOW_H
