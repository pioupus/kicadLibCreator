#include "textfile.h"
#include "ui_textfile.h"
#include <QFile>

TextFile::TextFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextFile)
{
    ui->setupUi(this);
}

void TextFile::setText(QString filepath)
{
    //test if textfile or not.
    QFile file(filepath);
    ui->textEdit->clear();
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        while (!file.atEnd()) {
             QByteArray line = file.readLine();
             ui->textEdit->append(line);
         }
    }
}

TextFile::~TextFile()
{
    delete ui;
}
