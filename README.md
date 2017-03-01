# Pompom

Reads a list of Maven POM files (one per line), parses their dependencies, and prints any cycles (circular dependencies) that are found.  These are maximal cycles, i.e., the strongly connected components of the graph(s).

Example use:

```
find . -name pom.xml | pompom
```
