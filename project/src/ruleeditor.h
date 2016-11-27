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

    ~RuleEditor();



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

private:
    Ui::RuleEditor *ui;

    PartCreationRuleList ruleList;

    void bringRuleToScreen(QString name);
    void saveRuleFromScreen(int index);
    void loadRuleList();

    QStringList proposedCategories;
    QStringList proposedSourceDevices;
    bool isNameAlreadyExisting(QString name);
    void moveLinksBetweenListboxes(QListWidget *dest, QListWidget * src);

    int ruleIndex_old;
};

#endif // RULEEDITOR_H
