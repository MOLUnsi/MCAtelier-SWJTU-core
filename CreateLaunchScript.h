#ifndef CREATELAUNCHSCRIPT_H
#define CREATELAUNCHSCRIPT_H


#include <QDebug>
#include <QUuid>
#include <QProcess>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QEventLoop>

#include "FileFunctions.h"
#include "SystemFunctions.h"

extern bool IfRulesObeyed(const QJsonObject &FileObject);

extern bool IfRuleObeyed(
    const QJsonValue &ArgValue,
    const QMap<QString,QString> &map);

extern void ResolvePlaceholders(
    QString& text,
    const QMap<QString, QString> &map);

extern QString qm(QString str);

extern QString SetMemory(
    QString MinecraftPath,
    QString GameName);

extern QString JVMArgs(
    QJsonObject RootObject,
    const QMap<QString,QString> &map);

extern QString ClassPathArgs(
    QString MinecraftPath,
    QJsonObject RootObject,
    QString GameName,
    const QMap<QString,QString> &map);

extern QString GameArgs(
    QJsonObject RootObject,
    const QMap<QString,QString> &map);

extern bool Create(
    QString MinecraftPath,
    QString GameName,
    QString UserName,
    QString UserType,
    QString OutputFileName);

#endif // CREATELAUNCHSCRIPT_H
