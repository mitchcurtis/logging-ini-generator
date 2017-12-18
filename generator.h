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

    void generate(const QStringList &arguments);

private slots:
    void onStarted();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess mGitProcess;
};

#endif // GENERATOR_H
