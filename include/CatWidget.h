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

class CatWidget: public QWidget
{
    Q_OBJECT
public:
    explicit CatWidget(QWidget *parent = nullptr);

public slots:
    void onBracketMatched(); // 匹配括号 -> 开心
    void onCodeDeleted();    // 狂删代码 -> 生气
    void feed();             // 投喂 -> 重置饥饿并开心

private slots:
    void updateHunger();
    void resetToDefault();

private:
    void setState(CatState state);
    
    QLabel *m_imageLabel;
    QPushButton *m_feedButton;
    QMovie *m_currentMovie;
    
    CatState m_currentState;
    int m_hungerLevel;       // 饥饿度，满了就会饿
    
    QTimer *m_moodTimer;     // 控制生气/开心持续时间的定时器
    QTimer *m_hungerTimer;   // 控制饥饿增长的定时器
};

#endif // CATWIDGET_H