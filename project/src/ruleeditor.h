#ifndef RULEEDITOR_H
#define RULEEDITOR_H

#include <QDialog>
#include "partcreationrule.h"
#include <QListWidgetItem>
#include <QListWidget>

namespace Ui {
class RuleEditor;
}

class RuleEditor : public QDialog
{
    Q_OBJECT

public:
    explicit RuleEditor(QWidget *parent = 0);
    void setRules(PartCreationRuleList ruleList, QStringList proposedCategories,QStringList proposedSourceDevices);

    void setCurrenRule(QString RuleName);

    PartCreationRuleList getRules(void);
    void setVariables(QMap<QString,QString> variables);

    ~RuleEditor();
    void addVariable(QString variableName);


private slots:
    void on_lst_rules_currentRowChanged(int currentRow);

    void on_edt_rule_name_textChanged(const QString &arg1);

    void on_btn_rules_add_clicked();

    void on_buttonBox_accepted();

    void on_btn_rules_remove_clicked();

    void on_btn_source_dev_add_clicked();

    void on_btn_category_add_clicked();

    void on_btn_category_remove_clicked();

    void on_btn_source_dev_remove_clicked();

    void on_lst_source_dev_proposed_itemDoubleClicked(QListWidgetItem *item);

    void on_lst_source_dev_used_itemDoubleClicked(QListWidgetItem *item);

    void on_lst_category_proposed_itemDoubleClicked(QListWidgetItem *item);

    void on_lst_category_used_itemDoubleClicked(QListWidgetItem *item);

    void on_btn_variables_designator_clicked();



    void on_btn_variables_name_clicked();

    void on_btn_variables_footprint_clicked();

    void on_btn_variables_datasheet_clicked();

    void on_btn_variables_id_clicked();

    void on_btn_variables_mpn_clicked();

    void on_btn_variables_manufacturer_clicked();

    void on_btn_variables_disp_value_clicked();

    void on_btn_variables_description_clicked();

    void on_btn_variables_libname_clicked();

    void onGlobalLinkClicked(QString link);
private:
    Ui::RuleEditor *ui;

    PartCreationRuleList ruleList;

    void bringRuleToScreen(QString name);
    void saveRuleFromScreen(int index);
    void loadRuleList();

    QStringList proposedCategories;
    QStringList proposedSourceDevices;
    bool isNameAlreadyExisting(QString name);
    QMap<QString,QString> variables;
    void moveLinksBetweenListboxes(QListWidget *dest, QListWidget * src);

    int ruleIndex_old;
    void showVariableWin();
};

#endif // RULEEDITOR_H
