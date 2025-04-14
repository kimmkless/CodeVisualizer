#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMap>
#include <QVector>
#include <QVariant>
#include "codeexecutor.h"

// 可视化类型枚举
enum class VisualizationType {
    Auto = 0,           // 自动检测
    Array = 1,          // 数组/向量
    LinkedList = 2,     // 链表
    Tree = 3,           // 树结构
    Graph = 4,          // 图结构
    Matrix = 5          // 矩阵
};

// 可视化组件类
class Visualizer : public QGraphicsView
{
    Q_OBJECT
    
public:
    explicit Visualizer(QWidget *parent = nullptr);
    ~Visualizer();
    
    void updateVisualization(const CodeState &state);
    void reset();
    void setVisualizationType(VisualizationType type);
    void updateState(const CodeState &state);
    
protected:
    void resizeEvent(QResizeEvent *event) override;
    
private:
    void visualizeArray(const QVector<QVariant> &array, const QMap<QString, QVariant> &customData);
    void visualizeLinkedList(const QMap<QString, QVariant> &customData);
    void visualizeTree(const QMap<QString, QVariant> &customData);
    void visualizeGraph(const QMap<QString, QVariant> &customData);
    void visualizeMatrix(const QVector<QVariant> &data, const QMap<QString, QVariant> &customData);
    void autoDetectVisualization(const CodeState &state);
    
    QGraphicsScene *scene;
    VisualizationType visualizationType;
    
    // 渲染设置
    int arrayElementWidth;
    int arrayElementHeight;
    int arrayElementSpacing;
    int nodeRadius;
    int nodeSpacing;
    int treeVerticalSpacing;
    int treeHorizontalSpacing;
    
    // 缓存上一个状态，用于动画过渡
    CodeState lastState;
};

#endif // VISUALIZER_H 