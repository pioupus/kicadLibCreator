#ifndef DIGIKEYWRAPPER_H
#define DIGIKEYWRAPPER_H

#include "libcreatorsettings.h"
#include "remotedatasource.h"
#include <QOAuth2AuthorizationCodeFlow>
#include <QObject>
#include <QtCore>
#include <QtNetwork>

class DigikeyWrapper : public QObject {
    Q_OBJECT
    public:
    DigikeyWrapper(QObject *parent);
    ~DigikeyWrapper();
    QNetworkReply *requestHotThreads();

    bool isPermanent() const;
    void setPermanent(bool value);
    void setSettings(LibCreatorSettings *settings);
    void query(QString sku);
    QList<RemoteDataSource_Result_QueryMPN_Entry> result_QueryMPN;

    public slots:
    void grant();
    void just_authenticated();

    signals:
    void authenticated();
    void got_data(const QMap<QString, QString> data, const QMap<QString, QString> additional_paramters);
    void supplier_error(const QString error_message);

    private:
    QOAuth2AuthorizationCodeFlow oauth2;
    QNetworkAccessManager *network_manager;
    LibCreatorSettings *m_settings = NULL;
    bool permanent = false;
    QString m_sku_to_query_after_auth;
    bool is_authenticated = false;

    signals:
    void request_started();
    void request_finished();
    void setProgressbar(int progress, int total);
};

#endif // DIGIKEYWRAPPER_H
