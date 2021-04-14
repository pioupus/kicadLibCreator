#ifndef OCTOPARTINTERFACE_H
#define OCTOPARTINTERFACE_H

#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>

#include "remotedatasource.h"
#include "restrequest.h"

class OctopartCategorieCache {
    public:
    OctopartCategorieCache();

    void save();
    RemoteDataSourceCategorie fetch(QString categorie_uid);

    void addCategorie(RemoteDataSourceCategorie octopartCategorie);

    private:
    QSettings cache;
    RemoteDataSourceCategorie request(QString uid);
};

class OctopartInterface : public QObject {
    Q_OBJECT
    public:
    enum RequestType { None, QueryMPN, Categorie };

    explicit OctopartInterface(QString apikey, QObject *parent = 0);
    ~OctopartInterface();

    void sendMPNQuery(OctopartCategorieCache &octopartCategorieCache, QString mpn, bool useFuzzyQuery);

    RemoteDataSourceCategorie getCategorie(OctopartCategorieCache &cache, QString category_id);
    RemoteDataSourceCategorie getCategorieByRequest(QString category_id);
    void setAPIKey(QString apikey);

    //   OctopartCategorie octopartResult_Categorie;
    QList<RemoteDataSource_Result_QueryMPN_Entry> octopartResult_QueryMPN;

    private:
    RESTRequest restRequester;
    QString apikey;

    private slots:
    void http_request_finished();

    signals:
    void octopart_request_finished();
    void octopart_request_started();
    void setProgressbar(int progress, int total);
};

#endif // OCTOPARTINTERFACE_H
