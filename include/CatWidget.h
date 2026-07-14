#ifndef CATWIDGET_H
#define CATWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMovie>
#include <QTimer>
#include <QPushButton>
#include <QTimer>

enum CatState{
    Default,
    Happy,
    Angry,
    Hungry
};

// 电子猫娘交互组件
class CatWidget: public QWidget
{
    Q_OBJECT
public:
    explicit CatWidget(QWidget *parent = nullptr); // 构造函数，初始化 UI 控件（标签、按钮）并启动饥饿倒计时

public slots:
    // 外部事件响应槽（接收编辑器信号）
    void onBracketMatched(); // 匹配括号 -> 切换为开心状态（Happy）
    void onCodeDeleted();    // 狂删代码 -> 切换为生气状态（Angry）
    void feed();             // 用户主动投喂 -> 重置饥饿度，满血复活并强制开心

private slots:
    // 内部生命周期管理
    void updateHunger();     // 饥饿定时器超时触发：暗中增加饥饿度，若到达阈值则锁死为 Hungry 状态
    void resetToDefault();   // 情绪定时器超时触发：情绪平复，自动从喜/怒重置回 Default（默认）状态

private:
    // 核心状态机控制
    void setState(CatState state); // 统一的状态切换入口：负责停止旧动画，加载新动画，防止内存泄漏和状态冲突
    
    // UI 呈现层控件
    QLabel *m_imageLabel;      // 用于承载并显示 GIF 动画的标签容器
    QLabel *m_statusLabel;     // 用于在猫娘饿了的时候在屏幕上显示红字“饿了！”的提示标签
    QPushButton *m_feedButton; // 投喂猫粮的交互按钮
    QMovie *m_currentMovie;    // 当前正在播放的 GIF 动图控制器实例
    
    // 核心数据流
    CatState m_currentState; // 核心变量：记录猫娘当前所处的内部状态枚举值
    int m_hungerLevel;       // 饥饿度累加器（满了就会进入强制饥饿状态，拒绝一切工作）
    
    // 时间维度控制 (QTimer)
    QTimer *m_moodTimer;     // 单次定时器（SingleShot）：控制瞬时情绪（生气/开心）只能持续有限的时间（例如 3 秒）
    QTimer *m_hungerTimer;   // 循环定时器：控制饥饿度的自然流失频率（例如每 10 秒扣血一次）
};

#endif // CATWIDGET_H