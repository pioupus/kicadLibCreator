#ifndef PARTCREATIONRULE_H
#define PARTCREATIONRULE_H
#include <QList>
#include <QMultiMap>
#include "octopartinterface.h"
#include "kicadfile_lib.h"

class PartCreationRule
{
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


    void setKicadDeviceFieldsByRule(KICADLibSchematicDevice &targetDevice, KICADLibSchematicDevice &sourceDevice, QMap<QString,QString> OctopartSource);

};

class PartCreationRuleList{
public:
    PartCreationRuleList();

    void loadFromFile(QString filename);
    void saveFile(QString filename);

    QList<PartCreationRule> findRuleByCategoryID(QList<OctopartCategorie> &categoryIDs);
    QList<PartCreationRule> ruleList;

    PartCreationRule getRuleByName(QString ruleName);

    void modified();
private:
    QMultiMap<QString,int> linkedCategoryDirectory;

    QMap<QString,int> nameDirectory;
};


#endif // PARTCREATIONRULE_H
