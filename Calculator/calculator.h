#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>
#include <QVector>

class treeNode
{
public:
    static const QString CUSTOMIZEFUNCTION;
    static const QString EMBEDDEDFUNCTION;
    static const QString CONSTVALUE;
    static const QString VARIABLE;
    static const QString OPTR;
    static const QString INBACKET ;
    QString type;
    QString content;
    QVector<treeNode*>childs;
    treeNode(QString t,QString c)
    {
        type = t;
        content = c;
    }
    void addchild(treeNode* child)
    {
        childs.push_back(child);
    }
};

class Calculator
{
public:

    static const QSet<QString>optrset;
    static const QSet<QString>funcset;

    QString expression;
    QVector<QVector<QString>>splitresult;
    treeNode*headnode = nullptr;
    QMap<QString,double>*value_variable;
    QMap<QString,QString>*functions;

    short status = 1;

    Calculator(QString expression,QMap<QString,double>*value_variable,QMap<QString,QString>*functions);
    ~Calculator();
    QVector<QVector<QString>> split(QString expression,bool updatevariable);
    void buildtree(treeNode*parent,int& index,bool isfunction);
    double calculate(treeNode*parentnode);
    double solve();
    double getNodeValue(treeNode* node);
    void destroytree(treeNode*p);
};

#endif // CALCULATOR_H
