#pragma once

#include <QString>
#include <QStringList>

struct Result {
    int exitCode;
    QString output;
};

Result runProc(const QString &program, const QStringList &arguments = {}) noexcept;
Result runCmd(const QString &cmd) noexcept;
int runSystem(const char *command) noexcept;
