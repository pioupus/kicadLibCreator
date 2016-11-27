#include "partcreationrule.h"
#include <QSettings>
#include <assert.h>

PartCreationRule::PartCreationRule(QString name)
{
    this->name = name;
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

void PartCreationRule::setKicadDeviceFieldsByRule(KICADLibSchematicDevice &targetDevice, KICADLibSchematicDevice &sourceDevice, QMap<QString, QString> OctopartSource)
{
    targetDevice = sourceDevice;
    targetDevice.def.reference = replaceVariable(targetRule_designator.join(" "),OctopartSource);
    targetDevice.def.name = replaceVariable(targetRule_name.join(" "),OctopartSource);
    KICADLibSchematicDeviceField f = targetDevice.fields[1];
    targetDevice.fields[1].text = targetDevice.def.name;
            //replaceVariable(targetRule_footprint.join(" "),OctopartSource);
    targetDevice.fields[2].text = replaceVariable(targetRule_footprint.join(" "),OctopartSource);
    f.name = "mpn";
    f.text  = replaceVariable(targetRule_mpn.join(" "),OctopartSource);
    f.fieldIndex.setRawIndex(5);
    targetDevice.setField(f);

    f.name = "manufacturer";
    f.text  = replaceVariable(targetRule_manufacturer.join(" "),OctopartSource);
    f.fieldIndex.setRawIndex(6);
    targetDevice.setField(f);

    f.name = "display_value";
    f.text  = replaceVariable(targetRule_display_value.join(" "),OctopartSource);
    f.fieldIndex.setRawIndex(7);
    targetDevice.setField(f);


    targetDevice.dcmEntry.description = replaceVariable(targetRule_description.join(" "),OctopartSource);
    targetDevice.libName = replaceVariable(targetRule_lib_name.join(" "),OctopartSource);
}


PartCreationRuleList::PartCreationRuleList()
{

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

    for(int i=0;i<ruleList.count();i++){
        nameDirectory.insert(ruleList[i].name,i);

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

