#ifndef GRAPHNODE_H
#define GRAPHNODE_H


#include <QMap>
#include <QObject>
#include <QString>


class GraphNode: public QObject
{
    Q_OBJECT

  public:
    QString name;
    QMap<QString, GraphNode*> children;
    bool onStack = false;
    int index = -1, lowLink = -1;

    GraphNode(QString n): name(n) {}
};


#endif // GRAPHNODE_H
