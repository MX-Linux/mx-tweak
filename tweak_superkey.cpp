#include <QFile>
#include <QFileDialog>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "tweak_superkey.h"

using namespace Qt::Literals::StringLiterals;

TweakSuperKey::TweakSuperKey(Ui::defaultlook *ui, bool verbose, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}
{
    setup();
    connect(ui->textSuperKeyCommand, &QLineEdit::textChanged,
        this, &TweakSuperKey::textSuperKeyCommand_textChanged);
    connect(ui->pushSuperKeyBrowseAppFile, &QToolButton::clicked,
        this, &TweakSuperKey::pushSuperKeyBrowseAppFile_clicked);
    connect(ui->pushSuperKeyApply, &QPushButton::clicked,
        this, &TweakSuperKey::pushSuperKeyApply_clicked);
}

void TweakSuperKey::setup() noexcept
{
    ui->pushSuperKeyApply->setEnabled(false);
    const QString &test = runCmd(u"grep -m1 -v -e '^#' -e '^$'"_s
        u" $HOME/.config/xfce-superkey/xfce-superkey.conf"_s).output;
    if (!test.isEmpty()){
        ui->textSuperKeyCommand->setText(test);
    }
}

bool TweakSuperKey::checkSuperKey() noexcept
{
    return QFile::exists(u"/usr/bin/xfce-superkey-launcher"_s);
}

void TweakSuperKey::textSuperKeyCommand_textChanged(const QString &) noexcept
{
    ui->pushSuperKeyApply->setEnabled(true);
}

void TweakSuperKey::pushSuperKeyBrowseAppFile_clicked() noexcept
{
    const QString &command = QFileDialog::getOpenFileName(ui->tabWidget, tr("Select application to run",
            "will show in file dialog when selection an application to run"), u"/usr/bin"_s);

    //process file
    QString cmd;
    if (QFileInfo(command).fileName().endsWith(".desktop"_L1)) {
        cmd = runCmd("grep Exec= "_L1 + command).output.section(u'=',1,1).section(u'%',0,0).trimmed();
        cmd = runCmd("which "_L1 + cmd).output;
    } else {
        cmd = runCmd("which "_L1 + command).output;
    }
    if (verbose) {
        qDebug() << "custom command is " << cmd;
    }
    ui->textSuperKeyCommand->setText(cmd);
    ui->pushSuperKeyApply->setEnabled(true);
}

void TweakSuperKey::pushSuperKeyApply_clicked() noexcept
{
    QString home_path = QDir::homePath();
    if (!QFile(home_path + "/.config/xfce-superkey/xfce-superkey.conf"_L1).exists()){
        runCmd("mkdir -p "_L1 + home_path + "/.config/xfce-superkey/xfce-superkey.conf"_L1);
        runCmd("cp /usr/share/xfce-superkey/xfce-superkey.conf "_L1 + home_path + "/.config/xfce-superkey/xfce-superkey.conf"_L1);
    }
    QString cmd = ui->textSuperKeyCommand->text();
    //add command if no uncommented lines
    if (runCmd(u"grep -m1 -v -e '^#' -e '^$' $HOME/.config/xfce-superkey/xfce-superkey.conf"_s).output.isEmpty()){
        runCmd("echo "_L1 + cmd + ">> $HOME/.config/xfce-superkey/xfce-superkey.conf"_L1);
    } else { //replace first uncommented line with new command
        runCmd("sed -i '/^[^#]/s;.*;"_L1 + cmd + ";' $HOME/.config/xfce-superkey/xfce-superkey.conf"_L1);
    }
    //restart xfce-superkey
    runCmd(u"pkill xfce-superkey"_s);
    runCmd(u"xfce-superkey-launcher"_s);
    setup();
}
