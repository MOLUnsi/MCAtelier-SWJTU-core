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

#include"log.h"

/**
 * @brief 获取当前设备的{总物理内存大小（单位：MB），可用内存大小（单位：MB）}
 * @return 当前设备的{总物理内存大小,可用内存大小}，返回{0,0}表示获取失败
 */
QPair<qint64,qint64> GetMemory()
{
    qint64 UsableMem=0;//单位：MB
    qint64 TotalMem=0;
#ifdef Q_OS_WIN
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);               // 必须先初始化
    if (!GlobalMemoryStatusEx(&ms)) {
        Log()<<"设备物理内存大小获取失败\n";
        return {0,0};
    }
    const quint64 MB=1024*1024;
    UsableMem=ms.ullAvailPhys/MB;
    TotalMem=ms.ullTotalPhys/MB;
#endif
    return {TotalMem,UsableMem};
}