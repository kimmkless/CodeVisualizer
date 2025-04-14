#include "visualizer.h"
#include <QtGui/QPainter>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QColor>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QDebug>
#include <cmath>
#include <QHash>

inline size_t qHash(const QVariant& key, size_t seed = 0) Q_DECL_NOTHROW
{
    return qHash(key.toString(), seed);
}

namespace std {
    template<>
    struct hash<QVariant> {
        size_t operator()(const QVariant& v) const {
            return qHash(v.toString());
        }
    };
}

// 可视化组件构造函数
Visualizer::Visualizer(QWidget *parent)
    : QGraphicsView(parent)
    , visualizationType(VisualizationType::Auto)
    , arrayElementWidth(50)
    , arrayElementHeight(40)
    , arrayElementSpacing(10)
    , nodeRadius(20)
    , nodeSpacing(60)
    , treeVerticalSpacing(80)
    , treeHorizontalSpacing(40)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    
    // 设置视图属性
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    setAlignment(Qt::AlignCenter);
    
    // 初始显示提示文本
    QGraphicsTextItem *hint = scene->addText("在此处显示代码执行的可视化效果\n\n请在左侧输入C++函数代码，然后单击'运行'按钮");
    QFont hintFont("Microsoft YaHei", 12);
    hint->setFont(hintFont);
    hint->setDefaultTextColor(Qt::gray);
    hint->setPos(-200, -50);
}

// 析构函数
Visualizer::~Visualizer()
{
    delete scene;
}

// 更新可视化
void Visualizer::updateVisualization(const CodeState &state)
{
    if (state.currentLine < 0) {
        return;
    }
    
    // 清除场景
    scene->clear();
    
    // 根据可视化类型选择不同的渲染方法
    if (visualizationType == VisualizationType::Auto) {
        autoDetectVisualization(state);
    } else if (visualizationType == VisualizationType::Array) {
        visualizeArray(state.arrayData, state.customData);
    } else if (visualizationType == VisualizationType::LinkedList) {
        visualizeLinkedList(state.customData);
    } else if (visualizationType == VisualizationType::Tree) {
        visualizeTree(state.customData);
    } else if (visualizationType == VisualizationType::Graph) {
        visualizeGraph(state.customData);
    } else if (visualizationType == VisualizationType::Matrix) {
        visualizeMatrix(state.arrayData, state.customData);
    }
    
    // 保存当前状态
    lastState = state;
    
    // 调整视图
    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-50, -50, 50, 50));
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

// 重置可视化
void Visualizer::reset()
{
    scene->clear();
    QGraphicsTextItem *hint = scene->addText("在此处显示代码执行的可视化效果\n\n请在左侧输入C++函数代码，然后单击'运行'按钮");
    QFont hintFont("Microsoft YaHei", 12);
    hint->setFont(hintFont);
    hint->setDefaultTextColor(Qt::gray);
    hint->setPos(-200, -50);
    
    // 重置上次状态
    lastState = CodeState();
}

// 设置可视化类型
void Visualizer::setVisualizationType(VisualizationType type)
{
    visualizationType = type;
    
    // 如果有上一个状态，使用新的可视化类型重新渲染
    if (lastState.currentLine >= 0) {
        updateVisualization(lastState);
    }
}

// 自动检测适合的可视化类型
void Visualizer::autoDetectVisualization(const CodeState &state)
{
    // 首先尝试从customData中获取信息
    if (state.customData.contains("type")) {
        QString type = state.customData["type"].toString();
        if (type == "binary_tree") {
            visualizeTree(state.customData);
            return;
        } else if (type == "linked_list") {
            visualizeLinkedList(state.customData);
            return;
        } else if (type.contains("graph")) {
            visualizeGraph(state.customData);
            return;
        }
    }
    
    // 检查是否有树数据
    if (state.customData.contains("treeData")) {
        visualizeTree(state.customData);
        return;
    }
    
    // 检查是否有图数据
    if (state.customData.contains("graphData")) {
        visualizeGraph(state.customData);
        return;
    }
    
    // 检查数组数据
    if (!state.arrayData.isEmpty()) {
        // 判断是一维数组还是二维数组（矩阵）
        bool isMatrix = false;
        for (const QVariant &item : state.arrayData) {
            if (item.canConvert<QVariantList>()) {
                isMatrix = true;
                break;
            }
        }
        
        if (isMatrix) {
            visualizeMatrix(state.arrayData, state.customData);
        } else {
            visualizeArray(state.arrayData, state.customData);
        }
        return;
    }
    
    // 默认使用数组可视化
    visualizeArray(state.arrayData, state.customData);
}

