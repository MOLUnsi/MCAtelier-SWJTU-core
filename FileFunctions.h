#ifndef FILEFUNCTIONS_H
#define FILEFUNCTIONS_H

#include <QDirIterator>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStandardPaths>
#include <QProcess>
#include <QCryptographicHash>

#include "log.h"

extern bool IsAllAscii(
    const QString &s);

extern bool FindFileAndSavePaths(
    const QString &RootPath,
    const QString &AimFile,
    const QString &OutputFile);

extern QStringList ReadTextFromFile(
    const QString &AimFile);

extern bool OutputTextIntoFile(
    const QString &Content,
    const QString &AimFile);

extern bool OutputTextIntoFile(
    const QStringList &Content,
    const QString &AimFile);

extern bool CreatPathIfNotExist(
    const QString &Path);

extern bool ClearFile(
    const QString &AimFile
    );

extern QStringList GetSubFolderNames(
    const QString &path);

extern QString AppDataPath();

extern QString DefaultMinecraftPath();

extern bool ExtractZip(
    QString ZipPath,
    QString OutDir);

extern bool CreatePathIfNotExist(
    QString path);

extern bool SearchForJava();

extern QByteArray FileSha1(
    const QString &FilePath);

extern bool VerifyFileSha1(
    const QString &FilePath,
    const QString &Expected);

extern bool FileExist(
    const QString &FilePath);

extern bool DownloadFileFromUrl(
    QString Url,
    QString SavePath);
#endif // FILEFUNCTIONS_H
