#include "codeexecutor.h"
#include "codeparser.h"
#include <QRegularExpression>
#include <QDebug>
#include <QRandomGenerator>
#include <QCoreApplication>

// 构造函数
CodeExecutor::CodeExecutor(QObject *parent)
    : QObject(parent)
    , executorThread(nullptr)
    , isRunning(false)
{
    executorThread = new ExecutorThread(this);
}

// 析构函数
CodeExecutor::~CodeExecutor()
{
    stop();
    if (executorThread) {
        executorThread->wait();
        delete executorThread;
    }
}

// 设置要执行的代码
void CodeExecutor::setCode(const QString &codeText)
{
    code = codeText;
}

// 开始执行代码
void CodeExecutor::start()
{
    if (isRunning || code.isEmpty()) {
        return;
    }
    
    isRunning = true;
    executorThread->start();
}

// 停止执行
void CodeExecutor::stop()
{
    if (!isRunning) {
        return;
    }
    
    executorThread->stop();
    isRunning = false;
}

// 解析代码，分析结构
QString CodeExecutor::parseCode(const QString &codeText)
{
    try {
        // 检查是否是有效的C++函数
        QRegularExpression funcRe("^\\s*(\\w+)\\s+(\\w+)\\s*\\((.*)\\)\\s*\\{");
        QRegularExpressionMatch match = funcRe.match(codeText);
        
        if (!match.hasMatch()) {
            return "代码必须是一个完整的C++函数定义";
        }
        
        // 这里只是简单的检查，真实情况中可能需要更复杂的解析器
        if (!codeText.contains("{") || !codeText.contains("}")) {
            return "代码必须包含完整的函数体 (缺少花括号)";
        }
        
        return QString(); // 返回空字符串表示解析成功
    } catch (const std::exception &e) {
        return QString("解析代码时出错: %1").arg(e.what());
    }
}

