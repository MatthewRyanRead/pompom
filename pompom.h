#ifndef POMPOM_H
#define POMPOM_H


#include <QObject>
#include <QFile>
#include <QStack>

#include <graphnode.h>


class PomPom: public QObject
{
    Q_OBJECT

  private:
    void strongConnect(GraphNode* node);

  public:
    PomPom(QObject *parent = 0): QObject(parent) {}

  private slots:
    void run();

  signals:
    void finished();

  private:
    QString getModuleName(QFile *pomFile);
    QStringList getDependencies(QFile *pomFile);

    int index = 0;
    bool cycleFound = false;
    QStack<GraphNode*> stack;
};


#endif // POMPOM_H
