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

    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        resultPaths << info.absoluteFilePath(); // 绝对路径
    }

    // 如果没找到任何文件，也生成空文件（可选）
    QFile outFile(OutputFile);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&outFile);
    out.setEncoding(QStringConverter::Utf8); // Qt6 写法

    for (const QString &path : resultPaths) {
        if(IsAllAscii(path))
        {
            out << path << "\n";
        }
    }

    outFile.close();
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

    return dir.entryList();   // 返回 QStringList
}

QString AppDataPath()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    while(dir.dirName()!=QStringLiteral("Roaming")&&dir.cdUp());
    return dir.absolutePath();
}


QString DefaultMinecraftPath()
{
    return AppDataPath()+"/.minecraft";
}


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

bool SearchForJava()
{
    if(!(QFileInfo("JavaPath.txt").exists()))
    {
        OutputTextIntoFile("正在检索所有Java的位置......\n","Log.txt");
        bool flag=FindFileAndSavePaths("C:/","java.exe","JavaPath.txt");
        if(!flag)
        {
            return false;
        }
        OutputTextIntoFile("已经检索所有Java的位置\n","Log.txt");
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
/*
bool DownloadFileFromUrl(QString Url,QString SavePath)
{
    QString cmd="start curl "+Url+" -o "+SavePath;
    qDebug()<<"\t"<<cmd;
    QProcess proc;
    proc.start("curl",{Url,"-o",SavePath});
    if(!proc.waitForStarted(10000))
    {
        proc.kill();
        return false;
    }
    return true;
}
*/
bool DownloadFileFromUrl(QString Url,QString SavePath)
{
    QProcess proc;
    proc.start("curl", {"-sS", "-L", "--fail", Url, "-o", SavePath});

    if (!proc.waitForStarted(10000))
        return false;

    if (!proc.waitForFinished(30 * 60 * 1000)) {   // 大文件给足时间
        proc.kill();
        proc.waitForFinished();
        return false;
    }

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        qWarning() << "curl failed:" << proc.errorString()
        << proc.readAllStandardError();
        return false;
    }
    return true;
}