// 分析代码结构
void CodeExecutor::analyzeCode()
{
    // 将代码按行分割
    QStringList lines = code.split("\n");
    
    // 存储分析结果
    QMap<QString, QVariant> analysisResult;
    
    // 函数识别
    QRegularExpression funcRe("^\\s*(\\w+)\\s+(\\w+)\\s*\\((.*)\\)\\s*\\{");
    QRegularExpressionMatch funcMatch = funcRe.match(code);
    
    QString returnType;
    QString functionName;
    QString parameters;
    
    if (funcMatch.hasMatch()) {
        returnType = funcMatch.captured(1);
        functionName = funcMatch.captured(2);
        parameters = funcMatch.captured(3);
        
        analysisResult["returnType"] = returnType;
        analysisResult["functionName"] = functionName;
        analysisResult["parameters"] = parameters;
    }
    
    // 识别参数列表
    QStringList paramList = parameters.split(",");
    QVariantList parsedParams;
    
    for (const QString &param : paramList) {
        QString trimmedParam = param.trimmed();
        if (!trimmedParam.isEmpty()) {
            // 尝试分离类型和名称
            QStringList parts = trimmedParam.split(" ");
            if (parts.size() >= 2) {
                QMap<QString, QString> paramInfo;
                paramInfo["type"] = parts[0];
                paramInfo["name"] = parts[1];
                parsedParams.append(QVariant::fromValue(paramInfo));
            } else {
                parsedParams.append(trimmedParam);
            }
        }
    }
    analysisResult["parsedParameters"] = QVariant::fromValue(parsedParams);
    
    // 识别变量声明
    QRegularExpression varRe("\\b(int|float|double|char|bool|string|void|long|short|unsigned|signed)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?:=|;|\\[)");
    QVariantList variables;
    
    for (const QString &line : lines) {
        QRegularExpressionMatchIterator it = varRe.globalMatch(line);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QMap<QString, QString> varInfo;
            varInfo["type"] = match.captured(1);
            varInfo["name"] = match.captured(2);
            variables.append(QVariant::fromValue(varInfo));
        }
    }
    analysisResult["variables"] = QVariant::fromValue(variables);
    
    // 识别循环结构
    QRegularExpression forRe("\\bfor\\s*\\(");
    QRegularExpression whileRe("\\bwhile\\s*\\(");
    QRegularExpression doWhileRe("\\bdo\\s*\\{");
    
    int forCount = 0;
    int whileCount = 0;
    int doWhileCount = 0;
    
    for (const QString &line : lines) {
        if (forRe.match(line).hasMatch()) forCount++;
        if (whileRe.match(line).hasMatch()) whileCount++;
        if (doWhileRe.match(line).hasMatch()) doWhileCount++;
    }
    
    analysisResult["forLoops"] = forCount;
    analysisResult["whileLoops"] = whileCount;
    analysisResult["doWhileLoops"] = doWhileCount;
    
    // 识别条件语句
    QRegularExpression ifRe("\\bif\\s*\\(");
    QRegularExpression elseRe("\\belse\\b");
    QRegularExpression switchRe("\\bswitch\\s*\\(");
    
    int ifCount = 0;
    int elseCount = 0;
    int switchCount = 0;
    
    for (const QString &line : lines) {
        if (ifRe.match(line).hasMatch()) ifCount++;
        if (elseRe.match(line).hasMatch()) elseCount++;
        if (switchRe.match(line).hasMatch()) switchCount++;
    }
    
    analysisResult["ifStatements"] = ifCount;
    analysisResult["elseStatements"] = elseCount;
    analysisResult["switchStatements"] = switchCount;
    
    // 识别数据结构
    bool hasArray = code.contains("[]") || code.contains("vector") || code.contains("array", Qt::CaseInsensitive);
    bool hasTree = code.contains("tree", Qt::CaseInsensitive) || 
                  (code.contains("node", Qt::CaseInsensitive) && 
                   code.contains("left", Qt::CaseInsensitive) && 
                   code.contains("right", Qt::CaseInsensitive));
    bool hasLinkedList = code.contains("list", Qt::CaseInsensitive) || code.contains("next", Qt::CaseInsensitive);
    bool hasGraph = code.contains("graph", Qt::CaseInsensitive) || 
                   code.contains("edge", Qt::CaseInsensitive) || 
                   code.contains("vertex", Qt::CaseInsensitive);
    bool hasStack = code.contains("stack", Qt::CaseInsensitive) || 
                   (code.contains("push", Qt::CaseInsensitive) && code.contains("pop", Qt::CaseInsensitive));
    bool hasQueue = (code.contains("queue", Qt::CaseInsensitive) && (hasTree || hasGraph));
    
    analysisResult["hasArray"] = hasArray;
    analysisResult["hasTree"] = hasTree;
    analysisResult["hasLinkedList"] = hasLinkedList;
    analysisResult["hasGraph"] = hasGraph;
    analysisResult["hasStack"] = hasStack;
    analysisResult["hasQueue"] = hasQueue;
    
    // 识别算法类型
    bool isSorting = functionName.contains("sort", Qt::CaseInsensitive);
    bool isSearch = functionName.contains("search", Qt::CaseInsensitive) || 
                   functionName.contains("find", Qt::CaseInsensitive);
    bool isTraversal = functionName.contains("traversal", Qt::CaseInsensitive) || 
                      functionName.contains("traverse", Qt::CaseInsensitive) ||
                      functionName.contains("visit", Qt::CaseInsensitive);
    bool isPathFinding = functionName.contains("path", Qt::CaseInsensitive) || 
                        functionName.contains("route", Qt::CaseInsensitive) ||
                        functionName.contains("distance", Qt::CaseInsensitive);
    
    analysisResult["isSorting"] = isSorting;
    analysisResult["isSearch"] = isSearch;
    analysisResult["isTraversal"] = isTraversal;
    analysisResult["isPathFinding"] = isPathFinding;
    
    // 识别特定排序算法
    bool isBubbleSort = code.contains("bubble", Qt::CaseInsensitive);
    bool isQuickSort = code.contains("quick", Qt::CaseInsensitive) || 
                      (code.contains("partition", Qt::CaseInsensitive) && code.contains("pivot", Qt::CaseInsensitive));
    bool isMergeSort = code.contains("merge", Qt::CaseInsensitive);
    bool isInsertionSort = code.contains("insertion", Qt::CaseInsensitive);
    bool isSelectionSort = code.contains("selection", Qt::CaseInsensitive);
    bool isHeapSort = code.contains("heap", Qt::CaseInsensitive);
    
    analysisResult["isBubbleSort"] = isBubbleSort;
    analysisResult["isQuickSort"] = isQuickSort;
    analysisResult["isMergeSort"] = isMergeSort;
    analysisResult["isInsertionSort"] = isInsertionSort;
    analysisResult["isSelectionSort"] = isSelectionSort;
    analysisResult["isHeapSort"] = isHeapSort;
    
    // 识别特定搜索算法
    bool isBinarySearch = code.contains("binary", Qt::CaseInsensitive) ||
                         (code.contains("mid", Qt::CaseInsensitive) && code.contains("left", Qt::CaseInsensitive) && code.contains("right", Qt::CaseInsensitive));
    bool isLinearSearch = code.contains("linear", Qt::CaseInsensitive);
    bool isDFS = code.contains("depth", Qt::CaseInsensitive) || 
                (code.contains("recursive", Qt::CaseInsensitive) && (hasTree || hasGraph));
    bool isBFS = code.contains("breadth", Qt::CaseInsensitive) || 
                code.contains("queue", Qt::CaseInsensitive) && (hasTree || hasGraph);
    
    analysisResult["isBinarySearch"] = isBinarySearch;
    analysisResult["isLinearSearch"] = isLinearSearch;
    analysisResult["isDFS"] = isDFS;
    analysisResult["isBFS"] = isBFS;
    
    // 计算函数的复杂度，基于嵌套循环数量的简单估计
    QString complexity = "O(1)"; // 默认为常数时间
    
    int nestedLevel = 0;
    int maxNestedLevel = 0;
    
    for (const QString &line : lines) {
        if (line.contains("{")) {
            nestedLevel++;
            maxNestedLevel = qMax(maxNestedLevel, nestedLevel);
        }
        if (line.contains("}")) {
            nestedLevel--;
        }
    }
    
    // 根据嵌套级别和循环数量估算复杂度
    if (forCount + whileCount + doWhileCount == 0) {
        complexity = "O(1)";
    } else if (forCount + whileCount + doWhileCount == 1) {
        complexity = "O(n)";
    } else if (maxNestedLevel >= 2 && (forCount + whileCount + doWhileCount >= 2)) {
        complexity = "O(n²)";
    } else if (maxNestedLevel >= 3 && (forCount + whileCount + doWhileCount >= 3)) {
        complexity = "O(n³)";
    } else if (code.contains("log") || isBinarySearch) {
        complexity = "O(log n)";
    } else if (isQuickSort || isMergeSort || isHeapSort) {
        complexity = "O(n log n)";
    }
    
    analysisResult["complexity"] = complexity;
    
    // 识别递归
    bool hasRecursion = false;
    if (!functionName.isEmpty()) {
        for (int i = 0; i < lines.size(); i++) {
            if (lines[i].contains(functionName) && lines[i].contains("(") && !lines[i].contains("=") && i > 0) {
                // 跳过函数定义本身
                if (i == 0 || !lines[i-1].contains(returnType)) {
                    hasRecursion = true;
                    break;
                }
            }
        }
    }
    
    analysisResult["hasRecursion"] = hasRecursion;
    
    // 输出分析结果，以便调试
    qDebug() << "代码分析完成:";
    qDebug() << "函数名: " << functionName;
    qDebug() << "返回类型: " << returnType;
    qDebug() << "参数: " << parameters;
    qDebug() << "循环数量: For=" << forCount << ", While=" << whileCount;
    qDebug() << "条件语句: If=" << ifCount << ", Else=" << elseCount;
    qDebug() << "数据结构: Array=" << hasArray << ", Tree=" << hasTree << ", LinkedList=" << hasLinkedList;
    qDebug() << "算法类型: Sorting=" << isSorting << ", Search=" << isSearch;
    qDebug() << "复杂度估计: " << complexity;
    qDebug() << "是否递归: " << hasRecursion;
}

