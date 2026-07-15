#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QPushButton;

class FindReplaceDialog: public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QWidget *parent = nullptr); 

signals:
    void findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward);
    void replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);
    void replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);

private slots:
    void onFindNextClicked();                
    void onFindPrevClicked();                
    void onReplaceClicked();                 
    void onReplaceAllClicked();              
    void enableButtons(const QString &text); 

private:
    QLineEdit *m_findLineEdit;       
    QLineEdit *m_replaceLineEdit;    
    
    QCheckBox *m_caseCheckBox;       
    QCheckBox *m_wholeWordCheckBox;  
    QCheckBox *m_regexCheckBox;      

    QPushButton *m_findNextButton;   
    QPushButton *m_findPrevButton;   
    QPushButton *m_replaceButton;    
    QPushButton *m_replaceAllButton; 
    QPushButton *m_cancelButton;     
};

#endif 
