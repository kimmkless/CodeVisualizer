#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 加载样式表
    QFile styleFile(":/resources/style.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }
    
    // 设置应用信息
    app.setApplicationName("代码可视化工具");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CodeVisualizer");
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
} 