// 模拟执行代码
void CodeExecutor::mockExecution()
{
    // 分析代码行
    QStringList lines = code.split("\n");
    
    // 识别函数名
    QRegularExpression funcRe("^\\s*(\\w+)\\s+(\\w+)\\s*\\((.*)\\)\\s*\\{");
    QRegularExpressionMatch match = funcRe.match(code);
    QString functionName;
    QString returnType;
    QString params;
    
    if (match.hasMatch()) {
        returnType = match.captured(1);
        functionName = match.captured(2);
        params = match.captured(3);
    }
    
    // 识别算法类型
    bool isSorting = functionName.contains("sort", Qt::CaseInsensitive);
    bool isSearch = functionName.contains("search", Qt::CaseInsensitive) || 
                   functionName.contains("find", Qt::CaseInsensitive);
    bool isTree = code.contains("tree", Qt::CaseInsensitive) || 
                 code.contains("node", Qt::CaseInsensitive) && 
                 code.contains("left", Qt::CaseInsensitive) && 
                 code.contains("right", Qt::CaseInsensitive);
    bool isGraph = code.contains("graph", Qt::CaseInsensitive) || 
                  code.contains("edge", Qt::CaseInsensitive) || 
                  code.contains("vertex", Qt::CaseInsensitive) || 
                  code.contains("adjacency", Qt::CaseInsensitive);
    bool isLinkedList = code.contains("list", Qt::CaseInsensitive) || 
                      code.contains("next", Qt::CaseInsensitive);
    
    // 识别数组和循环
    bool hasArray = code.contains("[]") || 
                   code.contains("vector") || 
                   code.contains("array", Qt::CaseInsensitive);
    
    // 识别循环
    int forLoops = code.count("for");
    int whileLoops = code.count("while");
    
    // 创建可视化数据
    QVector<QVariant> mockArray;
    
    // 生成示例数据
    int arraySize = 10;
    if (isSorting || isSearch) {
        // 生成随机数组用于排序和搜索算法
        for (int i = 0; i < arraySize; ++i) {
            mockArray.append(QRandomGenerator::global()->bounded(1, 100));
        }
    } else if (isTree) {
        // 创建树结构示例数据
        // 这里使用一个简单的结构，每个节点都有值、左子节点和右子节点
        QMap<QString, QVariant> rootNode;
        rootNode["value"] = 50;
        rootNode["left"] = 30;
        rootNode["right"] = 70;
        
        QMap<QString, QVariant> treeData;
        treeData["root"] = rootNode;
        treeData["type"] = "binary_tree";
        
        CodeState initialState(0);
        initialState.customData["treeData"] = treeData;
        emit executionStep(initialState);
    } else if (isGraph) {
        // 创建图结构示例数据
        QMap<QString, QVariant> graphData;
        graphData["type"] = "directed_graph";
        
        QVariantList vertices;
        for (int i = 0; i < 6; ++i) {
            vertices.append(i);
        }
        
        QVariantList edges;
        // 添加一些随机边
        for (int i = 0; i < 10; ++i) {
            QVariantList edge;
            edge.append(QRandomGenerator::global()->bounded(0, 6));
            edge.append(QRandomGenerator::global()->bounded(0, 6));
            edges.append(QVariant(edge));
        }
        
        graphData["vertices"] = vertices;
        graphData["edges"] = edges;
        
        CodeState initialState(0);
        initialState.customData["graphData"] = graphData;
        emit executionStep(initialState);
    } else if (isLinkedList) {
        // 创建链表示例数据
        QVariantList listData;
        for (int i = 0; i < 8; ++i) {
            listData.append(QRandomGenerator::global()->bounded(1, 100));
        }
        
        CodeState initialState(0);
        initialState.customData["listData"] = listData;
        initialState.customData["type"] = "linked_list";
        emit executionStep(initialState);
    } else if (hasArray) {
        // 处理通用数组算法
        CodeState initialState(0);
        initialState.arrayData = mockArray;
        emit executionStep(initialState);
    } else {
        // 默认情况，生成一些随机数据
        CodeState initialState(0);
        initialState.arrayData = mockArray;
        emit executionStep(initialState);
    }
    
    // 模拟代码执行过程
    // 逐行执行，生成不同状态
    int totalLines = lines.size();

    // 查找函数体的开始和结束位置
    int functionBodyStart = -1;
    int functionBodyEnd = -1;
    int braceCount = 0;
    
    for (int i = 0; i < totalLines; ++i) {
        if (lines[i].contains("{")) {
            if (functionBodyStart == -1) {
                functionBodyStart = i;
            }
            braceCount++;
        }
        
        if (lines[i].contains("}")) {
            braceCount--;
            if (braceCount == 0) {
                functionBodyEnd = i;
                break;
            }
        }
    }
    
    if (functionBodyStart == -1 || functionBodyEnd == -1) {
        emit executionError("无法识别函数体");
        return;
    }
    
    // 执行主体代码
    for (int i = functionBodyStart; i <= functionBodyEnd; ++i) {
        if (!isRunning) {
            break;
        }
        
        // 跳过空行和注释行
        if (lines[i].trimmed().isEmpty() || lines[i].trimmed().startsWith("//")) {
            continue;
        }
        
        CodeState state(i);
        
        // 模拟排序算法
        if (isSorting) {
            // 生成不同的排序状态
            QVector<QVariant> currentArray = mockArray;
            
            if (i > functionBodyStart + 2) {
                int sortedItems = qMin(arraySize, (i - functionBodyStart) / 2);
                
                // 为了演示，将前面的部分排序好
                std::sort(currentArray.begin(), currentArray.end(), [](const QVariant &a, const QVariant &b) {
                    return a.toInt() < b.toInt();
                });
                
                // 如果是冒泡排序，创建交换动画
                if (functionName.contains("bubble", Qt::CaseInsensitive)) {
                    QMap<QString, QVariant> customData;
                    customData["comparingIndices"] = QVariantList({sortedItems - 1, sortedItems});
                    if (sortedItems > 0 && sortedItems < arraySize) {
                        state.customData = customData;
                    }
                }
            }
            
            state.arrayData = currentArray;
        } 
        // 模拟搜索算法
        else if (isSearch) {
            QVector<QVariant> currentArray = mockArray;
            
            // 排序数组（如果是二分查找需要）
            if (functionName.contains("binary", Qt::CaseInsensitive)) {
                std::sort(currentArray.begin(), currentArray.end(), [](const QVariant &a, const QVariant &b) {
                    // 通用比较（假设数据类型可转换为 QString 或 int）
                    if (a.userType() == QMetaType::QString && b.userType() == QMetaType::QString) {
                        return a.toString() < b.toString();
                    } else {
                        return a.toInt() < b.toInt();
                    }
                });
            }
            
            int searchProgress = qMin(arraySize, (i - functionBodyStart));
            
            QMap<QString, QVariant> customData;
            customData["searchingIndex"] = searchProgress - 1;
            if (searchProgress > 0 && searchProgress <= arraySize) {
                state.customData = customData;
            }
            
            state.arrayData = currentArray;
        }
        // 树操作的可视化
        else if (isTree) {
            // 模拟树操作
            QMap<QString, QVariant> treeState;
            treeState["type"] = "binary_tree";
            
            QMap<QString, QVariant> rootNode;
            rootNode["value"] = 50;
            
            // 模拟树的成长
            int progress = (i - functionBodyStart);
            if (progress > 1) {
                rootNode["left"] = 30;
                
                QMap<QString, QVariant> leftNode;
                leftNode["value"] = 30;
                
                if (progress > 2) {
                    leftNode["left"] = 20;
                }
                
                if (progress > 3) {
                    leftNode["right"] = 40;
                }
                
                treeState["node_30"] = leftNode;
            }
            
            if (progress > 4) {
                rootNode["right"] = 70;
                
                QMap<QString, QVariant> rightNode;
                rightNode["value"] = 70;
                
                if (progress > 5) {
                    rightNode["left"] = 60;
                }
                
                if (progress > 6) {
                    rightNode["right"] = 80;
                }
                
                treeState["node_70"] = rightNode;
            }
            
            treeState["root"] = rootNode;
            
            // 当前操作节点
            if (progress > 0 && progress < 8) {
                treeState["currentNode"] = progress > 4 ? 70 : 30;
            }
            
            state.customData["treeData"] = treeState;
        }
        // 图操作的可视化
        else if (isGraph) {
            // 模拟图遍历/操作
            QMap<QString, QVariant> graphState;
            graphState["type"] = "directed_graph";
            
            QVariantList vertices;
            for (int v = 0; v < 6; ++v) {
                vertices.append(v);
            }
            
            QVariantList edges;
            // 添加一些边
            edges.append(QVariantList({0, 1}));
            edges.append(QVariantList({0, 2}));
            edges.append(QVariantList({1, 3}));
            edges.append(QVariantList({2, 3}));
            edges.append(QVariantList({2, 4}));
            edges.append(QVariantList({3, 5}));
            edges.append(QVariantList({4, 5}));
            
            graphState["vertices"] = vertices;
            graphState["edges"] = edges;
            
            // 当前访问的节点
            int progress = (i - functionBodyStart) % 6;
            graphState["currentVertex"] = progress;
            
            // 已访问的节点
            QVariantList visited;
            for (int v = 0; v < progress; ++v) {
                visited.append(v);
            }
            graphState["visitedVertices"] = visited;
            
            state.customData["graphData"] = graphState;
        }
        // 链表操作的可视化
        else if (isLinkedList) {
            // 模拟链表操作
            QVariantList listData;
            for (int n = 0; n < 8; ++n) {
                listData.append(10 + n * 10);
            }
            
            // 模拟链表的遍历/操作
            int progress = (i - functionBodyStart) % 8;
            
            QMap<QString, QVariant> customData;
            customData["listData"] = listData;
            customData["type"] = "linked_list";
            customData["currentNode"] = progress;
            
            state.customData = customData;
        }
        // 通用数组算法
        else if (hasArray) {
            QVector<QVariant> currentArray = mockArray;
            
            // 创建一些针对数组的操作
            int progress = (i - functionBodyStart);
            
            if (forLoops > 0 || whileLoops > 0) {
                progress = progress % arraySize;
                QMap<QString, QVariant> customData;
                customData["currentIndex"] = progress;
                
                if (progress > 0 && code.contains("swap") && progress < arraySize - 1) {
                    // 交换相邻元素以模拟某种操作
                    QVariant temp = currentArray[progress];
                    currentArray[progress] = currentArray[progress + 1];
                    currentArray[progress + 1] = temp;
                    
                    customData["swappingIndices"] = QVariantList({progress, progress + 1});
                }
                
                state.customData = customData;
            }
            
            state.arrayData = currentArray;
        }
        
        emit executionStep(state);
        
        // 模拟执行延迟
        for (int sleep = 0; sleep < 10 && isRunning; ++sleep) {
            QCoreApplication::processEvents();
            QThread::msleep(10);
        }
    }
    
    // 执行完成
    if (isRunning) {
        emit executionComplete();
    }
}

