#ifndef QUERYMEMORY_H
#define QUERYMEMORY_H
#include <QStringList>
#include <QComboBox>

class QueryMemory
{
public:
    QueryMemory();
    void addQuery(QString s);
    void save();
    void loadQueryList(QComboBox *cmb);
private:
    QStringList oldQueries;
};

#endif // QUERYMEMORY_H
