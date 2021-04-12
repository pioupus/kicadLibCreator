#include "digikeywrapper.h"
//#include "barcodescaninputwindow.h"
#include <QUrl>
#include <QtCore>
#include <QtGui>
#include <QtNetworkAuth>

const QString digikey_part_url("/Search/v3/Products/");

DigikeyWrapper::DigikeyWrapper(const LibCreatorSettings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings) {
    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl(m_settings.get_digikey_url_string() + "/v1/oauth2/authorize"));
    oauth2.setAccessTokenUrl(QUrl(m_settings.get_digikey_url_string() + "/v1/oauth2/token"));
    oauth2.setScope("identity read");
    oauth2.setClientIdentifier(m_settings.get_digikey_clientID());
    oauth2.setClientIdentifierSharedKey(m_settings.get_digikey_secret());

    network_manager = new QNetworkAccessManager(this);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::Granted)
            emit authenticated();
    });
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization && isPermanent())
            parameters->insert("duration", "permanent");
    });
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

    connect(this, &DigikeyWrapper::authenticated, this, &DigikeyWrapper::just_authenticated);
}

DigikeyWrapper::~DigikeyWrapper() {
    delete network_manager;
}

bool DigikeyWrapper::isPermanent() const {
    return permanent;
}

void DigikeyWrapper::setPermanent(bool value) {
    permanent = value;
}

void DigikeyWrapper::grant() {
    oauth2.grant();
}

void DigikeyWrapper::just_authenticated() {
    is_authenticated = true;
    query(m_sku_to_query_after_auth);
}

void DigikeyWrapper::query(QString sku) {
    // "296-19884-1-ND"
    m_sku_to_query_after_auth = "";
    if (!is_authenticated) {
        m_sku_to_query_after_auth = sku;
        grant();
        return;
    }
    auto url_percent_encoded = QString::fromUtf8(QUrl::toPercentEncoding(sku));
    // qDebug() << url_percent_encoded;
    QNetworkRequest request(QUrl{m_settings.get_digikey_url_string() + digikey_part_url + url_percent_encoded});
    request.setRawHeader("Authorization", "Bearer " + oauth2.token().toUtf8()); //convert authToken to QByteArray when we set header;
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("X-DIGIKEY-Client-Id", m_settings.get_digikey_clientID().toUtf8());

    QNetworkReply *reply = network_manager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit supplier_error("Digikey error: " + reply->errorString());
            qCritical() << "Digikey error: " << reply->errorString();
            return;
        }

        const auto json = reply->readAll();
        QMap<QString, QString> data;
        qDebug() << json;
        const auto document = QJsonDocument::fromJson(json);
        Q_ASSERT(document.isObject());
        const auto rootObject = document.object();
        data["description"] = rootObject["ProductDescription"].toString();
        data["image_url"] = rootObject["PrimaryPhoto"].toString();
        data["url"] = rootObject["ProductUrl"].toString();
        data["datasheet_url"] = rootObject["PrimaryDatasheet"].toString();
        data["manufacturer"] = rootObject["Manufacturer"].toObject()["Value"].toString();
        data["mpn"] = rootObject["ManufacturerPartNumber"].toString();

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
        auto taxonomy = rootObject["LimitedTaxonomy"].toObject();

        auto current_cat_array = taxonomy;
        auto taxo_array = taxonomy["Children"].toArray();
        QStringList path;
        while (taxo_array.count()) {
            auto taxo_obj = taxo_array[0].toObject();
            path.append(taxo_obj["Value"].toString());
            taxo_array = taxo_obj["Children"].toArray();
        }
        data["category"] = path.join("/");
        data["supplier"] = Supplier(Supplier::Digikey).toStr();
        qDebug() << data;

        QMap<QString, QString> additional_parameters;
        auto parameters_array = rootObject["Parameters"].toArray();
        for (auto param : parameters_array) {
            auto param_obj = param.toObject();
            auto param_name = param_obj["Parameter"].toString();
            auto param_value = param_obj["Value"].toString();
            if (param_name == "Packaging") {
                continue;
            }
            if ((param_value == "") || (param_value == "-")) {
                continue;
            }
            additional_parameters[param_name] = param_value;
        }
        emit got_data(data, additional_parameters);
#if 1
        QFile f("digikey.json");
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&f);
        out << json;
        //   f.close();
#endif
#if 0
        qDebug() << json;
        const auto document = QJsonDocument::fromJson(json);
        Q_ASSERT(document.isObject());
        const auto rootObject = document.object();
        const auto dataValue = rootObject.value("data");
        Q_ASSERT(dataValue.isObject());
        const auto dataObject = dataValue.toObject();
        const auto websocketUrlValue = dataObject.value("websocket_url");
        Q_ASSERT(websocketUrlValue.isString() && websocketUrlValue.toString().size());
        const QUrl websocketUrl(websocketUrlValue.toString());
        emit subscribed(websocketUrl);
#endif
    });
}
