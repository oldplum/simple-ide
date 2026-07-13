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
    // 查找信号（backward=true时代表向上查找）
    void findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward);
    // 替换当前信号
    void replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);
    // 全部替换信号
    void replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);

private slots:
    void onFindNextClicked();
    void onFindPrevClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();
    void enableButtons(const QString &text); // 根据输入框是否有字，动态禁用/启用按钮

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

#endif // FINDREPLACEDIALOG_H
