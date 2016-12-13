#include "partcreationrule.h"
#include <QSettings>
#include <assert.h>

PartCreationRule::PartCreationRule(QString name)
{
    this->name = name;
}


bool PartCreationRule::isRuleFieldUsed(QStringList &ruleField){
    bool result = false;
    if (ruleField.count()){
        for( auto s:ruleField){
            if (s != ""){
                result = true;
                break;
            }
        }
    }
    return result;
}

QString replaceVariable(QString text, QMap<QString, QString> values){

    QString result = text;
    QMapIterator<QString, QString> ii(values);
    while (ii.hasNext()) {
        ii.next();
        result = result.replace(ii.key(),ii.value());
    }
    return result;
}

PartCreationRuleResult PartCreationRule::setKicadDeviceFieldsByRule(QMap<QString, QString> OctopartSource)
{
    PartCreationRuleResult result;


    result.designator = replaceVariable(targetRule_designator.join(" "),OctopartSource);
    result.name = replaceVariable(targetRule_name.join(" "),OctopartSource);
    result.footprint = replaceVariable(targetRule_footprint.join(" "),OctopartSource);
    result.mpn  = replaceVariable(targetRule_mpn.join(" "),OctopartSource);
    result.manufacturer  = replaceVariable(targetRule_manufacturer.join(" "),OctopartSource);
    result.display_value  = replaceVariable(targetRule_display_value.join(" "),OctopartSource);
    result.description = replaceVariable(targetRule_description.join(" "),OctopartSource);
    result.lib_name = replaceVariable(targetRule_lib_name.join(" "),OctopartSource);
    result.datasheet = replaceVariable(targetRule_datsheet.join(" "),OctopartSource);
    result.id = replaceVariable(targetRule_id.join(" "),OctopartSource);

    result.designator = result.designator.replace("\n"," ");
    result.name = result.name.replace("\n"," ");
    result.mpn = result.mpn.replace("\n"," ");
    result.manufacturer = result.manufacturer.replace("\n"," ");
    result.display_value = result.display_value.replace("\n"," ");
    result.description = result.description.replace("\n"," ");
    result.lib_name = result.lib_name.replace("\n"," ");
    result.datasheet = result.datasheet.replace("\n"," ");
    result.id = result.id.replace("\n"," ");

    return result;
}


PartCreationRuleList::PartCreationRuleList():globalRule("global")
{
    globalRule_exists = false;
}

void PartCreationRuleList::loadFromFile(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);
    QStringList ruleNames = settings.childGroups();
    for (auto rName : ruleNames){
        PartCreationRule rule(rName);
        settings.beginGroup(rName);


        rule.targetRule_designator.append(settings.value("targetRule_designator").toStringList());
        rule.targetRule_name.append(settings.value("targetRule_name").toStringList());
        rule.targetRule_footprint.append(settings.value("targetRule_footprint").toStringList());
        rule.targetRule_datsheet.append(settings.value("targetRule_datsheet").toStringList());
        rule.targetRule_id.append(settings.value("targetRule_id").toStringList());
        rule.targetRule_mpn.append(settings.value("targetRule_mpn").toStringList());
        rule.targetRule_manufacturer.append(settings.value("targetRule_manufacturer").toStringList());
        rule.targetRule_display_value.append(settings.value("targetRule_display_value").toStringList());
        rule.targetRule_description.append(settings.value("targetRule_description").toStringList());
        rule.targetRule_lib_name.append(settings.value("targetRule_lib_name").toStringList());


        rule.key_octo_footprint_up.append(settings.value("key_octo_footprint_up").toStringList());
        rule.key_octo_descrption_up.append(settings.value("key_octo_descrption_up").toStringList());
        rule.key_octo_specification_up.append(settings.value("key_octo_specification_up").toStringList());

        rule.key_octo_footprint_down.append(settings.value("key_octo_footprint_down").toStringList());
        rule.key_octo_descrption_down.append(settings.value("key_octo_descrption_down").toStringList());
        rule.key_octo_specification_down.append(settings.value("key_octo_specification_down").toStringList());

        rule.links_category.append(settings.value("links_category").toStringList());
        rule.links_source_device.append(settings.value("links_source_device").toStringList());

        ruleList.append(rule);
        settings.endGroup();
    }
    modified();
    if (!globalRule_exists){

        PartCreationRule rule("global");
        rule.targetRule_datsheet.append("%rule.name%/%octo.manufacturer.saveFilename%/%octo.mpn.saveFilename%.pdf");
        rule.targetRule_mpn.append("%octo.mpn%");
        rule.targetRule_manufacturer.append( "%octo.manufacturer%");
        rule.targetRule_id.append("%target.id%");
        rule.targetRule_name.append("%octo.mpn%");
        ruleList.append(rule);
        modified();
    }
}

