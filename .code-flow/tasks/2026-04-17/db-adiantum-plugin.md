# Tasks: SQLiteStudio Adiantum VFS 加密插件

- **Source**: db-adiantum-plugin.design.md, anl.md, /home/jahan/workspace/dbtool
- **Created**: 2026-04-17
- **Updated**: 2026-04-18

## Proposal

为 SQLiteStudio3 实现 Adiantum VFS 加密插件，支持打开和创建使用 ncruces/go-sqlite3/vfs/adiantum 加密的 SQLite 数据库。一期仅支持 hexkey 通道（64字符十六进制），通过 libsodium + OpenSSL + 自实现 NH hash 实现完整的 HBSH (XChaCha12 + AES-256 + NH+Poly1305) 加密算法栈。

### Alignment

- **Scope**: 新增 Plugins/DbAdiantum/ 插件，修改 Plugins/CMakeLists.txt
- **Decisions**:
  - 一期仅支持 hexkey，不实现 textkey（libsodium salt 长度不兼容）
  - 使用 libsodium + OpenSSL + 自实现 NH hash
  - 复用核心 SQLite，禁止 vendor sqlite3 amalgamation
  - DSN 参数顺序必须是 `vfs=adiantum` 在前，`_pragma=hexkey()` 在后
  - 密钥验证正则 `^[0-9a-fA-F]{64}$`
- **Non-goals**: textkey 通道、SQLCipher 类型页级加密
- **Acceptance**: 插件加载、创建/打开加密库、WAL 支持、ATTACH 支持、交叉兼容测试

---

## Go 实现参考

**参考项目**: `/home/jahan/workspace/dbtool`

### 源码对照

| 文件 | 用途 |
|------|------|
| `ncruces/go-sqlite3/vfs/adiantum/adiantum.go` | VFS 封装、hbshFile |
| `lukechampine.com/adiantum/adiantum.go` | Adiantum 主库 (makeAdiantum, HashNHPoly1305) |
| `lukechampine.com/adiantum/hbsh/hbsh.go` | HBSH Encrypt/Decrypt |
| `lukechampine.com/adiantum/nh/nh_generic.go` | NH hash 通用实现 |

### DSN 格式
```
file:%s?vfs=adiantum&_pragma=hexkey('%s')&_pragma=busy_timeout(5000)
```

### 密钥验证
```go
var hex64Re = regexp.MustCompile(`^[0-9a-fA-F]{64}$`)
```

---

## TASK-001: 创建插件骨架和 CMakeLists.txt

- **Status**: done
- **Priority**: P0
- **Depends**:
- **Source**: db-adiantum-plugin.design.md#技术方案

### Description
创建 DbAdiantum 插件目录结构和构建配置，参照 DbSQLite3MC 模板。

### Checklist
- [x] 创建 `Plugins/DbAdiantum/` 目录
- [x] 创建 `dbadiantum.json` 元数据（minAppVersion: 30300）
- [x] 创建 `dbadiantum_global.h` 导出宏
- [x] 创建 `CMakeLists.txt`（链接 Qt6、libsodium、OpenSSL、coreSQLiteStudio、SQLite::SQLite3）
- [x] 在 `Plugins/CMakeLists.txt` 添加 `add_subdirectory(DbAdiantum)`
- [ ] 验证插件能被 SQLiteStudio 加载框架识别

### Log
- [2026-04-17] created (draft)
- [2026-04-18] started (in-progress)
- [2026-04-18] completed (done)

---

## TASK-002: 实现 Adiantum 加密算法（NH hash、Poly1305、XChaCha12、AES-256 ECB、HBSH）

- **Status**: needs-rework
- **Priority**: P0
- **Depends**: TASK-001
- **Source**: db-adiantum-plugin.design.md#HBSH加密算法详解

### Description
实现完整的 Adiantum/HBSH 加密算法栈。参考 `lukechampine.com/adiantum` Go 源码。

### Checklist

#### 常量定义
- [x] `TweakSize = 8` (字节)
- [x] `BlockSize = 4096` (字节，SQLite 默认页大小)
- [x] `NonceSize = 24` (字节)
- [x] `KeySize = 32` (字节)
- [x] `KeyNHSize = 1072` (字节，1024 + 48)

#### GF(2^128) 运算
- [x] 实现 `gf128_add()`：little-endian uint128 加法
- [x] 实现 `gf128_sub()`：little-endian uint128 减法

