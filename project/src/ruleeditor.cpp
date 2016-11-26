#include "ruleeditor.h"
#include "ui_ruleeditor.h"

RuleEditor::RuleEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RuleEditor)
{
    ui->setupUi(this);
}


void RuleEditor::setRules(PartCreationRuleList ruleList, QStringList proposedCategories, QStringList proposedSourceDevices)
{
    this->ruleList = ruleList;
    this->proposedCategories = proposedCategories;
    this->proposedSourceDevices = proposedSourceDevices;
    loadRuleList();

    ruleIndex_old = -1;
}

PartCreationRuleList RuleEditor::getRules()
{
    return ruleList;
}

RuleEditor::~RuleEditor()
{
    delete ui;
}

void RuleEditor::bringRuleToScreen(QString name)
{
    PartCreationRule rule("");
    for (auto rule_iter:ruleList.ruleList){
        if (rule_iter.name == name){
            rule = rule_iter;
            break;
        }
    }

    ui->edt_rule_name->setText(rule.name);

    ui->txt_fields_description->    clear();
    ui->txt_fields_datasheet->      clear();
    ui->txt_fields_designator->     clear();
    ui->txt_fields_display_value->  clear();
    ui->txt_fields_footprint->      clear();
    ui->txt_fields_id->             clear();
    ui->txt_fields_library_name->   clear();
    ui->txt_fields_manufacturer->   clear();
    ui->txt_fields_mpn->            clear();
    ui->txt_fields_name->           clear();


    ui->txt_fields_description->    appendPlainText(rule.targetRule_description.join("\n") );
    ui->txt_fields_datasheet->      appendPlainText(rule.targetRule_datsheet.join("\n"));
    ui->txt_fields_designator->     appendPlainText(rule.targetRule_designator.join("\n"));
    ui->txt_fields_display_value->  appendPlainText(rule.targetRule_display_value.join("\n"));
    ui->txt_fields_footprint->      appendPlainText(rule.targetRule_footprint.join("\n"));
    ui->txt_fields_id->             appendPlainText(rule.targetRule_id.join("\n"));
    ui->txt_fields_library_name->   appendPlainText(rule.targetRule_lib_name.join("\n"));
    ui->txt_fields_manufacturer->   appendPlainText(rule.targetRule_manufacturer.join("\n"));
    ui->txt_fields_mpn->            appendPlainText(rule.targetRule_mpn.join("\n"));
    ui->txt_fields_name->           appendPlainText(rule.targetRule_name.join("\n"));


    ui->txt_key_desc_down->         clear();
    ui->txt_key_desc_up->           clear();
    ui->txt_key_foot_down->         clear();
    ui->txt_key_foot_up->           clear();
    ui->txt_key_spec_down->         clear();
    ui->txt_key_spec_up->           clear();

    ui->txt_key_desc_down->         appendPlainText(rule.key_octo_descrption_down.join("\n"));
    ui->txt_key_desc_up->           appendPlainText(rule.key_octo_descrption_up.join("\n"));
    ui->txt_key_foot_down->         appendPlainText(rule.key_octo_footprint_down.join("\n"));
    ui->txt_key_foot_up->           appendPlainText(rule.key_octo_footprint_up.join("\n"));
    ui->txt_key_spec_down->         appendPlainText(rule.key_octo_specification_down.join("\n"));
    ui->txt_key_spec_up->           appendPlainText(rule.key_octo_specification_up.join("\n"));

    ui->lst_source_dev_proposed->clear();
    ui->lst_source_dev_used->clear();

    ui->lst_source_dev_proposed->addItems(proposedSourceDevices);
    ui->lst_source_dev_proposed->addItems(rule.links_source_device);

    ui->lst_category_proposed->clear();
    ui->lst_category_used->clear();

    ui->lst_category_proposed->addItems(proposedCategories);
    ui->lst_category_used->addItems(rule.links_category);
}

