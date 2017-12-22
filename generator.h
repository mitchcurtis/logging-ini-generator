#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class Generator : public QObject
{
    Q_OBJECT
public:
    explicit Generator(QObject *parent = nullptr);

    enum SubmoduleMode {
        IgnoreSubmodules,
        CheckSubmodules,
        CheckSubmodulesRecursively
    };

    enum RuleMode {
        SetAllRulesToFalse,
        GroupAndDisable
    };

    void setSubmoduleMode(SubmoduleMode submoduleMode);
    void setRuleMode(RuleMode ruleMode);

    void generate(const QString projectDirPath);

private:
    void handleErrors(QProcess *process) const;
    void extractCategories(QProcess *process);
    void outputCatgeories();

    SubmoduleMode mSubmoduleMode;
    RuleMode mRuleMode;
    QProcess mGitProcess;
    QProcess mGitSubmoduleProcess;
    QStringList mCategories;
};

#endif // GENERATOR_H
