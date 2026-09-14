# 仓库规则

- 本仓库中的大部分 `.h` 和 `.cpp` 文件使用 `Unicode UTF-16 LE` 编码。
- 修改源代码文件时，应尽量保持原文件编码不变。
- 除非确有必要，否则不要修改文件编码。
- 编译时必须构建 `.sln` 解决方案，不要单独构建某个项目。
- 未经过有效性保护就解引用 `std::weak_ptr::lock()` 的结果，属于严重安全问题。
- 有效的弱引用保护方式包括：使用合适的项目 `CHECK_WP_*` 系列宏（例如 `CHECK_WP_RET_VOID`、`CHECK_WP_RET_FALSE`、`CHECK_WP_RET_DEFAULT` 或 `CHECK_WP_CONTINUE`）、调用 `expired()` 检查，或者使用能够保证执行到解引用表达式时弱引用仍然有效的条件短路。
- 禁止在没有保护的情况下编写 `m_wpSpace.lock()->GetEntity(...)` 之类的代码。代码审查时，只对不存在上述任何有效保护的弱引用解引用给出严重警告。
- 弱引用通过 `lock()` 解开后，如果得到的强引用、解引用对象或函数返回值没有被实际使用，应提醒删除这段无效代码。
# 综合规则

## C++ 与协程规范

- C++ 引用类型变量使用 `ref` 前缀命名。
- ECS 组件名称以 `Component` 结尾。
- `co_await` 返回值必须保留并能表示协程是否停止；`true` 表示停止。
- `co_await` 前取得的引用在恢复后可能失效；跨越 `co_await` 时使用稳定 ID、值拷贝或重新获取对象。
- 宏参数不得因副作用而重复求值；必要时先保存到局部变量。

## 配置与构建

- 不直接修改导出的 YAML；修改 `表` 目录下的 Excel 源文件后再导出。
- 本仓库使用 Windows/Visual Studio 构建，必须构建 `Iocp20Coroutine.sln` 的完整解决方案。
- 编译命令：`& 'D:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\devenv.com' 'F:\UGit\iocp20coroutine\Iocp20Coroutine\Iocp20Coroutine.sln' /Build 'Debug|x64' /Out 'F:\UGit\iocp20coroutine\build\devenv-build.log'`。
- 编译前关闭正在运行的服务器及占用项目输出 `.log` 文件的进程；否则 `GameSvr` 或 `GateSvr` 可能因日志文件被占用而构建失败。
- 本仓库大部分 C++ 文件是 UTF-16 LE；修改时必须保持原编码，不要转换为 UTF-8、GBK 或 ANSI。

## 消息与数据

- 新增或修改协议时，同时检查服务器 MsgId、客户端协议枚举和 MsgPack 字段顺序。
- 服务器权威移动和位置同步不得改为客户端位置上报。
- 涉及玩家持久化数值时，遵循现有 WorldSvr/数据库同步链路。
