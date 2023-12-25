#include "digikeyinterface.h"
//#include "barcodescaninputwindow.h"
#include "remotedatasource.h"
#include <QApplication>
#include <QUrl>
#include <QtCore>
#include <QtGui>
#include <QtNetworkAuth>
#include <math.h>
#include <QRegularExpression>
#include <QMultiMap>

const QString digikey_part_url("/Search/v3/Products/");
const QString digikey_keyword_url("/Search/v3/Products/Keyword");

DigikeyWrapper::DigikeyWrapper(QObject *parent)
    : QObject(parent) {}

DigikeyWrapper::~DigikeyWrapper() {
    delete network_manager;
}

bool DigikeyWrapper::isPermanent() const {
    assert(m_settings);
    return permanent;
}

void DigikeyWrapper::setPermanent(bool value) {
    assert(m_settings);
    permanent = value;
}

void DigikeyWrapper::grant() {
    assert(m_settings);
    oauth2.grant();
}

void DigikeyWrapper::just_authenticated() {
    assert(m_settings);
    is_authenticated = true;
    //  query(m_sku_to_query_after_auth);
}

const QJsonObject get_paramter_by_name(const QJsonArray &parameters, QString paramter_name) {
#if 0
{
    ParameterId:	16
    ValueId:	"39158"
    Parameter:	"Package / Case"
    Value:	"0402 (1005 Metric)"
}
#endif
    for (const auto &value : parameters) {
        const auto &obj = value.toObject();
        if (obj["Parameter"].toString().contains(paramter_name)) {
            return obj;
        }
    }
    return QJsonObject();
}

void DigikeyWrapper::query(QString sku) {
    assert(m_settings);
    // "296-19884-1-ND"
    m_sku_to_query_after_auth = "";
    if (!is_authenticated) {
        m_sku_to_query_after_auth = sku;
        grant();

        {
            // pattern from https://stackoverflow.com/a/31364338
            QEventLoop loop;
            connect(this, &DigikeyWrapper::authenticated, &loop, &QEventLoop::quit);
            loop.exec();
        }
    }
    bool by_keyword = true;
    auto url_percent_encoded = QString::fromUtf8(QUrl::toPercentEncoding(sku));
    QNetworkRequest request = by_keyword ? QNetworkRequest(QUrl{m_settings->get_digikey_url_string() + digikey_keyword_url}) :
                                           QNetworkRequest(QUrl{m_settings->get_digikey_url_string() + digikey_part_url + url_percent_encoded});

    // qDebug() << url_percent_encoded;

    request.setRawHeader("Authorization", "Bearer " + oauth2.token().toUtf8()); //convert authToken to QByteArray when we set header;
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("X-DIGIKEY-Client-Id", m_settings->get_digikey_clientID().toUtf8());
    emit request_started();

    QNetworkReply *reply;
    if (by_keyword) {
        QString data = QString(
                           R"foo(
{
  "Keywords": "%1",
  "RecordCount": 10,
  "RecordStartPosition": 0,
  "Sort": {
    "SortOption": "SortByDigiKeyPartNumber",
    "Direction": "Ascending",
    "SortParameterId": 0
  },
  "RequestedQuantity": 0,
  "SearchOptions": [
    "ManufacturerPartSearch"
  ],
  "ExcludeMarketPlaceProducts": true
}
)foo")
                           //  .arg("CL05A104KA5NNNC");
                           // .arg("YC124-JR-0722RL");
                           .arg(sku);

        reply = network_manager->post(request, data.toUtf8());
    } else {
        reply = network_manager->get(request);
    }
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit supplier_error("Digikey error: " + reply->errorString());
            qCritical() << "Digikey error: " << reply->errorString();
            return;
        }

        const auto json = reply->readAll();
        qDebug() << json;
        const auto document = QJsonDocument::fromJson(json);
        Q_ASSERT(document.isObject());
        const auto rootObject = document.object();
        auto product_object = rootObject;
        int product_count = 1;
        const auto product_array = rootObject["Products"].toArray();
        if (by_keyword) {
            product_count = product_array.count();
            product_object = product_array[0].toObject();
        }
        auto taxonomy = rootObject["LimitedTaxonomy"].toObject();

        auto current_cat_array = taxonomy;
        auto taxo_array = taxonomy["Children"].toArray();
        RemoteDataSourceCategorie remote_categorie;
        QStringList category_uids;
        QStringList category_names;
        QString current_path;
        while (taxo_array.count()) {
            auto taxo_obj = taxo_array[0].toObject();
            category_uids.append(taxo_obj["ValueId"].toString());
            current_path += taxo_obj["Value"].toString() + "/";
            auto s = current_path;
            s = s.mid(0, s.size() - 1);
            category_names.append(s);
            taxo_array = taxo_obj["Children"].toArray();
        }

        QList<RemoteDataSourceCategorie> remote_categories;

        for (int j = 0; j < category_names.size(); j++) {
            RemoteDataSourceCategorie cat;
            cat.categorieNameTree = category_names[j].split("/");
            cat.categorie_uid = category_uids[j];
            remote_categories.append(cat);
        }

        result_QueryMPN.clear();
        for (int i = 0; i < product_count; i++) {
            RemoteDataSource_Result_QueryMPN_Entry entry("digikey");
            entry.categories.append(remote_categories);
            entry.setMpn(product_object["ManufacturerPartNumber"].toString());
            entry.manufacturer = product_object["Manufacturer"].toObject()["Value"].toString();
            entry.urlProduct = product_object["ProductUrl"].toString();
            entry.footprint = get_paramter_by_name(product_object["Parameters"].toArray(), "Package")["Value"].toString().split(" ")[0];
            entry.footprint = entry.footprint.remove(',');
            entry.description = product_object["ProductDescription"].toString();
            entry.urlDataSheet = product_object["PrimaryDatasheet"].toString();

            const auto &specs = product_object["Parameters"].toArray();

            for (const auto &spec_entry : specs) {
                const auto &specObject = spec_entry.toObject();
#if 0
{
    ParameterId:	16
    ValueId:	"39158"
    Parameter:	"Package / Case"
    Value:	"0402 (1005 Metric)"
}
#endif

                RemoteDataSourceSpecEntry specEntry;
                specEntry.name = specObject["Parameter"].toString();
                QString key = specEntry.name;
                QString value = specObject["Value"].toString();

                QRegularExpression re("(\\d*(\\.\\d*)?)");
                QRegularExpressionMatch match = re.match(value);

                //if ((match.captured().size() > 0) && (!match.captured().contains(QRegExp("^0\\d0\\d")))) { //patterns like 0402 are not ment to be a number
                    if ((match.captured().size() > 0) && (!match.captured().contains(QRegularExpression("^0\\d0\\d")))) { //patterns like 0402 are not ment to be a number
                    QString number_string = match.captured();
                    specEntry.value = number_string.toDouble();
                    int number_length = number_string.size();
                    auto unit = value.mid(number_length).trimmed();
                    unit = unit.replace("µ", "u");
                    specEntry.unitName = unit;
                    int exponent_from_unit_prefix = exponentFromUnitPrefix(specEntry.unitName[0].toLatin1());
                    if (unit.size() > 1) {
                        specEntry.value = specEntry.value.toDouble() * pow(10.0, exponent_from_unit_prefix);
                        if ((exponent_from_unit_prefix != 0)) {
                            specEntry.unitName = specEntry.unitName.remove(0, 1);
                        }
                        specEntry.unitName = specEntry.unitName.split(" ")[0];
                        specEntry.unitName = specEntry.unitName.remove(',');
                        specEntry.unitName = specEntry.unitName.remove('(');
                        specEntry.unitName = specEntry.unitName.remove(')');
                        specEntry.unitName = specEntry.unitName.remove('_');
                        specEntry.unitName = specEntry.unitName.remove('-');
                        specEntry.unitName = specEntry.unitName.remove('.');
                        specEntry.unitName = specEntry.unitName.trimmed();
                    }

                } else {
                    specEntry.value = value;
                }
                if (specEntry.value == "C0G, NP0") {
                    specEntry.value = "C0G";
                }
                specEntry.displayValue = specObject["Value"].toString();
                entry.specs.insert(key, specEntry);
            }

            product_object = product_array[i + 1].toObject();
            result_QueryMPN.append(entry);
        }
