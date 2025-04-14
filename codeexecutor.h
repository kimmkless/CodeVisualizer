#ifndef CODEEXECUTOR_H
#define CODEEXECUTOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QThread>
#include <QMutex>
#include "codeparser.h"

// 代码执行状态
struct CodeState {
    int currentLine;                      // 当前执行行号
    QMap<QString, QVariant> variables;    // 变量名到值的映射
    QVector<QVariant> arrayData;          // 数组数据
    QMap<QString, QVariant> customData;   // 其他需要可视化的自定义数据
    QString currentFunction;              // 当前执行的函数
    QString currentControlFlow;           // 当前的控制流
    
    CodeState() : currentLine(-1) {}
    CodeState(int line) : currentLine(line) {}
};

// 代码解析和执行类
class CodeExecutor : public QObject
{
    Q_OBJECT
    
public:
    explicit CodeExecutor(QObject *parent = nullptr);
    ~CodeExecutor();
    
    void setCode(const QString &code);
    void start();
    void stop();
    
signals:
    void executionStep(const CodeState &state);
    void executionError(const QString &error);
    void executionFinished();
    
private:
    class ExecutorThread : public QThread
    {
    public:
        explicit ExecutorThread(CodeExecutor *executor);
        void stop();
        
    protected:
        void run() override;
        
    private:
        CodeExecutor *executor;
        bool stopRequested;
        QMutex mutex;
    };
    
    QString parseCode(const QString &code);
    void executeCode();
    void analyzeCode();
    void mockExecution();
    
    QString code;
    ExecutorThread *executorThread;
    bool isRunning;
    CodeState currentState;
    QMap<QString, QVariant> variableValues;
    
    void updateVariable(const QString &name, const QVariant &value);
    void executeLine(const QString &line);
    bool evaluateCondition(const QString &condition);
    
    friend class ExecutorThread;
};

#endif // CODEEXECUTOR_H 