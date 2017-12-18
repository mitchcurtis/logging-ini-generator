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

    enum RuleMode {
        SetAllRulesToFalse,
        GroupAndDisable
    };

    void setRuleMode(RuleMode ruleMode);

    void generate(const QString projectDirPath);

private slots:
    void onStarted();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess mGitProcess;
    RuleMode mRuleMode;
};

#endif // GENERATOR_H
