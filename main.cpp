#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include "generator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser cliParser;
    cliParser.setApplicationDescription("Qt Logging INI Generator");
    cliParser.addHelpOption();
    cliParser.addVersionOption();
    cliParser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    cliParser.addPositionalArgument("dir", "The path to the project's directory");

    QCommandLineOption submodulesOption(QStringList() << "s" << "submodules", "Collect categories from the given project and its direct submodules.");
    cliParser.addOption(submodulesOption);
    QCommandLineOption recursiveOption(QStringList() << "r" << "recursive", "Collect categories from the given project and its submodules, recursively (requires -s)");
    cliParser.addOption(recursiveOption);

    cliParser.process(app);

    if (cliParser.positionalArguments().isEmpty())
        cliParser.showHelp(1);

    if (cliParser.isSet(recursiveOption) && !cliParser.isSet(submodulesOption))
        cliParser.showHelp(1);

    const QString projectDirPath = cliParser.positionalArguments().first();
    if (!QFile::exists(projectDirPath)) {
        qWarning() << "Project directory" << projectDirPath << "does not exist";
        return 1;
    }

    Generator generator;
    generator.setSubmoduleMode(cliParser.isSet(submodulesOption)
        ? Generator::CheckSubmodules : cliParser.isSet(recursiveOption)
        ? Generator::CheckSubmodulesRecursively : Generator::IgnoreSubmodules);
    generator.setRuleMode(Generator::GroupAndDisable);
    generator.generate(cliParser.positionalArguments().first());

    return 0;
}
