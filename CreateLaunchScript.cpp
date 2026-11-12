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
#include"CreateLaunchScript.h"
#ifdef QT_DEBUG
#define IF_ENDL 1
#else
#define IF_ENDL 0
#endif
#define endl() if(IF_ENDL){OutputTextIntoFile("\n",OutputFileName);}else{OutputTextIntoFile(" ",OutputFileName);}
const QString LauncherName="MCAtelier-SWJTU";
const QString LauncherVersion="dev";

/**
 * @brief 判断某个对象是否符合要求。如果没有rules规则，默认符合；如果有rules规则，按照rules规则判断
 * @param FileObject 对象的名称
 * @return true     符合
 * @return false    不符合
 */
bool IfRulesObeyed(const QJsonObject &FileObject)
{
    if(!FileObject.contains("rules"))
    {
        return true;
    }
    QJsonArray RulesArray=FileObject.value("rules").toArray();
    bool win=false,
        osx=false,
        linux=false;
    for(const QJsonValue &SingleRuleValue:RulesArray)
    {
        QJsonObject SingleRuleObject=SingleRuleValue.toObject();
        bool allowance=(SingleRuleObject["action"]=="allow");
        if(SingleRuleObject.contains("os"))
        {
            QJsonObject OSObject=SingleRuleObject["os"].toObject();
            QString OSName=OSObject["name"].toString();
            if(OSName=="windows")
            {
                win=allowance;
            }
            if(OSName=="osx")
            {
                osx=allowance;
            }
            if(OSName=="linux")
            {
                linux=allowance;
            }
        }
        else
        {
            win=osx=linux=allowance;
        }
    }
#ifdef Q_OS_WIN
    return win;
#endif
#ifdef Q_OS_LINUX
    return linux;
#endif
#ifdef Q_OS_MACOS
    return osx;
#endif
}

bool IfRuleObeyed(const QJsonValue &ArgValue,const QMap<QString,QString> &map)
{
    if(ArgValue.isString())
    {
        return true;
    }
    return false;
    QJsonObject ArgObject=ArgValue.toObject();

}

/**
 * @brief 匹配所有的"${*}"占位符，并通过查表替换掉原字符串
 * @param text 需要被处理的字符串
 * @param map 对照表 * -> map[*]
 */
void ResolvePlaceholders(QString &text, const QMap<QString, QString>& map)
{
    static const QRegularExpression re("\\$\\{([^}]+)\\}");
    if(text.size()==0)
    {
        return;
    }
    for(;;)
    {
        QRegularExpressionMatch m = re.match(text);
        if (!m.hasMatch()) break;

        QString key   = m.captured(1);
        QString value = map.value(key);

        // 值里含空格、或值本身为空，就包引号
        // （空值包引号变成 ""，比裸着更明确）
        if (value.contains(' ') || value.isEmpty())
            value = QString("\"%1\"").arg(value);

        text.replace(m.capturedStart(), m.capturedLength(), value);
    }
}

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
/**
 * @brief 设定分配给Java的内存大小，以生成-Xmx参数
 * @return -Xmx参数
 *
 * 算法描述：
 * 我可以先算术平均几次启动前瞬间的已用内存数（注意是启动前），
 * 作为已用内存经验统计基准值，
 * 与此时实际可访问的物理内存大小作比得到系统正常运行时的内存占用率。
 * 在每次启动时，
 * 如果可用内存不多了，
 * 那就定死用一个设定好的内存上限大小，
 * 保证不会占用太多；
 * 如果可用内存比较宽裕，
 * 那就一直给java送内存直到正常占用率就好了，
 * 大不了再加一个设定好的内存上限大小
 */
QString SetMemory(QString MinecraftPath,QString GameName)
{
    QPair<qint64,qint64> MemInfo=GetMemory();//{总内存,可用内存}
    qint64 TotalMemory=MemInfo.first;
    qint64 AvailableMemory=MemInfo.second;
    qint64 GB=1024;
    qint64 mem;//最终确定的分配内存大小
    mem=qMin(qMin(TotalMemory/2,TotalMemory-4*GB),AvailableMemory-qMax(2*GB,TotalMemory*3/20));
    mem=qMax(mem,2*GB);//下界2GB
    mem=qMin(mem,10*GB);//上界10GB
    return "-Xmx"+QString::number(mem)+"m";
}

