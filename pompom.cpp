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
    QTextStream out(stdout);

    while (!in.atEnd()) {
        QString filepath = in.readLine().trimmed();
        if (filepath == "EOF") {
            break;
        }

        QFile pomfile(filepath);
        if (filepath.length() == 0 || !pomfile.open(QIODevice::ReadOnly)) {
//qDebug() << "Bad file: " << filepath;
            continue;
        }

        QXmlQuery moduleNameQuery, dependencyNameQuery;
        moduleNameQuery.bindVariable("pomfile", &pomfile);
        dependencyNameQuery.bindVariable("pomfile", &pomfile);
        moduleNameQuery.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                  declare variable $pomfile external; \
                                  doc($pomfile)/project/artifactId/string()");
        dependencyNameQuery.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                      declare variable $pomfile external; \
                                      doc($pomfile)/project/dependencies/dependency/artifactId/string()");

        QString moduleName;
        moduleNameQuery.evaluateTo(&moduleName);
        moduleName = moduleName.trimmed();
        GraphNode *module = modules[moduleName];
        if (module == nullptr) {
            module = new GraphNode(moduleName);
            modules.insert(moduleName, module);
        }
//qDebug() << moduleName;
        QStringList dependencyNames;
        pomfile.seek(0);
        dependencyNameQuery.evaluateTo(&dependencyNames);

        foreach (QString dependencyName, dependencyNames) {
            dependencyName = dependencyName.trimmed();
//qDebug() << "    " << dependencyName;
            GraphNode *dependency = modules[dependencyName];
            if (dependency == nullptr) {
                dependency = new GraphNode(dependencyName);
                modules.insert(dependencyName, dependency);
            }
            module->children.insert(dependencyName, dependency);
        }
    }

    foreach(GraphNode *const&node, modules) {
        if (node->index < 0) strongConnect(node);
    }

    foreach(GraphNode *const&node, modules) {
//qDebug() << node->name << ": " << node->index << ' ' << node->lowLink;
        delete node;
    }

    out << "Press Enter to finish." << flush;

    in.readLine();
    out << endl;

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
        if (cycle.size() <= 1) return;

        QTextStream out(stdout);
        out << "Cycle detected: " << cycle.pop()->name;
        foreach (GraphNode* n, cycle) {
            out << " -> " << n->name;
        }
        out << endl;
    }
}