#if 0
  "LimitedTaxonomy":{
    "Children":[
      {
        "Children":[
          {
            "Children":[

            ],
            "ProductCount":1,
            "NewProductCount":0,
            "ParameterId":-8,
            "ValueId":"58",
            "Parameter":"Categories",
            "Value":"Aluminum Electrolytic Capacitors"
          }
        ],
        "ProductCount":1,
        "NewProductCount":0,
        "ParameterId":-8,
        "ValueId":"3",
        "Parameter":"Categories",
        "Value":"Capacitors"
      }
    ],
    "ProductCount":1,
    "NewProductCount":0,
    "ParameterId":-8,
    "ValueId":"0",
    "Parameter":"Categories",
    "Value":"Out of Bounds"
  },
#endif

#if 1
        QFile f("digikey.json");
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&f);
        out << json;
        //   f.close();
#endif
        emit request_finished();
    });
    // pattern from https://stackoverflow.com/a/31364338
    {
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(this, &DigikeyWrapper::request_finished, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        const int timeout_ms = 5000;
        timer.start(timeout_ms);
        loop.exec();

        if (timer.isActive())
            qDebug("received data");
        else
            qDebug("timeout");
    }
}

void DigikeyWrapper::setSettings(LibCreatorSettings *settings) {
    assert(m_settings == NULL);
    assert(settings);
    m_settings = settings;
    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl(m_settings->get_digikey_url_string() + "/v1/oauth2/authorize"));
    oauth2.setAccessTokenUrl(QUrl(m_settings->get_digikey_url_string() + "/v1/oauth2/token"));
    //oauth2.setScope("identity read");
    oauth2.setClientIdentifier(m_settings->get_digikey_clientID());
    oauth2.setClientIdentifierSharedKey(m_settings->get_digikey_secret());

    network_manager = new QNetworkAccessManager(this);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::Granted)
            emit authenticated();
    });
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization && isPermanent())
            parameters->insert("duration", "permanent");
    });
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

    connect(this, &DigikeyWrapper::authenticated, this, &DigikeyWrapper::just_authenticated);
}
