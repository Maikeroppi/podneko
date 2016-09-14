#ifndef PODNEKOMAINWINDOW_H
#define PODNEKOMAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <QXmlStreamReader>

namespace Ui {
class PodnekoMainWindow;
}

class PodnekoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PodnekoMainWindow(QWidget *parent = 0);
    ~PodnekoMainWindow();

    void showStatusMessage(const QString& message) const;

    void handleError(QNetworkReply::NetworkError);
    void handleReply();

private:
    Ui::PodnekoMainWindow *ui;
    QNetworkAccessManager* m_networkManager;
    QStandardItemModel* m_feedModel;
};

#endif // PODNEKOMAINWINDOW_H
