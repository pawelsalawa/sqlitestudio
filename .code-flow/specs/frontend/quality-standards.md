# Frontend Quality Standards (Qt GUI)

## Rules
- 使用 Qt 类型 (QString, QList, QHash, QVariant) 而非 STL
- 继承 QObject 的类必须使用 Q_OBJECT macro
- GUI 类必须使用 signals/slots 进行通信
- 关键交互必须有明确的错误提示 (QMessageBox)

## Patterns
- 使用 Qt::ConnectionType 处理信号槽跨线程
- 使用 QScopedPointer/QSharedPointer 管理内存
- 事件处理使用 event() 重写而非直接覆盖

## Anti-Patterns
- 禁止在 GUI 线程执行耗时操作（使用 QThread/Worker）
- 禁止使用原始指针而不使用 parent 关系或 smart pointer
