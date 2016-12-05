#include "ruleeditor.h"
#include "ui_ruleeditor.h"
#include "variablesform.h"
#include <QDebug>
#include <assert.h>

RuleEditor::RuleEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RuleEditor)
{
    ui->setupUi(this);
    ui->lblGlobalRule->setText("<a href=global>global</a>");
    ui->lblGlobalRule->setTextFormat(Qt::RichText);
    connect( ui->lblGlobalRule,SIGNAL(linkActivated(QString)),this,SLOT(onGlobalLinkClicked(QString)));
}


void RuleEditor::setRules(PartCreationRuleList ruleList, QStringList proposedCategories, QStringList proposedSourceDevices)
{
    this->ruleList = ruleList;
    this->proposedCategories = proposedCategories;
    this->proposedSourceDevices = proposedSourceDevices;
    loadRuleList();

    if (ruleList.ruleList.count()){
        ruleIndex_old = -1;
        ui->lst_rules->setCurrentRow(0);
    }else{
        ruleIndex_old = -1;
    }
}

void RuleEditor::setCurrenRule(QString RuleName)
{
    auto nameFinds = ui->lst_rules->findItems(RuleName, Qt::MatchExactly);
    if (nameFinds.count()){
        ui->lst_rules->setCurrentItem(nameFinds[0]);
    }
}

PartCreationRuleList RuleEditor::getRules()
{
    return ruleList;
}

void RuleEditor::setVariables(QMap<QString, QString> variables)
{
    this->variables = variables;
}

RuleEditor::~RuleEditor()
{
    delete ui;
}

void setManagedByGlobal(QString &ruleName, QStringList &targetField, QLabel *managedLabel, QPlainTextEdit *plaintext, QPushButton *btn) {

    if (PartCreationRule::isRuleFieldUsed(targetField) && (ruleName != "global")){

        managedLabel->setVisible(true);
        plaintext->setEnabled(false);
        btn->setEnabled(false);
    }else{
        managedLabel->setVisible(false);
        plaintext->setEnabled(true);
        btn->setEnabled(true);
    }
}

