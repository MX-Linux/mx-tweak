#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "about.h"
#include <unistd.h>

using namespace Qt::Literals::StringLiterals;

// display doc as nomal user when run as root
void displayDoc(const QString &url, const QString &title) noexcept
{
    bool started_as_root = false;
    // prefer mx-viewer otherwise use xdg-open (use runuser to run that as logname user)
    if (QFile::exists(u"/usr/bin/mx-viewer"_s)) {
        QProcess::startDetached(u"mx-viewer"_s, {url, title});
    } else {
        if (getuid() != 0) {
            QProcess::startDetached(u"xdg-open"_s, {url});
            return;
        } else {
            QProcess proc;
            proc.start(u"logname"_s, {}, QIODevice::ReadOnly);
            proc.waitForFinished();
            QString user = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            QProcess::startDetached(u"runuser"_s, {u"-u"_s, user, u"--"_s, u"xdg-open"_s, url});
        }
    }
    if (started_as_root)
        qputenv("HOME", "/root");
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url, const QString &license_title) noexcept
{
    const auto width  = 600;
    const auto height = 500;
    QMessageBox msgBox(QMessageBox::NoIcon, title, message);
    auto *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    auto *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme(u"window-close"_s));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(licence_url, license_title);
    } else if (msgBox.clickedButton() == btnChangelog) {
        auto *changelog = new QDialog;
        changelog->setWindowTitle(QObject::tr("Changelog"));
        changelog->resize(width, height);

        auto *text = new QTextEdit(changelog);
        text->setReadOnly(true);
        QProcess proc;
        const QString &appfile = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        proc.start(u"zless"_s, {"/usr/share/doc/"_L1 + appfile + "/changelog.gz"_L1}, QIODevice::ReadOnly);
        proc.waitForFinished();
        text->setText(proc.readAllStandardOutput());

        auto *btnClose = new QPushButton(QObject::tr("&Close"), changelog);
        btnClose->setIcon(QIcon::fromTheme(u"window-close"_s));
        QObject::connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
        delete changelog;
    }
}
