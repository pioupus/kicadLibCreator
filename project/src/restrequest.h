#ifndef RESTREQUEST_H
#define RESTREQUEST_H

#include <QBuffer>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class RequestSession {
    public:
    RequestSession();
    QBuffer *outFile;
    QNetworkReply *reply;
    QMultiMap<QString, QString> *map;
    bool mutex;
};

class RESTRequest : public QObject {
    Q_OBJECT
    public:
    explicit RESTRequest(QObject *parent = 0);
    ~RESTRequest();

    void startRequest(QString url, QMultiMap<QString, QString> params, QBuffer *result);

    signals:
    void http_request_finished();
    void http_request_started();
    void setProgressbar(int progress, int total);

    private slots:
    void httpFinished();

    void httpReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);

    private:
    void startRequest_raw(QUrl url, QMultiMap<QString, QString> params, QBuffer *result, int sessionIndex);

    // QMap<QString,QString> map;
    QNetworkAccessManager qnam;

    QList<RequestSession *> sessions;

    int findSession(QObject *networkReply);
};

#endif // RESTREQUSET_H
