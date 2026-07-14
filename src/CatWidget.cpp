#include "CatWidget.h"
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QPushButton>
#include <QImageReader>

CatWidget::CatWidget(QWidget *parent): QWidget(parent), m_currentMovie(nullptr), m_currentState(Default), m_hungerLevel(0)
{
    // 设置猫咪窗口大小和悬浮属性
    setFixedSize(200, 200);
    setToolTip(tr("快捷键 Ctrl+Shift+F 也可以投喂猫粮哦！"));
    
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    m_statusLabel->hide();
    
    m_feedButton = new QPushButton(tr("投喂猫粮"), this);
    m_feedButton->setCursor(Qt::PointingHandCursor);
    connect(m_feedButton, &QPushButton::clicked, this, &CatWidget::feed);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_imageLabel);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_feedButton);
    
    // 初始化定时器
    m_moodTimer = new QTimer(this);
    m_moodTimer->setSingleShot(true);
    connect(m_moodTimer, &QTimer::timeout, this, &CatWidget::resetToDefault);
    
    m_hungerTimer = new QTimer(this);
    connect(m_hungerTimer, &QTimer::timeout, this, &CatWidget::updateHunger);
    m_hungerTimer->start(10000); // 测试用：每10秒增加一次饥饿度，真实可用60000(1分钟)

    setState(Default);
}

void CatWidget::setState(CatState state)
{
    m_currentState = state;
    
    // 决定要加载的动图名字
    QString fileName;
    QString textPlaceholder;
    switch(state){
        case Default: fileName = "default"; textPlaceholder = "( 默认 )"   ; break;
        case Happy  : fileName = "happy"  ; textPlaceholder = "( 开心! )"  ; break;
        case Angry  : fileName = "angry"  ; textPlaceholder = "( 生气! )"  ; break;
        case Hungry : fileName = "hungry" ; textPlaceholder = "( 饿了... )"; break;
    } 
    
    // 尝试寻找动图或静态图片文件 (支持 .gif 和 .png)
    QString basePath = QCoreApplication::applicationDirPath() + "/../resources/cat_gifs/" + fileName;
    QString path = "";
    if (QFile::exists(basePath + ".gif"))
        path = basePath + ".gif";
    else if (QFile::exists(basePath + ".png")) 
        path = basePath + ".png";

    if (!path.isEmpty()){
        if (m_currentMovie)
            m_currentMovie->deleteLater();
        m_currentMovie = new QMovie(path);

        // 使用 QImageReader 快速读取原始尺寸并等比例缩放
        QImageReader reader(path);
        QSize imgSize = reader.size();
        if (imgSize.isValid()){
            imgSize.scale(180, 140, Qt::KeepAspectRatio);
            m_currentMovie->setScaledSize(imgSize);
        } 
        else
            m_currentMovie->setScaledSize(QSize(160, 140)); // 降级处理

        m_imageLabel->setMovie(m_currentMovie);
        m_currentMovie->start();
    } 
    else{
        // 如果没有图片，先用文字和背景颜色占位
        m_imageLabel->setText("🐱\n" + textPlaceholder);
        QString color = (state == Happy) ? "lightgreen" : (state == Angry) ? "lightcoral" : (state == Hungry) ? "khaki" : "lightgray";
        m_imageLabel->setStyleSheet("QLabel { background-color: " + color + "; border-radius: 10px; font-weight: bold;}");
    }
    
    // 动态显示饥饿提示文字
    if (state == Hungry){
        m_statusLabel->setText(tr("喵~ 饿了……"));
        m_statusLabel->show();
    } 
    else
        m_statusLabel->hide();
}

void CatWidget::onBracketMatched()
{
    if (m_currentState == Hungry) return; // 饿的时候不开心
    setState(Happy);
    m_moodTimer->start(3000); // 开心3秒
}

void CatWidget::onCodeDeleted()
{
    if (m_currentState == Hungry) return; // 饿的时候没力气生气
    setState(Angry);
    m_moodTimer->start(3000); // 生气3秒
}

void CatWidget::feed()
{
    m_hungerLevel = 0;
    setState(Happy);
    m_moodTimer->start(4000);
}

void CatWidget::updateHunger()
{
    if (m_currentState == Hungry) return;
    
    m_hungerLevel++;
    if (m_hungerLevel >= 3) { // 测试用：累积3次就饿了
        setState(Hungry);
    }
}

void CatWidget::resetToDefault()
{
    if (m_hungerLevel >= 3)
        setState(Hungry);
    else
        setState(Default);
}
