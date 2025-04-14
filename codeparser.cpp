#include "codeparser.h"
#include <QRegularExpression>

void CodeParser::parseVariables(const QString &line, ParseResult &result)
{
    // 匹配变量声明
    QRegularExpression varRegex("(int|double|float|char|bool)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?:=\\s*([^;]+))?\\s*;");
    QRegularExpressionMatch match = varRegex.match(line);
    
    if (match.hasMatch()) {
        QString type = match.captured(1);
        QString name = match.captured(2);
        QString value = match.captured(3);
        
        result.variables[result.codeLines.size()] = type + " " + name;
        if (!value.isEmpty()) {
            result.customData[name] = value;
        }
    }
}

void CodeParser::parseArray(const QString &line, ParseResult &result)
{
    // 匹配数组声明
    QRegularExpression arrayRegex("int\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\[\\s*\\]\\s*=\\s*\\{([^}]*)\\}");
    QRegularExpressionMatch match = arrayRegex.match(line);
    
    if (match.hasMatch()) {
        QString arrayName = match.captured(1);
        QString arrayValues = match.captured(2);
        
        QStringList values = arrayValues.split(',', Qt::SkipEmptyParts);
        QVector<int> arrayData;
        for (const QString &value : values) {
            bool ok;
            int num = value.trimmed().toInt(&ok);
            if (ok) {
                arrayData.append(num);
            }
        }
        
        result.arrayData[arrayName] = arrayData;
        result.variables[result.codeLines.size()] = "int[] " + arrayName;
    }
}

void CodeParser::parseFunction(const QString &line, ParseResult &result)
{
    // 匹配函数定义
    QRegularExpression funcRegex("(int|double|float|char|bool|void)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)");
    QRegularExpressionMatch match = funcRegex.match(line);
    
    if (match.hasMatch()) {
        QString returnType = match.captured(1);
        QString funcName = match.captured(2);
        QString params = match.captured(3);
        
        result.customData["function"] = funcName;
        result.customData["returnType"] = returnType;
        result.customData["parameters"] = params;
    }
}

void CodeParser::parseControlFlow(const QString &line, ParseResult &result)
{
    // 匹配控制流语句
    QRegularExpression controlRegex("(if|while|for)\\s*\\(([^)]*)\\)");
    QRegularExpressionMatch match = controlRegex.match(line);
    
    if (match.hasMatch()) {
        QString type = match.captured(1);
        QString condition = match.captured(2);
        
        result.customData["controlFlow"] = type;
        result.customData["condition"] = condition;
    }
}

CodeParser::ParseResult CodeParser::parse(const QString &code)
{
    ParseResult result;
    result.success = true;
    
    // 分割代码行
    QStringList lines = code.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;
        
        result.codeLines.append(trimmedLine);
        
        // 解析不同类型的代码
        parseVariables(trimmedLine, result);
        parseArray(trimmedLine, result);
        parseFunction(trimmedLine, result);
        parseControlFlow(trimmedLine, result);
    }
    
    return result;
} 