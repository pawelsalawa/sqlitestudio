# Backend Platform Rules

## Rules
- API 变更必须保持向后兼容
- 插件接口使用版本化 (PluginInterface)
- 外部依赖变更需更新 CMakeLists.txt

## Patterns
- 使用 CMake option 控制编译特性
- 使用 Qt6 required components

## Anti-Patterns
- 禁止在生产环境使用 Debug build
