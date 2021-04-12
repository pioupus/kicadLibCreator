#ifndef PARTCREATIONRULE_H
#define PARTCREATIONRULE_H
#include "kicadfile_lib.h"
#include "octopartinterface.h"
#include <QList>
#include <QMultiMap>

class PartCreationRuleResult {
    public:
    QString name;
    QString designator;
    QString datasheet;
    QString id;
    QString mpn;
    QString manufacturer;
    QString display_value;
    QString description;
    QString lib_name;
    QString footprint;
};

class PartCreationRule {
    public:
    PartCreationRule(QString name);

    QString name;

    QStringList targetRule_designator;
    QStringList targetRule_name;
    QStringList targetRule_footprint;
    QStringList targetRule_datsheet;
    QStringList targetRule_id;
    QStringList targetRule_mpn;
    QStringList targetRule_manufacturer;
    QStringList targetRule_display_value;
    QStringList targetRule_description;
    QStringList targetRule_lib_name;

    QStringList key_octo_footprint_up;
    QStringList key_octo_descrption_up;
    QStringList key_octo_specification_up;

    QStringList key_octo_footprint_down;
    QStringList key_octo_descrption_down;
    QStringList key_octo_specification_down;

    QStringList links_category;
    QStringList links_source_device;

    PartCreationRuleResult setKicadDeviceFieldsByRule(QMap<QString, QString> OctopartSource);

    static bool isRuleFieldUsed(QStringList &ruleField);
};

class PartCreationRuleList {
    public:
    PartCreationRuleList();

    void loadFromFile(QString filename);
    void saveFile(QString filename);

    QList<PartCreationRule> findRuleByCategoryID(QList<OctopartCategorie> &categoryIDs);
    QList<PartCreationRule> ruleList;

    PartCreationRule getRuleByNameForAppliaction(QString ruleName);

    void modified();
    QStringList namesWithoutGlobal;

    private:
    QMultiMap<QString, int> linkedCategoryDirectory;
    PartCreationRule getRuleByName(QString ruleName);
    QMap<QString, int> nameDirectory;
    PartCreationRule globalRule;
    bool globalRule_exists;
};

#endif // PARTCREATIONRULE_H
