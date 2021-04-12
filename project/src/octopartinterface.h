#ifndef OCTOPARTINTERFACE_H
#define OCTOPARTINTERFACE_H

#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>

#include "restrequest.h"

class OctopartCategorie {
    public:
    void clear();

    QString categorie_uid;
    QStringList categorieNameTree;

    bool isEmpty();
};

class OctopartCategorieCache {
    public:
    OctopartCategorieCache();

    void save();
    OctopartCategorie fetch(QString categorie_uid);

    void addCategorie(OctopartCategorie octopartCategorie);

    private:
    QSettings cache;
    OctopartCategorie request(QString uid);
};

class OctopartOffer {
    public:
    QString url;
    QString sku;
    QList<QPair<int, float>> prices;
    QString currency;
};

class OctopartSpecEntry {
    public:
    OctopartSpecEntry();

    QString name;
    QString unitName;
    QString unitSymbol;
    QString dataType;

    QVariant value;
    QVariant min_value;
    QVariant max_value;
    QString displayValue;

    QString toString();
};

class OctopartResult_QueryMPN_Entry {
    public:
    OctopartResult_QueryMPN_Entry();

    void clear();
    void copyFrom(OctopartResult_QueryMPN_Entry &copy);

    QString manufacturer;
    QString description;
    QString footprint;

    QString urlOctoPart;
    QString urlProduct;
    QString urlDataSheet;
    QString url3DModel;
    QList<OctopartCategorie> categories;

    QMap<QString, OctopartSpecEntry> specs;

    QMap<QString, QString> getQueryResultMap();
    void setMpn(const QString &value);

    QString getMpn() const;
    void setDebugPrintMpn(bool b);

    QString toString();

    private:
    QString mpn;
    bool debugPrintMpn;
};

class OctopartInterface : public QObject {
    Q_OBJECT
    public:
    enum RequestType { None, QueryMPN, Categorie };

    explicit OctopartInterface(QString apikey, QObject *parent = 0);
    ~OctopartInterface();

    void sendMPNQuery(OctopartCategorieCache &octopartCategorieCache, QString mpn, bool useFuzzyQuery);

    OctopartCategorie getCategorie(OctopartCategorieCache &cache, QString category_id);
    OctopartCategorie getCategorieByRequest(QString category_id);
    void setAPIKey(QString apikey);

    //   OctopartCategorie octopartResult_Categorie;
    QList<OctopartResult_QueryMPN_Entry> octopartResult_QueryMPN;

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
