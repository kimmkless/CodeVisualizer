#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QMessageBox>
#include <QStatusBar>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentStateIndex(0)
    , animationSpeed(500)
    , isExecutionRunning(false)
{
    ui->setupUi(this);
    setupUI();
    connectSignals();
    
    setWindowTitle("代码可视化工具");
    resize(1200, 800);
    
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateVisualization);
    
    executor = new CodeExecutor(this);
    connect(executor, &CodeExecutor::executionStep, this, &MainWindow::onExecutionStep);
    connect(executor, &CodeExecutor::executionFinished, this, &MainWindow::onExecutionComplete);
    connect(executor, &CodeExecutor::executionError, this, &MainWindow::onExecutionError);
    
    statusBar()->showMessage(QString::fromUtf8("准备就绪。请输入C++函数代码，然后单击运行按钮。"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 创建中央部件和主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建分割器，左侧是代码编辑器，右侧是可视化区域
    QSplitter *splitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // 创建代码编辑器
    codeEditor = new CodeEditor(splitter);
    codeEditor->setPlaceholderText("在此输入完整的C++函数代码...\n\n例如：\nvoid bubbleSort(int arr[], int n) {\n    for (int i = 0; i < n-1; i++) {\n        for (int j = 0; j < n-i-1; j++) {\n            if (arr[j] > arr[j+1]) {\n                int temp = arr[j];\n                arr[j] = arr[j+1];\n                arr[j+1] = temp;\n            }\n        }\n    }\n}");
    
    // 创建可视化区域
    visualizer = new Visualizer(splitter);
    
    // 添加到分割器
    splitter->addWidget(codeEditor);
    splitter->addWidget(visualizer);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    
    // 创建控制面板
    QWidget *controlPanel = new QWidget(centralWidget);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    
    // 可视化类型选择下拉框
    QLabel *visualTypeLabel = new QLabel("可视化类型:", controlPanel);
    QComboBox *visualTypeCombo = new QComboBox(controlPanel);
    visualTypeCombo->addItem("自动检测");
    visualTypeCombo->addItem("数组/向量");
    visualTypeCombo->addItem("链表");
    visualTypeCombo->addItem("树结构");
    visualTypeCombo->addItem("图结构");
    visualTypeCombo->addItem("矩阵");
    controlLayout->addWidget(visualTypeLabel);
    controlLayout->addWidget(visualTypeCombo);
    
    // 添加控制按钮
    runButton = new QPushButton("运行", controlPanel);
    stepButton = new QPushButton("单步执行", controlPanel);
    stopButton = new QPushButton("停止", controlPanel);
    speedSlider = new QSlider(Qt::Horizontal, controlPanel);
    visualTypeCombo = new QComboBox(controlPanel);
    controlLayout->addWidget(runButton);
    controlLayout->addWidget(stepButton);
    controlLayout->addWidget(stopButton);
    
    // 添加速度控制滑块
    QLabel *speedLabel = new QLabel("动画速度:", controlPanel);
    QSlider *speedSlider = new QSlider(Qt::Horizontal, controlPanel);
    speedSlider->setRange(1, 10);
    speedSlider->setValue(5);
    speedSlider->setTickPosition(QSlider::TicksBelow);
    speedSlider->setTickInterval(1);
    controlLayout->addWidget(speedLabel);
    controlLayout->addWidget(speedSlider);
    
    controlLayout->addStretch();
    
    // 添加到主布局
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(controlPanel);
    
    // 设置中央部件
    setCentralWidget(centralWidget);
    
}

void MainWindow::connectSignals()
{
    connect(runButton, &QPushButton::clicked, this, &MainWindow::onRunButtonClicked);
    connect(stepButton, &QPushButton::clicked, this, &MainWindow::onStepButtonClicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopButtonClicked);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedSliderValueChanged);
    connect(visualTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                if (visualizer) {
                    visualizer->setVisualizationType(static_cast<VisualizationType>(index));
                }
            });
}

void MainWindow::onRunButtonClicked()
{
    QString code = codeEditor->toPlainText();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先输入C++函数代码。");
        return;
    }
    
    resetVisualization();
    executor->setCode(code);
    executor->start();
    
    runButton->setEnabled(false);
    stepButton->setEnabled(false);
    stopButton->setEnabled(true);
    isExecutionRunning = true;
    
    statusBar()->showMessage("代码执行中...");
}

void MainWindow::onStepButtonClicked()
{
    if (executionStates.isEmpty()) {
        onRunButtonClicked();
        return;
    }
    
    if (currentStateIndex < executionStates.size() - 1) {
        currentStateIndex++;
        updateVisualization();
    }
}

void MainWindow::onStopButtonClicked()
{
    executor->stop();
    animationTimer->stop();
    isExecutionRunning = false;
    
    runButton->setEnabled(true);
    stepButton->setEnabled(!executionStates.isEmpty());
    stopButton->setEnabled(false);
    
    statusBar()->showMessage("执行已停止。");
}

void MainWindow::onSpeedSliderValueChanged(int value)
{
    // 速度值设置为 1100 - value*100，这样滑块值越大执行速度越快
    animationSpeed = 1100 - value * 100;
    
    if (animationTimer->isActive()) {
        animationTimer->setInterval(animationSpeed);
    }
}

void MainWindow::onExecutionStep(const CodeState& state)
{
    executionStates.append(state);
    
    if (!animationTimer->isActive() && isExecutionRunning) {
        currentStateIndex = 0;
        animationTimer->start(animationSpeed);
    }
}

void MainWindow::onExecutionComplete()
{
    statusBar()->showMessage("代码执行完成。");
    
    runButton->setEnabled(true);
    stepButton->setEnabled(!executionStates.isEmpty());
    stopButton->setEnabled(false);
    isExecutionRunning = false;
    
    // 如果没有启动动画，则显示最终结果
    if (!animationTimer->isActive() && !executionStates.isEmpty()) {
        currentStateIndex = executionStates.size() - 1;
        updateVisualization();
    }
}

void MainWindow::onExecutionError(const QString& error)
{
    QMessageBox::critical(this, "执行错误", "代码执行出错：\n" + error);
    
    runButton->setEnabled(true);
    stepButton->setEnabled(!executionStates.isEmpty());
    stopButton->setEnabled(false);
    isExecutionRunning = false;
    
    statusBar()->showMessage("执行出错。");
}

void MainWindow::updateVisualization()
{
    if (executionStates.isEmpty() || currentStateIndex >= executionStates.size()) {
        return;
    }
    
    const CodeState &state = executionStates[currentStateIndex];
    
    // 高亮当前执行行
    codeEditor->highlightLine(state.currentLine);
    
    // 更新可视化
    visualizer->updateVisualization(state);
    
    // 如果是自动运行模式，判断是否需要前进到下一个状态
    if (animationTimer->isActive()) {
        currentStateIndex++;
        
        if (currentStateIndex >= executionStates.size()) {
            animationTimer->stop();
            statusBar()->showMessage("动画播放完成。");
        }
    }
    
    // 更新状态栏
    statusBar()->showMessage(QString("执行步骤 %1/%2").arg(currentStateIndex + 1).arg(executionStates.size()));
}

void MainWindow::resetVisualization()
{
    executionStates.clear();
    currentStateIndex = 0;
    codeEditor->clearHighlight();
    visualizer->reset();
    animationTimer->stop();
    
    statusBar()->showMessage("准备就绪。");
} 
