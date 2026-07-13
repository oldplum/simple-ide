#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent, const QString &extension)
    : QSyntaxHighlighter(parent)
{
    // 在这里初始化所有高亮规则，往 m_rules 里塞 HighlightRule

    QString ext = extension.toLower();

    // 分支1：如果是JSON文件
    if (ext == "json"){
        HighlightRule rule;

        // 1. 先把所有带引号的都当成 字符串值 -> 深红色
        rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
        rule.format.setForeground(QColor(180, 0, 0));
        rule.format.setFontWeight(QFont::Normal);
        m_rules.append(rule);

        // 2. 然后再找那些带有冒号的，把它们单独覆盖成 JSON 键名 -> 卡布应援色加粗
        rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"\\s*(?=:)");
        rule.format.setForeground(QColor(34, 66, 148)); 
        rule.format.setFontWeight(QFont::Bold);
        m_rules.append(rule);

        // 3. 数字 (负号、小数等) -> 紫色
        rule.pattern = QRegularExpression("\\b-?[0-9]+\\.?[0-9]*\\b");
        rule.format.setForeground(QColor(160, 0, 160));
        m_rules.append(rule);

        // 4. 布尔值和 null -> 深青色
        rule.pattern = QRegularExpression("\\b(true|false|null)\\b");
        rule.format.setForeground(QColor(0, 128, 128));
        m_rules.append(rule);

        // JSON 没有多行注释，正则置空
        commentStartExpression = QRegularExpression();
        commentEndExpression = QRegularExpression();
    }

    // 分支2：如果是Python文件
    else if (ext == "py") {
        // 1. Python 关键字 -> 卡布应援色加粗
        QTextCharFormat keywordFormat;
        keywordFormat.setForeground(QColor(34, 66, 148));
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywords;
        keywords << "\\bdef\\b"     << "\\bclass\\b"  << "\\bimport\\b" << "\\bfrom\\b" 
                 << "\\bif\\b"      << "\\belif\\b"   << "\\belse\\b"   << "\\bfor\\b" 
                 << "\\bwhile\\b"   << "\\breturn\\b" << "\\btry\\b"    << "\\bexcept\\b" 
                 << "\\bfinally\\b" << "\\bas\\b"     << "\\bin\\b"     << "\\bis\\b" 
                 << "\\bnot\\b"     << "\\band\\b"    << "\\bor\\b"     << "\\bpass\\b"
                 << "\\bwith\\b";
        for (const QString &kw : keywords){
            HighlightRule rule;
            rule.pattern = QRegularExpression(kw);
            rule.format = keywordFormat;
            m_rules.append(rule);
        }

        // 2. Python 内置类型和常用函数 -> 深青色
        QTextCharFormat typeFormat;
        typeFormat.setForeground(QColor(0, 128, 128));
        QStringList types;
        types << "\\bint\\b"  << "\\bfloat\\b" << "\\bstr\\b"   << "\\blist\\b" 
              << "\\bdict\\b" << "\\bset\\b"   << "\\btuple\\b" << "\\bbool\\b" 
              << "\\bNone\\b" << "\\bTrue\\b"  << "\\bFalse\\b" << "\\bprint\\b" 
              << "\\blen\\b";
        for (const QString &t : types){
            HighlightRule rule;
            rule.pattern = QRegularExpression(t);
            rule.format = typeFormat;
            m_rules.append(rule);
        }

        // 3. 单行注释 (# 开头) -> 灰色斜体
        HighlightRule commentRule;
        commentRule.pattern = QRegularExpression("#[^\n]*");
        commentRule.format.setForeground(QColor(128, 128, 128));
        commentRule.format.setFontItalic(true);
        m_rules.append(commentRule);

        // 4. 字符串 (单双引号) -> 深红色
        //    加了断言，避免把三引号 """ 的前两个引号误认为空字符串
        QTextCharFormat stringFormat;
        stringFormat.setForeground(QColor(180, 0, 0));
        HighlightRule doubleQuoteRule;
        doubleQuoteRule.pattern = QRegularExpression("(?<!\")\"(?!\"\")(?:[^\"\\\\]|\\\\.)*\"");
        doubleQuoteRule.format = stringFormat;
        m_rules.append(doubleQuoteRule);
        
        HighlightRule singleQuoteRule;
        singleQuoteRule.pattern = QRegularExpression("(?<!')'(?!'')(?:[^'\\\\]|\\\\.)*'");
        singleQuoteRule.format = stringFormat;
        m_rules.append(singleQuoteRule);

        // 5. Python 多行三引号字符串（和 VS Code 一样，当成字符串渲染成深红色）
        multiLineCommentFormat.setForeground(QColor(180, 0, 0));
        multiLineCommentFormat.setFontItalic(false);
        commentStartExpression = QRegularExpression("\"\"\"");
        commentEndExpression = QRegularExpression("\"\"\"");
    }

    // 分支3：如果是 C++ / Java / C 等其他语言
    else {
        // 关键字（周深应援色，加粗）
        QTextCharFormat keywordFormat;
        keywordFormat.setForeground(QColor(34, 66, 148));
        keywordFormat.setFontWeight(QFont::Bold);

        QStringList keywords;
        keywords << "\\bauto\\b"      << "\\bbreak\\b"    << "\\bcase\\b"
                << "\\bcatch\\b"     << "\\bclass\\b"    << "\\bconst\\b"
                << "\\bconstexpr\\b" << "\\bcontinue\\b" << "\\bdefault\\b"
                << "\\bdelete\\b"    << "\\bdo\\b"       << "\\belse\\b"
                << "\\benum\\b"      << "\\bexplicit\\b" << "\\bextern\\b"
                << "\\bfor\\b"       << "\\bfriend\\b"   << "\\bgoto\\b"
                << "\\bif\\b"        << "\\binline\\b"   << "\\bnamespace\\b"
                << "\\bnew\\b"       << "\\bnoexcept\\b" << "\\bnullptr\\b"
                << "\\boperator\\b"  << "\\boverride\\b" << "\\bprivate\\b"
                << "\\bprotected\\b" << "\\bpublic\\b"   << "\\breturn\\b"
                << "\\bsizeof\\b"    << "\\bstatic\\b"   << "\\bstruct\\b"
                << "\\bswitch\\b"    << "\\btemplate\\b" << "\\bthis\\b"
                << "\\bthrow\\b"     << "\\btry\\b"      << "\\btypedef\\b"
                << "\\btypename\\b"  << "\\bunion\\b"    << "\\busing\\b"
                << "\\bvirtual\\b"   << "\\bvolatile\\b" << "\\bwhile\\b";
        
        for (const QString &kw : keywords) {
            HighlightRule rule;
            rule.pattern = QRegularExpression(kw);
            rule.format = keywordFormat;
            m_rules.append(rule);
        }

        //数据类型（深青色）
        QTextCharFormat typeFormat;
        typeFormat.setForeground(QColor(0, 128, 128));

        QStringList types;
        types << "\\bbool\\b"  << "\\bchar\\b"   << "\\bdouble\\b"
            << "\\bfloat\\b" << "\\bint\\b"    << "\\blong\\b"
            << "\\bshort\\b" << "\\bsigned\\b" << "\\bunsigned\\b"
            << "\\bvoid\\b"  << "\\bsize_t\\b" << "\\bstring\\b"
            << "\\btrue\\b"  << "\\bfalse\\b";

        for (const QString &t : types) {
            HighlightRule rule;
            rule.pattern = QRegularExpression(t);
            rule.format = typeFormat;
            m_rules.append(rule);
        }

        //预处理指令（深绿色）
        QTextCharFormat preprocessorFormat;
        preprocessorFormat.setForeground(QColor(0, 128, 0));

        HighlightRule preprocessorRule;
        preprocessorRule.pattern = QRegularExpression("^\\s*#\\s*\\w+");
        preprocessorRule.format = preprocessorFormat;
        m_rules.append(preprocessorRule);

        //数字字面量（深品红色）
        QTextCharFormat numberFormat;
        numberFormat.setForeground(QColor(160, 0, 160));

        HighlightRule numberRule;
        numberRule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
        numberRule.format = numberFormat;
        m_rules.append(numberRule);

        //函数名（蓝色）
        QTextCharFormat functionFormat;
        functionFormat.setForeground(QColor(0, 100, 200));

        HighlightRule functionRule;
        functionRule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_]*\\s*(?=\\()");
        functionRule.format = functionFormat;
        m_rules.append(functionRule);

        //字符串字面量（深红色）
        QTextCharFormat stringFormat;
        stringFormat.setForeground(QColor(180, 0, 0));
        
        HighlightRule doubleQuoteRule;//双引号字符串
        doubleQuoteRule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
        doubleQuoteRule.format = stringFormat;
        m_rules.append(doubleQuoteRule);

        HighlightRule singleQuoteRule;//单引号字符串
        singleQuoteRule.pattern = QRegularExpression("'(?:[^'\\\\]|\\\\.)*'");
        singleQuoteRule.format = stringFormat;
        m_rules.append(singleQuoteRule);

        //单行注释（灰色，斜体）
        QTextCharFormat commentFormat;
        commentFormat.setForeground(QColor(128, 128, 128));
        commentFormat.setFontItalic(true);

        HighlightRule commentRule;
        commentRule.pattern = QRegularExpression("//[^\n]*");
        commentRule.format = commentFormat;
        m_rules.append(commentRule);

        //多行注释格式初始化
        multiLineCommentFormat.setForeground(QColor(128, 128, 128));
        multiLineCommentFormat.setFontItalic(true);
        // 告诉程序 C++ 的多行注释符号是什么
        commentStartExpression = QRegularExpression("/\\*");
        commentEndExpression = QRegularExpression("\\*/");
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    // 1.遍历所有单行高亮规则
    for (const HighlightRule &rule : m_rules){
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()){
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    // 如果当前语言没有多行注释（比如 JSON），直接结束
    if (commentStartExpression.pattern().isEmpty())
        return;

    // 2.处理多行注释 / 多行字符串
    setCurrentBlockState(0);
    int startIndex = 0;
    bool continuingFromPrevious = (previousBlockState() == 1);

    // 如果上一行不是多行注释状态，那就从本行开头找开始符
    if (!continuingFromPrevious) {
        QRegularExpressionMatch startMatch = commentStartExpression.match(text);
        startIndex = startMatch.hasMatch() ? startMatch.capturedStart() : -1;
    }

    while (startIndex >= 0) {
        // 如果是延续上一行的多行注释，从当前位置开始找结束符
        // 否则跳过开始符的长度，避免 Python """ 把同一个位置当成结束
        int endSearchStart;
        if (continuingFromPrevious) {
            endSearchStart = startIndex;
        } else {
            QRegularExpressionMatch sm = commentStartExpression.match(text, startIndex);
            endSearchStart = startIndex + sm.capturedLength();
        }

        QRegularExpressionMatch endMatch = commentEndExpression.match(text, endSearchStart);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int commentLength;
        if (endIndex == -1) {
            // 本行没有找到结束符，注释延续到行尾，通知下一行继续
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            // 找到了结束符，计算这一段的长度
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);

        // 继续在当前行剩下的地方找有没有下一个开始符
        QRegularExpressionMatch nextStart = commentStartExpression.match(text, startIndex + commentLength);
        startIndex = nextStart.hasMatch() ? nextStart.capturedStart() : -1;
        continuingFromPrevious = false; // 后续的匹配都是新的开始
    }
}