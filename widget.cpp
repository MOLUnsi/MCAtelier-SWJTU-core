#include "widget.h"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->lineEdit->clear();
    ui->lineEdit->setText("Steve");
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
    QString MinecraftPath=ui->MinecraftPathLineEdit->text();
    if(MinecraftPath.endsWith("/") || MinecraftPath.endsWith("\\"))
    {
        MinecraftPath.chop(1);
    }
    Create(MinecraftPath,ui->comboBox->currentText(),ui->lineEdit->text(),"offline","Script.bat");
#ifdef QT_DEBUG
    return;
#endif
//    system("Script.bat");
    QProcess proc;
    proc.startDetached(QStringLiteral("cmd.exe"),{QStringLiteral("/c"),"start","Script.bat"});
/*    bool started=proc.waitForStarted(5000);
    bool alive=proc.state()==QProcess::Running;
    if(!started)
    {
        OutputTextIntoFile("错误：脚本未能正常唤起\n","Log.txt");
    }
    if(!alive)
    {
        OutputTextIntoFile("错误：Minecraft未能启动\n","Log.txt");
    }*/
    OutputTextIntoFile("启动操作已完成\n------------------------------------------------------------\n\n","Log.txt");
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