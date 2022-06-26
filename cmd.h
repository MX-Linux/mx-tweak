#ifndef CMD_H
#define CMD_H

#include <QEventLoop>
#include <QProcess>
#include <QString>

struct Result {
    int exitCode;
    QString output;
};

inline Result runCmd(const QString &cmd)
{
    QEventLoop loop;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    proc.start(QStringLiteral("/bin/bash"), {QStringLiteral("-c"), cmd});
    loop.exec();
    QObject::disconnect(&proc, nullptr, nullptr, nullptr);
    return {proc.exitCode(), proc.readAll().trimmed()};
}


#endif // CMD_H
