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
    void onBracketMatched(); 
    void onCodeDeleted();    
    void feed();             

private slots:
    void updateHunger();     
    void resetToDefault();   

private:
    void setState(CatState state); 
    
    QLabel *m_imageLabel;      
    QLabel *m_statusLabel;     
    QPushButton *m_feedButton; 
    QMovie *m_currentMovie;    
    
    CatState m_currentState; 
    int m_hungerLevel;       
    
    QTimer *m_moodTimer;     
    QTimer *m_hungerTimer;   
};

#endif 