/**
 * @brief 生成JVM参数
 * @param RootObject    版本Json文件的根对象
 * @param map           占位符对照表
 * @return JVM参数
 */
QString JVMArgs(QJsonObject RootObject,const QMap<QString,QString> &map)
{
    QStringList Args;
    if(RootObject.contains("arguments"))
    {
        QJsonArray jvm=RootObject.value("arguments").toObject().value("jvm").toArray();
        for(const QJsonValue &it:jvm)
        {
            QString arg;
            if(it.type()==QJsonValue::Object)
            {
                if(!IfRulesObeyed(it.toObject()))
                {
                    continue;
                }
                if(it.toObject().value("value").type()==QJsonValue::String)
                {
                    arg=it.toObject().value("value").toString();
                }
                if(it.toObject().value("value").type()==QJsonValue::Array)
                {
                    QStringList ArgList;
                    for(const QJsonValue &u:it.toObject().value("value").toArray())
                    {
                        ArgList.append(u.toString());
                    }
                    arg=ArgList.join(IF_ENDL?"\n":" ");
                }
            }
            else
            {
                arg=it.toString();
            }
            while(arg.contains("$"))
            {
                ResolvePlaceholders(arg,map);
            }
            arg=qm(arg);
            Args.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+arg);
        }
    }
    else
    {
        Args.append(
            (IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+
            "-XX:HeapDumpPath=MojangTricksIntelDriversForPerformance_javaw.exe_minecraft.exe.heapdump"
        );
        Args.append(
            (IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+
            "-Djava.library.path="+map.value("natives_directory")
        );
        Args.append(
            (IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+
            "-cp"
        );
        Args.append(qm(map.value("classpath")));
    }
    return Args.join(IF_ENDL?"\n":" ");
}

/**
 * @brief 生成-cp参数
 * @param MinecraftPath .minecraft文件夹的路径
 * @param RootObject    版本Json文件的根对象
 * @param map           占位符对照表
 * @return -cp参数
 */

QString ClassPathArgs(QString MinecraftPath,QJsonObject RootObject,QString GameName,const QMap<QString,QString> &map)
{
    QStringList Cp;
    QJsonArray LibrariesArray=RootObject.value("libraries").toArray();
//判断natives文件夹是否正常，即是否需要解压natives文件
    bool NativesFolderNormal=(
            QFileInfo(map.value("natives_directory")).exists()&&
            QFileInfo(map.value("natives_directory")).isDir()&&
            QFileInfo(map.value("natives_directory")+"/lwjgl64.dll").exists()&&
            QFileInfo(map.value("natives_directory")+"/lwjgl64.dll").isFile()&&
            QFileInfo(map.value("natives_directory")+"/openal64.dll").exists()&&
            QFileInfo(map.value("natives_directory")+"/openal64.dll").isFile()&&
            QFileInfo(map.value("natives_directory")+"/jinput-dx8_64.dll").exists()&&
            QFileInfo(map.value("natives_directory")+"/jinput-dx8_64.dll").isFile()
        );
    Log()<<"natives文件夹状态:\t"+(NativesFolderNormal?QStringLiteral("正常"):QStringLiteral("需被补全"))+"\n";
    if(!NativesFolderNormal)
    {
        CreatePathIfNotExist(map["natives_directory"]);
    }
    QStringList UrlsToDld,PathsToDld;
    for(const QJsonValue &FIleValue:LibrariesArray)
    {
        QJsonObject FileObject=FIleValue.toObject();
        QStringList FinalFileName;//带相对路径的文件
        QMap<QString,QString>sha1s;//从相对路径FinalFileName映射到sha1
        QMap<QString,QString>urls;//从相对路径FinalFileName映射到下载网址
        if(FileObject.contains("downloads"))
        {
            if(FileObject.value("downloads").toObject().contains("artifact"))
            {
                QString path=FileObject.value("downloads").toObject()
                                    .value("artifact").toObject()
                                    .value("path").toString();
                QString sha1=FileObject.value("downloads").toObject()
                                   .value("artifact").toObject()
                                   .value("sha1").toString();
                QString url=FileObject.value("downloads").toObject()
                                   .value("artifact").toObject()
                                   .value("url").toString();
                FinalFileName.append(path);
                sha1s[path]=sha1;
                urls[path]=url;
            }
            if( FileObject.value("downloads").toObject().contains("classifiers")&&
                FileObject.value("downloads").toObject().value("classifiers").toObject().contains("natives-windows"))
            {
                QString path=FileObject.value("downloads").toObject()
                                   .value("classifiers").toObject()
                                   .value("natives-windows").toObject()
                                   .value("path").toString();
                QString sha1=FileObject.value("downloads").toObject()
                                   .value("classifiers").toObject()
                                   .value("natives-windows").toObject()
                                   .value("sha1").toString();
                QString url=FileObject.value("downloads").toObject()
                                   .value("classifiers").toObject()
                                   .value("natives-windows").toObject()
                                   .value("url").toString();
                FinalFileName.append(path);
                sha1s[path]=sha1;
                urls[path]=url;
            }
        }
        else
        {         //TODO : 这里手拼的文件名还没有记录sha1和url
            QString FileName=FileObject.value("name").toString();
            QStringList parts=FileName.split(":");
            if(parts.size()<3)
            {
                continue;
            }
            QString group=parts.at(0);
            QString ArtifactId=parts.at(1);
            QString version=parts.at(2);
            QString GroupPath=group.replace(".","/");
            QString comment="";
            QString path=GroupPath+"/"+ArtifactId+"/"+version+"/"+ArtifactId+"-"+version+".jar";
            QString url=FileObject.value("url").toString();
            urls[path]=url+path;
#ifdef QT_DEBUG
            comment="\t<--这个文件名是手动拼接的";
#endif
            FinalFileName.append(path+comment);
        }
        if(IfRulesObeyed(FileObject))
        {
            for(const QString &it:FinalFileName)
            {
                QString WholeFinalFileName=MinecraftPath+QStringLiteral("/libraries/")+it;//带绝对路径的文件
                //开始检验sha1，看文件是否损坏
//                Log()<<"正在检查文件：\nname:\t"+WholeFinalFileName+"\n预期sha1:\t"+sha1s.value(it)+"\n实际sha1:\t"+QString::fromLatin1(FileSha1(WholeFinalFileName).toHex())+"\n";
                if(!FileExist(WholeFinalFileName) || !sha1s.contains(it) || !VerifyFileSha1(WholeFinalFileName,sha1s.value(it)))
                {
                    Log()<<"需要下载文件："<<WholeFinalFileName<<"\t预期sha1:\t"+sha1s.value(it)+"\t实际sha1:\t"+QString::fromLatin1(FileSha1(WholeFinalFileName).toHex())+"\n";
/*                    QProcess dld;
                    qDebug()<<urls.value(it)<<QStringLiteral("\t -o \t")<<WholeFinalFileName;
                    dld.startDetached(QStringLiteral("curl"),{urls.value(it),QStringLiteral("-o"),WholeFinalFileName});
*/
 //                   CreatePathIfNotExist(QFileInfo(WholeFinalFileName).path());
                    std::srand(time(0));
                    UrlsToDld.append(urls.value(it));
                    PathsToDld.append(WholeFinalFileName);
//                    DownloadFileFromUrl(urls.value(it),"C:/Users/31093/Downloads/"+QString::number(std::rand())+".jar");
                }
                //TODO:损坏文件自动下载功能
                Cp.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+WholeFinalFileName);
//                qDebug()<<MinecraftPath+QStringLiteral("/libraries/")+it;
            }
        }
    }
    DownloadFileFromUrl(UrlsToDld,PathsToDld);
#ifndef QT_DEBUG
    for(const QString &FileName:Cp)
    {
        if(!NativesFolderNormal && FileName.contains("natives-windows.jar"))
        {
            Log()<<"需要被解压到natives文件夹的.jar文件为:\t"<<FileName<<"\n";
//            ExtractZip(FileName,map.value("natives_directory"));
        }
    }
#endif
    Cp.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+MinecraftPath+"/versions/"+GameName+"/"+GameName+".jar");
    return Cp.join(map.value("classpath_separator"));
}

