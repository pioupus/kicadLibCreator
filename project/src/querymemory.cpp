#include "querymemory.h"
#include <QFile>
#include <QTextStream>
#include <QLineEdit>

QueryMemory::QueryMemory()
{
    QFile file ("oldqueries.conf");
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            oldQueries.append(in.readLine());

        }
        file.close();
    }
    oldQueries.sort();
}

void QueryMemory::addQuery(QString s)
{
    if (oldQueries.indexOf(s) == -1){
        oldQueries.append(s);
        oldQueries.sort();
    }
}

void QueryMemory::save()
{
    QFile file ("oldqueries.conf");
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        for(auto s:oldQueries){
            out << s << endl;
        }
        file.close();
    }
}

void QueryMemory::loadQueryList(QComboBox *cmb)
{
    QString tmp = cmb->currentText();
    cmb->setProperty("ignoreChanges",true);
    cmb->clear();

    for(auto s:oldQueries){
        cmb->addItem(s);
    }
    cmb->setCurrentText(tmp);
    cmb->setProperty("ignoreChanges",false);
}

