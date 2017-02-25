# Pompom

Reads a list of Maven POM files (one per line), parses their dependencies, and prints any cycles (circular dependencies) that are found.

Example use:

```
find . -name pom.xml | pompom
```
