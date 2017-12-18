#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include "generator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (app.arguments().size() < 2) {
        qDebug() << "Usage:";
        qDebug() << "logging-ini-generator <project-dir> > logging.ini";
        return 1;
    }

    const QString projectDirPath = app.arguments().at(1);
    if (!QFile::exists(projectDirPath)) {
        qWarning() << "Project directory" << projectDirPath << "does not exist";
        return 1;
    }

    Generator generator;
    generator.setRuleMode(Generator::GroupAndDisable);
    generator.generate(projectDirPath);

    return app.exec();
}