void PartCreationRuleList::saveFile(QString filename)
{

    QSettings settings(filename, QSettings::IniFormat);
    settings.clear();
    for (auto rule : ruleList){

        settings.beginGroup(rule.name);


        settings.setValue("targetRule_designator",      rule.targetRule_designator);
        settings.setValue("targetRule_name",            rule.targetRule_name);
        settings.setValue("targetRule_footprint",       rule.targetRule_footprint);
        settings.setValue("targetRule_datsheet",        rule.targetRule_datsheet);
        settings.setValue("targetRule_id",              rule.targetRule_id);
        settings.setValue("targetRule_mpn",             rule.targetRule_mpn);
        settings.setValue("targetRule_manufacturer",    rule.targetRule_manufacturer);
        settings.setValue("targetRule_display_value",   rule.targetRule_display_value);
        settings.setValue("targetRule_description",     rule.targetRule_description);
        settings.setValue("targetRule_lib_name",        rule.targetRule_lib_name);

        settings.setValue("key_octo_footprint_up",      rule.key_octo_footprint_up);
        settings.setValue("key_octo_descrption_up",     rule.key_octo_descrption_up);
        settings.setValue("key_octo_specification_up",  rule.key_octo_specification_up);

        settings.setValue("key_octo_footprint_down",         rule.key_octo_footprint_down);
        settings.setValue("key_octo_descrption_down",        rule.key_octo_descrption_down);
        settings.setValue("key_octo_specification_down",     rule.key_octo_specification_down);

        settings.setValue("links_category",             rule.links_category);
        settings.setValue("links_source_device",        rule.links_source_device);
        settings.endGroup();
    }
}

void PartCreationRuleList::modified()
{
    linkedCategoryDirectory.clear();
    nameDirectory.clear();
    namesWithoutGlobal.clear();
    globalRule_exists = false;

    for(int i=0;i<ruleList.count();i++){
        nameDirectory.insert(ruleList[i].name,i);
        if (ruleList[i].name == "global"){
            globalRule = ruleList[i];
            globalRule_exists = true;
        }else {
            namesWithoutGlobal.append(ruleList[i].name);
        }
        auto cats = ruleList[i].links_category;
        for (auto cat:cats){
            auto sl = cat.split("~");
            assert(sl.count() == 2);
            linkedCategoryDirectory.insertMulti(sl[0],i);
        }
    }




}


QList<PartCreationRule> PartCreationRuleList::findRuleByCategoryID(QList<OctopartCategorie> &categoryIDs)
{
    QList<PartCreationRule> result;
    for (auto categoryID : categoryIDs){
        auto ints =  linkedCategoryDirectory.values(categoryID.categorie_uid);
        for (auto val : ints){
            result.append(ruleList[val]);
        }
    }
    return result;
}

PartCreationRule PartCreationRuleList::getRuleByName(QString ruleName)
{
    PartCreationRule result("");
    if (nameDirectory.contains(ruleName)){
        int index = nameDirectory[ruleName];
        result = ruleList[index];
    }
    return result;
}


PartCreationRule PartCreationRuleList::getRuleByNameForAppliaction(QString ruleName)
{
    PartCreationRule result = getRuleByName(ruleName);
    if (globalRule_exists){
        PartCreationRule global = globalRule;

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_designator)){
            result.targetRule_designator = global.targetRule_designator;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_name)){
            result.targetRule_name = global.targetRule_name;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_footprint)){
            result.targetRule_footprint = global.targetRule_footprint;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_datsheet)){
            result.targetRule_datsheet = global.targetRule_datsheet;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_id)){
            result.targetRule_id = global.targetRule_id;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_mpn)){
            result.targetRule_mpn = global.targetRule_mpn;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_manufacturer)){
            result.targetRule_manufacturer = global.targetRule_manufacturer;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_display_value)){
            result.targetRule_display_value = global.targetRule_display_value;
        }

        if (PartCreationRule::isRuleFieldUsed(global.targetRule_description)){
            result.targetRule_description = global.targetRule_description;
        }
        if (PartCreationRule::isRuleFieldUsed(global.targetRule_lib_name)){
            result.targetRule_lib_name = global.targetRule_lib_name;
        }
    }
    return result;
}

