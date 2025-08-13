#ifndef ABOUT_H
#define ABOUT_H

class QString;

void displayDoc(const QString &url, const QString &title) noexcept;
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url, const QString &license_title) noexcept;

#endif // ABOUT_H
