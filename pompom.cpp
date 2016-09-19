#include "pompom.h"

#include <QDebug>
#include <QFile>
#include <QStack>
#include <QTextStream>
#include <QVector>
#include <QXmlQuery>


void PomPom::run() {
    QMap<QString, GraphNode*> modules;

    QTextStream in(stdin);
    while (!in.atEnd()) {
        QString filepath = in.readLine();
        QFile pomfile(filepath);
        pomfile.open(QIODevice::ReadOnly);

        QXmlQuery moduleNameQuery, dependencyNameQuery;
        moduleNameQuery.bindVariable("pomfile", &pomfile);
        dependencyNameQuery.bindVariable("pomfile", &pomfile);
        moduleNameQuery.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                  declare variable $pomfile external; \
                                  doc($pomfile)/project/artifactId/string()");
        dependencyNameQuery.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                      declare variable $pomfile external; \
                                      doc($pomfile)/project/dependencies/dependency[1]/artifactId/string()");

        QString moduleName;
        moduleNameQuery.evaluateTo(&moduleName);
        pomfile.seek(0);
        QString dependencyNames;
        dependencyNameQuery.evaluateTo(&dependencyNames);

        GraphNode *module = modules[moduleName];
        module->name = moduleName;
        QString foo = dependencyNames.at(0);
        qDebug() << foo;
        foreach(const QString &dependencyName, dependencyNames) {
qDebug() << dependencyName << endl;
            GraphNode *dependency = modules[dependencyName];
            module->children.insert(dependencyName, dependency);
        }

        modules.insert(moduleName, module);
    }

    foreach(GraphNode *const&node, modules) {
        if (node->index < 0) strongConnect(node);
    }

    foreach(GraphNode *const&node, modules) {
        delete node;
    }

    emit finished();
}

void PomPom::strongConnect(GraphNode *node) {
    node->index = index;
    node->lowLink = index++;
    stack.push(node);
    node->onStack = true;

    foreach(GraphNode *const&child, node->children.values()) {
        if (child->index < 0) {
            strongConnect(child);
            node->lowLink = node->lowLink < child->lowLink ? node->lowLink : child->lowLink;
        }
        else if (child->onStack) {
            node->lowLink = node->lowLink < child->index   ? node->lowLink : child->index;
        }
    }

    if (node->lowLink == node->index) {
        QStack<GraphNode*> cycle;
        GraphNode *node2 = nullptr;

        do {
            node2 = stack.pop();
            node2->onStack = false;
            cycle.push(node2);
        }
        while (node2 != node);

        QTextStream out(stdout);
        out << "Cycle detected: " << cycle.pop()->name;
        foreach (GraphNode* n, cycle) {
            out << " -> " << n->name;
        }
    }
}

