#include "SystemFunctions.h"

/**
 * @brief 获取当前设备的{总物理内存大小（单位：MB），可用内存大小（单位：MB）}
 * @param LogFileName 输出的日志文件的路径
 * @return 当前设备的{总物理内存大小,可用内存大小}，返回{0,0}表示获取失败
 */
QPair<qint64,qint64> GetMemoryQt(QString LogFileName)
{
    qint64 UsableMem=0;//单位：MB
    qint64 TotalMem=0;
#ifdef Q_OS_WIN
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);               // 必须先初始化
    if (!GlobalMemoryStatusEx(&ms)) {
        OutputTextIntoFile("设备物理内存大小获取失败\n", LogFileName);
        return {0,0};
    }
    const quint64 MB=1024*1024;
    UsableMem=ms.ullAvailPhys/MB;
    TotalMem=ms.ullTotalPhys/MB;
#endif
    return {TotalMem,UsableMem};
}