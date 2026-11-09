#include"CreateLaunchScript.h"
#ifdef QT_DEBUG
#define IF_ENDL 1
#else
#define IF_ENDL 0
#endif
#define endl() if(IF_ENDL){OutputTextIntoFile("\n",OutputFileName);}else{OutputTextIntoFile(" ",OutputFileName);}
const QString LauncherName="?Launcher";
const QString LauncherVersion="0.0.1";

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
    while (true) {
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
 * @param LogFileName 输出的日志文件名称
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
QString SetMemory(QString MinecraftPath,QString GameName,QString LogFileName)
{
    QPair<qint64,qint64> mem=GetMemoryQt(LogFileName);//{总内存,可用内存}
    qint64 TotalMemory=mem.first;
    qint64 AvailableMemoryNow=mem.second;
    double RateNow=(double(TotalMemory-AvailableMemoryNow))/(double(TotalMemory));//此刻的内存占用率
    if(mem.first==0)//读取内存信息失败
    {
        return QString();
    }
    QStringList MemoryHistory=ReadTextFromFile("MemoryHistory.txt");
    if(MemoryHistory.size()==0)
    {
        OutputTextIntoFile(
            QStringLiteral("1")+"\n"+QString::number(RateNow),
            "MemoryHistory.txt"
        );
        MemoryHistory=ReadTextFromFile("MemoryHistory.txt");
    }
    qint64 n=MemoryHistory.at(0).toLongLong();//已启动次数，初始空串返回0
    double avg=MemoryHistory.at(1).toDouble();//已统计的平均内存占用率，初始空串返回0.0
    qint64 GiveMemory=0;
    avg=(avg*double(n)+RateNow)/(n+1);

//由此开始补齐至基准值

    if(RateNow<avg)//如果还不到基准值，就用GiveMemory先补到基准值
    {
        GiveMemory+=qint64( (double(TotalMemory))*(avg-RateNow) );
        qDebug()<<"没达到基准值\nGiveMemory先加"<<GiveMemory;
    }
    else
    {
        qDebug()<<"达到基准值\n";
    }

//由此开始依据模组多少给内存。映射函数：(1+5x) / (4+4x)，把0~+∞映射到0.25~0.5上

    QDir Mods(MinecraftPath+"/versions/"+GameName+"/mods/");
    double rate=0;
    if(Mods.exists())
    {
        // NoDotAndDotDot 排除 "." 和 ".."，是得到真实数量的关键
        QFileInfoList list = Mods.entryInfoList(
            QDir::AllEntries | QDir::NoDotAndDotDot);
        int cnt=list.count();
        rate=(1.0+double(cnt))/(4.0+4.0*double(cnt));
    }
 //   GiveMemory+=

    n++;
    ClearFile("MemoryHistory.txt");
    OutputTextIntoFile(
        QString::number(n)+"\n"+
        QString::number(avg)+
        "\nAvailableMemoryNow="+
        QString::number(AvailableMemoryNow)+
        "\nTotalMemory="+
        QString::number(TotalMemory)+
        "\nrate="+
        QString::number(
            (double(AvailableMemoryNow))/
            (double(TotalMemory))
        ),
        "MemoryHistory.txt");
    qDebug()<<"GiveMemory="<<GiveMemory;
//    return "-Xmx"+QString::number(TotalMemory/4/128*128)+"m";
    return QString::number(TotalMemory/4/124*128);
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
                arg=it.toObject().value("value").toString();
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
    return Args.join(" ");
}

/**
 * @brief 生成-cp参数
 * @param MinecraftPath .minecraft文件夹的路径
 * @param RootObject    版本Json文件的根对象
 * @param map           占位符对照表
 * @param LogFileName   日志文件名称
 * @return -cp参数
 */

QString ClassPathArgs(QString MinecraftPath,QJsonObject RootObject,QString GameName,const QMap<QString,QString> &map,QString LogFileName)
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
    OutputTextIntoFile("natives文件夹状态:\t"+(NativesFolderNormal?QStringLiteral("正常"):QStringLiteral("需被补全"))+"\n",LogFileName);
    if(!NativesFolderNormal)
    {
        CreatePathIfNotExist(map["natives_directory"],LogFileName);
    }
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
/*
                OutputTextIntoFile("正在检查文件：\nname:\t"+WholeFinalFileName+"\n预期sha1:\t"+sha1s.value(it)+"\n实际sha1:\t"+QString::fromLatin1(FileSha1(WholeFinalFileName).toHex())+"\n",LogFileName);
                if(!FileExist(WholeFinalFileName) || !sha1s.contains(it) || !VerifyFileSha1(WholeFinalFileName,sha1s.value(it)))
                {
                    OutputTextIntoFile("正在下载文件：\t"+WholeFinalFileName+"\n",LogFileName);
                    QProcess dld;
                    qDebug()<<urls.value(it)<<QStringLiteral("\t -o \t")<<WholeFinalFileName;
                    dld.startDetached(QStringLiteral("curl"),{urls.value(it),QStringLiteral("-o"),WholeFinalFileName});
                }
*/
                Cp.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+WholeFinalFileName);
                qDebug()<<MinecraftPath+QStringLiteral("/libraries/")+it;
            }
        }
    }
