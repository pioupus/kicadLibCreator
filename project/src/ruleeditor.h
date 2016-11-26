#ifndef RULEEDITOR_H
#define RULEEDITOR_H

#include <QDialog>
#include "partcreationrule.h"

namespace Ui {
class RuleEditor;
}

class RuleEditor : public QDialog
{
    Q_OBJECT

public:
    explicit RuleEditor(QWidget *parent = 0);
    void setRules(PartCreationRuleList ruleList, QStringList proposedCategories,QStringList proposedSourceDevices);

    PartCreationRuleList getRules(void);

    ~RuleEditor();



private slots:
    void on_lst_rules_currentRowChanged(int currentRow);

    void on_edt_rule_name_textChanged(const QString &arg1);

    void on_btn_rules_add_clicked();

private:
    Ui::RuleEditor *ui;

    PartCreationRuleList ruleList;

    void bringRuleToScreen(QString name);
    void saveRuleFromScreen(int index);
    void loadRuleList();

    QStringList proposedCategories;
    QStringList proposedSourceDevices;

    int ruleIndex_old;
};

#endif // RULEEDITOR_H
