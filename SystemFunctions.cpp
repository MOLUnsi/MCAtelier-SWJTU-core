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
#include "SystemFunctions.h"

#include "FileFunctions.h"



/**
 * @brief 获取当前设备的{总物理内存大小（单位：MB），可用内存大小（单位：MB）}
 * @return 当前设备的{总物理内存大小,可用内存大小}，返回{0,0}表示获取失败
 */
QPair<qint64,qint64> GetMemory()
{
    level++;
    Log()<<"正在获取设备物理内存信息";
    qint64 UsableMem=0;//单位：MB
    qint64 TotalMem=0;
#ifdef Q_OS_WIN
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);               // 必须先初始化
    if (!GlobalMemoryStatusEx(&ms)) {
        Log()<<"└获取失败";
        return {0,0};
    }
    const quint64 MB=1024*1024;
    UsableMem=ms.ullAvailPhys/MB;
    TotalMem=ms.ullTotalPhys/MB;
#endif
#ifdef Q_OS_LINUX
    const qint64 MB=1024;
    QFile file("/proc/meminfo");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Log()<<"└获取失败";
        return {0,0};
    }
    QTextStream in(&file);
    QStringList Text=in.readAll().split("\n");
    for(const QString &line:Text)
    {
        if(line.size()==0)
        {
            continue;
        }
        qint64 mem=QStringList(line.split(" ",Qt::SkipEmptyParts)).at(1).toLongLong();
        if(line.startsWith("MemTotal:"))
        {
            qDebug()<<qPrintable("Tot:"+line);
            TotalMem=mem/MB;
        }
        if(line.startsWith("MemAvailable:"))
        {
            qDebug()<<qPrintable("Ava:"+line);
            UsableMem=mem/MB;
        }
    }
    file.close();
#endif
    Log()<<"└获取成功";
    level--;
    return {TotalMem,UsableMem};
}

/**
 * @brief 获取当前操作系统的版本
 * @return 当前操作系统的版本
 */
QString GetOSVersion()
{
    QString Version;

#ifdef Q_OS_WIN
    QStringList args;
    args<<"/c"<<"ver";
    Version=ProgramCallback("cmd",args,-1).first;
    static const QRegularExpression re(R"(Version ([\d.]+))");
    auto m=re.match(Version);
    if(m.hasMatch())
    {
        Version=m.captured(1);
    }
#endif

#ifdef Q_OS_LINUX

#endif

#ifdef Q_OS_MACOS

#endif

    return Version;
}