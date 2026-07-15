#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent, const QString &extension): QSyntaxHighlighter(parent)
{
    loadRulesFromFile(extension);
}

void Highlighter::loadRulesFromFile(const QString &ext)
{
    QString fileName;
    QString lowerExt = ext.toLower();
    if (lowerExt == "json") 
        fileName = "json.json";
    else if (lowerExt == "py") 
        fileName = "python.json";
    else if (lowerExt == "cpp" || lowerExt == "c" || lowerExt == "h" || lowerExt == "cc" || lowerExt == "hpp") 
        fileName = "cpp.json";
    else return; 

    QStringList searchPaths = {
        "../resources/rules/" + fileName,
        "resources/rules/" + fileName,
        QCoreApplication::applicationDirPath() + "/../resources/rules/" + fileName,
        QCoreApplication::applicationDirPath() + "/resources/rules/" + fileName
    };

    QFile file;
    bool fileFound = false;
    for (const QString &path: searchPaths){
        file.setFileName(path);
        if (file.exists()){
            fileFound = true;
            break;
        }
    }

    if (!fileFound || !file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "无法找到或打开高亮配置文件:" << fileName;
        return; 
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) 
        return;
    QJsonObject root = doc.object();

    if (root.contains("singleLineRules")){
        QJsonArray rulesArray = root["singleLineRules"].toArray();
        for (const QJsonValue &val : rulesArray){
            QJsonObject ruleObj = val.toObject();
            HighlightRule rule;
            rule.pattern = QRegularExpression(ruleObj["pattern"].toString());
            
            QJsonObject formatObj = ruleObj["format"].toObject();
            if (formatObj.contains("foreground"))
                rule.format.setForeground(QColor(formatObj["foreground"].toString()));
            if (formatObj.contains("fontWeight") && formatObj["fontWeight"].toString() == "bold") 
                rule.format.setFontWeight(QFont::Bold);
            if (formatObj.contains("fontItalic") && formatObj["fontItalic"].toBool())
                rule.format.setFontItalic(true);
            m_rules.append(rule);
        }
    }

    if (root.contains("multiLineComment")){
        QJsonObject multiObj = root["multiLineComment"].toObject();
        commentStartExpression = QRegularExpression(multiObj["startPattern"].toString());
        commentEndExpression = QRegularExpression(multiObj["endPattern"].toString());

        QJsonObject formatObj = multiObj["format"].toObject();
        if (formatObj.contains("foreground"))
            multiLineCommentFormat.setForeground(QColor(formatObj["foreground"].toString()));
        if (formatObj.contains("fontWeight") && formatObj["fontWeight"].toString() == "bold")
            multiLineCommentFormat.setFontWeight(QFont::Bold);
        if (formatObj.contains("fontItalic") && formatObj["fontItalic"].toBool())
            multiLineCommentFormat.setFontItalic(true);
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : qAsConst(m_rules)){
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()){
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    if (commentStartExpression.pattern().isEmpty())
        return;

    setCurrentBlockState(0);
    int startIndex = 0;
    bool continuingFromPrevious = (previousBlockState() == 1);

    if (!continuingFromPrevious){
        QRegularExpressionMatch startMatch = commentStartExpression.match(text);
        startIndex = startMatch.hasMatch() ? startMatch.capturedStart() : -1;
    }

    while (startIndex >= 0){
        int endSearchStart;
        if (continuingFromPrevious)
            endSearchStart = startIndex;
        else{
            QRegularExpressionMatch sm = commentStartExpression.match(text, startIndex);
            endSearchStart = startIndex + sm.capturedLength();
        }

        QRegularExpressionMatch endMatch = commentEndExpression.match(text, endSearchStart);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int commentLength;
        if (endIndex == -1){
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } 
        else
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        setFormat(startIndex, commentLength, multiLineCommentFormat);

        QRegularExpressionMatch nextStart = commentStartExpression.match(text, startIndex + commentLength);
        startIndex = nextStart.hasMatch() ? nextStart.capturedStart() : -1;
        continuingFromPrevious = false; 
    }
}