#ifndef REMOTEDATASOURCE_H
#define REMOTEDATASOURCE_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVariant>

int exponentFromUnitPrefix(char unit_prefix);

class RemoteDataSourceCategorie {
    public:
    void clear();

    QString categorie_uid;
    QStringList categorieNameTree;
    QString get_full_path();
    bool isEmpty();
};

class RemoteDataSourceOffer {
    public:
    QString url;
    QString sku;
    QList<QPair<int, float>> prices;
    QString currency;
};

class RemoteDataSourceSpecEntry {
    public:
    RemoteDataSourceSpecEntry();

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

class RemoteDataSource_Result_QueryMPN_Entry {
    public:
    RemoteDataSource_Result_QueryMPN_Entry(QString supplier_prefix);

    void clear();
    void copyFrom(RemoteDataSource_Result_QueryMPN_Entry &copy);

    QString manufacturer;
    QString description;
    QString footprint;

    QString urlOctoPart;
    QString urlProduct;
    QString urlDataSheet;
    QString url3DModel;
    QList<RemoteDataSourceCategorie> categories;
    QMap<QString, RemoteDataSourceSpecEntry> specs;

    QMap<QString, QString> getQueryResultMap();
    void setMpn(const QString &value);

    QString getMpn() const;
    void setDebugPrintMpn(bool b);

    QString toString();
    QString supplier_prefix;

    private:
    QString mpn;
    bool debugPrintMpn;
};

#endif // REMOTEDATASOURCE_H