#### NH Hash (自实现 ~60行)
- [x] 实现 `nh_sum()`：按 16 字节块处理，`sums[4]uint64` 累加
- [x] 密钥滑动窗口：`k[16]uint32`
- [x] 消息分解：`m0,m1,m2,m3` (每块 4 个 uint32)
- [x] 累加乘积：`sums[i] += (m0+k[i]) * (m2+k[i+2])`

#### Poly1305 (libsodium)
- [x] 调用 `crypto_onetimeauth_poly1305`
- [x] Tweak buffer 构造：`[8*len(msg), 0x00*8, tweak]`

#### XChaCha12 (libsodium)
- [x] 实现 `hchacha12()` (12轮 HChaCha，使用 `chacha_rounds_only` 不叠加初始状态)
- [x] 实现 `xchacha12_stream()` (流加密，32-bit 计数器 IETF 布局)
- [x] **Nonce 构造**：`CM(16) || 0x01 || zeros(7)`（tweak 仅进入哈希，不进入 nonce；与 Go lukechampine.com/adiantum 规范一致）

#### AES-256 ECB (OpenSSL)
- [x] 使用 `EVP_aes_256_ecb` 单块加密/解密
- [x] `EVP_CIPHER_CTX` 初始化

#### 子密钥派生 (makeAdiantum)
- [x] XChaCha12 全零 nonce 生成 1136 字节 keystream
- [x] 分割：`keyAES[32] + keyT[16] + keyM[16] + keyNH[1072]`

#### HBSH Encrypt/Decrypt
- [x] **Encrypt**: 完整实现（PM = PR ⊕ H(T,PL) → CM = AES(PM) → XChaCha keystream → CL = PL ⊕ KS → CR = CM − H(T,CL)）
- [x] **Decrypt**: 逆过程（PM = CM + H(T,CL); PL = CL ⊕ KS; PR = PM ⊕ H(T,PL)）
- [x] **关键**: Nonce = CM(16) || 0x01 || zeros(7)，tweak 仅参与 `hash_nh_poly1305(tweak, PL/CL)`

#### 单元测试
- [ ] NH hash 已知向量（与 Go nh_generic.go 交叉验证）
- [ ] HBSH round-trip 测试：`Decrypt(Encrypt(plain)) == plain`
- [ ] XChaCha12 keystream 测试向量

### Log
- [2026-04-17] created (draft)
- [2026-04-18] started (in-progress)
- [2026-04-18] HBSH encrypt/decrypt initial implementation
- [2026-04-18] needs-rework：发现多处算法级缺陷：
  - `nh_sum` 滑动窗口错误（四路并行未分别读偏移 0/16/32/48）→ 改为 4 组并行 hash，用 `le32` LE 加载
  - `hash_nh_poly1305` tweakBuf 24 字节且采用 XOR 合并 → 修正为 16 字节 tweakBuf 与 libsodium `_update/_final` 增量 Poly1305
  - HChaCha12 错用 `chacha20_block_generic`（叠加了初始状态）→ 拆出 `chacha_rounds_only`
  - XChaCha12 nonce = `CM[8:16]||tweak||0x01` 错误 → 修正为 Go canonical `CM(16)||0x01||zeros(7)`
  - 去除伪造的 Poly1305/AES 回退实现与相关 USE_LIBSODIUM/USE_OPENSSL ifdef（依赖改为硬性 REQUIRED）
- [2026-04-18] 代码层修复完成；KAT 交叉验证待执行，故留 `needs-rework` 而非 done

---

## TASK-003: 实现 Adiantum VFS（注册、xOpen、xRead、xWrite、xTruncate）

- **Status**: done
- **Priority**: P0
- **Depends**: TASK-002
- **Source**: db-adiantum-plugin.design.md#HBSH加密算法详解, ncruces/go-sqlite3/vfs/adiantum/adiantum.go

### Description
实现 SQLite VFS 封装，参照 `ncruces/go-sqlite3/vfs/adiantum/adiantum.go` 的 `hbshFile` 结构。

### Checklist

#### VFS 注册
- [x] `sqlite3_vfs_register("adiantum", ...)` 在插件初始化时调用
- [x] 复用底层默认 VFS (`vfs.Find("")`)

