#ifndef QUERYMEMORY_H
#define QUERYMEMORY_H
#include <QComboBox>
#include <QStringList>

class QueryMemory {
    public:
    QueryMemory();
    void addQuery(QString s);
    void save();
    void loadQueryList(QComboBox *cmb);

    private:
    QStringList oldQueries;
};

#endif // QUERYMEMORY_H
