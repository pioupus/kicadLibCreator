#include "octopartinterface.h"
#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>

OctopartInterface::OctopartInterface(QString apikey, QObject *parent)
    : QObject(parent)
    , restRequester(parent) {
    this->apikey = apikey;
    connect(&restRequester, SIGNAL(http_request_finished()), this, SLOT(http_request_finished()));

    connect(&restRequester, SIGNAL(http_request_finished()), this, SIGNAL(octopart_request_finished()));

    connect(&restRequester, SIGNAL(http_request_started()), this, SIGNAL(octopart_request_started()));

    connect(&restRequester, SIGNAL(setProgressbar(int, int)), this, SIGNAL(setProgressbar(int, int)));
}

OctopartInterface::~OctopartInterface() {}

void OctopartInterface::sendMPNQuery(OctopartCategorieCache &octopartCategorieCache, QString mpn, bool useFuzzyQuery) {
    QString url_str;
    QMultiMap<QString, QString> map;
    if (useFuzzyQuery) {
        url_str = "http://octopart.com/api/v3/parts/search";
        map.insert("q", mpn);
    } else {
        url_str = "http://octopart.com/api/v3/parts/match";
        map.insert("queries", "[{\"mpn\":\"" + mpn + "\"}]");
    }

    map.insert("apikey", apikey);

    map.insert("pretty_print", "true");
    map.insert("include[]", "category_uids");
    map.insert("include[]", "short_description");
    map.insert("include[]", "specs");
    map.insert("include[]", "cad_models");

    map.insert("include[]", "external_links"); //for product link
    map.insert("include[]", "datasheets");

    // map.insert("hide[]", "offers");
    // map.insert("show[]", "offers");

    octopartResult_QueryMPN.clear();

    QBuffer buffer;
    restRequester.startRequest(url_str, map, &buffer);
    buffer.open(QIODevice::ReadOnly);
    QByteArray resultData = buffer.readAll();
    QJsonDocument loadDoc = QJsonDocument::fromJson(resultData);

    QJsonObject jsonObject(loadDoc.object());

    octopartResult_QueryMPN.clear();
    QJsonArray jsonArray = jsonObject["results"].toArray();
    QJsonArray jsonItemsArray;
    if (useFuzzyQuery) {
        jsonItemsArray = jsonArray;
    } else {
        if (jsonArray.count() == 0) {
            return;
        }
        QJsonObject jsonItem = jsonArray[0].toObject();
        jsonItemsArray = jsonItem["items"].toArray();
    }

    foreach (const QJsonValue &value, jsonItemsArray) {
        QJsonObject obj = value.toObject();
        if (useFuzzyQuery) {
            obj = obj["item"].toObject();
        }
        RemoteDataSource_Result_QueryMPN_Entry entry("octo");
        entry.setMpn(obj["mpn"].toString());
        entry.manufacturer = obj["manufacturer"].toObject()["name"].toString();
        entry.urlProduct = obj["external_links"].toObject()["product_url"].toString();
        entry.footprint = obj["specs"].toObject()["case_package"].toObject()["display_value"].toString();
        entry.description = obj["short_description"].toString();
        QJsonArray categorie_Array = obj["category_uids"].toArray();

        foreach (const QJsonValue &categorie, categorie_Array) {
            QString categorie_str = categorie.toString();
            RemoteDataSourceCategorie octopartCategorie = getCategorie(octopartCategorieCache, categorie_str);
            // qDebug() << octopartCategorie.categorieNameTree;
            entry.categories.append(octopartCategorie);
        }

#if 0
"specs": {
    "resistance": {
        "__class__": "SpecValue",
        "attribution": {
            "__class__": "Attribution",
            "first_acquired": null,
            "sources": null
        },
        "display_value": "22.0 k\u03a9",
        "max_value": null,
        "metadata": {
            "__class__": "SpecMetadata",
            "datatype": "decimal",
            "key": "resistance",
            "name": "Resistance",
            "unit": {
                "__class__": "UnitOfMeasurement",
                "name": "ohms",
                "symbol": "\u03a9"
            }
        },
        "min_value": null,
        "value": [
            "22000.0"
        ]
    },
#endif

        QJsonArray cad_object = obj["cad_models"].toArray();
        if (cad_object.count()) {
            entry.url3DModel = cad_object[0].toObject()["url"].toString();
        } else {
            entry.url3DModel = "";
        }
        // qDebug() << "3dmodel" << entry.url3DModel;
        QJsonObject specs_object = obj["specs"].toObject();

        foreach (const QJsonValue &spec_entry, specs_object) {
            QJsonObject specObject = spec_entry.toObject();
            QJsonObject metaObject = specObject["metadata"].toObject();
            QString key = metaObject["key"].toString();

            RemoteDataSourceSpecEntry specEntry;
            specEntry.name = metaObject["name"].toString();
            specEntry.dataType = metaObject["datatype"].toString();

            QJsonObject unitObject = metaObject["unit"].toObject();
            specEntry.unitName = unitObject["name"].toString();
            specEntry.unitSymbol = unitObject["symbol"].toString();
            if (specObject["value"].toArray().count()) {
                specEntry.value = specObject["value"].toArray()[0].toString();
            } else {
                specEntry.value = "";
            }
            specEntry.min_value = specObject["min_value"].toString();
            specEntry.max_value = specObject["max_value"].toString();
            specEntry.displayValue = specObject["display_value"].toString();

            // qDebug() << specEntry.toString();
            entry.specs.insert(key, specEntry);
        }
        bool urlManufacturerFound = entry.urlProduct.count();
        bool urlDatasheetFound = false;
        entry.urlDataSheet = "";
        int pdfPages = 0;

        for (auto item : obj["datasheets"].toArray()) {
            QString mimetype = item.toObject()["mimetype"].toString();
            if (mimetype == "text/html") {
                //lets get the first link
                if (urlManufacturerFound == false) {
                    entry.urlProduct = item.toObject()["url"].toString();
                    urlManufacturerFound = true;
                }
            } else if (mimetype == "application/pdf") {
                //lets get the datasheet with most pages
                int pdfPages_local = item.toObject()["metadata"].toObject()["num_pages"].toInt();
                if (pdfPages_local > pdfPages) {
                    entry.urlDataSheet = item.toObject()["url"].toString();
                    urlDatasheetFound = true;
                    pdfPages = pdfPages_local;
                }
            }
            if (urlManufacturerFound && urlDatasheetFound) {
                break;
            }
        }

        entry.urlOctoPart = obj["octopart_url"].toString();

        octopartResult_QueryMPN.append(entry);
#if 0
        qDebug() << "mpn" << entry.mpn;
        qDebug() << "manufacturer" << entry.manufacturer;
        qDebug() << "footprint" << entry.footprint;
        qDebug() << "description" << entry.description;
        qDebug() << "urlDataSheet" << entry.urlDataSheet;
        qDebug() << "urlOctoPart" << entry.urlOctoPart;
#endif

        // qDebug() << octopartResult_QueryMPN;
    }

    QFile jsonfile("jsonfile.txt");
    jsonfile.open(QIODevice::WriteOnly);
    jsonfile.write(resultData);
    jsonfile.close();
}

RemoteDataSourceCategorie OctopartInterface::getCategorie(OctopartCategorieCache &cache, QString category_id) {
    RemoteDataSourceCategorie result = cache.fetch(category_id);
    if (result.isEmpty()) {
        result = getCategorieByRequest(category_id);
        cache.addCategorie(result);
    }
    return result;
}

RemoteDataSourceCategorie OctopartInterface::getCategorieByRequest(QString category_id) {
    QString url_str = "https://octopart.com/api/v3/categories/" + category_id;
    RemoteDataSourceCategorie result;
    QMap<QString, QString> map;

    map.insert("apikey", apikey);

    map.insert("pretty_print", "true");

    QBuffer buffer;
    restRequester.startRequest(url_str, map, &buffer);
    buffer.open(QIODevice::ReadOnly);

    QByteArray resultData = buffer.readAll();
    QJsonDocument loadDoc = QJsonDocument::fromJson(resultData);
    QJsonObject jsonObject(loadDoc.object());

    QJsonArray jsonAncestorNames = jsonObject["ancestor_names"].toArray();
    // qDebug() << jsonAncestorNames;
    foreach (const QJsonValue &value, jsonAncestorNames) { result.categorieNameTree.append(value.toString()); }
    result.categorie_uid = jsonObject["uid"].toString();
    result.categorieNameTree.append(jsonObject["name"].toString());
    qDebug() << result.categorieNameTree;

    QFile jsonfile("jsonfile.txt");
    jsonfile.open(QIODevice::WriteOnly);
    jsonfile.write(resultData);
    jsonfile.close();
    return result;

    /*{
    "__class__": "Category",
    "ancestor_names": [
        "Electronic Parts",
        "Passive Components",
        "Resistors"
    ],
    "ancestor_uids": [
        "8a1e4714bb3951d9",
        "7542b8484461ae85",
        "5c6a91606d4187ad"
    ],
    "children_uids": [],
    "name": "Chip SMD Resistors",
    "num_parts": 653581,
    "parent_uid": "5c6a91606d4187ad",
    "uid": "a2f46ffe9b377933"
}*/
}

void OctopartInterface::setAPIKey(QString apikey) {
    this->apikey = apikey;
}

void OctopartInterface::http_request_finished() {}

OctopartCategorieCache::OctopartCategorieCache()
    : cache("octopartCategorieCache.ini", QSettings::IniFormat) {}

void OctopartCategorieCache::save() {
    cache.sync();
}

RemoteDataSourceCategorie OctopartCategorieCache::fetch(QString categorie_uid) {
    RemoteDataSourceCategorie result;
    QStringList groups = cache.childGroups();

    if (groups.contains(categorie_uid)) {
        cache.beginGroup(categorie_uid);
        result.categorie_uid = categorie_uid;
        result.categorieNameTree = cache.value("categorieNameTree").toStringList();

        cache.endGroup();
    }
    return result;
}

void OctopartCategorieCache::addCategorie(RemoteDataSourceCategorie octopartCategorie) {
    cache.beginGroup(octopartCategorie.categorie_uid);
    cache.setValue("categorieNameTree", octopartCategorie.categorieNameTree);

    cache.endGroup();
}
