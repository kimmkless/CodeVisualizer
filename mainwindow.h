#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QPushButton>
#include<QSlider>
#include<QComboBox>
#include <QSplitter>
#include <QTimer>
#include <QVector>
#include "codeeditor.h"
#include "visualizer.h"
#include "codeexecutor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRunButtonClicked();
    void onStepButtonClicked();
    void onStopButtonClicked();
    void onSpeedSliderValueChanged(int value);
    void onExecutionStep(const CodeState& state);
    void onExecutionComplete();
    void onExecutionError(const QString& error);
    void updateVisualization();

private:
    void setupUI();
    void connectSignals();
    void resetVisualization();

    Ui::MainWindow *ui;
    CodeEditor *codeEditor;
    Visualizer *visualizer;
    CodeExecutor *executor;
    
    QVector<CodeState> executionStates;
    int currentStateIndex;
    QTimer *animationTimer;
    int animationSpeed;
    bool isExecutionRunning;

    QPushButton *runButton;
    QPushButton *stepButton;
    QPushButton *stopButton;
    QSlider *speedSlider;
    QComboBox *visualTypeCombo;
};

#endif // MAINWINDOW_H 
