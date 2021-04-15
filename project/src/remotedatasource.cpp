#include "remotedatasource.h"
#include <QMapIterator>

void RemoteDataSourceCategorie::clear() {
    categorie_uid = "";
    categorieNameTree.clear();
}

bool RemoteDataSourceCategorie::isEmpty() {
    return categorieNameTree.length() == 0 || categorie_uid == "";
}

RemoteDataSourceSpecEntry::RemoteDataSourceSpecEntry() {}

QString RemoteDataSourceSpecEntry::toString() {
    return "name:" + name + ", unitName:" + unitName + ", unitSymbol:" + unitSymbol + ", dataType:" + dataType + ", value:" + value.toString() +
           ", min_value:" + min_value.toString() + ", max_value:" + max_value.toString() + ", displayValue:" + displayValue;
}

QString unitPrefixFromExponent(int exponent) {
    switch (exponent) {
        case 24:
            return "Y";
            break;
        case 21:
            return "Z";
            break;
        case 18:
            return "E";
            break;
        case 15:
            return "P";
            break;
        case 12:
            return "T";
            break;
        case 9:
            return "G";
            break;
        case 6:
            return "M";
            break;
        case 3:
            return "k";
            break;
        case -3:
            return "m";
            break;
        case -6:
            return "u";
            break;
        case -9:
            return "n";
            break;
        case -12:
            return "p";
            break;
    }
    return "";
}

QString getNiceUnitPrefix(double val, QString &prefix) {
    int e = 0;
    bool sign_negative = val < 0;
    if (sign_negative) {
        val = -val;
    }
    if (val >= 1000) {
        while (val >= 1000) {
            val /= 1000;
            e += 3;
            if (e > 24) {
                break;
            }
        }
    } else if (val == 0) {
    } else if (val == std::numeric_limits<double>::infinity()) {
    } else if (val == -std::numeric_limits<double>::infinity()) {
    } else if (val < 1) {
        while (val < 1) {
            val *= 1000;
            e -= 3;
            if (e < -12) {
                break;
            }
        }
    }

    if (sign_negative) {
        val = -val;
    }
    prefix = unitPrefixFromExponent(e);
    QString result = QString::number(val);
    return result;
}

RemoteDataSource_Result_QueryMPN_Entry::RemoteDataSource_Result_QueryMPN_Entry(QString supplier_prefix) {
    debugPrintMpn = false;
    this->supplier_prefix = supplier_prefix;
}

void RemoteDataSource_Result_QueryMPN_Entry::clear() {
    setMpn("");
    manufacturer = "";
    description = "";
    footprint = "";
    urlOctoPart = "";
    urlDataSheet = "";
    url3DModel = "";
    urlDataSheet = "";
    urlDataSheet = "";
    supplier_prefix = "";
    categories.clear();

    specs.clear();
}

void RemoteDataSource_Result_QueryMPN_Entry::copyFrom(RemoteDataSource_Result_QueryMPN_Entry &copy) {
    this->manufacturer = copy.manufacturer;
    this->description = copy.description;
    this->footprint = copy.footprint;
    this->urlOctoPart = copy.urlOctoPart;
    this->urlDataSheet = copy.urlDataSheet;
    this->url3DModel = copy.url3DModel;
    this->urlProduct = copy.urlProduct;

    this->categories = copy.categories;
    this->specs = copy.specs;
    this->supplier_prefix = copy.supplier_prefix;
    setMpn(copy.getMpn());
}

QMap<QString, QString> RemoteDataSource_Result_QueryMPN_Entry::getQueryResultMap() {
    QMap<QString, QString> result;

    result.insert("%" + supplier_prefix + ".mpn%", mpn);
    result.insert("%" + supplier_prefix + ".manufacturer%", manufacturer);
    result.insert("%" + supplier_prefix + ".description%", description);
    result.insert("%" + supplier_prefix + ".footprint%", footprint);

    QMapIterator<QString, RemoteDataSourceSpecEntry> i(specs);
    while (i.hasNext()) {
        i.next();
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".name%", i.value().name);
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".value%", i.value().value.toString().remove("±"));
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".dispval%", i.value().displayValue);
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".unitname%", i.value().unitName);
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".unitsymbol%", i.value().unitSymbol);
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".minvalue%", i.value().min_value.toString());
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".maxvalue%", i.value().max_value.toString());

        QString prefix;
        QString niceVal = getNiceUnitPrefix(i.value().value.toDouble(), prefix);
        if (!niceVal.contains(".")) {
            niceVal += ".";
        }
        niceVal = niceVal.replace(".", prefix);
        result.insert("%" + supplier_prefix + ".spec." + i.key() + ".nicenum%", niceVal);
        if (i.value().unitSymbol == "m") {
            result.insert("%" + supplier_prefix + ".spec." + i.key() + ".ipc%", QString::number(i.value().value.toDouble() * 1000 * 100));
        }
    }

    return result;
}

void RemoteDataSource_Result_QueryMPN_Entry::setMpn(const QString &value) {
    mpn = value;
#if 0
    if (debugPrintMpn){
        qDebug() << "mpn set to" << value;
    }
#endif
}

QString RemoteDataSource_Result_QueryMPN_Entry::getMpn() const {
    return mpn;
}

void RemoteDataSource_Result_QueryMPN_Entry::setDebugPrintMpn(bool b) {
    debugPrintMpn = b;
}

QString RemoteDataSource_Result_QueryMPN_Entry::toString() {
    return "query result: mpn " + mpn + ", manufacturer " + manufacturer + " desc: " + description + " footprint " + footprint;
}
