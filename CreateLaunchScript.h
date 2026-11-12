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

extern void ResolvePlaceholders(
    QString& text,
    const QMap<QString, QString> &map);

extern QString qm(QString str);

extern QString SetMemory(
    bool Xm_,
    QString DefauleXm_
    );

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