// 执行代码
void CodeExecutor::executeCode()
{
    try {
        // 解析代码
        CodeParser::ParseResult parseResult = CodeParser::parse(code);
        if (!parseResult.success) {
            emit executionError("解析代码失败");
            return;
        }

        // 初始化状态
        currentState = CodeState();
        currentState.currentLine = 0;
        currentState.customData = parseResult.customData;
        
        // 转换数组数据格式
        QVector<QVariant> arrayData;
        if (!parseResult.arrayData.isEmpty()) {
            QString arrayName = parseResult.arrayData.keys().first();
            const QVector<int>& values = parseResult.arrayData[arrayName];
            for (int value : values) {
                arrayData.append(value);
            }
        }
        currentState.arrayData = arrayData;
        
        emit executionStep(currentState);

        // 执行代码
        for (const QString &line : parseResult.codeLines) {
            executeLine(line);
            
            // 检查控制流
            if (currentState.customData.contains("controlFlow")) {
                QString controlFlow = currentState.customData["controlFlow"].toString();
                QString condition = currentState.customData["condition"].toString();
                
                if (controlFlow == "if" || controlFlow == "while") {
                    bool result = evaluateCondition(condition);
                    if (!result) {
                        // 跳过控制流块
                        while (currentState.currentLine < parseResult.codeLines.size() - 1) {
                            currentState.currentLine++;
                            if (parseResult.codeLines[currentState.currentLine].contains("}")) {
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        emit executionFinished();
    } catch (const std::exception &e) {
        emit executionError(QString("执行代码时出错: %1").arg(e.what()));
    }
}

// 执行线程构造函数
CodeExecutor::ExecutorThread::ExecutorThread(CodeExecutor *exec)
    : executor(exec)
    , stopRequested(false)
{
}

// 执行线程运行函数
void CodeExecutor::ExecutorThread::run()
{
    mutex.lock();
    stopRequested = false;
    mutex.unlock();
    
    executor->executeCode();
}

// 停止执行线程
void CodeExecutor::ExecutorThread::stop()
{
    mutex.lock();
    stopRequested = true;
    mutex.unlock();
}

void CodeExecutor::updateVariable(const QString &name, const QVariant &value)
{
    variableValues[name] = value;
    currentState.variables[name] = value;
    emit executionStep(currentState);
}

void CodeExecutor::executeLine(const QString &line)
{
    // 更新当前行号
    currentState.currentLine++;
    emit executionStep(currentState);
    
    // 解析并执行代码行
    QRegularExpression assignRegex("([a-zA-Z_][a-zA-Z0-9_]*)\\s*=\\s*([^;]+);");
    QRegularExpressionMatch match = assignRegex.match(line);
    
    if (match.hasMatch()) {
        QString varName = match.captured(1);
        QString expression = match.captured(2);
        
        // 简单的表达式求值
        bool ok;
        int value = expression.toInt(&ok);
        if (ok) {
            updateVariable(varName, value);
        }
    }
}

bool CodeExecutor::evaluateCondition(const QString &condition)
{
    // 简单的条件求值
    QRegularExpression compareRegex("([a-zA-Z_][a-zA-Z0-9_]*)\\s*(==|!=|>|<|>=|<=)\\s*([0-9]+)");
    QRegularExpressionMatch match = compareRegex.match(condition);
    
    if (match.hasMatch()) {
        QString varName = match.captured(1);
        QString op = match.captured(2);
        int value = match.captured(3).toInt();
        
        int varValue = variableValues[varName].toInt();
        
        if (op == "==") return varValue == value;
        if (op == "!=") return varValue != value;
        if (op == ">") return varValue > value;
        if (op == "<") return varValue < value;
        if (op == ">=") return varValue >= value;
        if (op == "<=") return varValue <= value;
    }
    
    return false;
} 
