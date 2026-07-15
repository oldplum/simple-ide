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
    setModal(false); 

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

    m_findNextButton->setEnabled(false);
    m_findPrevButton->setEnabled(false);
    m_replaceButton->setEnabled(false);
    m_replaceAllButton->setEnabled(false);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(); 

    QGridLayout *formLayout = new QGridLayout();
    formLayout->addWidget(new QLabel(tr("查找内容:")), 0, 0);
    formLayout->addWidget(m_findLineEdit, 0, 1);
    formLayout->addWidget(new QLabel(tr("替换为:")), 1, 0);
    formLayout->addWidget(m_replaceLineEdit, 1, 1);
    leftLayout->addLayout(formLayout);

    QHBoxLayout *optionsLayout = new QHBoxLayout();
    optionsLayout->addWidget(m_caseCheckBox);
    optionsLayout->addWidget(m_wholeWordCheckBox);
    optionsLayout->addWidget(m_regexCheckBox);
    leftLayout->addLayout(optionsLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_findPrevButton);
    rightLayout->addWidget(m_findNextButton);
    rightLayout->addWidget(m_replaceButton);
    rightLayout->addWidget(m_replaceAllButton);
    rightLayout->addWidget(m_cancelButton);
    rightLayout->addStretch(); 

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    connect(m_findLineEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::enableButtons);

    connect(m_findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindNextClicked);
    connect(m_findPrevButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindPrevClicked);
    connect(m_replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceClicked);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAllClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject); 
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
                  false); 
}

void FindReplaceDialog::onFindPrevClicked()
{
    emit findNext(m_findLineEdit->text(),
                  m_caseCheckBox->isChecked(),
                  m_wholeWordCheckBox->isChecked(),
                  m_regexCheckBox->isChecked(),
                  true); 
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
