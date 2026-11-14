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
#include "FileFunctions.h"

const QString LogFileName="log.txt";
const QString CachePath=AppDataPath()+"/MCAtelier";

/**
 * @brief 如果字符串中带有空格，则在它的两端加上双引号；否则不做任何处理
 * @param str 需要处理的字符串
 * @return 处理后的字符串
 */
QString qm(QString str)
{
    if(str.contains(" ")&&!str.contains("\""))
        return "\""+str+"\"";
    return str;
}

Log::Log() {}

Log & Log::operator<<(const QString &Content)
{
    OutputTextIntoFile(Content,LogFileName);
    return *this;
}

bool IsAllAscii(const QString &s)
{
    for (QChar c : s) {
        if (c.unicode() > 0x7F)
            return false;
    }
    return true;
}

/**
 * @brief 在 RootPath 下递归查找名为 AimFile 的文件，
 *        并将所有匹配文件的绝对路径逐行写入 OutputFile
 *
 * @param RootPath   要搜索的根目录
 * @param AimFile    目标文件名（如 "test.txt"）
 * @param OutputFile 输出的文本文件路径
 * @return true      查找并写入成功
 * @return false     发生错误
 */
bool FindFileAndSavePaths(
    const QString &RootPath,
    const QString &AimFile,
    const QString &OutputFile)
{
    // 参数检查
    if (RootPath.isEmpty() || AimFile.isEmpty() || OutputFile.isEmpty())
        return false;

    QDir rootDir(RootPath);
    if (!rootDir.exists())
        return false;

    // 以行为单位保存结果
    QStringList resultPaths;

    // 使用 QDirIterator 递归遍历
    QDirIterator it(
        RootPath,
        QStringList() << AimFile,   // 文件名过滤（支持通配符）
        QDir::Files | QDir::NoSymLinks,
        QDirIterator::Subdirectories
        );

    while (it.hasNext())
    {
        qDebug()<<it.filePath();
        it.next();
        resultPaths << it.fileInfo().absoluteFilePath(); // 绝对路径
    }
    for(QString path:resultPaths)
    {
        if(path.size()==0)
        {
            continue;
        }
        if(!IsAllAscii(path))
        {
            path=QFileInfo(path).absolutePath();//去掉文件名的路径
            AutoMKLink(path,AppDataPath()+"/MCAtelier/JavaPath");
            path+="/"+AimFile;
        }
        OutputTextIntoFile(path+"\n",OutputFile);
    }
    return true;
}

/**
 * @brief 从一个文本文件中读取所有内容，
 *        并分行储存在一个QStringList对象中作为返回值
 *
 * @param AimFile   要读取的文本文件（如"test.txt"）
 * @return          文本文件的内容（分行）
 */
QStringList ReadTextFromFile(const QString &AimFile)
{
    QFile in(AimFile);
    if(!in.exists())
    {
        in.open(QIODevice::WriteOnly);
        in.close();
    }
    QStringList ans;
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringList();
    while (!in.atEnd()) {
        ans.append(in.readLine());  // 每次只读一行
    }
    in.close();
    return ans;
}

/**
 * @brief 向一个文本文件中输出指定内容，
 *        并返回输出是否成功
 * @attention 注意：会在原内容后追加
 *
 * @param Content   要输出的内容（QString格式）
 * @param AimFile   要输出的文本文件（如"test.txt"）
 * @return true     写入成功
 * @return false    写入失败
 */
bool OutputTextIntoFile(
    const QString &Content,
    const QString &AimFile)
{
    QFile outFile(AimFile);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        return false;
    }
    QTextStream out(&outFile);
    out.setEncoding(QStringConverter::Utf8); // Qt6 写法
    out<<Content;
    outFile.close();
    return true;
}

/**
 * @brief 向一个文本文件中输出指定内容，
 *        并返回输出是否成功
 * @attention 注意：会在原内容后追加
 *
 * @param Content   要输出的内容（QStringList格式）
 * @param AimFile   要输出的文本文件（如"test.txt"）
 * @return true     写入成功
 * @return false    写入失败
 */
