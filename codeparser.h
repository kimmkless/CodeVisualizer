#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QRegularExpression>

class CodeParser
{
public:
    struct ParseResult {
        bool success;
        QMap<QString, QVector<int>> arrayData;
        QMap<QString, QVariant> customData;
        QVector<QString> codeLines;  // 存储代码行
        QMap<int, QString> variables; // 存储变量名和类型
    };

    static ParseResult parse(const QString &code);
    
private:
    static void parseVariables(const QString &line, ParseResult &result);
    static void parseArray(const QString &line, ParseResult &result);
    static void parseFunction(const QString &line, ParseResult &result);
    static void parseControlFlow(const QString &line, ParseResult &result);
};

#endif // CODEPARSER_H 