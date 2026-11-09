#include "log.h"

Log::Log() {}

Log & Log::operator<<(const QString &Content)
{
    OutputTextIntoFile(Content,LogFileName);
    return *this;
}