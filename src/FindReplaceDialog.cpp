#include "FindReplaceDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

FindReplaceDialog::FindReplaceDialog(QWidget *parent): QDialog(parent)
{
    setWindowTitle(tr("查找/替换"));
    setModal(false); // 设置为非模态！允许用户同时在编辑器里打字和看弹窗

    // 1.创建 UI 控件
    m_findLineEdit = new QLineEdit(this);
    m_replaceLineEdit = new QLineEdit(this);

    m_caseCheckBox = new QCheckBox(tr("区分大小写"), this);
    m_wholeWordCheckBox = new QCheckBox(tr("全词匹配"), this);
    m_regexCheckBox = new QCheckBox(tr("正则表达式"), this);

    m_findNextButton = new QPushButton(tr("查找下一个(&N)"), this);
    m_findPrevButton = new QPushButton(tr("查找上一个(&P)"), this);
    m_replaceButton = new QPushButton(tr("替换(&R)"), this);
    m_replaceAllButton = new QPushButton(tr("全部替换(&A)"), this);
    m_cancelButton = new QPushButton(tr("关闭"), this);

    // 初始时输入框为空，先禁用掉操作按钮
    m_findNextButton->setEnabled(false);
    m_findPrevButton->setEnabled(false);
    m_replaceButton->setEnabled(false);
    m_replaceAllButton->setEnabled(false);

    // 2.界面排版 (Layout)
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(); // 放左侧输入和选项

    // 网格布局，对齐“查找”和“替换”
    QGridLayout *formLayout = new QGridLayout();
    formLayout->addWidget(new QLabel(tr("查找内容:")), 0, 0);
    formLayout->addWidget(m_findLineEdit, 0, 1);
    formLayout->addWidget(new QLabel(tr("替换为:")), 1, 0);
    formLayout->addWidget(m_replaceLineEdit, 1, 1);
    leftLayout->addLayout(formLayout);

    // 水平排开复选框
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    optionsLayout->addWidget(m_caseCheckBox);
    optionsLayout->addWidget(m_wholeWordCheckBox);
    optionsLayout->addWidget(m_regexCheckBox);
    leftLayout->addLayout(optionsLayout);

    // 垂直排开右侧按钮
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_findNextButton);
    rightLayout->addWidget(m_findPrevButton);
    rightLayout->addWidget(m_replaceButton);
    rightLayout->addWidget(m_replaceAllButton);
    rightLayout->addWidget(m_cancelButton);
    rightLayout->addStretch(); // 弹簧，把按钮往上挤

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    // 3.事件关联
    // 监听输入框变化，控制按钮的可用状态
    connect(m_findLineEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::enableButtons);

    // 绑定按钮点击事件到我们写的槽函数
    connect(m_findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindNextClicked);
    connect(m_findPrevButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindPrevClicked);
    connect(m_replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceClicked);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAllClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject); // reject() 会隐藏对话框
}

void FindReplaceDialog::enableButtons(const QString &text)
{
    bool hasText = !text.isEmpty();
    m_findNextButton->setEnabled(hasText);
    m_findPrevButton->setEnabled(hasText);
    m_replaceButton->setEnabled(hasText);
    m_replaceAllButton->setEnabled(hasText);
}

void FindReplaceDialog::onFindNextClicked()
{
    emit findNext(m_findLineEdit->text(),
                  m_caseCheckBox->isChecked(),
                  m_wholeWordCheckBox->isChecked(),
                  m_regexCheckBox->isChecked(),
                  false); // backward = false 向下查找
}

void FindReplaceDialog::onFindPrevClicked()
{
    emit findNext(m_findLineEdit->text(),
                  m_caseCheckBox->isChecked(),
                  m_wholeWordCheckBox->isChecked(),
                  m_regexCheckBox->isChecked(),
                  true); // backward = true 向上查找
}

void FindReplaceDialog::onReplaceClicked()
{
    emit replace(m_findLineEdit->text(),
                 m_replaceLineEdit->text(),
                 m_caseCheckBox->isChecked(),
                 m_wholeWordCheckBox->isChecked(),
                 m_regexCheckBox->isChecked());
}

void FindReplaceDialog::onReplaceAllClicked()
{
    emit replaceAll(m_findLineEdit->text(),
                    m_replaceLineEdit->text(),
                    m_caseCheckBox->isChecked(),
                    m_wholeWordCheckBox->isChecked(),
                    m_regexCheckBox->isChecked());
}
