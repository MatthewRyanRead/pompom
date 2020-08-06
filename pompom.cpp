#include "pompom.h"

#include <QDebug>
#include <QFile>
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

        QFile pomFile(filepath);
        if (filepath.length() == 0 || !pomFile.open(QIODevice::ReadOnly)) {
qDebug() << "Bad file: " << filepath;
            continue;
        }

        QString moduleName = getModuleName(&pomFile);
        GraphNode *module = modules[moduleName];
        if (module == nullptr) {
            module = new GraphNode(moduleName);
            modules.insert(moduleName, module);
        }

qDebug() << moduleName;

        QStringList dependencyNames = getDependencies(&pomFile);

        foreach (QString dependencyName, dependencyNames) {
            dependencyName = dependencyName.trimmed();

qDebug() << "    " << dependencyName;

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
        delete node;
    }

    if (!cycleFound) {
        out << "No cycles found." << endl;
    }

    emit finished();
}

QString PomPom::getModuleName(QFile *pomFile) {
    QXmlQuery moduleNameQuery;
    moduleNameQuery.bindVariable("pomFile", pomFile);
    moduleNameQuery.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                      declare variable $pomFile external; \
                                      doc($pomFile)/project/artifactId/string()");

    QString moduleName;
    moduleNameQuery.evaluateTo(&moduleName);
    moduleName = moduleName.trimmed();
    pomFile->seek(0);

    return moduleName;
}

QStringList PomPom::getDependencies(QFile *pomFile) {
    QXmlQuery dependencyNameQuery, dependencyNameQuery2, dependencyNameQuery3;
    dependencyNameQuery.bindVariable("pomFile", pomFile);
    dependencyNameQuery2.bindVariable("pomFile", pomFile);
    dependencyNameQuery3.bindVariable("pomFile", pomFile);

    dependencyNameQuery.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                  declare variable $pomFile external; \
                                  doc($pomFile)/project/dependencies/dependency/artifactId/string()");
    dependencyNameQuery2.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                   declare variable $pomFile external; \
                                   doc($pomFile)/project/dependencyManagement/dependencies/dependency/artifactId/string()");
    dependencyNameQuery3.setQuery("declare default element namespace \"http://maven.apache.org/POM/4.0.0\"; \
                                   declare variable $pomFile external; \
                                   doc($pomFile)/project/modules/module/string()");

    QStringList dependencyNames, dependencyNames2, dependencyNames3;
    dependencyNameQuery.evaluateTo(&dependencyNames);
    pomFile->seek(0);
    dependencyNameQuery2.evaluateTo(&dependencyNames2);
    pomFile->seek(0);
    dependencyNameQuery3.evaluateTo(&dependencyNames3);
    pomFile->seek(0);

    dependencyNames.append(dependencyNames2);
    dependencyNames.append(dependencyNames3);
    return dependencyNames;
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

        cycleFound = true;
        QTextStream out(stdout);
        QString firstNode = cycle.pop()->name;
        out << "Cycle detected:" << endl << "    " << firstNode;
        foreach (GraphNode* n, cycle) {
            out << " -> " << n->name;
        }
        out << " -> " << firstNode << endl;
    }
}
