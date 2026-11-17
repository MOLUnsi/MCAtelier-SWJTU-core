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
#include "widget.h"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle(LauncherName);
    ui->lineEdit->clear();
    ui->lineEdit->setText("Steve");
    ClearFile(LogFileName);
    QString MinecraftPath;

    QFileInfo file("Current.MinecraftPath.txt");
    if(!file.exists() || !file.isFile() || !file.size())
    {
        MinecraftPath=DefaultMinecraftPath();
    }
    else
    {
        MinecraftPath=ReadTextFromFile("Current.MinecraftPath.txt").first();
    }
    ui->MinecraftPathLineEdit->setText(MinecraftPath);
    ui->comboBox->addItems(GetSubFolderNames(MinecraftPath+"/versions"));

    CreatePathIfNotExist(CachePath+"/JavaPath");
    ui->Condition->setText("正在检索所有Java的位置，请稍等......");
    QFuture<bool> future = QtConcurrent::run([this] { return SearchForJava(); });
    auto *w = new QFutureWatcher<bool>(this);
    connect(w, &QFutureWatcher<bool>::finished, this, [w, this] {
        ui->Condition->setText("已经检索所有Java的位置");
        w->deleteLater();
    });
    w->setFuture(future);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    level++;
    Log()<<"开始启动Minecraft";
    QString MinecraftPath=ui->MinecraftPathLineEdit->text();
    while(MinecraftPath.endsWith("/") || MinecraftPath.endsWith("\\"))
    {
        MinecraftPath.chop(1);
    }
#ifdef QT_DEBUG
    return;
#endif
    ui->Condition->setText("正在生成Minecraft启动脚本......");
    Create(MinecraftPath,ui->comboBox->currentText(),ui->lineEdit->text(),"offline",Script);
    ui->Condition->setText("正在启动Minecraft......");
    if(m_proc)
    {
        m_proc->deleteLater();
        m_proc=nullptr;
    }
    m_proc=new QProcess(this);
#ifdef Q_OS_WIN
    m_proc->setProgram("cmd");
    m_proc->setArguments(QStringList()<<"/c"<<Script);
#else
    m_proc->setProgram("sh");
    m_proc->setArguments(QStringList()<<Script);
#endif
    m_proc->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_proc,&QProcess::started,this,[this]()
    {
        ui->Condition->setText("Minecraft启动成功");
        Log()<<"Minecraft启动成功";
    });
    connect(m_proc,&QProcess::readyReadStandardError,this,[this]()
    {
        QString Error=m_proc->readAllStandardError();
        Error.remove("\n");
        Error.remove("\r");
        Log()<<"产生报错："<<Error;
    });
    connect(m_proc,&QProcess::finished,this,[this](qint64 ExitCode,QProcess::ExitStatus ExitStatus)
    {
        if(ExitStatus==QProcess::NormalExit)
        {
            ui->Condition->setText("Minecraft已退出");
            Log()<<"Minecraft已退出。exit code="+QString::number(ExitCode);
        }
        else
        {
            ui->Condition->setText("游戏退出");
            Log()<<"Minecraft已退出。exit code="+QString::number(ExitCode);
        }
        m_proc->deleteLater();
        m_proc=nullptr;
    });
    m_proc->start();
    Log()<<"└启动操作已结束"<<"-------------------------------------------------------------------";
    level--;
}


void Widget::on_MinecraftPathLineEdit_editingFinished()
{
    QString MinecraftPath=ui->MinecraftPathLineEdit->text();
    if(MinecraftPath.endsWith("/") || MinecraftPath.endsWith("\\"))
    {
        MinecraftPath.chop(1);
    }
    ui->comboBox->clear();
    ui->comboBox->addItems(GetSubFolderNames(MinecraftPath+"/versions"));
    ClearFile("Current.MinecraftPath.txt");
    OutputTextIntoFile(MinecraftPath,"Current.MinecraftPath.txt");
}


void Widget::on_ChooseMinecraftPathFolder_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,                           // 父窗口，对话框会居中在它上面
        tr("选择.minecraft文件夹"),      // 对话框标题
        QDir::homePath(),              // 默认打开路径（用户主目录）
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    if (!folderPath.isEmpty()) {       // 关键：点取消返回空字符串
        ui->MinecraftPathLineEdit->setText(folderPath);  // 显示到输入框
        // 后续可用 QDir(folderPath) 做遍历、读写
    }

    QString MinecraftPath=ui->MinecraftPathLineEdit->text();
    ui->comboBox->clear();
    ui->comboBox->addItems(GetSubFolderNames(MinecraftPath+"/versions"));
    ClearFile("Current.MinecraftPath.txt");
    OutputTextIntoFile(MinecraftPath,"Current.MinecraftPath.txt");
}


void Widget::on_comboBox_activated(int index)
{

}