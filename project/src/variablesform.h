#ifndef VARIABLESFORM_H
#define VARIABLESFORM_H
#include "ruleeditor.h"

#include <QDialog>

#include <QTableWidgetItem>

namespace Ui {
class VariablesForm;
}

class VariablesForm : public QDialog
{
    Q_OBJECT

public:
    explicit VariablesForm(QMap<QString,QString> variables, QWidget *parent = 0);
    void setRuleEditor(RuleEditor* ruleEditor);
    ~VariablesForm();

private slots:


    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

    void on_pushButton_clicked();

private:
    Ui::VariablesForm *ui;
    QMap<QString,QString> variables;
    RuleEditor* ruleEditor;
};

#endif // VARIABLESFORM_H
