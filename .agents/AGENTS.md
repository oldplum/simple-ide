# Simple IDE — 项目指南（AI 助手须知）

## 项目概述

这是一个使用 **Qt (C++)** 开发的轻量级跨平台代码编辑器，核心特性是为多种编程语言提供**实时语法高亮**。

## 技术栈

- **语言**：C++17
- **框架**：Qt（QtWidgets / QtGui / QtCore）
- **构建系统**：qmake（`.pro` 文件）
- **核心 Qt 类**：
  - `QPlainTextEdit` — 主编辑控件（`CodeEditor` 继承自此）
  - `QSyntaxHighlighter` — 语法高亮引擎（`Highlighter` 继承自此）
  - `QRegularExpression` — 正则表达式驱动的词法分析
  - `QTextCharFormat` — 文本着色格式
  - `QMainWindow`, `QTabWidget` — 主界面框架（尚未实现）

## 目录结构

```
simple_ide/
├── src/                 # .cpp 源文件
│   ├── main.cpp         # 程序入口
│   ├── CodeEditor.cpp   # 编辑器控件实现
│   ├── Highlighter.cpp  # 语法高亮实现（空文件，待开发）
│   └── CatWidget.cpp    # 电子猫咪控件实现（待开发）
├── include/             # .h 头文件
│   ├── CodeEditor.h     # 编辑器控件声明
│   ├── Highlighter.h    # 语法高亮声明
│   └── CatWidget.h      # 电子猫咪控件声明（待开发）
├── ui/                  # Qt Designer 的 .ui 文件（暂空）
├── resources/           # 静态资源
│   └── cat_gifs/        # 猫咪各情绪状态的 GIF 动图
├── docs/                # 项目文档
│   ├── 项目要求.md       # 完整功能需求
│   ├── 项目分工.md       # 双人开发分工策略
│   └── 项目目录结构.md   # 目录结构说明
├── build/               # 构建输出目录（已 gitignore）
└── simple_ide.pro       # qmake 项目文件
```

## 构建与运行

```bash
# 在项目根目录下
mkdir -p build && cd build
qmake ../simple_ide.pro
make -j$(nproc)
./simple_ide
```

> **注意**：构建产物应放在 `build/` 目录中，不要在项目根目录直接构建。

## 编码规范

### 命名风格
- **类名**：大驼峰（PascalCase），如 `CodeEditor`, `Highlighter`
- **成员变量**：`m_` 前缀 + 小驼峰，如 `m_rules`
- **槽函数 (slots)**：小驼峰，如 `highlightCurrentline()`
- **文件命名**：与类名一致，头文件放 `include/`，源文件放 `src/`

### 代码风格
- 使用 C++17 标准
- 花括号风格：函数和类定义的 `{` 另起一行，控制流语句的 `{` 跟在同一行（参照现有代码）
- 头文件使用 `#ifndef` / `#define` / `#endif` 守卫（非 `#pragma once`）
- Qt 信号槽连接使用新语法（函数指针风格）：
  ```cpp
  connect(sender, &SenderClass::signal, receiver, &ReceiverClass::slot);
  ```

## 架构要点

### 核心类关系
```
QMainWindow
  ├── QTabWidget (多标签页)
  │     └── CodeEditor (继承自 QPlainTextEdit)
  │           └── Highlighter (继承自 QSyntaxHighlighter，绑定到 document)
  └── CatWidget (电子猫咪，QLabel + QMovie + 状态机)
```

### 已实现功能
- [x] `CodeEditor` 基础编辑控件
- [x] 当前行高亮（浅灰色背景 `rgb(232,232,232)`）

### 待实现功能（按优先级）
- [ ] `Highlighter::highlightBlock()` — 正则表达式逐行词法分析
- [ ] 内置 C++、Python、JSON 三种语言的高亮规则
- [ ] 行号显示（左侧行号栏，与文本滚动同步）
- [ ] `MainWindow` + `QTabWidget` 多标签页界面
- [ ] 文件操作（新建 / 打开 / 保存 / 另存为）
- [ ] 自动语言检测（根据文件扩展名）
- [ ] 查找与替换对话框（支持正则、全词匹配）
- [ ] 代码自动缩进（回车继承上一行缩进）
- [ ] 括号匹配高亮
- [ ] 主题切换（明亮 / 暗黑）
- [ ] 未保存修改提示
- [ ] 从外部 JSON/XML 加载高亮规则（可扩展性）
- [ ] 编码支持（UTF-8 / GBK）
- [ ] 会话恢复（可选）
- [ ] 代码折叠（可选高级功能）
- [ ] 电子猫咪互动系统（`CatWidget`：QLabel + QMovie + 有限状态机）
  - [ ] 猫咪 GIF 动画界面（悬浮于编辑器右下角或 QDockWidget 侧边栏）
  - [ ] 情绪状态机（默认/开心/生气/饥饿 四种状态）
  - [ ] 互动触发逻辑（括号匹配→开心，狂删代码→生气，定时器→饥饿）
  - [ ] 投喂猫粮功能（按钮或快捷键 Ctrl+Shift+F，重置饥饿倒计时）

## 注意事项

1. **不要修改 `.pro` 文件结构**：新增源文件时，记得将 `.h` 加入 `HEADERS +=`、`.cpp` 加入 `SOURCES +=`。
2. **Highlighter.h 已经声明但 .pro 中未注册**：`Highlighter.h` 和 `Highlighter.cpp` 尚未添加到 `simple_ide.pro` 的 `HEADERS` 和 `SOURCES` 列表中，新增代码前需先更新 `.pro` 文件。
3. **保持中文注释**：文档和注释可以使用中文。
4. **Git 协作**：两人尽量修改不同的文件，避免合并冲突。
5. **参考文档**：详细功能需求见 `docs/项目要求.md`，分工细节见 `docs/项目分工.md`。
