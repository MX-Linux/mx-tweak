#ifndef CMD_H
#define CMD_H

#include <QEventLoop>
#include <QProcess>
#include <QString>

struct Result {
    int exitCode;
    QString output;
};

Result runProc(const QString &program, const QStringList &arguments = {}) noexcept;
Result runCmd(const QString &cmd) noexcept;

#endif // CMD_H
