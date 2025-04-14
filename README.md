# 代码可视化工具

这是一个基于Qt的C++代码可视化工具，可以将用户输入的C++函数代码转换为可视化的动画或图片，展示代码执行的过程。

## 功能特点

- 支持输入完整的C++函数代码
- 实时解析和执行代码
- 根据代码内容自动检测适合的可视化类型
- 支持多种数据结构的可视化：
  - 数组/向量
  - 链表
  - 树结构
  - 图结构
  - 矩阵
- 可以单步执行和连续播放代码动画
- 可调节动画播放速度
- 代码编辑器支持语法高亮和行号显示

## 环境要求

- Qt 5.12或更高版本
- C++11或更高版本的编译器

## 编译和运行

1. 使用Qt Creator打开`CodeVisualizer.pro`项目文件
2. 配置项目
3. 编译和运行项目

或者使用命令行：

```bash
cd CodeVisualizer
qmake
make
./CodeVisualizer
```

## 使用方法

1. 在左侧代码编辑器中输入一个完整的C++函数
2. 选择合适的可视化类型，或使用"自动检测"
3. 点击"运行"按钮开始可视化
4. 使用"单步执行"或调整动画速度控制可视化过程

## 示例代码

```cpp
// 冒泡排序
void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                int temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}
```

```cpp
// 二叉树遍历
void inorderTraversal(TreeNode* root) {
    if (root == nullptr) {
        return;
    }
    inorderTraversal(root->left);
    visit(root->value);
    inorderTraversal(root->right);
}
```

## 许可证

MIT

## 作者

CodeVisualizer Team 