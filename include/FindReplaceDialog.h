#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QPushButton;

//查找与替换对话框组件
class FindReplaceDialog: public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QWidget *parent = nullptr); // 构造函数，负责使用纯代码进行 UI 控件的实例化和网格布局

signals:
    // 查找信号（backward=true时代表向上查找）
    void findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward);
    // 替换当前信号
    void replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);
    // 全部替换信号
    void replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);

private slots:
    // 内部 UI 响应逻辑
    void onFindNextClicked();                // 响应“查找下一个”按钮点击，读取复选框状态并发出 findNext 信号 (backward=false)
    void onFindPrevClicked();                // 响应“查找上一个”按钮点击，读取复选框状态并发出 findNext 信号 (backward=true)
    void onReplaceClicked();                 // 响应“替换”按钮点击，读取复选框状态并发出 replace 信号
    void onReplaceAllClicked();              // 响应“全部替换”按钮点击，读取复选框状态并发出 replaceAll 信号
    void enableButtons(const QString &text); // 槽函数：根据输入框是否有字，动态禁用/启用右侧功能按钮（防止无字查找引发崩溃）

private:
    // UI 控件指针
    QLineEdit *m_findLineEdit;       // 查找内容输入框
    QLineEdit *m_replaceLineEdit;    // 替换内容输入框
    
    QCheckBox *m_caseCheckBox;       // “区分大小写”复选框
    QCheckBox *m_wholeWordCheckBox;  // “全词匹配”复选框
    QCheckBox *m_regexCheckBox;      // “使用正则表达式”复选框

    QPushButton *m_findNextButton;   // 查找下一个按钮
    QPushButton *m_findPrevButton;   // 查找上一个按钮
    QPushButton *m_replaceButton;    // 替换按钮
    QPushButton *m_replaceAllButton; // 全部替换按钮
    QPushButton *m_cancelButton;     // 取消（关闭弹窗）按钮
};

#endif // FINDREPLACEDIALOG_H