bool OutputTextIntoFile(
    const QStringList &Content,
    const QString &AimFile)
{
    QFile outFile(AimFile);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        return false;
    }
    QTextStream out(&outFile);
    out.setEncoding(QStringConverter::Utf8); // Qt6 写法
    for(const QString &line:Content)
    {
        out<<line<<"\n";
    }
    outFile.close();
    return true;
}

/**
 * @brief 清除文本文件所有内容
 * @param AimFile   要清除的文本文件（如"test.txt"）
 * @return true     清除成功
 * @return false    清除失败
 */
bool ClearFile(const QString &AimFile)
{
    QFile file(AimFile);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        return false;
    }
    file.close();
    return true;
}

/**
 * @brief 取得指定文件夹下的所有子文件夹的名称，并存储在一个QStringList对象中作为返回值
 * @param path 指定文件夹路径
 * @return 由指定文件夹下的所有子文件夹的名称构成的QStringList对象
 */
QStringList GetSubFolderNames(const QString &path)
{
    QDir dir(path);
    // 只保留目录，不保留 "." 和 ".."
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    dir.sorting();
    return dir.entryList();   // 返回 QStringList
}

#ifdef Q_OS_WIN
/**
 * @brief 获取%AppData%/Roaming文件夹位置
 * @return %AppData%/Roaming文件夹位置
 */
QString AppDataPath()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    while(dir.dirName()!=QStringLiteral("Roaming")&&dir.cdUp());
    return dir.absolutePath();
}

/**
 * @brief 获取官方启动器默认的.minecraft文件夹位置
 * @return 官方启动器默认的.minecraft文件夹位置
 */
QString DefaultMinecraftPath()
{
    return AppDataPath()+"/.minecraft";
}
#endif

/**
 * @brief 把指定压缩包文件解压到目标目录中
 * @param zipPath   压缩包文件位置
 * @param outDir    目标目录位置
 * @return true     解压成功
 * @return false    解压失败
 * @p 使用miniz 自带的 zip 解压函数（高层 API）
 */
bool ExtractZip(QString ZipPath,QString OutDir)
{
    QFile src(ZipPath);
    if (!src.exists()) {
        Log()<<"jar不存在:\t"<<ZipPath<<"\n";
        return false;
    }
    if (!QDir().mkpath(OutDir)) {
        Log()<<"无法创建目录:"<<OutDir;
        return false;
    }

    // tar.exe 是 Windows 自带的，通常在 C:\Windows\System32\tar.exe
    QProcess proc;
    QStringList args;
    args << "-xf" << ZipPath << "-C" << OutDir;   // -x 解压 -f 指定文件 -C 指定目录

    proc.start("tar", args);
    if (!proc.waitForFinished(30000)) {           // 最多等 30 秒
        Log()<<"解压超时:\t"<<ZipPath<<"\n";
        return false;
    }

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        Log()<<"tar解压失败:\t"<<proc.readAllStandardError();
        return false;
    }
    return true;
}

bool CreatePathIfNotExist(QString path)
{
    if (path.isEmpty())
        return false;

    QFileInfo info(path);

    // 已存在但不是目录：说明被文件占了，直接报错，不要强行创建
    if (info.exists() && !info.isDir()) {
        Log()<<"路径已存在但不是目录:\t"+path+"\n";
        return false;
    }

    // 目录已存在：直接返回成功
    if (info.exists())
        return true;

    // mkpath 会逐级创建父目录，比 mkdir 好用
    if (!QDir().mkpath(path)) {
        Log()<<"创建目录失败:\t"+path;
        return false;
    }
    return true;
}

bool DeletePathIfExist(QString path)
{
    bool rt=true;
    for(auto it:GetSubFolderNames(path))
    {
        QStringList args;
        args<<"/c"<<"rmdir"<<path+"/"+it;
        bool flag=ProgramCallback("cmd",args,-1).second.size()==0;
        rt=rt&&flag;
    }
    QStringList args;
    args<<"/c"<<"rmdir"<<path;
    bool flag=(ProgramCallback("cmd",args,-1).second.size()==0);
    return rt&&flag;
}

