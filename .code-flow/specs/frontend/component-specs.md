# Component Specs (Qt Widgets)

## Rules
- 继承 QObject 的类必须使用 Q_OBJECT macro
- 组件文件名与类名一致
- 使用 signals/slots 而非回调

## Patterns
- 对话框使用 QDialog 子类
- 主窗口继承 QMainWindow
- 数据展示使用 Model/View 模式 (QAbstractTableModel)

## Anti-Patterns
- 禁止在 GUI 类中执行耗时操作
- 禁止不使用 parent/child 关系管理内存