/**
 * @brief 生成default-user-jvm参数
 * @param RootObject    版本Json文件的根对象
 * @param map           占位符对照表
 * @return 游戏参数
 */
QString DUJArgs(QJsonObject RootObject,const QMap<QString,QString> &map)
{
    QStringList Args;
    if(RootObject.contains("arguments"))
    {
        QJsonArray game=RootObject.value("arguments").toObject().value("default-user-jvm").toArray();
        for(const QJsonValue &it:game)
        {
            QString arg;
            if(!IfRuleObeyed(it,map))
            {
                continue;
            }
            arg=it.toString();
            while(arg.contains("$"))
            {
                ResolvePlaceholders(arg,map);
            }
            Args.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+arg);
        }
    }
    return Args.join(" ");
}

/**
 * @brief 生成游戏参数
 * @param RootObject    版本Json文件的根对象
 * @param map           占位符对照表
 * @return 游戏参数
 */
QString GameArgs(QJsonObject RootObject,const QMap<QString,QString> &map)
{
    QStringList Args;
    if(RootObject.contains("arguments"))
    {
        QJsonArray game=RootObject.value("arguments").toObject().value("game").toArray();
        for(const QJsonValue &it:game)
        {
            QString arg;
            if(!IfRuleObeyed(it,map))
            {
                continue;
            }
            arg=it.toString();
            while(arg.contains("$"))
            {
                ResolvePlaceholders(arg,map);
            }
            Args.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+arg);
        }
    }
    else
    {
        QString arg=RootObject.value("minecraftArguments").toString();
        while(arg.contains("$"))
        {
            ResolvePlaceholders(arg,map);
        }
        Args.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+arg);
    }
    return Args.join(" ");
}

