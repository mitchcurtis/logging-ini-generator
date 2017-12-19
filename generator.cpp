#include "generator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

Generator::Generator(QObject *parent) :
    QObject(parent),
    mRuleMode(SetAllRulesToFalse)
{
}

void Generator::setRuleMode(RuleMode ruleMode)
{
    mRuleMode = ruleMode;
}

void Generator::generate(const QString projectDirPath)
{
    qDebug() << "Generating logging.ini for project at" << projectDirPath << "...";

    QStringList nameFilters;
    nameFilters << ".git";
    QDir projectDir(projectDirPath);
    const QStringList dirs = projectDir.entryList(nameFilters, QDir::Dirs | QDir::Hidden | QDir::Readable);

    if (dirs.size() > 1) {
        qWarning() << "Somehow found more than one .git directory.. aborting";
        qApp->exit(1);
    }

    mGitProcess.setWorkingDirectory(projectDirPath);

    QStringList gitArguments;
    gitArguments << "grep" << "Q_LOGGING_CATEGORY\(";
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

    QStringList categories;

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
        categories.append(category);
    }

    if (categories.isEmpty()) {
        qDebug() << "No categories found";
        return qApp->exit(0);
    }

    stdOut << "[Rules]\n";

    if (mRuleMode == SetAllRulesToFalse) {
        std::sort(categories.begin(), categories.end());

        for (const QString &category : qAsConst(categories)) {
            stdOut << category << " = false\n";
        }
    } else {
        // For each category, construct a group using the first token,
        // and add subsequent categories whose first token matches that token as subcategories.
        // For example, given the following categories:
        //
        // app.main
        // app.blah
        // nocategory
        // tests.test1
        // tests.test2
        //
        // The end product will look something like this:
        //
        // app.* = false
        // ;app.main = true
        // ;app.blah = true
        //
        // tests.* = false
        // ;tests.test1 = true
        // ;tests.test2 = true
        //
        // nocategory = false

        static const QChar separator = QLatin1Char('.');

        QHash<QString, QStringList> groupedCategories;
        QStringList grouplessCategories;

        for (const QString &category : qAsConst(categories)) {
            const int periodIndex = category.indexOf(separator);
            if (periodIndex != -1) {
                const QString groupName = category.left(periodIndex);
                const QString subCategory = category.mid(periodIndex + 1);
                groupedCategories[groupName].append(subCategory);
            } else {
                grouplessCategories.append(category);
            }
        }

        // Sort the groups, as QHash::keys() is unsorted.
        QList<QString> sortedGroups = groupedCategories.keys();
        std::sort(sortedGroups.begin(), sortedGroups.end());

        for (const QString &groupName : sortedGroups) {
            stdOut << groupName << ".* = false\n";
            for (const QString &subCategory : groupedCategories.value(groupName)) {
                stdOut << ";" << groupName << separator << subCategory << " = true\n";
            }

            stdOut << "\n";
        }

        // Groupless categories go at the end.
        for (const QString &category : qAsConst(grouplessCategories)) {
            stdOut << category << " = false\n";
        }
    }

    qApp->exit(0);
}
