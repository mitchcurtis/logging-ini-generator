#include "generator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

Generator::Generator(QObject *parent) : QObject(parent)
{
}

void Generator::generate(const QStringList &arguments)
{
    if (arguments.size() < 2) {
        qDebug() << "Usage:";
        qDebug() << "logging-ini-generator <project-dir> > logging.ini";
        qApp->exit(1);
        return;
    }

    const QString projectDirPath = arguments.at(1);
    QDir projectDir(projectDirPath);
    if (!projectDir.exists()) {
        qWarning() << "Project directory" << projectDirPath << "does not exist";
        qApp->exit(1);
    }

    qDebug() << "Generating logging.ini for project at" << projectDirPath << "...";

    QStringList nameFilters;
    nameFilters << ".git";
    const QStringList dirs = projectDir.entryList(nameFilters, QDir::Dirs | QDir::Hidden | QDir::Readable);

    if (dirs.size() > 1) {
        qWarning() << "Somehow found more than one .git directory.. aborting";
        qApp->exit(1);
    }

    mGitProcess.setWorkingDirectory(projectDirPath);

    QStringList gitArguments;
    gitArguments << "grep" << "Q_LOGGING_CATEGORY";
    mGitProcess.start(QStringLiteral("git"), gitArguments);

    connect(&mGitProcess, &QProcess::started, this, &Generator::onStarted);
    // I think we can't use the new syntax here because of finished()'s overloads...
    connect(&mGitProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
}

void Generator::onStarted()
{
}

void Generator::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        qWarning() << "git crashed! aborting..";
        qApp->exit(1);
    }

    if (exitCode != 0) {
        qWarning() << "git returned non-zero exit code (" << exitCode << "):";
        qWarning() << mGitProcess.readAllStandardError();
        qWarning() << "aborting..";
        qApp->exit(1);
    }

    QTextStream stdOut(stdout);

    while (1) {
        const QString line = mGitProcess.readLine();
        if (line.isEmpty())
            break;

        const QChar quote = QLatin1Char('"');
        const int openingQuoteIndex = line.indexOf(quote);
        if (openingQuoteIndex == -1)
            continue;

        const int closingQuoteIndex = line.lastIndexOf(quote);
        if (closingQuoteIndex == -1 || closingQuoteIndex == openingQuoteIndex)
            continue;

        const QString category = line.mid(openingQuoteIndex + 1, closingQuoteIndex - openingQuoteIndex - 1);

        stdOut << category << "\n";
    }

    qApp->exit(0);
}