/**
 * @brief 生成启动脚本
 * @param MinecraftPath     .minecraft文件夹的地址
 * @param GameName          该版本地游戏名称，如“1.20.1-Fabric 0.9.13”
 * @param UserName          用户名
 * @param UsetType          用户类型（正版登录，离线登录，...）
 * @param OutoutFileName    输出的脚本文件的名称
 * @return true             启动成功
 * @return false            启动失败
 *
 * map表包含的占位符：
 * library_directory
 * classpath_separator
 * natives_directory
 * classpath
 * launcher_name
 * launcher_version
 * version_name
 *
 * auth_player_name
 * version_name
 * game_directory
 * assets_root
 * assets_index_name
 * auth_uuid
 * auth_access_token
 * clientid
 * auth_xuid
 * user_type
 * version_type
 *
 * user_properties
 * game_assets
 * auth_session
 *
 * arch
 */
bool Create(
    QString MinecraftPath,
    QString GameName,
    QString UserName,
    QString UserType,
    QString OutputFileName)
{
    ClearFile(OutputFileName);
    Log()<<QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz ddd")+"\t此刻进行了一次启动\n"
        <<"启动的游戏版本名称:\t"+GameName+"\n"
        <<".minecraft文件夹路径:\t"+MinecraftPath+"\n"
        <<"启动的用户名:\t"+UserName+"\n";

    QMap<QString,QString>map;
    map["version_name"]=GameName;
    map["library_directory"]=MinecraftPath+"/libraries";
    QString VersionPath=MinecraftPath+"/versions/"+GameName;
    map["natives_directory"]=VersionPath+"/"+GameName+"-natives";
    map["user_properties"]="{}";
    map["game_assets"]=MinecraftPath+"/assets";
    map["auth_session"]="{}";

#ifdef Q_OS_WIN
    map["classpath_separator"]=";";
#else
    map["classpath_separator"]=":";
#endif

//由此开始解析游戏json文件

    QFile GameJson(VersionPath+"/"+GameName+".json");
    if(!GameJson.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Log()<<VersionPath+"/"+GameName+".json打开失败";
        return false;
    }
    QTextStream in(&GameJson);
    in.setEncoding(QStringConverter::Utf8);
    QString Content=in.readAll();
    GameJson.close();
    QJsonParseError JsonError;
    QJsonDocument doc=QJsonDocument::fromJson(Content.toUtf8(),&JsonError);
    if (JsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        Log()<<"游戏json格式错误:\n\t错误信息:\t"+JsonError.errorString()+"\n";
        return false;
    }
    QJsonObject GameJsonRootObject=doc.object();
    map["classpath"]=ClassPathArgs(MinecraftPath,GameJsonRootObject,GameName,map);

//由此开始找到所有版本的Java

    QStringList JavaPath=ReadTextFromFile("JavaPath.txt");

//由此开始选择合适的java

    QString majorVersion=QString::number(GameJsonRootObject.value("javaVersion").toObject().value("majorVersion").toInt());//目标Java版本,Java8值为8
    QString ChosenJavaPath;
    int MinVersion=INT_MAX;     //大于等于majorVersion的所有Java版本里最小的Java版本（最接近majorVersion）
    for(const QString &it:JavaPath)
    {
        QString JavaNow=it;
        if(*JavaNow.rbegin()=='\n')//去掉结尾的"\n"
        {
            JavaNow.chop(1);
        }
        QProcess pr;
        pr.setProgram(JavaNow); // 不加引号
        pr.setArguments(QStringList() << "-fullversion");
        pr.setProcessChannelMode(QProcess::MergedChannels);   // 关键：标准错误并入标准输出
        pr.start();
        if (!pr.waitForFinished(3000)) {           // 最多等 3 秒
            Log()<<"测试获取各个java版本时启动失败或超时:\n\t所测试的Java:\t"+JavaNow+"\n\t错误信息:\t"+pr.errorString()+"\n";
            continue;
        }
        QString rt=QString::fromLocal8Bit(pr.readAllStandardOutput());//返回字符串
        QString ver = rt.split("\"").at(1);
        QString VersionId;

        if(ver.startsWith("1.8."))
        {
            VersionId="8";
        }
        else
        {
            VersionId = ver.split('.', Qt::SkipEmptyParts).first();   // 用字符重载，无正则问题
        }
        if(VersionId.toInt()>=majorVersion.toInt()&&VersionId.toInt()<MinVersion)//找到符合要求的Java版本
        {
            ChosenJavaPath=JavaNow;
            MinVersion=VersionId.toInt();
        }
    }

//由此开始依据Java位数（32 or 64）推测物理内存大小

    QProcess pr;
    pr.setProgram(ChosenJavaPath); // 不加引号
    pr.setArguments(QStringList() << "-fullversion");
    pr.setProcessChannelMode(QProcess::MergedChannels);   // 关键：标准错误并入标准输出
    pr.start();
    if (!pr.waitForFinished(3000)) {           // 最多等 3 秒
        Log()<<"测试java位数(32 or 64)时启动失败或超时:\n\t所测试的Java:\t"+ChosenJavaPath+"\n\t错误信息:\t"+pr.errorString()+"\n";
        return false;
    }
    QString rt=QString::fromLocal8Bit(pr.readAllStandardOutput());//返回字符串
    if(rt.contains("64-Bit") || rt.contains("64-bit"))//64位系统
    {
        map["arch"]="64";
    }
    else//32位系统
    {
        map["arch"]="32";
    }
//由此开始输出脚本内容

    map["launcher_name"]=LauncherName;
    map["launcher_version"]=LauncherVersion;
    OutputTextIntoFile("chcp 936>nul\n",OutputFileName);

    OutputTextIntoFile("cd /D \""+VersionPath+"\"\n\n",OutputFileName);

    OutputTextIntoFile("\""+ChosenJavaPath+"\"",OutputFileName);//输出Java路径
    endl();

    OutputTextIntoFile(JVMArgs(GameJsonRootObject,map),OutputFileName);
    endl();

    map["version_name"]=GameName;
    map["auth_player_name"]=UserName;
    map["game_directory"]=VersionPath;
    map["assets_root"]=MinecraftPath+"/assets";
    map["assets_index_name"]=GameJsonRootObject.value("assetIndex").toObject().value("id").toString();
    QUuid uuid=QUuid::createUuidV3(
        QUuid(),                                    // 命名空间：用空 UUID
        ("OfflinePlayer:" + UserName).toUtf8()      // 命名空间 + 名字
        );
    map["auth_uuid"]=uuid.toString(QUuid::WithoutBraces);
    map["auth_access_token"]=uuid.toString(QUuid::WithoutBraces);
    map["clientid"]="nop";
    map["auth_xuid"]="nop";
    map["user_type"]=UserType;
    map["version_type"]=LauncherName;

    OutputTextIntoFile(GameJsonRootObject.value("mainClass").toString(),OutputFileName);
    endl();

    OutputTextIntoFile(GameArgs(GameJsonRootObject,map),OutputFileName);
    endl();

    return true;
}
