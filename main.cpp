#include <QCoreApplication>
#include <QTimer>

#include <pompom.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PomPom *pp = new PomPom(&a);
    QObject::connect(pp, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, pp, SLOT(run()));

    return a.exec();
}
