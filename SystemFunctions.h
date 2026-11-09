#ifndef SYSTEMFUNCTIONS_H
#define SYSTEMFUNCTIONS_H

#include <QProcess>
#include <QDebug>

#ifdef Q_OS_WIN

#include <windows.h>

#endif

#include "FileFunctions.h"

extern QPair<qint64,qint64> GetMemoryQt(QString LogFileName);



#endif // SYSTEMFUNCTIONS_H