bool SearchForJava()
{
    if(!(QFileInfo("JavaPath.txt").exists()))
    {
        DeletePathIfExist(CachePath+"/JavaPath");
        CreatePathIfNotExist(CachePath+"/JavaPath");
        ClearFile("JavaPath.txt");
        Log()<<"正在检索所有Java的位置......\n";
        QStringList JavaPathAllowlist;
        JavaPathAllowlist
            <<"C:/Program Files/Java"
            <<"C:/Program Files (x86)/Java"
            <<"C:/Program Files/Eclipse Adoptium"
            <<"C:/Program Files/Microsoft"
            <<"C:/Program Files/Zulu"
            <<"C:/Program Files/BellSoft"
            <<"C:/Program Files/Amazon Corretto"
            <<QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            <<QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
            <<QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            <<QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            <<QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        bool flag=false;
        for(const QString &str:JavaPathAllowlist)
        {
            qDebug()<<str<<":";
            bool found=FindFileAndSavePaths(str,"java.exe","JavaPath.txt");
            flag=flag||found;
        }
        if(!flag)
        {
            return false;
        }
        Log()<<"已经检索所有Java的位置\n";
    }
    return true;
}
/**
 * @brief 获取某文件的sha1值
 * @param filePath  文件地址
 * @return 文件的sha1值。如果失败，返回QByteArry()
 */
QByteArray FileSha1(const QString &FilePath)
{
    QFile file(FilePath);
    if (!file.open(QFile::ReadOnly))
        return QByteArray();    //打不开文件

    QCryptographicHash hash(QCryptographicHash::Sha1);

    // 分块读取，避免大文件一次性占满内存
    constexpr qint64 kChunk = 1024 * 1024; // 1MB
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(kChunk);
        if (chunk.isEmpty())
            return QByteArray();      // 读失败
        hash.addData(chunk);
    }
    return hash.result();
}

//
/**
 * @brief 校验某文件的sha1值是否与预期sha1值相同
 * @param FilePath  文件的地址
 * @param Expected  预期sha1值，不带引号的十六进制字符串，大小写均可
 * @return true     相同
 * @return false    不相同
 */
bool VerifyFileSha1(const QString &FilePath, const QString &Expected)
{
    const QByteArray digest = FileSha1(FilePath);
    if (digest.isEmpty())
        return false;

    const QString actual = QString::fromLatin1(digest.toHex());
    return actual.compare(Expected.trimmed(), Qt::CaseInsensitive) == 0;
}

bool FileExist(const QString &FilePath)
{
    QFile file(FilePath);
    return file.exists();
}

bool DownloadFileFromUrl(const QString &Url, const QString &Path)
{
    if(Url.size()==0)
    {
        return true;
    }
    QFileInfo fileInfo(Path);
    QDir dir = fileInfo.absoluteDir();
    QStringList arguments;
    arguments << "-L" << "--fail" << "-o" << Path << Url;
    QProcess proc;
    proc.start("curl.exe", arguments);
    if(!proc.waitForStarted(-1))
    {
        Log()<<"curl启动失败：\t"<<proc.errorString()<<"\n";
        qDebug()<<"curl启动失败：\t"<<proc.errorString();
        return false;
    }
    Log()<<"正在从"<<Url<<"下载文件到"<<Path<<"\n";
    qDebug()<<"正在从"<<Url<<"下载文件到"<<Path;
    if(!proc.waitForFinished(-1))
    {
        Log()<<"curl未正常结束：\t"<<proc.errorString()<<"\n";
        qDebug()<<"curl未正常结束：\t"<<proc.errorString();
        proc.kill();
        proc.waitForFinished();
        return false;
    }
    if(proc.exitStatus()!=QProcess::NormalExit||proc.exitCode()!=0)
    {
        qDebug()<<"下载失败，退出码="<<QString::number(proc.exitCode())<<"\tstderr="<<proc.readAllStandardError();
        Log()<<"下载失败，退出码="<<QString::number(proc.exitCode())<<"\tstderr="<<QString::fromUtf8(proc.readAllStandardError())<<"\n";
        return false;
    }
    return true;
}

