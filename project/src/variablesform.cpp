#include "variablesform.h"
#include "ui_variablesform.h"
#include <QDebug>

VariablesForm::VariablesForm(QMap<QString, QString> variables, QWidget *parent):
    QDialog(parent),
    ui(new Ui::VariablesForm)
{
    ruleEditor = NULL;
    ui->setupUi(this);
    QMapIterator<QString, QString> ii(variables);

    QStringList hHeader;
    hHeader << "Name" << "Value";
    ui->tableWidget->setHorizontalHeaderLabels(hHeader);

    ui->tableWidget->setRowCount(variables.count()+1);
    ui->tableWidget->setColumnCount(2);
    int i=0;
    while (ii.hasNext()) {
        ii.next();
        QTableWidgetItem *newKeyItem = new QTableWidgetItem(ii.key());
        QTableWidgetItem *newValueItem = new QTableWidgetItem(ii.value());
        ui->tableWidget->setItem(i,0,newKeyItem);
        ui->tableWidget->setItem(i,1,newValueItem);

        i++;
    }
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
}

void VariablesForm::setRuleEditor(RuleEditor *ruleEditor)
{
    this->ruleEditor = ruleEditor;
}

VariablesForm::~VariablesForm()
{
    delete ui;
}



void VariablesForm::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    QString varname = ui->tableWidget->item(item->row(),0)->text();
    ruleEditor->addVariable(varname);
}

void VariablesForm::on_pushButton_clicked()
{
    close();
}