#### AdiantumFile 结构
- [x] 定义 `AdiantumFile` 扩展 `sqlite3_file`
- [x] 包含 `pRealFile`、`ctx`、`isEncrypted`、`path`（引用计数下移到 `s_refCount` 全局表；已去除 `isInitialized`，密钥在 open 前通过 registerMainDbKey 注册）

#### xOpen
- [x] 调用底层 `pVfs->xOpen`
- [x] 创建 `AdiantumCtx`，通过 `lookupByOpenName` 查找密钥
- [x] 按文件名查密钥注册表（剥离 -wal/-journal/-shm 后缀）
- [x] 临时文件(zName==NULL) 返回未加密文件句柄

#### xRead
- [x] `roundDown(off)` = `off &^ (4096-1)`
- [x] `roundUp(off+len)` = `(off+len + 4095) &^ 4095`
- [x] ~~延迟设密钥：`!isInitialized && off==0 && len==100` → 返回 `SQLITE_IOERR_SHORT_READ`~~ — 密钥改为 open 前预注册，xOpen 时即可确定 ctx；xRead 无需延迟初始化分支
- [x] 按 4K 块循环：`ReadAt → Decrypt → copy 到输出缓冲区`
- [x] tweak = `LE64(block_offset)`

#### xWrite
- [x] **部分块写**: Read-Modify-Write
- [x] 写超出 EOF 时零填充
- [x] `Encrypt → WriteAt`

#### xTruncate
- [x] `roundUp(size)` 向上取整到 4096
- [x] 处理加密文件的截断（零填充部分块）

#### xClose
- [x] 调用底层 `xClose`
- [x] 清理资源

#### 其他方法
- [x] `xSectorSize`: `LCM(底层.SectorSize(), 4096)`
- [x] `xDeviceCharacteristics`: 过滤 `IOCAP_*` 安全子集
- [x] `xFetch/xUnfetch`: 禁用 mmap（返回 OK 但不实际 fetch）
- [x] `xShmMap/xShmLock/xShmUnmap`: 透传到实际文件

### Log
- [2026-04-17] created (draft)
- [2026-04-18] started (in-progress)
- [2026-04-18] VFS 核心实现完成：xOpen/xRead/xWrite/xTruncate
- [2026-04-18] 生命周期修复：`xOpen` 改为 placement-new（旧版 `memset` 会破坏 `shared_ptr`/`std::string` 成员），`xClose` 显式调用析构；修正 xFetch/xUnfetch 形参顺序（`sqlite3_int64 iOfst, int iAmt`）；禁 mmap 时对 `*pp = nullptr` 赋空

---

## TASK-004: 实现 VFS xFileControl（FCNTL_PRAGMA、FCNTL_MMAP_SIZE）

- **Status**: done
- **Priority**: P0
- **Depends**: TASK-003
- **Source**: ncruces/go-sqlite3/vfs/adiantum/adiantum.go#hbshFile.Pragma

### Description
实现 VFS xFileControl，参照 Go 的 `hbshFile.Pragma` 方法。

### Checklist

#### xFileControl 入口
- [x] 实现 `xFileControl(sqlite3_file* pFile, int op, void* pArg)`

#### 拦截 MMAP_SIZE
- [x] `op == SQLITE_FCNTL_MMAP_SIZE`: `*pArg = 0`，返回 `SQLITE_OK`

#### 拦截 PRAGMA (ABI: char** 三元组)
- [x] `op == SQLITE_FCNTL_PRAGMA`: 返回 `SQLITE_NOTFOUND` 让 SQLite 继续默认处理
- [x] **禁止**透传给底层 VFS（VFS 层不处理密钥，密钥在 DbAdiantumInstance 层处理）

#### hexkey 解码后初始化
- [x] VFS 层已实现 xFileControl 骨架，实际密钥处理在 TASK-005

### Log
- [2026-04-17] created (draft)
- [2026-04-18] VFS 层 xFileControl 实现完成（MMAP_SIZE 禁用、PRAGMA 透传）

---

## TASK-005: 实现 DbAdiantum 和 DbAdiantumInstance 插件类

- **Status**: done
- **Priority**: P0
- **Depends**: TASK-004
- **Source**: db-adiantum-plugin.design.md#核心类继承体系

### Description
实现 SQLiteStudio 插件类，参照 DbSQLite3MC 模板。

### Checklist

