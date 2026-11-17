/*
 * MCAtelier-SWJTU-core - 西南交通大学Minecraft启动器的启动核心
 * Copyright (C) 2026 陌栀子（MOLUnsi）
 * GitHub：https://github.com/MOLUnsi/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
#include <QJsonDocument>
#include <QCoreApplication>

#ifdef Q_OS_WIN

#include <Windows.h>

#endif

extern QString qm(QString str);

extern const QString LogFileName;
extern const QString CachePath;
extern const QString Script;
extern qint64 level;

class Log
{
public:
    Log();
    Log & operator << (const QString &Content);
};

extern bool IsAscii(
    const QChar &c);

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

extern QString UserDir();

extern QString CacheDir();

extern QString DefaultMinecraftPath();

extern bool ExtractZip(
    QString ZipPath,
    QString OutDir);

extern bool CreatePathIfNotExist(
    QString path);

extern bool DeletePathIfExist(
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
    const QString &Url,
    const QString &SavePath);

extern bool DownloadFileFromUrl(
    QStringList UrlList,
    QStringList SavePathList);

extern QPair<QString,QString> ProgramCallback(
    const QString &program,
    const QStringList &args,
    qint64 WaitFor);

extern QJsonDocument ReadJson(
    const QString &file);

extern bool mklink(
    QString from,
    QString to);

extern bool AutoMKLink(
    QString &path,
    QString LinkPath);

#endif // FILEFUNCTIONS_H