void RuleEditor::bringRuleToScreen(QString name)
{
    PartCreationRule rule("");
    PartCreationRule globalRule("global");
    int found=0;
    for (auto rule_iter:ruleList.ruleList){
        if (rule_iter.name == name){
            rule = rule_iter;
            found++;
        }
        if (rule_iter.name == "global"){
            globalRule = rule_iter;
            found++;
        }
        if(found == 2){
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
    ui->lst_source_dev_used->addItems(rule.links_source_device);

    ui->lst_category_proposed->clear();
    ui->lst_category_used->clear();

    for (auto s : rule.links_category){
        auto sl = s.split("~");
        assert(sl.count() == 2);
        auto lwi = new QListWidgetItem();
        lwi->setText(sl[1]);
        lwi->setData(Qt::UserRole,sl[0]);
        ui->lst_category_used->addItem(lwi);
    }

    for (auto s : proposedCategories){
        auto sl = s.split("~");
        assert(sl.count() == 2);
        auto lwi = new QListWidgetItem();
        lwi->setText(sl[1]);
        lwi->setData(Qt::UserRole,sl[0]);
        ui->lst_category_proposed->addItem(lwi);
    }



    setManagedByGlobal(name,globalRule.targetRule_datsheet, ui->lbl_by_global_rule_datasheet, ui->txt_fields_datasheet, ui->btn_variables_datasheet);
    setManagedByGlobal(name,globalRule.targetRule_description, ui->lbl_by_global_rule_description, ui->txt_fields_description, ui->btn_variables_description);

    setManagedByGlobal(name,globalRule.targetRule_designator, ui->lbl_by_global_rule_designator, ui->txt_fields_designator, ui->btn_variables_designator);
    setManagedByGlobal(name,globalRule.targetRule_display_value, ui->lbl_by_global_rule_display_value, ui->txt_fields_display_value, ui->btn_variables_disp_value);
    setManagedByGlobal(name,globalRule.targetRule_footprint, ui->lbl_by_global_rule_footprint, ui->txt_fields_footprint, ui->btn_variables_footprint);
    setManagedByGlobal(name,globalRule.targetRule_id, ui->lbl_by_global_rule_id, ui->txt_fields_id, ui->btn_variables_id);
    setManagedByGlobal(name,globalRule.targetRule_lib_name, ui->lbl_by_global_rule_libname, ui->txt_fields_library_name, ui->btn_variables_libname);
    setManagedByGlobal(name,globalRule.targetRule_manufacturer, ui->lbl_by_global_rule_manufacturer, ui->txt_fields_manufacturer, ui->btn_variables_manufacturer);

    setManagedByGlobal(name,globalRule.targetRule_mpn, ui->lbl_by_global_rule_mpn, ui->txt_fields_mpn, ui->btn_variables_mpn);
    setManagedByGlobal(name,globalRule.targetRule_name, ui->lbl_by_global_rule_name, ui->txt_fields_name, ui->btn_variables_name);
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
        ruleList.ruleList[index].links_category.append(item->data(Qt::UserRole).toString()+"~"+item->text());
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

bool RuleEditor::isNameAlreadyExisting(QString name)
{
    bool result = false;
    for (auto rule_iter:ruleList.ruleList){
        if (rule_iter.name == name){
            result = true;
            break;
        }
    }
    return result;
}



void RuleEditor::on_edt_rule_name_textChanged(const QString &arg1)
{
    if (ruleIndex_old < 0 || ruleIndex_old >= ui->lst_rules->count()){
        return;
    }
    if(isNameAlreadyExisting(arg1)){
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

void RuleEditor::onGlobalLinkClicked(QString link){
    (void)link;
    auto widgetList = ui->lst_rules->findItems("global",Qt::MatchExactly);
    if (widgetList.count()){
        ui->lst_rules->setCurrentItem(widgetList[0]);
    }else{
        on_btn_rules_add_clicked();
        ui->edt_rule_name->setText("global");
    }
}

void RuleEditor::on_btn_rules_remove_clicked()
{
    if (ruleIndex_old < 0 || ruleIndex_old >= ui->lst_rules->count()){
        return;
    }
    int index = ruleIndex_old;
    ruleIndex_old = -1;
    ruleList.ruleList.removeAt(index);
    QListWidgetItem *item = ui->lst_rules->takeItem(index);
    delete item;
    ruleIndex_old = ui->lst_rules->currentRow();
   // qDebug() << ruleList.ruleList.count();
}


void RuleEditor::on_buttonBox_accepted()
{
        saveRuleFromScreen(ruleIndex_old);
}



void RuleEditor::on_btn_rules_add_clicked()
{
    bool isDuplicate = true;
    QString currentName;
    int nameindex = ui->lst_rules->count();
    while (isDuplicate){
        nameindex += 1;
        currentName = "new Rule"+QString::number(nameindex);
        isDuplicate = isNameAlreadyExisting(currentName);
    }
    PartCreationRule rule(currentName);
    ruleList.ruleList.append(rule);
    loadRuleList();
    ui->lst_rules->setCurrentRow(ui->lst_rules->count()-1);
}

void RuleEditor::moveLinksBetweenListboxes(QListWidget *dest, QListWidget *src)
{
    QListWidgetItem *item = src->takeItem(src->currentRow());
    if (dest->findItems(item->text(),Qt::MatchExactly).count() == 0){
        dest->addItem(item);
    }else{
        delete item;
    }
}

void RuleEditor::on_btn_source_dev_add_clicked()
{
    moveLinksBetweenListboxes(ui->lst_source_dev_used, ui->lst_source_dev_proposed);
}

void RuleEditor::on_lst_source_dev_proposed_itemDoubleClicked(QListWidgetItem *item)
{
    (void)item;
    on_btn_source_dev_add_clicked();
}

void RuleEditor::on_btn_source_dev_remove_clicked()
{
    moveLinksBetweenListboxes(ui->lst_source_dev_proposed, ui->lst_source_dev_used);
}

void RuleEditor::on_lst_source_dev_used_itemDoubleClicked(QListWidgetItem *item)
{
    (void)item;
    on_btn_source_dev_remove_clicked();
}

void RuleEditor::on_btn_category_add_clicked()
{
    moveLinksBetweenListboxes(ui->lst_category_used, ui->lst_category_proposed);
}

void RuleEditor::on_btn_category_remove_clicked()
{
    moveLinksBetweenListboxes(ui->lst_category_proposed, ui->lst_category_used);
}

void RuleEditor::on_lst_category_proposed_itemDoubleClicked(QListWidgetItem *item)
{
    (void)item;
    on_btn_category_add_clicked();
}

void RuleEditor::on_lst_category_used_itemDoubleClicked(QListWidgetItem *item)
{
    (void)item;
    on_btn_category_remove_clicked();
}

void RuleEditor::showVariableWin(){

    variables.insert("%rule.name%",ui->edt_rule_name->text());
    VariablesForm* variablesForm = new VariablesForm(variables,this);
    variablesForm->setRuleEditor(this);
    variablesForm->show();
}

void RuleEditor::on_btn_variables_designator_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_name_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_footprint_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_datasheet_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_id_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_mpn_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_manufacturer_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_disp_value_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_description_clicked()
{
    showVariableWin();
}

void RuleEditor::on_btn_variables_libname_clicked()
{
    showVariableWin();
}

void RuleEditor::addVariable(QString variableName)
{
    if (ui->tabWidget->currentIndex() == 0){
        if (ui->toolbox->currentIndex() == 0){
            ui->txt_fields_designator->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 1){
            ui->txt_fields_name->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 2){
            ui->txt_fields_footprint->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 3){
            ui->txt_fields_datasheet->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 4){
            ui->txt_fields_id->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 5){
            ui->txt_fields_mpn->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 6){
            ui->txt_fields_manufacturer->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 7){
            ui->txt_fields_display_value->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 8){
            ui->txt_fields_description->appendPlainText(variableName);
        }else if (ui->toolbox->currentIndex() == 9){
            ui->txt_fields_library_name->appendPlainText(variableName);
        }
    }
}