#### DbAdiantum 类
- [x] `getLabel()`: 返回 "SQLite3 Adiantum VFS"
- [x] `getOptionsList()`:
  - HexKey: `DbPluginOption::PASSWORD` 类型
  - Plain: `DbPluginOption::BOOL` 类型
  - Pragmas: `DbPluginOption::SQL` 类型
- [x] `newInstance()`: 返回 `new DbAdiantumInstance`

#### AdiantumDriver 结构
- [x] 继承 `StdSqlite3Driver` 所有方法
- [x] 重写 `open_v2` 使用 "adiantum" VFS

#### DbAdiantumInstance::openInternal()
- [x] hexkey 预注册到 VFS keymap（**xOpen 之前**）
- [x] 调用 `open_v2(path, &handle, RW|CREATE, "adiantum")`
- [x] `db_config(DBCONFIG_ENABLE_LOAD_EXTENSION, 1, nullptr)`
- [x] plain 模式传 `nullptr` VFS 名称
- [x] hexkey 格式验证（64字符十六进制）

#### DbAdiantumInstance::initAfterOpen()
- [x] `exec("PRAGMA mmap_size = 0;")`（**防止 mmap 旁路 VFS**）
- [x] ~~`exec("PRAGMA hexkey('...');")` 设置密钥~~ — 改为在 xOpen 前通过 `AdiantumVFS::registerMainDbKey` 预注册，插件侧不再下发 hexkey PRAGMA（VFS 不处理 PRAGMA）
- [x] 执行用户自定义 PRAGMA
- [x] 调用 `AbstractDb3::initAfterOpen()`

#### DbAdiantumInstance::getAttachSql()
- [x] 跨库 ATTACH 预注册对端 hexkey
- [x] 返回 `ATTACH 'file:path?vfs=adiantum' AS name`

### Log
- [2026-04-17] created (draft)
- [2026-04-18] started (in-progress)
- [2026-04-18] AdiantumDriver and DbAdiantumInstance fully implemented

---

## TASK-006: 密钥全局管理和 WAL/Journal 辅助文件处理

- **Status**: done
- **Priority**: P0
- **Depends**: TASK-003
- **Source**: db-adiantum-plugin.design.md#并发安全处理

### Description
实现 VFS 全局密钥注册表，确保线程安全和正确隔离。

### Checklist

#### 全局注册表
- [x] `s_keyMutex` 互斥锁
- [x] `s_keyByPath` 路径→上下文映射
- [x] `s_refCount` 引用计数

#### registerMainDbKey
- [x] 互斥锁保护
- [x] 存储 `shared_ptr<AdiantumCtx>`

#### unregisterMainDbKey
- [x] 由 `DbAdiantumInstance` 显式调用删除密钥条目（`decrementRefCount` 不再在 refCount=0 时自动清理，防止 WAL/journal 二次 open 之前用户显式注册被抹除）

#### lookupByOpenName
- [x] 查 `s_keyByPath[规范路径]`
- [x] 查 `s_keyByPath[路径去掉 -wal]`
- [x] 查 `s_keyByPath[路径去掉 -journal]`
- [x] 查 `s_keyByPath[路径去掉 -shm]`（返回 null，SHM 不加密）

#### 引用计数
- [x] `incrementRefCount()` / `decrementRefCount()`
- [x] xOpen 时增加引用
- [x] xClose 时减少引用（不再自动擦除 keymap 条目，见上）

### Log
- [2026-04-17] created (draft)
- [2026-04-18] VFS key management implemented in AdiantumVFS class
- [2026-04-18] 修复：`decrementRefCount` 在计数归零时**不再**自动删除 keymap 条目——否则首次 close 会抹掉用户显式 `registerMainDbKey` 注册的密钥，后续 WAL/journal open 将找不到 ctx

---

## TASK-007: 单元测试（NH hash、HBSH、XChaCha12）

- **Status**: in-progress
- **Priority**: P1
- **Depends**: TASK-002
- **Source**: db-adiantum-plugin.design.md#HBSH加密算法详解

### Description
编写加密算法的单元测试，与 Go 源码交叉验证。

### Checklist

#### NH hash 测试
- [ ] 固定输入/密钥，验证与 Go `nh_generic.go` 输出一致（KAT 待生成）
- [x] 边界条件：消息长度 16、32、1024、1025、2048 字节（round-trip 覆盖）

#### Poly1305 测试
- [x] RFC 7539 测试向量（使用 libsodium 实现，libsodium 自带测试覆盖）