#ifndef QT_DEBUG
    for(const QString &FileName:Cp)
    {
        if(!NativesFolderNormal && FileName.contains("natives-windows.jar"))
        {
            OutputTextIntoFile("需要被解压到natives文件夹的.jar文件为:\t"+FileName+"\n",LogFileName);
//            ExtractZip(FileName,map.value("natives_directory"),LogFileName);
        }
    }
#endif
    Cp.append((IF_ENDL?QStringLiteral("\n"):QStringLiteral(""))+MinecraftPath+"/versions/"+GameName+"/"+GameName+".jar");
    return Cp.join(map.value("classpath_separator"));
}


/**
 * @brief 生成-cp参数
 * @param MinecraftPath .minecraft文件夹的路径
 * @param RootObject    版本Json文件的根对象
 * @param map           占位符对照表
 * @param LogFileName   日志文件名称
 * @return -cp参数
 */
/*
QString ClassPathArgs(QString MinecraftPath, QJsonObject RootObject, QString GameName,
                      const QMap<QString, QString> &map, QString LogFileName)
{
    const QString LibraryDir = MinecraftPath + QStringLiteral("/libraries/");
    const QString Sep = (IF_ENDL ? QStringLiteral("\n") : QStringLiteral(""));

    // ---------- 1. 收集所有需要下载的文件 ----------
    // key: 相对路径(libraries/ 之后的部分)
    struct LibFile {
        QString relPath;   // 相对路径，可能带 natives 标记前缀
        QString realPath;  // 实际落盘的相对路径（去掉标记）
        QString sha1;
        QString url;
        bool    isNative;  // 是否是 natives jar
    };
    QVector<LibFile> libFiles;

    // natives 是否由游戏自己解压（1.19-pre1 起 json 里不再有 natives 字段）
    bool gameExtractsNatives = false;

    for (const QJsonValue &FileValue : RootObject.value("libraries").toArray())
    {
        const QJsonObject FileObject = FileValue.toObject();

        // 规则不通过（如只给其他 OS 用的库 / has_failed 依赖）整条跳过
        if (!IfRulesObeyed(FileObject))
            continue;

        const QJsonObject Downloads = FileObject.value("downloads").toObject();

        // --- 普通 artifact ---
        if (Downloads.contains("artifact"))
        {
            const QJsonObject Artifact = Downloads.value("artifact").toObject();
            const QString path = Artifact.value("path").toString();
            if (!path.isEmpty())
                libFiles.append({path, path, Artifact.value("sha1").toString(),
                                 Artifact.value("url").toString(), false});
        }
        // --- natives classifier（按当前系统选择，这里以 Windows 为例）---
        const QStringList NativeKeys{QStringLiteral("natives-windows"),
                                     QStringLiteral("natives-windows-x86"),
                                     QStringLiteral("natives-windows-arm64")};
        const QJsonObject Classifiers = Downloads.value("classifiers").toObject();
        for (const QString &key : NativeKeys)
        {
            if (!Classifiers.contains(key))
                continue;
            const QJsonObject Native = Classifiers.value(key).toObject();
            const QString path = Native.value("path").toString();
            if (!path.isEmpty())
            {
                // 用 "\1" 做标记：既要加进 classpath，又要解压
                libFiles.append({"\1" + path, path, Native.value("sha1").toString(),
                                 Native.value("url").toString(), true});
                gameExtractsNatives = true; // 有 classifier => 老版本，启动器负责解压
            }
            break; // 只取第一个命中的
        }

        // --- 没有 downloads 字段的老式 json：手拼路径，无法校验/下载 ---
        if (!Downloads.contains("artifact") && !Downloads.contains("classifiers"))
        {
            const QStringList parts = FileObject.value("name").toString().split(':');
            if (parts.size() < 3)
                continue;
            const QString GroupPath = QString(parts.at(0)).replace(QStringLiteral("."), QStringLiteral("/"));
            const QString rel = GroupPath + '/' + parts.at(1) + '/' + parts.at(2)
                                + '/' + parts.at(1) + '-' + parts.at(2) + ".jar";
            OutputTextIntoFile("手动拼接路径（无 sha1/url，跳过校验）：\t" + rel + "\n", LogFileName);
            libFiles.append({rel, rel, QString(), QString(), false});
        }
    }
    const bool NeedExtract = !gameExtractsNatives; // 仅老版本需要启动器解压

    // ---------- 2. 校验 + 并发下载，全部完成后再继续 ----------
    QVector<LibFile> toDownload;
    for (const LibFile &f : libFiles)
    {
        const QString abs = LibraryDir + f.realPath;
        if (!VerifyFileSha1(abs, f.sha1))          // sha1 为空时也应返回 false
        {
            if (f.url.isEmpty())
            {
                OutputTextIntoFile("缺少下载地址，无法下载：\t" + abs + "\n", LogFileName);
                continue;
            }
            toDownload.append(f);
        }
    }

    if (!toDownload.isEmpty())
    {
        QEventLoop loop;                       // 局部事件循环：把异步下载"等"成同步
        int remaining = toDownload.size();
        int failed    = 0;
        auto onOne = [&](bool ok) {
            --remaining;
            if (!ok) ++failed;
            if (remaining == 0) loop.quit();
        };

        for (const LibFile &f : toDownload)
        {
            const QString abs = LibraryDir + f.realPath;
            OutputTextIntoFile("正在下载文件：\t" + abs + "\n", LogFileName);

            QFuture<bool> future = QtConcurrent::run([f, abs] {
                return DownloadFileFormUrl(f.url, abs) && VerifyFileSha1(abs, f.sha1);
            });
            auto *w = new QFutureWatcher<bool>(&loop);   // loop 作为父对象，出作用域自动析构
            connect(w, &QFutureWatcher<bool>::finished, w, [w, onOne] {
                const bool ok = w->result();
                w->deleteLater();
                onOne(ok);
            });
            w->setFuture(future);
        }
        loop.exec();   // 阻塞在这里，直到所有下载结束；期间事件循环照常运行，下载正常进行

        OutputTextIntoFile(QString("库文件下载完成：成功 %1，失败 %2\n")
                               .arg(toDownload.size() - failed).arg(failed), LogFileName);
    }

    // ---------- 3. 组装 classpath，同时收集需要解压的 natives ----------
    QStringList Cp;
    QStringList nativesJars;
    for (const LibFile &f : libFiles)
    {
        const QString abs = LibraryDir + f.realPath;
        if (!QFileInfo(abs).isFile())       // 下载失败 / 缺 url 的，跳过而不是塞个不存在的路径
        {
            OutputTextIntoFile("文件缺失，已跳过：\t" + abs + "\n", LogFileName);
            continue;
        }
        Cp.append(Sep + abs);
        if (f.isNative && NeedExtract)
            nativesJars.append(abs);
    }
    Cp.append(Sep + MinecraftPath + "/versions/" + GameName + "/" + GameName + ".jar");

    // ---------- 4. 解压 natives（仅 1.19 之前的版本）----------
    const QString nativesDir = map.value("natives_directory");
    auto hasDll = [&](const QString &name) {
        return QFileInfo(nativesDir + '/' + name).isFile();
    };
    const bool NativesFolderNormal = QFileInfo(nativesDir).isDir()
                                     && (hasDll("lwjgl.dll") || hasDll("lwjgl64.dll"))
                                     && (hasDll("openal.dll") || hasDll("openal64.dll"))
                                     && (hasDll("jinput-dx8.dll") || hasDll("jinput-dx8_64.dll"));

    OutputTextIntoFile("natives文件夹状态:\t" + (NativesFolderNormal ? QStringLiteral("正常")
                                                                     : QStringLiteral("需被补全")) + "\n",
                       LogFileName);
    if (!NativesFolderNormal)
    {
        CreatePathIfNotExist(nativesDir, LogFileName);
        for (const QString &jar : nativesJars)
        {
            OutputTextIntoFile("需要被解压到natives文件夹的.jar文件为:\t" + jar + "\n", LogFileName);
            ExtractZip(jar, nativesDir, LogFileName);
        }
    }

    return Cp.join(map.value("classpath_separator"));
}
*/

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
//            arg=qm(arg);
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
 * @param OutoutFileName    输出的脚本文件的名称
 * @param LogFileName       输出的日志文件的名称
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
    QString OutputFileName,
    QString LogFileName)
{
    ClearFile(OutputFileName);
    OutputTextIntoFile(
        QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz ddd")+"  此刻进行了一次启动\n"
        +"启动的游戏版本名称:\t"+GameName+"\n"
        +".minecraft文件夹路径:\t"+MinecraftPath+"\n"
        +"启动的用户名:\t"+UserName+"\n",
        LogFileName);

    QMap<QString,QString>map;
    map["version_name"]=GameName;
    map["library_directory"]=MinecraftPath+"/libraries";
    QString VersionsPath=MinecraftPath+"/versions/"+GameName;
    map["natives_directory"]=VersionsPath+"/"+GameName+"-natives";
    map["user_properties"]="{}";
    map["game_assets"]=MinecraftPath+"/assets";
    map["auth_session"]="{}";

#ifdef Q_OS_WIN
    map["classpath_separator"]=";";
#else
    map["classpath_separator"]=":";
#endif

//由此开始解析游戏json文件

    QFile GameJson(VersionsPath+"/"+GameName+".json");
    if(!GameJson.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        OutputTextIntoFile(VersionsPath+"/"+GameName+".json打开失败",LogFileName);
        return false;
    }
    QTextStream in(&GameJson);
    in.setEncoding(QStringConverter::Utf8);
    QString Content=in.readAll();
    GameJson.close();
    QJsonParseError JsonError;
    QJsonDocument doc=QJsonDocument::fromJson(Content.toUtf8(),&JsonError);
    if (JsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        OutputTextIntoFile("游戏json格式错误:\n\t错误信息:\t"+JsonError.errorString()+"\n",LogFileName);
        return false;
    }
    QJsonObject GameJsonRootObject=doc.object();
    map["classpath"]=ClassPathArgs(MinecraftPath,GameJsonRootObject,GameName,map,LogFileName);

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
            OutputTextIntoFile("测试获取各个java版本时启动失败或超时:\n\t所测试的Java:\t"+JavaNow+"\n\t错误信息:\t"+pr.errorString()+"\n",LogFileName);
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
        OutputTextIntoFile("测试java位数(32 or 64)时启动失败或超时:\n\t所测试的Java:\t"+ChosenJavaPath+"\n\t错误信息:\t"+pr.errorString()+"\n",LogFileName);
        return false;
    }
    QString rt=QString::fromLocal8Bit(pr.readAllStandardOutput());//返回字符串
    QString Xmx;
    if(rt.contains("64-Bit") || rt.contains("64-bit"))//64位系统
    {
        map["arch"]="64";
        Xmx="";
    }
    else//32位系统
    {
        map["arch"]="32";
        Xmx="";
    }
