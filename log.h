#ifndef LOG_H
#define LOG_H

#include <QString>

#include "FileFunctions.h"

const QString LogFileName="log.txt";

class Log
{
public:
    Log();
    Log & operator << (const QString &Content);
};

#endif // LOG_H
