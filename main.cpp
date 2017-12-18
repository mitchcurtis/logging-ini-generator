#include <QCoreApplication>

#include "generator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Generator generator;
    generator.generate(app.arguments());

    return app.exec();
}