//由此开始输出脚本内容

    map["launcher_name"]=LauncherName;
    map["launcher_version"]=LauncherVersion;
    OutputTextIntoFile("chcp 936>nul\n",OutputFileName);

    OutputTextIntoFile("cd /D \""+VersionsPath+"\"\n\n",OutputFileName);

    OutputTextIntoFile("\""+ChosenJavaPath+"\"",OutputFileName);//输出Java路径
    endl();

//    OutputTextIntoFile(SetMemory(LogFileName),OutputFileName);
//    endl();

    OutputTextIntoFile(JVMArgs(GameJsonRootObject,map),OutputFileName);
    endl();


/*
    OutputTextIntoFile("--add-opens java.base/java.lang.invoke=ALL-UNNAMED",OutputFileName);
    endl();

    OutputTextIntoFile("-XX:HeapDumpPath=MojangTricksIntelDriversForPerformance_javaw.exe_minecraft.exe.heapdump",OutputFileName);
    endl();

    OutputTextIntoFile("\"-Djava.library.path="+map.value("natives_directory")+"\"",OutputFileName);
    endl();

    OutputTextIntoFile("\"-Djna.tmpdir="+map.value("natives_directory")+"\"",OutputFileName);
    endl();

    OutputTextIntoFile("\"-Dorg.lwjgl.system.SharedLibraryExtractPath="+map.value("natives_directory")+"\"",OutputFileName);
    endl();

    OutputTextIntoFile("\"-Dio.netty.native.workdir="+map.value("natives_directory")+"\"",OutputFileName);
    endl();

    OutputTextIntoFile("-Dlog4j2.formatMsgNoLookups=true",OutputFileName);
    endl();

    OutputTextIntoFile("-Dminecraft.launcher.brand="+LauncherName,OutputFileName);
    endl();

    OutputTextIntoFile("-Dminecraft.launcher.version="+LauncherVersion,OutputFileName);
    endl();

    OutputTextIntoFile("-cp \""+map.value("classpath")+"\"",OutputFileName);
    endl();
*/
    map["version_name"]=GameName;
    map["auth_player_name"]=UserName;
    map["game_directory"]=VersionsPath;
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
    map["user_type"]="offline";
    map["version_type"]=LauncherName;

    OutputTextIntoFile(GameJsonRootObject.value("mainClass").toString(),OutputFileName);
    endl();

    OutputTextIntoFile(GameArgs(GameJsonRootObject,map),OutputFileName);
    endl();

/*
    OutputTextIntoFile("--username "+map.value("auth_player_name"),OutputFileName);
    endl();

    OutputTextIntoFile("--version \""+GameName+"\"",OutputFileName);
    endl();

    OutputTextIntoFile("--gamedir \""+VersionsPath+"\"",OutputFileName);
    endl();

    OutputTextIntoFile("--assetsDir \""+MinecraftPath+"/assets\"",OutputFileName);
    endl();

    OutputTextIntoFile("--assetIndex "+GameJsonRootObject.value("assetIndex").toObject().value("id").toString(),OutputFileName);
    endl();

    OutputTextIntoFile("--uuid "+map.value("auth_uuid"),OutputFileName);
    endl();

    OutputTextIntoFile("--accessToken "+uuid.toString(QUuid::WithoutBraces),OutputFileName);
    endl();

    OutputTextIntoFile("--clientId ${clientid}",OutputFileName);
    endl();

    OutputTextIntoFile("--xuid ${auth_xuid}",OutputFileName);
    endl();

    OutputTextIntoFile("--userType offline",OutputFileName);
    endl();

    OutputTextIntoFile("--versionType "+LauncherName,OutputFileName);
*/
 //   OutputTextIntoFile("\n\nexit",OutputFileName);
    return true;
}