bool DownloadFileFromUrl(const QStringList &UrlList, const QStringList &PathList)
{
    if(UrlList.empty())
    {
        return true;
    }
    QStringList arguments;
    arguments <<"-Z"<< "-L" << "--fail";
    for(int i=0;i<PathList.size();i++)
    {
        arguments<<"-o"<<PathList.at(i)<<UrlList.at(i);
    }
    QProcess proc;
#ifdef Q_OS_WIN
    proc.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NEW_CONSOLE;          // 创建新控制台窗口
        args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES; // 不使用父进程的句柄
    });
#endif
    proc.start("curl.exe", arguments);
    if(!proc.waitForStarted(-1))
    {
        Log()<<"curl启动失败：\t"<<proc.errorString()<<"\n";
        qDebug()<<"curl启动失败：\t"<<proc.errorString();
        return false;
    }
    if(!proc.waitForFinished(-1))
    {
        Log()<<"curl未正常结束：\t"<<proc.errorString()<<"\n";
        qDebug()<<"curl未正常结束：\t"<<proc.errorString();
        proc.kill();
        proc.waitForFinished();
        return false;
    }
    if(proc.exitStatus()!=QProcess::NormalExit||proc.exitCode()!=0)
    {
        qDebug()<<"下载失败，退出码="<<QString::number(proc.exitCode())<<"\tstderr="<<proc.readAllStandardError();
        Log()<<"下载失败，退出码="<<QString::number(proc.exitCode())<<"\tstderr="<<QString::fromUtf8(proc.readAllStandardError())<<"\n";
        return false;
    }
    return true;
}

/**
 * @brief 调用指定程序并给出回调内容，返回形式：{回调输出，错误反馈}
 * @param cmd   调用的程序名称
 * @param args  传给程序的参数
 * @param WaitFor   等待时长（直到结束）（默认值是无限长等待）（单位：毫秒）
 * @return 回调内容，形式：{回调输出，错误反馈}
 */
QPair<QString,QString> ProgramCallback(const QString &program,const QStringList &args,qint64 WaitFor=-1)
{
    QProcess proc;
    proc.start(program,args);
    proc.waitForFinished(WaitFor);
    QString output=proc.readAllStandardOutput();
    QString error =proc.readAllStandardError();
    return QPair<QString,QString>(output,error);

}

QJsonDocument ReadJson(const QString &file)
{
    QFile JsonFile(file);
    if(!JsonFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        Log()<<file<<"打开失败";
        return QJsonDocument();
    }
    QTextStream in(&JsonFile);
    in.setEncoding(QStringConverter::Utf8);
    QString Content=in.readAll();
    JsonFile.close();
    QJsonParseError JsonError;
    QJsonDocument doc=QJsonDocument::fromJson(Content.toUtf8(),&JsonError);
    if (JsonError.error != QJsonParseError::NoError&&!doc.isNull())
    {
        Log()<<file<<"格式错误。错误信息:"+JsonError.errorString()+"\n";
        return QJsonDocument();
    }
    return doc;
}

bool mklink(QString from,QString to)
{
    from.replace("/","\\");
    to.replace("/","\\");
    QStringList args;
    args<<"/c"<<"mklink"<<"/J"<<to<<from;
    QString fail=qPrintable(ProgramCallback("cmd",args,500).second);
    if(fail.size()!=0)
    {
        qDebug()<<fail;
        return false;
    }
    return true;
}
/**
 * @brief 自动建立链接
 * @param path 原路径
 * @param LinkPath 链接存储处
 * @return
 */
bool AutoMKLink(QString &path,QString LinkPath)
{
    QByteArray digest = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha224);
    QString HashPath = QString::fromLatin1(digest.toHex());
    HashPath=HashPath.left(16);//取前16位
    QString NewAbsPath=LinkPath+"/"+HashPath;
    qDebug()<<qPrintable("mklink: from "+path+"\tto "+NewAbsPath);
    bool ret=mklink(path,NewAbsPath);
    path=NewAbsPath;
    return ret;
}