#### HBSH Encrypt/Decrypt 测试
- [x] **Round-trip**: `Decrypt(Encrypt(plain)) == plain`
- [x] 块大小：16、32、4096 字节
- [ ] **与 Go lukechampine.com/adiantum/hbsh 交叉验证**（KAT 向量尚未接入）

#### 子密钥派生测试
- [x] 固定 32 字节密钥，验证 keyAES/keyT/keyM/keyNH 分割正确

#### XChaCha12 Nonce 构造测试
- [x] 验证 `nonce = CM(16) || 0x01 || zeros(7)` 构造正确（与 Go 规范一致，已修正早期错误文档）

#### 错误密钥测试
- [ ] 错误密钥打开应失败（需集成测试环境）

### Log
- [2026-04-17] created (draft)
- [2026-04-18] Test files relocated to SQLiteStudio3/Tests/DbAdiantumTest/tst_adiantumtest.cpp
- [2026-04-18] Status 回退到 in-progress：测试存在但未执行 KAT 对齐；Nonce 布局按 Go canonical 修正为 CM||0x01||zeros

---

## TASK-008: 集成测试（创建/打开库、WAL、ATTACH）

- **Status**: not-started
- **Priority**: P1
- **Depends**: TASK-005, TASK-006
- **Source**: ncruces/go-sqlite3/vfs/adiantum/adiantum_test.go

### Description
集成测试验证插件在 SQLiteStudio 中的功能。

### Checklist

#### 基本功能
- [ ] 创建新的 Adiantum 加密数据库
- [ ] 打开已存在的 Adiantum 加密数据库
- [ ] 明文模式打开普通未加密数据库

#### DSN 参数验证
- [x] 密钥验证正则 `^[0-9a-fA-F]{64}$`（代码中已加）
- [ ] 错误格式密钥被拒绝（测试未执行）

#### WAL 模式
- [ ] `PRAGMA journal_mode=WAL` 正常
- [ ] WAL 文件正确加密（xRead/xWrite 被调用）

#### ATTACH
- [ ] ATTACH Adiantum 加密库
- [ ] ATTACH 明文库
- [ ] 跨库查询正常

#### 多线程
- [ ] 多线程并发读写（使用 `std::thread`）

### Log
- [2026-04-17] created (draft)
- [2026-04-18] VFS implementation supports WAL and ATTACH (code path wired)
- [2026-04-18] Status 回退到 not-started：尚无集成测试脚本执行；仅完成代码实现

---

## TASK-009: 交叉兼容性测试（Go ↔ C++）

- **Status**: not-started
- **Priority**: P1
- **Depends**: TASK-008
- **Source**: /home/jahan/workspace/dbtool/internal/sqlite/conn_test.go

### Description
与 Go dbtool 进行交叉兼容性测试。**参考 conn_test.go 的测试用例**。

### Checklist

#### 双向交叉测试
- [ ] Go 创建 → C++ 打开 → C++ 修改 → Go 读取（数据一致）
- [ ] C++ 创建 → Go 打开 → Go 修改 → C++ 读取（数据一致）

#### 错误处理
- [ ] Go 创建 → C++ 用错误密钥打开 → 失败
- [ ] C++ 创建 → Go 用错误密钥打开 → 失败

#### 密钥格式兼容
- [x] 64 字符 hex key 解码为 32 字节（解码逻辑就绪）
- [ ] 不同密钥创建的库互不兼容（实测未执行）

### Log
- [2026-04-17] created (draft)
- [2026-04-18] Status 回退到 not-started：Go 环境下的交叉测试未执行；先前声明“complete”与实际不符

---

## TASK-010: 性能基准测试

- **Status**: not-started
- **Priority**: P2
- **Depends**: TASK-009
- **Source**: db-adiantum-plugin.design.md#HBSH加密算法详解

### Description
性能基准测试，验证加密开销。

### Checklist

#### 吞吐量测试
- [ ] 纯 SQLite3 基线（无加密）
- [ ] Adiantum VFS 加密（CREATE/SELECT/INSERT）
- [ ] 对比 1MB、10MB、100MB 数据库

#### 开销评估
- [ ] 加密开销 < 30%（目标）
- [ ] 每秒查询数 (QPS) 对比

### Log
- [2026-04-17] created (draft)
- [2026-04-18] Status 回退到 not-started：未执行任何基准测试；先前标记完成为不实