void RuleEditor::saveRuleFromScreen(int index)
{
    if (index < 0 || index >= ruleList.ruleList.count()){
        return;
    }

    ruleList.ruleList[index].key_octo_descrption_down.      clear();
    ruleList.ruleList[index].key_octo_descrption_up.        clear();
    ruleList.ruleList[index].key_octo_footprint_down.       clear();
    ruleList.ruleList[index].key_octo_footprint_up.         clear();
    ruleList.ruleList[index].key_octo_specification_down.   clear();
    ruleList.ruleList[index].key_octo_specification_up.     clear();

    ruleList.ruleList[index].targetRule_description.        clear();
    ruleList.ruleList[index].targetRule_datsheet.           clear();
    ruleList.ruleList[index].targetRule_designator.         clear();
    ruleList.ruleList[index].targetRule_display_value.      clear();
    ruleList.ruleList[index].targetRule_footprint.          clear();
    ruleList.ruleList[index].targetRule_id.                 clear();
    ruleList.ruleList[index].targetRule_lib_name.           clear();
    ruleList.ruleList[index].targetRule_manufacturer.       clear();
    ruleList.ruleList[index].targetRule_mpn.                clear();
    ruleList.ruleList[index].targetRule_name.               clear();


    ruleList.ruleList[index].links_category.                clear();
    ruleList.ruleList[index].links_source_device.           clear();

    ruleList.ruleList[index].key_octo_descrption_down.      append(ui->txt_key_desc_down  ->toPlainText());
    ruleList.ruleList[index].key_octo_descrption_up.        append(ui->txt_key_desc_up    ->toPlainText());
    ruleList.ruleList[index].key_octo_footprint_down.       append(ui->txt_key_foot_down  ->toPlainText());
    ruleList.ruleList[index].key_octo_footprint_up.         append(ui->txt_key_foot_up    ->toPlainText());
    ruleList.ruleList[index].key_octo_specification_down.   append(ui->txt_key_spec_down  ->toPlainText());
    ruleList.ruleList[index].key_octo_specification_up.     append(ui->txt_key_spec_up    ->toPlainText());

    ruleList.ruleList[index].targetRule_description.        append(  ui->txt_fields_description->  toPlainText());
    ruleList.ruleList[index].targetRule_datsheet.           append(  ui->txt_fields_datasheet->    toPlainText());
    ruleList.ruleList[index].targetRule_designator.         append(  ui->txt_fields_designator->   toPlainText());
    ruleList.ruleList[index].targetRule_display_value.      append(  ui->txt_fields_display_value->toPlainText());
    ruleList.ruleList[index].targetRule_footprint.          append(  ui->txt_fields_footprint->    toPlainText());
    ruleList.ruleList[index].targetRule_id.                 append(  ui->txt_fields_id->           toPlainText());
    ruleList.ruleList[index].targetRule_lib_name.           append(  ui->txt_fields_library_name-> toPlainText());
    ruleList.ruleList[index].targetRule_manufacturer.       append(  ui->txt_fields_manufacturer-> toPlainText());
    ruleList.ruleList[index].targetRule_mpn.                append(  ui->txt_fields_mpn->          toPlainText());
    ruleList.ruleList[index].targetRule_name.               append(  ui->txt_fields_name->         toPlainText());

    for(int i=0;i< ui->lst_source_dev_used->count();i++){
        auto item = ui->lst_source_dev_used->item(i);
        ruleList.ruleList[index].links_source_device.append(item->text());
    }

    for(int i=0;i< ui->lst_category_used->count();i++){
        auto item = ui->lst_category_used->item(i);
        ruleList.ruleList[index].links_category.append(item->text());
    }

    on_edt_rule_name_textChanged(ui->edt_rule_name->text());
}

void RuleEditor::loadRuleList()
{
    ui->lst_rules->clear();

    for (auto rule:ruleList.ruleList){
        ui->lst_rules->addItem(rule.name);
    }
}

void RuleEditor::on_edt_rule_name_textChanged(const QString &arg1)
{
    if (ruleIndex_old < 0 || ruleIndex_old >= ui->lst_rules->count()){
        return;
    }
    ruleList.ruleList[ruleIndex_old].name = arg1;
    ui->lst_rules->item(ruleIndex_old)->setText(arg1);
}

void RuleEditor::on_lst_rules_currentRowChanged(int currentRow)
{
    if (currentRow < 0 || currentRow >= ui->lst_rules->count()){
        return;
    }
    saveRuleFromScreen(ruleIndex_old);
    ruleIndex_old = -1;
    bringRuleToScreen(ui->lst_rules->item(currentRow)->text());
    ruleIndex_old = currentRow;
}

void RuleEditor::on_btn_rules_add_clicked()
{
    PartCreationRule rule("new Rule"+QString::number(ui->lst_rules->count()+1));
    ruleList.ruleList.append(rule);
    loadRuleList();
}































