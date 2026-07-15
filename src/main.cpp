#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); // 初始化 Qt 应用程序对象，处理命令行参数，并管理应用程序的控制流

    MainWindow window;            // 实例化主窗口（在这里会级联初始化所有的代码编辑器、查找面板和电子猫娘）
    
    // 设置主窗口的默认标题和启动时的默认尺寸
    window.setWindowTitle("Simple IDE");
    window.resize(800, 600);

    window.show();                // 将主窗口显示在屏幕上

    return app.exec();            // 进入 Qt 的主事件循环（Event Loop），开始等待并处理鼠标、键盘、定时器等异步事件
}
