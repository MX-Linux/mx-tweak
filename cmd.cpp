#include "cmd.h"

#include <QEventLoop>
#include <QProcess>
#include <QThread>
#include <QString>

using namespace Qt::Literals::StringLiterals;

// Used by most modules for command line stuff
Result runProc(const QString &program, const QStringList &arguments) noexcept
{
    QEventLoop loop;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&proc, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred), &loop, &QEventLoop::quit);
    QObject::connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    proc.start(program, arguments);
    loop.exec();
    proc.disconnect(&loop);
    return {proc.exitCode(), proc.readAll().trimmed()};
}
Result runCmd(const QString &cmd) noexcept
{
    return runProc(u"/bin/sh"_s, {u"-c"_s, cmd});
}
int runSystem(const char *command) noexcept
{
    QEventLoop loop;
    int rc = 0;
    QThread *thread = QThread::create([command, &rc]() noexcept {
        rc = system(command);
    });
    if (thread != nullptr) {
        QObject::connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
        thread->start();
        loop.exec();
        delete thread;
    }
    return rc;
}