// 数组可视化
void Visualizer::visualizeArray(const QVector<QVariant> &array, const QMap<QString, QVariant> &customData)
{
    if (array.isEmpty()) {
        // 如果没有数组数据，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("数组为空");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    int size = array.size();
    int totalWidth = size * (arrayElementWidth + arrayElementSpacing) - arrayElementSpacing;
    int startX = -totalWidth / 2;
    int startY = -arrayElementHeight / 2;
    
    // 获取可能的高亮索引
    QVariantList comparingIndices;
    QVariantList swappingIndices;
    int searchingIndex = -1;
    int currentIndex = -1;
    
    if (customData.contains("comparingIndices")) {
        comparingIndices = customData["comparingIndices"].toList();
    }
    if (customData.contains("swappingIndices")) {
        swappingIndices = customData["swappingIndices"].toList();
    }
    if (customData.contains("searchingIndex")) {
        searchingIndex = customData["searchingIndex"].toInt();
    }
    if (customData.contains("currentIndex")) {
        currentIndex = customData["currentIndex"].toInt();
    }
    
    // 绘制数组元素
    for (int i = 0; i < size; ++i) {
        QVariant value = array[i];
        QGraphicsRectItem *rect = scene->addRect(
            startX + i * (arrayElementWidth + arrayElementSpacing),
            startY,
            arrayElementWidth,
            arrayElementHeight
        );
        
        // 设置元素外观
        QBrush brush(Qt::white);
        QPen pen(Qt::black, 1);
        
        // 高亮比较中的元素
        if (comparingIndices.contains(i)) {
            brush.setColor(QColor(255, 255, 200)); // 淡黄色
            pen.setColor(Qt::blue);
            pen.setWidth(2);
        }
        
        // 高亮交换中的元素
        if (swappingIndices.contains(i)) {
            brush.setColor(QColor(255, 200, 200)); // 淡红色
            pen.setColor(Qt::red);
            pen.setWidth(2);
        }
        
        // 高亮搜索中的元素
        if (i == searchingIndex) {
            brush.setColor(QColor(200, 255, 200)); // 淡绿色
            pen.setColor(Qt::darkGreen);
            pen.setWidth(2);
        }
        
        // 高亮当前索引
        if (i == currentIndex) {
            brush.setColor(QColor(200, 200, 255)); // 淡蓝色
            pen.setColor(Qt::darkBlue);
            pen.setWidth(2);
        }
        
        rect->setBrush(brush);
        rect->setPen(pen);
        
        // 添加值文本
        QGraphicsTextItem *textItem = scene->addText(value.toString());
        textItem->setDefaultTextColor(Qt::black);
        
        // 居中文本
        QFontMetrics fm(textItem->font());
        int textWidth = fm.horizontalAdvance(value.toString());
        int textHeight = fm.height();
        
        textItem->setPos(
            startX + i * (arrayElementWidth + arrayElementSpacing) + (arrayElementWidth - textWidth) / 2,
            startY + (arrayElementHeight - textHeight) / 2
        );
        
        // 添加索引标签
        QGraphicsTextItem *indexItem = scene->addText(QString::number(i));
        indexItem->setDefaultTextColor(Qt::gray);
        indexItem->setPos(
            startX + i * (arrayElementWidth + arrayElementSpacing) + arrayElementWidth / 2 - 5,
            startY + arrayElementHeight + 5
        );
    }
}

// 链表可视化
void Visualizer::visualizeLinkedList(const QMap<QString, QVariant> &customData)
{
    if (!customData.contains("listData")) {
        // 如果没有链表数据，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("链表为空");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    QVariantList listData = customData["listData"].toList();
    int currentNode = -1;
    
    if (customData.contains("currentNode")) {
        currentNode = customData["currentNode"].toInt();
    }
    
    int size = listData.size();
    int totalWidth = size * (nodeRadius * 2 + nodeSpacing) - nodeSpacing;
    int startX = -totalWidth / 2 + nodeRadius;
    int startY = 0;
    
    for (int i = 0; i < size; ++i) {
        QVariant value = listData[i];
        
        // 绘制节点
        QGraphicsEllipseItem *node = scene->addEllipse(
            startX + i * (nodeRadius * 2 + nodeSpacing) - nodeRadius,
            startY - nodeRadius,
            nodeRadius * 2,
            nodeRadius * 2
        );
        
        // 设置节点外观
        QBrush brush(Qt::white);
        QPen pen(Qt::black, 1);
        
        // 高亮当前节点
        if (i == currentNode) {
            brush.setColor(QColor(200, 200, 255)); // 淡蓝色
            pen.setColor(Qt::blue);
            pen.setWidth(2);
        }
        
        node->setBrush(brush);
        node->setPen(pen);
        
        // 添加值文本
        QGraphicsTextItem *textItem = scene->addText(value.toString());
        textItem->setDefaultTextColor(Qt::black);
        
        // 居中文本
        QFontMetrics fm(textItem->font());
        int textWidth = fm.horizontalAdvance(value.toString());
        int textHeight = fm.height();
        
        textItem->setPos(
            startX + i * (nodeRadius * 2 + nodeSpacing) - textWidth / 2,
            startY - textHeight / 2
        );
        
        // 绘制指针箭头（如果不是最后一个节点）
        if (i < size - 1) {
            // 绘制箭头线
            QGraphicsLineItem *line = scene->addLine(
                startX + i * (nodeRadius * 2 + nodeSpacing) + nodeRadius,
                startY,
                startX + (i + 1) * (nodeRadius * 2 + nodeSpacing) - nodeRadius - 10,
                startY
            );
            
            // 绘制箭头头部
            QPolygonF arrowHead;
            arrowHead << QPointF(startX + (i + 1) * (nodeRadius * 2 + nodeSpacing) - nodeRadius - 10, startY - 5)
                     << QPointF(startX + (i + 1) * (nodeRadius * 2 + nodeSpacing) - nodeRadius, startY)
                     << QPointF(startX + (i + 1) * (nodeRadius * 2 + nodeSpacing) - nodeRadius - 10, startY + 5);
            
            QGraphicsPolygonItem *arrow = scene->addPolygon(arrowHead);
            
            pen.setColor(Qt::black);
            pen.setWidth(1);
            line->setPen(pen);
            arrow->setPen(pen);
            arrow->setBrush(Qt::black);
        }
    }
    
    // 添加NULL终止符
    if (size > 0) {
        QGraphicsTextItem *nullItem = scene->addText("NULL");
        nullItem->setDefaultTextColor(Qt::red);
        nullItem->setPos(
            startX + size * (nodeRadius * 2 + nodeSpacing),
            startY - nullItem->boundingRect().height() / 2
        );
        
        // 绘制最后一个节点到NULL的箭头
        QGraphicsLineItem *line = scene->addLine(
            startX + (size - 1) * (nodeRadius * 2 + nodeSpacing) + nodeRadius,
            startY,
            startX + size * (nodeRadius * 2 + nodeSpacing) - 10,
            startY
        );
        
        // 绘制箭头头部
        QPolygonF arrowHead;
        arrowHead << QPointF(startX + size * (nodeRadius * 2 + nodeSpacing) - 10, startY - 5)
                 << QPointF(startX + size * (nodeRadius * 2 + nodeSpacing), startY)
                 << QPointF(startX + size * (nodeRadius * 2 + nodeSpacing) - 10, startY + 5);
        
        QGraphicsPolygonItem *arrow = scene->addPolygon(arrowHead);
        
        QPen pen(Qt::black, 1);
        line->setPen(pen);
        arrow->setPen(pen);
        arrow->setBrush(Qt::black);
    }
}

// 树结构可视化
void Visualizer::visualizeTree(const QMap<QString, QVariant> &customData)
{
    QMap<QString, QVariant> treeData;
    
    // 获取树数据
    if (customData.contains("treeData")) {
        treeData = customData["treeData"].toMap();
    } else {
        // 如果没有树数据，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("树为空");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    // 获取根节点
    QMap<QString, QVariant> rootNode;
    if (treeData.contains("root")) {
        rootNode = treeData["root"].toMap();
    } else {
        // 如果没有根节点，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("树为空（无根节点）");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    // 获取当前操作的节点
    QVariant currentNodeValue;
    if (treeData.contains("currentNode")) {
        currentNodeValue = treeData["currentNode"];
    }
    
    // 递归绘制树结构
    struct TreeNode {
        QVariant value;
        QMap<QString, QVariant> data;
        int x;
        int y;
        int level;
        bool isCurrent;
    };
    
    // 构建节点
    std::function<void(TreeNode&, int, int, int)> buildTree = [&](TreeNode &node, int x, int y, int level) {
        node.x = x;
        node.y = y;
        node.level = level;
        node.isCurrent = (currentNodeValue.isValid() && node.value == currentNodeValue);
        
        // 绘制节点
        QGraphicsEllipseItem *circle = scene->addEllipse(
            x - nodeRadius,
            y - nodeRadius,
            nodeRadius * 2,
            nodeRadius * 2
        );
        
        // 设置节点外观
        QBrush brush(Qt::white);
        QPen pen(Qt::black, 1);
        
        // 高亮当前节点
        if (node.isCurrent) {
            brush.setColor(QColor(200, 200, 255)); // 淡蓝色
            pen.setColor(Qt::blue);
            pen.setWidth(2);
        }
        
        circle->setBrush(brush);
        circle->setPen(pen);
        
        // 添加值文本
        QGraphicsTextItem *textItem = scene->addText(node.value.toString());
        textItem->setDefaultTextColor(Qt::black);
        
        // 居中文本
        QFontMetrics fm(textItem->font());
        int textWidth = fm.horizontalAdvance(node.value.toString());
        int textHeight = fm.height();
        
        textItem->setPos(
            x - textWidth / 2,
            y - textHeight / 2
        );
        
        // 处理左子节点
        if (node.data.contains("left") && node.data["left"].isValid()) {
            QVariant leftValue = node.data["left"];
            int leftX = x - treeHorizontalSpacing * (1 << (3 - level));
            int leftY = y + treeVerticalSpacing;
            
            // 绘制连接线
            QPen linePen(Qt::black, 1);
            QGraphicsLineItem *line = scene->addLine(x, y + nodeRadius, leftX, leftY - nodeRadius);
            line->setPen(linePen);
            
            // 递归绘制左子节点
            TreeNode leftNode;
            leftNode.value = leftValue;
            
            // 获取左子节点的完整数据
            QString leftNodeKey = QString("node_%1").arg(leftValue.toString());
            if (treeData.contains(leftNodeKey)) {
                leftNode.data = treeData[leftNodeKey].toMap();
            }
            
            buildTree(leftNode, leftX, leftY, level + 1);
        }
        
        // 处理右子节点
        if (node.data.contains("right") && node.data["right"].isValid()) {
            QVariant rightValue = node.data["right"];
            int rightX = x + treeHorizontalSpacing * (1 << (3 - level));
            int rightY = y + treeVerticalSpacing;
            
            // 绘制连接线
            QPen linePen(Qt::black, 1);
            QGraphicsLineItem *line = scene->addLine(x, y + nodeRadius, rightX, rightY - nodeRadius);
            line->setPen(linePen);
            
            // 递归绘制右子节点
            TreeNode rightNode;
            rightNode.value = rightValue;
            
            // 获取右子节点的完整数据
            QString rightNodeKey = QString("node_%1").arg(rightValue.toString());
            if (treeData.contains(rightNodeKey)) {
                rightNode.data = treeData[rightNodeKey].toMap();
            }
            
            buildTree(rightNode, rightX, rightY, level + 1);
        }
    };
    
    // 从根节点开始绘制
    TreeNode root;
    root.value = rootNode["value"];
    root.data = rootNode;
    buildTree(root, 0, -100, 0);
}

// 图结构可视化
void Visualizer::visualizeGraph(const QMap<QString, QVariant> &customData)
{
    QMap<QString, QVariant> graphData;
    
    // 获取图数据
    if (customData.contains("graphData")) {
        graphData = customData["graphData"].toMap();
    } else {
        // 如果没有图数据，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("图为空");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    // 获取顶点和边
    QVariantList vertices;
    QVariantList edges;
    QVariant currentVertex;
    QVariantList visitedVertices;
    
    if (graphData.contains("vertices")) {
        vertices = graphData["vertices"].toList();
    }
    
    if (graphData.contains("edges")) {
        edges = graphData["edges"].toList();
    }
    
    if (graphData.contains("currentVertex")) {
        currentVertex = graphData["currentVertex"];
    }
    
    if (graphData.contains("visitedVertices")) {
        visitedVertices = graphData["visitedVertices"].toList();
    }
    
    if (vertices.isEmpty()) {
        // 如果没有顶点，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("图为空（无顶点）");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    // 计算顶点位置（圆形布局）
    QHash<QVariant, QPointF> vertexPositions;  // 使用 QHash 替代 QMap
    int numVertices = vertices.size();
    double radius = numVertices * 25;
    
    for (int i = 0; i < numVertices; ++i) {
        double angle = 2 * M_PI * i / numVertices;
        double x = radius * std::cos(angle);
        double y = radius * std::sin(angle);
        vertexPositions[vertices[i]] = QPointF(x, y);
    }
    
    // 先绘制边
    for (const QVariant &edgeVar : edges) {
        QVariantList edge = edgeVar.toList();
        if (edge.size() < 2) continue;
        
        QVariant from = edge[0];
        QVariant to = edge[1];
        
        if (!vertexPositions.contains(from) || !vertexPositions.contains(to)) continue;
        
        QPointF fromPos = vertexPositions[from];
        QPointF toPos = vertexPositions[to];
        
        // 计算有向边的箭头
        double angle = std::atan2(toPos.y() - fromPos.y(), toPos.x() - fromPos.x());
        double arrowLength = nodeRadius;
        QPointF arrowTip(toPos.x() - arrowLength * std::cos(angle), 
                         toPos.y() - arrowLength * std::sin(angle));
        
        // 绘制边
        QGraphicsLineItem *line = scene->addLine(
            fromPos.x(), fromPos.y(),
            arrowTip.x(), arrowTip.y()
        );
        
        // 设置边的样式
        QPen pen(Qt::gray, 1);
        line->setPen(pen);
        
        // 绘制箭头
        QPointF arrowP1(arrowTip.x() - 10 * std::cos(angle - M_PI / 6),
                       arrowTip.y() - 10 * std::sin(angle - M_PI / 6));
        QPointF arrowP2(arrowTip.x() - 10 * std::cos(angle + M_PI / 6),
                       arrowTip.y() - 10 * std::sin(angle + M_PI / 6));
        
        QPolygonF arrowHead;
        arrowHead << arrowTip << arrowP1 << arrowP2;
        
        QGraphicsPolygonItem *arrow = scene->addPolygon(arrowHead);
        arrow->setPen(pen);
        arrow->setBrush(Qt::gray);
    }
    
    // 绘制顶点
    for (const QVariant &vertex : vertices) {
        QPointF pos = vertexPositions[vertex];
        
        // 绘制顶点
        QGraphicsEllipseItem *circle = scene->addEllipse(
            pos.x() - nodeRadius,
            pos.y() - nodeRadius,
            nodeRadius * 2,
            nodeRadius * 2
        );
        
        // 设置顶点外观
        QBrush brush(Qt::white);
        QPen pen(Qt::black, 1);
        
        // 高亮当前顶点
        if (currentVertex.isValid() && vertex == currentVertex) {
            brush.setColor(QColor(200, 200, 255)); // 淡蓝色
            pen.setColor(Qt::blue);
            pen.setWidth(2);
        }
        
        // 高亮已访问顶点
        if (visitedVertices.contains(vertex)) {
            brush.setColor(QColor(200, 255, 200)); // 淡绿色
        }
        
        circle->setBrush(brush);
        circle->setPen(pen);
        
        // 添加值文本
        QGraphicsTextItem *textItem = scene->addText(vertex.toString());
        textItem->setDefaultTextColor(Qt::black);
        
        // 居中文本
        QFontMetrics fm(textItem->font());
        int textWidth = fm.horizontalAdvance(vertex.toString());
        int textHeight = fm.height();
        
        textItem->setPos(
            pos.x() - textWidth / 2,
            pos.y() - textHeight / 2
        );
    }
}

// 矩阵可视化
void Visualizer::visualizeMatrix(const QVector<QVariant> &data, const QMap<QString, QVariant> &customData)
{
    if (data.isEmpty()) {
        // 如果没有矩阵数据，显示默认的空状态
        QGraphicsTextItem *emptyText = scene->addText("矩阵为空");
        emptyText->setDefaultTextColor(Qt::gray);
        emptyText->setPos(-50, -20);
        return;
    }
    
    // 检查是否是矩阵数据结构
    int rows = data.size();
    int cols = 0;
    
    // 获取列数
    for (const QVariant &row : data) {
        if (row.canConvert<QVariantList>()) {
            cols = qMax(cols, row.toList().size());
        } else {
            // 一维数组，按行显示
            cols = data.size();
            rows = 1;
            break;
        }
    }
    
    // 计算矩阵的总宽度和高度
    int totalWidth = cols * (arrayElementWidth + arrayElementSpacing) - arrayElementSpacing;
    int totalHeight = rows * (arrayElementHeight + arrayElementSpacing) - arrayElementSpacing;
    int startX = -totalWidth / 2;
    int startY = -totalHeight / 2;
    
    // 绘制矩阵
    for (int i = 0; i < rows; ++i) {
        QVariantList rowData;
        
        if (rows == 1) {
            // 一维数组
            for (const QVariant &item : data) {
                rowData.append(item);
            }
        } else {
            // 二维数组
            if (data[i].canConvert<QVariantList>()) {
                rowData = data[i].toList();
            }
        }
        
        for (int j = 0; j < rowData.size() && j < cols; ++j) {
            QVariant value = rowData[j];
            
            // 绘制矩阵元素
            QGraphicsRectItem *rect = scene->addRect(
                startX + j * (arrayElementWidth + arrayElementSpacing),
                startY + i * (arrayElementHeight + arrayElementSpacing),
                arrayElementWidth,
                arrayElementHeight
            );
            
            // 设置元素外观
            rect->setBrush(QBrush(Qt::white));
            rect->setPen(QPen(Qt::black, 1));
            
            // 添加值文本
            QGraphicsTextItem *textItem = scene->addText(value.toString());
            textItem->setDefaultTextColor(Qt::black);
            
            // 居中文本
            QFontMetrics fm(textItem->font());
            int textWidth = fm.horizontalAdvance(value.toString());
            int textHeight = fm.height();
            
            textItem->setPos(
                startX + j * (arrayElementWidth + arrayElementSpacing) + (arrayElementWidth - textWidth) / 2,
                startY + i * (arrayElementHeight + arrayElementSpacing) + (arrayElementHeight - textHeight) / 2
            );
        }
    }
    
    // 添加行列标签
    for (int i = 0; i < rows; ++i) {
        QGraphicsTextItem *rowLabel = scene->addText(QString::number(i));
        rowLabel->setDefaultTextColor(Qt::gray);
        rowLabel->setPos(
            startX - 20,
            startY + i * (arrayElementHeight + arrayElementSpacing) + arrayElementHeight / 2 - 10
        );
    }
    
    for (int j = 0; j < cols; ++j) {
        QGraphicsTextItem *colLabel = scene->addText(QString::number(j));
        colLabel->setDefaultTextColor(Qt::gray);
        colLabel->setPos(
            startX + j * (arrayElementWidth + arrayElementSpacing) + arrayElementWidth / 2 - 5,
            startY - 20
        );
    }
}

// 窗口大小改变事件
void Visualizer::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    
    if (scene->items().isEmpty()) {
        return;
    }
    
    // 调整视图以适应内容
    fitInView(scene->itemsBoundingRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
}

void Visualizer::updateState(const CodeState &state)
{
    if (!state.arrayData.isEmpty()) {
        // 直接使用 arrayData 和 customData
        visualizeArray(state.arrayData, state.customData);
    } else {
        // 显示数组为空的消息
        QPainter painter(this);
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, "数组为空");
    }
    
    // 更新其他可视化元素
    update();
} 
