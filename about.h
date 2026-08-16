#pragma once

#include <QString>

class QWidget;

void displayDoc(const QString &url, const QString &title, bool largeWindow = false, QWidget *parent = nullptr);
void displayHelpDoc(const QString &path, const QString &title, QWidget *parent = nullptr);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title, QWidget *parent = nullptr);
