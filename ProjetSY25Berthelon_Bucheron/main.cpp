#include "projetsy25main.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProjetSY25main w;
    w.show();

    return a.exec();
}
