#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent, const QString &extension)
    : QSyntaxHighlighter(parent)
{
    loadRulesFromFile(extension);
}

void Highlighter::loadRulesFromFile(const QString &ext)
{
    // 1. 将后缀名映射为对应的 JSON 文件名
    QString fileName;
    QString lowerExt = ext.toLower();
    if (lowerExt == "json") fileName = "json.json";
    else if (lowerExt == "py") fileName = "python.json";
    else if (lowerExt == "cpp" || lowerExt == "c" || lowerExt == "h" || lowerExt == "cc" || lowerExt == "hpp") fileName = "cpp.json";
    else return; // 其他不支持的格式不加载高亮

    // 2. 智能搜寻配置文件的路径（兼容开发环境和未来打包环境）
    QStringList searchPaths = {
        "../resources/rules/" + fileName,
        "resources/rules/" + fileName,
        QCoreApplication::applicationDirPath() + "/../resources/rules/" + fileName,
        QCoreApplication::applicationDirPath() + "/resources/rules/" + fileName
    };

    QFile file;
    bool fileFound = false;
    for (const QString &path : searchPaths) {
        file.setFileName(path);
        if (file.exists()) {
            fileFound = true;
            break;
        }
    }

    if (!fileFound || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法找到或打开高亮配置文件:" << fileName;
        return; // 找不到规则文件，则不进行高亮
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    QJsonObject root = doc.object();

    // 3. 解析单行规则 (关键字、数字、普通字符串等)
    if (root.contains("singleLineRules")) {
        QJsonArray rulesArray = root["singleLineRules"].toArray();
        for (const QJsonValue &val : rulesArray) {
            QJsonObject ruleObj = val.toObject();
            HighlightRule rule;
            rule.pattern = QRegularExpression(ruleObj["pattern"].toString());
            
            QJsonObject formatObj = ruleObj["format"].toObject();
            if (formatObj.contains("foreground")) {
                rule.format.setForeground(QColor(formatObj["foreground"].toString()));
            }
            if (formatObj.contains("fontWeight") && formatObj["fontWeight"].toString() == "bold") {
                rule.format.setFontWeight(QFont::Bold);
            }
            if (formatObj.contains("fontItalic") && formatObj["fontItalic"].toBool()) {
                rule.format.setFontItalic(true);
            }
            m_rules.append(rule);
        }
    }

    // 4. 解析多行块规则 (如多行注释、Python三引号等)
    if (root.contains("multiLineComment")) {
        QJsonObject multiObj = root["multiLineComment"].toObject();
        commentStartExpression = QRegularExpression(multiObj["startPattern"].toString());
        commentEndExpression = QRegularExpression(multiObj["endPattern"].toString());

        QJsonObject formatObj = multiObj["format"].toObject();
        if (formatObj.contains("foreground")) {
            multiLineCommentFormat.setForeground(QColor(formatObj["foreground"].toString()));
        }
        if (formatObj.contains("fontWeight") && formatObj["fontWeight"].toString() == "bold") {
            multiLineCommentFormat.setFontWeight(QFont::Bold);
        }
        if (formatObj.contains("fontItalic") && formatObj["fontItalic"].toBool()) {
            multiLineCommentFormat.setFontItalic(true);
        }
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    // 1.遍历所有单行高亮规则
    for (const HighlightRule &rule : qAsConst(m_rules)){
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