# 在 SQLiteStudio 中增加 ncruces/go-sqlite3/vfs/adiantum 支持的详细技术分析报告

## 一、背景

- **SQLite3MC** 是 SQLite 的多加密方案扩展（页级 PRAGMA 加密），当前 **不支持** Adiantum 算法。
- **ncruces/go-sqlite3/vfs/adiantum** 通过 **VFS 文件层** 实现 Adiantum 加密，使用 HBSH(XChaCha12 + AES) 构造，以 4KiB 块为单位进行 **长度保持**（length-preserving）加密，**不提供完整性保护**。
- 两者加密原理完全不同（页级 PRAGMA vs 文件层 VFS），**无法互相解密**。
- Go 版本源码位置: `github.com/ncruces/go-sqlite3@v0.24.0/vfs/adiantum/`
  - 核心文件: `hbsh.go`(VFS封装), `adiantum.go`(Adiantum构造器), `api.go`(VFS注册)
  - 算法库: `lukechampine.com/adiantum@v1.1.1`

### 业务代码实际用法参考

以下为生产环境 Go 端接入示例（来自 `/Users/jahan/workspace/dbtool/internal/sqlite/conn.go`）：

```go
// 实际业务使用的 64 位十六进制密钥 (= 32 字节 / 256 bits 原始密钥)
const dbEncryptionHexKey = "f14a6099bb6c0cc99e0c213198dd3d585c69b8c373584deffbf7026427ca79a9"

func EncryptedDSN(filePath string) string {
    return "file:" + filePath +
        "?vfs=adiantum" +                          // 指定 Adiantum VFS
        "&_pragma=hexkey('" + dbEncryptionHexKey + "')" +  // 设置 64 字符 hex 密钥
        "&_pragma=journal_mode(WAL)" +             // 启用 WAL 模式
        "&_pragma=busy_timeout(5000)" +            // 写锁超时 5 秒
        "&_txlock=immediate"                       // 事务锁策略
}
```

**关键参数解析：**

| DSN 参数 | 值 | 说明 |
|----------|-----|------|
| `vfs` | `adiantum` | ★ 必须指定，告诉 SQLite 使用自定义 VFS |
| `_pragma=hexkey('...')` | 64 字符 hex 字符串 | 解码后为 **32 字节原始密钥**，直接用于 HBSH |
| `_pragma=journal_mode(WAL)` | WAL | 启用 Write-Ahead Logging 并发读写模式 |
| `_pragma=busy_timeout(5000)` | 5000ms | 锁等待超时，避免并发写立即报 SQLITE_BUSY |
| `_txlock=immediate` | immediate | 事务开始时获取 IMMEDIATE 锁，减少死锁 |

> **密钥格式说明**: `dbEncryptionHexKey` 是 **64 个十六进制字符**（不是"32位"），
> 经 `hex.DecodeString()` 后变为 **32 字节 (256 bits)** 的原始二进制密钥，
> 这正是 HBSH 算法要求的主密钥 K 长度。

---

## 二、Go 源码精确分析 —— C++ 移植必须遵循的规范

### 2.1 完整算法栈（HBSH 构造）

```
Adiantum = HBSH(StreamCipher=XChaCha12, BlockCipher=AES-256, Hash=NH+Poly1305)
```

#### 2.1.1 密钥派生（子密钥生成）

输入: 32 字节主密钥 K (master key)

```
stream = XChaCha12(K, nonce=[0x00*23, 0x01])   // 用全零输入生成 keystream
keyBuf = stream.XORKeyStream(zeros(1136))        // 生成 1136 字节子密钥材料
```

子密钥分配:

| 偏移 | 长度 | 用途 |
|------|------|------|
| [0:32) | 32 bytes | AES-256 块加密密钥 (`keyAES`) |
| [32:48) | 16 bytes | Poly1305 密钥，用于 tweak 哈希 (`keyT`) |
| [48:64) | 16 bytes | Poly1305 密钥，用于消息哈希 (`keyM`) |
| [64:1136) | 1072 bytes | NH 哈希密钥 (`keyNH`) |

**关键**: XChaCha12 的 nonce 固定为 `[0x00 * 23, 0x01]`（24字节），rounds=12。

#### 2.1.2 HBSH 加密操作

给定 N 字节的块 B (N >= 16) 和 tweak T (8字节):

```
┌─── 输入分割 ─────────────────────────────────────┐
│ PL = B[0 : N-16]    (前 N-16 字节 = 明文左部)     │
│ PR = B[N-16 : N]    (后 16 字节 = 明文右部)       │
└───────────────────────────────────────────────────┘

步骤 1: PM ← PR ⊕ Hash(T, PL)           // GF(2^128) LE 加法
步骤 2: CM ← AES256_ECB_Encrypt(PM)      // AES 单块加密
步骤 3: CL ← CM ⊕ StreamXOR(CM, PL)      // XChaCha12 流密码 XOR
步骤 4: CR ← CM ⊖ Hash(T, CL)            // GF(2^128) LE 减法

输出: CL || CR    (与输入等长)
```

其中 `StreamXOR(CM, msg)` 的 XChaCha nonce 构造:
```
nonce[0:16]  = CM 的最后 16 字节
nonce[16:24] = 8 字节 tweak
nonce[23]    = 0x01 (固定)
```

#### 2.1.3 HBSH 解密操作（加密的逆过程）

```
┌─── 密文分割 ─────────────────────────────────────┐
│ CL = CT[0 : N-16]    (前 N-16 字节)               │
│ CR = CT[N-16 : N]    (后 16 字节)                 │
└───────────────────────────────────────────────────┘

步骤 1: CM ← CR ⊕ Hash(T, CL)              // GF(2^128) LE 加法
步骤 2: PL ← CM ⊕ StreamXOR(CM, CL)         // XChaCha12 流密码 XOR
步骤 3: PM ← AES256_ECB_Decrypt(CM)          // AES 单块解密
步骤 4: PR ← PM ⊖ Hash(T, PL)                // GF(2^128) LE 减法

输出: PL || PR
```

#### 2.1.4 Hash 函数: NH + Poly1305

计算 `Hash(tweak, message)` → 16 字节:

**Step A — Tweak Poly1305 (outT):**
```python
tweakBuf = LE64(8*len(msg)) || [0x00*8] || tweak    # 总长度 = 16 + len(tweak)
outT = Poly1305(keyT, tweakBuf)
```
- 前 8 字节为 `8 × message_length_in_bytes` 的 uint64 小端表示

**Step B — Message NH+Poly1305 (outM):**
```python
mac = Poly1305(keyM)
for each ≤1024 byte chunk of message:
    outNH = NH(chunk, keyNH)          # 输出 32 字节
    mac.Write(outNH)
# 最后不足一块的部分: 零填充到 16 字节边界后再 NH
outM = mac.Sum()                       # 16 字节
```

**Step C — 合并:**
```
result = outT ⊕ outM    # 128-bit little-endian addition (GF(2^128))
return result (16 bytes)
```

**NH 算法细节:**
- 对 16 字节消息块和密钥段的乘累加运算
- 密钥必须 ≥ len(message) + 48 字节
- 输出 4 × uint64_LE = 32 字节

### 2.2 密钥处理完整流程

Go 版本支持 **5 种密钥输入方式** (按优先级):

| 优先级 | 方式 | URI 参数 | 处理逻辑 |
|--------|------|----------|----------|
| 1 | 继承主 DB 密钥 | — | 直接复用同一个 `*hbsh.HBSH` 实例 |
| 2 | 随机密钥（临时文件） | name==nil | 生成 32 字节随机密钥 (`crypto/rand`) |
| 3 | 原始二进制密钥 | `key=<binary>` | **直接作为 32 字节使用** |
| 4 | Hex 编码密钥 | `hexkey=<hex>` | `hex.DecodeString()` → 32 字节原始密钥 |
| 5 | 文本口令 | `textkey=<pass>` | **Argon2id KDF 派生** → 32 字节 |

**关键约束**: 所有方式最终都必须产出 **恰好 32 字节** 的密钥。长度不符则返回错误。

#### 2.2.1 Argon2id 参数（仅 textkey 使用）

```go
argon2.IDKey(
    password: []byte(text),                              // 用户输入的文本口令
    salt:     []byte("github.com/ncruces/go-sqlite3/vfs/adiantum"),  // 固定 pepper
    time:     3,                                          // 迭代次数
    memory:   64 * 1024,                                  // 64 MiB
    threads:  4,                                          // 并行度
    keyLen:   32,                                         // 输出 256 位
)
```

> ⚠️ pepper 可通过 `-ldflags="-X ...pepper=custom"` 覆盖。C++ 端应保持默认值一致。

#### 2.2.2 PRAGMA 延迟设密钥机制（重要！）

对于 `OPEN_MAIN_DB` 标志的文件，支持 **延迟设密钥**:
1. 先打开一个 `hbshFile`，其 `hbsh == nil`（未初始化）
2. SQLite 尝试读取文件头 (offset=0, size=100) 时，ReadAt 返回 **EOF**
3. SQLite 认为新空数据库，等待用户执行 `PRAGMA hexkey='...'`
4. 收到 PRAGMA 后初始化 cipher，后续读取正常

支持的 PRAGMA 名称 (小写匹配):
- `"key"` → 原始二进制字节直接使用
- `"hexkey"` → hex 解码
- `"textkey"` → Argon2id 派生

### 2.3 块大小与对齐规则

```
常量定义:
  tweakSize = 8      字节 (uint64 little-endian)
  blockSize = 4096    字节 (= 默认 SQLite 页大小)

对齐函数:
  roundDown(i) = i &^ (4096 - 1)     // 向下对齐到块起始
  roundUp(i)   = (i + 4095) &^ 4095  // 向上对齐到块边界
```

### 2.4 Nonce/Tweak 生成

tweak 即为 **块的文件偏移量**，编码为 uint64 little-endian:

```
Block 0: offset 0x0000000000000000  → tweak = \x00\x00\x00\x00\x00\x00\x00\x00
Block 1: offset 0x0000000000001000  → tweak = \x00\x00\x00\x00\x10\x00\x00\x00
Block N: offset N×4096              → tweak = LE64(N×4096)
```

### 2.5 文件格式

**无 magic number、无 header、无版本字段、无 IV。**

- 加密后文件大小 = 原始文件大小（Truncate 向上取整到 4096 的倍数）
- 文件的前 4096 字节就是第一页的 HBSH 密文（解密后为 "SQLite format 3\000" 开头）
- 无盐值、无 MAC、无签名

### 2.6 VFS 方法覆盖清单

`hbshFile` 覆盖的 sqlite3_io_methods / sqlite3_vfs 方法:

| 方法 | 行为 | 是否需要自定义实现 |
|------|------|-------------------|
| **ReadAt** | 按 4K 块读取→解密→提取请求范围 | ✅ 核心 |
| **WriteAt** | Read-Modify-Write 部分块→加密→写回 | ✅ 核心 |
| **Truncate** | 向上取整到 4096 对齐 | ✅ 修改 |
| **SectorSize** | LCM(底层.SectorSize(), 4096) | ✅ 修改 |
| **DeviceCharacteristics** | 过滤安全子集 | ✅ 修改 |
| **ChunkSize** | 向上取整到 4096 | ✅ 修改 |
| **SizeHint** | 向上取整到 4096 | ✅ 修改 |
| **Pragma** | 处理 key/hexkey/textkey | ✅ 修改 |
| Close | 继承底层 | ❌ 透传 |
| Sync | 继承底层 | ❌ 透传 |
| Size | 继承底层 | ❌ 透传 |
| Lock/Unlock | 继承底层 | ❌ 透传 |
| CheckReservedLock | 继承底层 | ❌ 透传 |

#### 2.6.1 ReadAt 详细流程

```
min = roundDown(off)                    // 块对齐起始偏移
max = roundUp(off + len(p))             // 块对齐结束偏移

for each block from min to max (step 4096):
    1. 从底层文件读取 4096 字节 → buffer[]
    2. 设 tweak = LE64(block_offset)
    3. 就地解密: HBSH.Decrypt(buffer[0:4096], tweak)
    4. 若读中间部分: data = buffer[off-min:]  (跳过头部多余字节)
    5. copy(p[n:], data)                  // 复制到输出缓冲区

特殊: hbsh==nil 且是头读取(off==0,len==100) →
        memset(p, 0, len);
        return SQLITE_IOERR_SHORT_READ;     // ★ 修正：不能返回 SQLITE_OK / EOF
                                             // SQLite 凭此判断 "新空文件"
      其他无密钥读取 → 返回 SQLITE_IOERR  (不再用 CANTOPEN，避免误导)
```

> **早期 anl.md 写"立即返回 EOF"是不严谨的**。SQLite 没有 "EOF" 这个返回码；
> 短读必须用 `SQLITE_IOERR_SHORT_READ` 并把缓冲区清零，否则 SQLite 会
> 把它当成 IO 故障而不是空文件，PRAGMA 延迟设密钥流程就走不通。

#### 2.6.2 WriteAt 详细流程

```
for each block from min to max (step 4096):
    1. 设 tweak = LE64(block_offset)
    2. data = 工作缓冲区 (4096 字节)

    3. 若是部分块写入 (跨块或末尾不足 4096 字节):
       a. 从底层文件读取现有 4096 字节
       b. 若读到 EOF (写入超出文件尾):
          - 清零整个缓冲区 (零填充)
       否则:
          - 解密现有块: HBSH.Decrypt(buffer, tweak)
       c. 若从中间偏移开始: data = data[off-min:]

    4. copy(data, p[n:])                // 覆盖新明文数据
    5. 就地加密: HBSH.Encrypt(buffer, tweak)
    6. 将 4096 字节写回文件

注意: 写入超出 EOF 会零填充到下一个 4096 边界
```

### 2.7 辅助文件处理

| 文件类型 | 打开标志 | 是否加密 | 说明 |
|----------|----------|---------|------|
| 主数据库 | OPEN_MAIN_DB | ✅ 是 | 支持 PRAGMA 延迟设密钥 |
| WAL 文件 | OPEN_WAL | ✅ 是 | 继承主 DB 密钥 |
| 主日志 | OPEN_MAIN_JOURNAL | ✅ 是 | 从 URI 参数获取密钥 |
| 临时日志 | OPEN_TEMP_JOURNAL | ✅ 是 | 从 URI 参数获取密钥 |
| 子日志 | OPEN_SUBJOURNAL | ✅ 是 | 从 URI 参数获取密钥 |
| 超级日志 | OPEN_SUPER_JOURNAL | ❌ 否 | 明文透传 |
| 临时数据库 | OPEN_TEMP_DB | ✅ 是 | name==nil 时用随机密钥 |
| 内存数据库 | OPEN_MEMORY | ❌ 否 | 明文透传 |
| SHM 文件 | — | ❌ 否 | 非 regular file，走 SharedMemory 接口 |

---

## 三、插件架构设计

### 3.1 类继承体系

```
QObject
  └── GenericPlugin ──── virtual public Plugin (JSON 元数据自动加载)
        │
        └── DbPluginStdFileBase ─── public DbPlugin
              │  提供: getInstance()模板方法, generateDbName()
              │  纯虚: newInstance()
              │
              └── DbAdiantum (Q_OBJECT, SQLITESTUDIO_PLUGIN("dbadiantum.json"))
                    覆盖: getLabel(), getOptionsList(),
                          checkIfDbServedByPlugin(), newInstance()
                    │
                    │ newInstance() 创建 ↓
                    │
              AbstractDb3<AdiantumDriver> ─── public AbstractDb
                    │
                    └── DbAdiantumInstance
                          覆盖: openInternal(), initAfterOpen(),
                                getAttachSql(), clone(),
                                getTypeClassName(), getTypeLabel()
```

### 3.2 需要创建的文件

```
Plugins/DbAdiantum/
├── CMakeLists.txt                  # 构建脚本
├── dbadiantum_global.h             # 共享库导出/导入宏
├── dbadiantum.json                 # 插件元数据 (type:"DbPlugin")
├── dbadiantum.h                    # 插件管理类声明
├── dbadiantum.cpp                  # 插件管理类实现
├── dbadiantuminstance.h            # 数据库实例类声明 + Driver 结构体
├── dbadiantuminstance.cpp          # 数据库实例类实现
├── adiantum_vfs.h                  # Adiantum VFS 实现（核心加密层）
├── adiantum_vfs.cpp                # VFS 方法实现
├── adiantum_cipher.h               # Adiantum/HBSH/XChaCha/NH/Poly1305 纯算法实现
├── adiantum_cipher.cpp             # 加密算法实现
└── translations/
    └── dbadiantum_zh_CN.ts         # 中文翻译（可选）
```

### 3.3 插件元数据 (dbadiantum.json)

```json
{
    "type":          "DbPlugin",
    "title":         "SQLite3 Adiantum VFS",
    "description":   "Provides support for ncruces/go-sqlite3/vfs/adiantum encrypted databases using HBSH length-preserving encryption.",
    "minAppVersion": 30300,
    "version":       10000,
    "author":        "Your Name"
}
```

> **`minAppVersion` 必须用 30300**（与现役 `Plugins/DbSQLite3MC/dbsqlitemc.json:5`、
> `Plugins/DbAndroid/dbandroid.json:7` 一致）。早期版本写过 40000，会被插件加载器
> 直接拒绝（运行时主程序版本远小于 40000）。

### 3.4 连接对话框 UI 选项 (getOptionsList)

参考截图中的三种类型 (SQLCipher / SQLite 3 / WxSQLite3)，新增第四种。
布局对齐 `Plugins/DbSQLite3MC/dbsqlitemc.cpp:28-69` 的三段式（密钥 / 配置 / 自定义 PRAGMA）：

```cpp
QList<DbPluginOption> DbAdiantum::getOptionsList() const
{
    QList<DbPluginOption> opts;

    // ===== 选项 1: Hex Key (64位十六进制) =====
    DbPluginOption optKey;
    optKey.type = DbPluginOption::PASSWORD;        // 用 PASSWORD 类型获得 UI 遮罩
    optKey.key = "hexkey";
    optKey.label = tr("Hex Key (64 hex characters)");
    optKey.toolTip = tr("64-character hexadecimal encryption key. "
                        "Leave empty (and check Plain mode) to connect to an unencrypted database.");
    optKey.placeholderText = tr("00000000...00000000 (64 chars)");
    opts << optKey;

    // ===== 选项 2: 明文模式开关 =====
    DbPluginOption optPlain;
    optPlain.type = DbPluginOption::BOOL;
    optPlain.key = "plain";
    optPlain.label = tr("Plain text mode (no encryption)");
    optPlain.toolTip = tr("Enable to create or connect to unencrypted database. "
                          "Bypasses the Adiantum VFS entirely.");
    optPlain.defaultValue = false;
    opts << optPlain;

    // ===== 选项 3: 自定义 PRAGMAs (对齐 DbSqliteMc 的 optPragmas) =====
    // 用于代替 anl.md 早期版本中硬编码的 journal_mode=WAL / busy_timeout
    DbPluginOption optPragmas;
    optPragmas.type = DbPluginOption::SQL;
    optPragmas.key = "pragmas";
    optPragmas.label = tr("Custom PRAGMAs (optional)");
    optPragmas.defaultValue = "-- Recommended for the Adiantum VFS:\n"
                              "--PRAGMA journal_mode = WAL;\n"
                              "--PRAGMA busy_timeout = 5000;";
    optPragmas.toolTip = tr("PRAGMA statements executed after the database is opened. "
                            "Use them to opt into WAL, busy_timeout, etc.");
    opts << optPragmas;

    return opts;
}
```

> **不再硬编码** `journal_mode=WAL` / `busy_timeout=5000`：用户用 `delete/truncate` 模式
> 打开既有库时不应被强制改成 WAL（影响多进程访问、网络盘）。
>
> **textkey(Argon2id) 通道一期不暴露**：见 §5.4 — Go 端使用的 43 字节 salt
> 与 libsodium 强制的 16 字节 salt 不兼容，二期引入 `libargon2` 后再加 PASSWORD
> 类型的 textkey 选项。

### 3.5 核心数据流

```
┌─────────────────────────────────────────────────────────────────────┐
│  用户在 Add Database 对话框中选择 "SQLite3 Adiantum VFS" 类型         │
│                                                                     │
│  ① 框架调用 DbAdiantum::getOptionsList()                            │
│     → 返回 [HexKey(STRING), Plain(BOOL)]                            │
│     → 用户填写 64 位 hex key 或选择明文模式                           │
│                                                                     │
│  ② 用户点击 OK                                                      │
│     → options = {                                                   │
│         "hexkey": "a1b2c3...f4e5d6",   // 64字符hex                 │
│         "plain": false                                             │
│       }                                                             │
│     → 调用 DbAdiantum::getInstance(name, path, options)             │
│                                                                     │
│  ③ DbPluginStdFileBase::getInstance():                             │
│     a. newInstance(name, path, options)                             │
│        → new DbAdiantumInstance(name, path, options)                │
│     b. openForProbing()                                             │
│                                                                     │
│  ④ DbAdiantumInstance::openInternal() [覆盖]:                       │
│     a. 注册/获取 "adiantum" VFS                                     │
│     b. AdiantumVFS::setKey(hex_decoded_key)  // 设置 32 字节密钥     │
│     c. open_v2(path, &handle, RW|CREATE, "adiantum") // ★ VFS名称  │
│                                                                     │
│  ⑤ DbAdiantumInstance::initAfterOpen() [覆盖]:                      │
│     a. 若 plain==false && hexkey 非空:                               │
│        exec("PRAGMA hexkey('...');", Flag::NO_LOCK)                 │
│     b. 执行 busy_timeout 等 URI pragmas                              │
│     c. AbstractDb3<Driver>::initAfterOpen()                         │
│        → foreign_keys=1, recursive_triggers=1, ...                  │
│                                                                     │
│  ⑥ 验证: SELECT COUNT(1) FROM sqlite_master                        │
│     → 成功 → 返回 db 实例                                           │
└─────────────────────────────────────────────────────────────────────┘
```

### 3.6 与 DbSQLite3MC 的关键差异

| 维度 | DbSQLite3MC | DbAdiantum (新插件) |
|------|-------------|---------------------|
| **加密层级** | SQL 层 (PRAGMA cipher/key) | VFS 文件层 (自定义 sqlite3_vfs) |
| **加密算法** | AES-CBC/ChaCha20-Poly1305 等 (可配置) | Adiantum/HBSH (固定) |
| **密钥设置** | `PRAGMA key='password'` | `PRAGMA hexkey='64hex'` 或 VFS 注册时预设 |
| **open_v2 第 4 参数** | nullptr (默认 VFS) | **"adiantum"** (自定义 VFS 名称) |
| **是否重写 openInternal()** | ❌ 使用基类默认实现 | **✅ 必须重写**，传入 VFS 名 |
| **SQLite 变体** | 自带 amalgamation (mc_ 前缀符号) | **使用标准 sqlite3** (无需改符号) |
| **辅助库依赖** | SQLite3MC amalgamation (~12MB) | libsodium (或自实现加密原语) |
| **ATTACH 语法** | `ATTACH 'path' AS name KEY 'pwd'` | `ATTACH 'path' AS name` (VFS 自动继承) |

---

## 四、C++ VFS 实现设计

### 4.1 VFS 包装架构

```
SQLite 引擎
  │
  ├── sqlite3_open_v2(path, &db, flags, "adiantum")
  │                                        │
  ▼                                        ▼
sqlite3_vfs ("adiantum")              底层默认 VFS ("unix"/"win32")
  │                                    │
  ├── xOpen ──────────────────────► 包装底层 xOpen
  │                                 │
  ▼                                 ▼
sqlite3_file (adiantum)         sqlite3_file (真实)
  ├─ pMethods: adiantum_io       ├─ pMethods: 默认 io
  │                               │
  ├── xRead:  读密文→解密→返明文  │
  ├── xWrite: 接明文→加密→写密文  │
  ├── xTruncate: 向上取整到 4096  │
  ├── xSectorSize: LCM(底,4096)   │
  └── xFileControl(Pragma): 设密钥 │
```

### 4.2 关键结构体设计

```cpp
// ========== Adiantum 加密上下文 (每个文件一个实例) ==========
struct AdiantumCtx {
    // 密码学状态
    uint8_t  aesKey[32];           // AES-256 密钥
    uint8_t  polyKeyT[16];         // Poly1305 tweak 哈希密钥
    uint8_t  polyKeyM[16];         // Poly1305 消息哈希密钥
    uint8_t  nhKey[1072];          // NH 哈希密钥
    bool     initialized;          // 是否已设置密钥

    // I/O 缓冲区 (避免频繁分配)
    uint8_t  blockBuf[4096];       // 加解密工作缓冲区
    uint8_t  tweakBuf[8];          // LE64 偏移量缓存

    // 底层文件句柄
    sqlite3_file* pRealFile;       // 被包装的真实文件
};

// ========== VFS 全局状态 (每个进程一个) ==========
struct AdiantumVFS {
    sqlite3_vfs base;              // VFS 虚表
    sqlite3_vfs* pDefaultVFS;      // 底层默认 VFS
    char vfsName[32];              // "adiantum"

    // ★ 全局密钥注册表 — 必备
    // 解决两类问题：
    //   (a) 多库并发：同进程同时打开两个不同 hexkey 的库时，
    //       全局 setKey() 会被后到者覆盖。必须按文件名隔离。
    //   (b) WAL/Journal/SHM：SQLite 自动派生 <path>-wal/-journal/-shm
    //       并通过同一个 VFS 的 xOpen 打开，VFS 必须按文件名后缀
    //       回查主库 entry，复用同一 ctx。
    static std::mutex                                              s_keyMutex;
    static std::unordered_map<std::string, std::shared_ptr<AdiantumCtx>>
                                                                   s_keyByMainDbPath;
    static std::unordered_map<std::string, int>                    s_refCount;

    // openInternal 在 open_v2 之前调用
    static void registerMainDbKey(const std::string& canonicalPath,
                                  const QByteArray& rawKey32);

    // close 时由引用计数触发清理
    static void unregisterMainDbKey(const std::string& canonicalPath);

    // xOpen 内部使用：按文件名查 ctx；自动剥离 -wal/-journal/-shm 后缀
    static std::shared_ptr<AdiantumCtx> lookupByOpenName(const char* zName);
};
```

**密钥查表规则**（`lookupByOpenName` 实现要点）：

| 传入 zName | 查询路径 | 命中后行为 |
|---|---|---|
| `/path/db.sqlite`              | 原样查        | 返回主库 ctx |
| `/path/db.sqlite-journal`      | 去掉 `-journal` | 返回主库 ctx（共享同一 HBSH） |
| `/path/db.sqlite-wal`          | 去掉 `-wal`    | 返回主库 ctx |
| `/path/db.sqlite-shm`          | 去掉 `-shm`    | **不加密**，明文 mmap 走 xShmMap |
| `etilqs_*` 临时文件 (zName==NULL) | 不查           | 生成 32 字节随机密钥 |
| 其他                           | 不查           | 视 OPEN flag 处理（见 §3.8） |

### 4.3 openInternal 重写

```cpp
// MUST mirror AbstractDb3<T>::openInternal (abstractdb3.h:452-469).
// Only differences:
//   1. Pass "adiantum" (or nullptr in plain mode) as the 4th open_v2 arg.
//   2. Pre-register hexkey into the VFS key map before calling open_v2.
bool DbAdiantumInstance::openInternal()
{
    resetError();

    // 解析连接选项
    const bool isPlain = connOptions["plain"].toBool();
    const QString hexKey = connOptions["hexkey"].toString().trimmed();
    const QString canonical = QFileInfo(path).canonicalFilePath().isEmpty()
                                ? path
                                : QFileInfo(path).canonicalFilePath();

    // 非明文模式: 把 hexkey 预注册到 VFS 全局 keymap，xOpen 期间 lookup
    if (!isPlain) {
        if (hexKey.isEmpty()) {
            dbErrorMessage = tr("hexkey is required for encrypted Adiantum databases.");
            dbErrorCode = AdiantumDriver::ERROR;
            return false;
        }
        QByteArray rawKey = QByteArray::fromHex(hexKey.toLatin1());
        if (rawKey.size() != 32) {
            dbErrorMessage = tr("hexkey must decode to exactly 32 bytes (got %1).")
                                .arg(rawKey.size());
            dbErrorCode = AdiantumDriver::ERROR;
            return false;
        }
        AdiantumVFS::registerMainDbKey(canonical.toStdString(), rawKey);
    }

    // ★ 关键差异: 第 4 个参数传入 VFS 名称（明文模式则传 nullptr 走默认 VFS）
    typename AdiantumDriver::handle* handle = nullptr;
    const char* vfsName = isPlain ? nullptr : "adiantum";
    int res = AdiantumDriver::open_v2(
        path.toUtf8().constData(),
        &handle,
        AdiantumDriver::OPEN_READWRITE | AdiantumDriver::OPEN_CREATE,
        vfsName
    );

    if (res != AdiantumDriver::OK) {
        dbErrorMessage = tr("Could not open Adiantum database: %1")
                            .arg(extractLastError(handle));
        dbErrorCode = res;
        if (handle) AdiantumDriver::close(handle);
        if (!isPlain)
            AdiantumVFS::unregisterMainDbKey(canonical.toStdString());
        return false;
    }

    dbHandle = handle;
    // 镜像基类 abstractdb3.h:467 —— enableLoadExtension() 不存在于基类的实际实现
    AdiantumDriver::db_config(dbHandle,
                              AdiantumDriver::DBCONFIG_ENABLE_LOAD_EXTENSION,
                              1, nullptr);
    return true;
}
```

> **修正点回顾**：
> - anl.md 早期版本调用 `enableLoadExtension()` 是错的——`AbstractDb3<T>::openInternal`
>   实际用 `T::db_config(handle, T::DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL)`
>   （`coreSQLiteStudio/db/abstractdb3.h:467`）。重写时必须保留该调用。
> - 当 `plain==true` 时直接传 `nullptr`，走默认 VFS，等价于普通 SQLite3 连接。
> - hexkey 必须在 `open_v2` **之前** 注册——SQLite 在 open_v2 内会立刻调用 xOpen
>   读取头 100 字节，此时 VFS 已经需要密钥。详见 §4.2 的 keymap 设计。

### 4.4 initAfterOpen 实现

```cpp
void DbAdiantumInstance::initAfterOpen()
{
    SqlQueryPtr res;

    const bool isPlain = connOptions["plain"].toBool();

    if (!isPlain) {
        // ★ 第一步: 强制禁用 mmap，防止 SQLite 用 xFetch/xUnfetch 旁路 xRead
        //   导致上层读到原始密文。配合 VFS xFileControl(SQLITE_FCNTL_MMAP_SIZE)
        //   把后续任何 PRAGMA mmap_size=N 调用强制锁回 0，参见 §4.6。
        res = exec("PRAGMA mmap_size = 0;", Flag::NO_LOCK);

        // ★ 第二步: 通过 PRAGMA 把 hexkey 推进 VFS（xFileControl(SQLITE_FCNTL_PRAGMA)
        //   会拦截这个 PRAGMA 并做 hex 解码 + 子密钥派生）。
        //   注：openInternal() 已经把 hexkey 预注册到 VFS keymap，
        //   这里再走一次 PRAGMA 是为了与延迟设密钥流程统一（§2.2.2），
        //   且对新建空文件的分支也必须走 PRAGMA 才能初始化 ctx。
        const QString hexKey = connOptions["hexkey"].toString().trimmed();
        if (!hexKey.isEmpty()) {
            // 复用 coreSQLiteStudio 的 escapeString() 而不是手工 replace，
            // 与 Plugins/DbSQLite3MC/dbsqlitemcinstance.cpp:53 风格一致
            res = exec(QString("PRAGMA hexkey('%1');").arg(escapeString(hexKey)),
                       Flag::NO_LOCK);
            if (res->isError()) {
                qWarning() << "Adiantum: Failed to set hexkey:" << res->getErrorText();
                return;  // 密钥错误时不继续初始化
            }
        }
    }

    // ★ 第三步: 用户自定义 PRAGMA（journal_mode / busy_timeout 由用户在 UI
    //   填写——已不在此处硬编码，避免强制改写既有库的 journal_mode）
    const QString pragmas = connOptions["pragmas"].toString();
    for (const QString& pragma : quickSplitQueries(pragmas)) {
        const QString sql = removeComments(pragma).trimmed();
        if (sql.isEmpty()) continue;
        res = exec(sql, Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Adiantum: PRAGMA failed:" << pragma
                       << ":" << res->getErrorText();
    }

    // 基类标准初始化: foreign_keys, recursive_triggers, collations...
    AbstractDb3<AdiantumDriver>::initAfterOpen();
}
```

**与 Go 端 DSN 参数的对应关系：**

```
Go 端 URI 参数                    C++ 端等价操作
─────────────────────────────    ─────────────────────────
?vfs=adiantum                   → openInternal() 第4参数传 "adiantum"
&_pragma=hexkey('...')           → initAfterOpen() 中 exec("PRAGMA hexkey('...');")
                                  + openInternal() 中预注册到 VFS keymap
&_pragma=journal_mode(WAL)       → 用户在 UI "Custom PRAGMAs" 填写（不再硬编码）
&_pragma=busy_timeout(5000)      → 用户在 UI "Custom PRAGMAs" 填写（不再硬编码）
&_txlock=immediate              → SQLiteStudio 框架层处理 (BEGIN IMMEDIATE)
(隐式)                          → initAfterOpen() 强制 PRAGMA mmap_size=0
                                  防止 mmap 旁路 VFS（Go 端不需要，因为 Go
                                  driver 默认就走 ReadAt/WriteAt 路径）
```

### 4.5 ATTACH SQL

VFS 层加密的 ATTACH 比 SQLCipher 更微妙——ATTACH 不能在 SQL 里传 KEY，
密钥必须**事先**注入 VFS keymap。

```cpp
QString DbAdiantumInstance::getAttachSql(Db* otherDb,
                                          const QString& generatedAttachName)
{
    auto* otherAdiantum = dynamic_cast<DbAdiantumInstance*>(otherDb);
    const QString otherPath = otherDb->getPath();

    if (otherAdiantum) {
        // 跨库 ATTACH: 必须把对端 hexkey 预注册到 VFS keymap，
        // 让 SQLite 在 ATTACH 阶段调 xOpen 时能拿到密钥。
        const auto& opts = otherAdiantum->getConnectionOptions();
        const bool otherPlain = opts["plain"].toBool();
        if (!otherPlain) {
            const QString otherHex = opts["hexkey"].toString().trimmed();
            QByteArray rawKey = QByteArray::fromHex(otherHex.toLatin1());
            if (rawKey.size() == 32) {
                const QString canonical =
                    QFileInfo(otherPath).canonicalFilePath().isEmpty()
                        ? otherPath
                        : QFileInfo(otherPath).canonicalFilePath();
                AdiantumVFS::registerMainDbKey(canonical.toStdString(), rawKey);
            }
            // URI 形式 ATTACH 才能指定 vfs=adiantum
            // 前提: SQLiteStudio 链接的 sqlite3 启用了 SQLITE_USE_URI（默认开）
            return QString("ATTACH 'file:%1?vfs=adiantum' AS %2;")
                   .arg(otherPath, generatedAttachName);
        }
        // 对端是明文的 Adiantum 实例 → 走默认 VFS
        return QString("ATTACH 'file:%1' AS %2;")
               .arg(otherPath, generatedAttachName);
    }

    // 对端不是 Adiantum 实例 → 走默认 VFS
    return QString("ATTACH '%1' AS %2;")
           .arg(otherPath, generatedAttachName);
}
```

> **回退清理**：跨密钥 attach 后，若主库或被 attach 库关闭，
> `unregisterMainDbKey` 应在 VFS xClose 内基于引用计数触发，避免
> keymap 泄漏。详见 §4.2 keymap 设计。

### 4.6 VFS Pragma 处理 (xFileControl / FCNTL_PRAGMA)

> ⚠️ **关键 ABI 修正**：早期 anl.md 把 `pArg` 当作虚构的 `sqlite3_pragma_handler*` 结构体，
> **该结构体不存在**。SQLite 头注释（仓库内 `Plugins/DbSQLite3MC/sqlite3mc_amalgamation.h:1075-1093`）：
>
> > The argument to the SQLITE_FCNTL_PRAGMA file control is an array of pointers to strings (`char**`)
> > in which the second element is the name of the pragma and the third element is the argument…
> > The handler can optionally make the first element point to a string obtained from `sqlite3_mprintf()`…
> > If the file control returns `SQLITE_NOTFOUND`, then normal PRAGMA processing continues.
>
> 必须按 `char**` 三元组访问，未知 pragma 必须返回 `SQLITE_NOTFOUND`（让 SQLite
> 自己解析），**绝不能**透传给底层 unix VFS 的 xFileControl——底层不识别 PRAGMA 字面量。

```cpp
// 在 adiantum_vfs.cpp 中实现
static int adiantumFileControl(sqlite3_file* pFile, int op, void* pArg) {
    AdiantumCtx* ctx = getContext(pFile);

    // ★ 拦截 mmap_size 设置：强制锁为 0，禁止 SQLite 走 xFetch/xUnfetch 旁路 xRead
    if (op == SQLITE_FCNTL_MMAP_SIZE) {
        sqlite3_int64* p = static_cast<sqlite3_int64*>(pArg);
        *p = 0;                                            // 同时把回传给上层的值清零
        return SQLITE_OK;
    }

    if (op == SQLITE_FCNTL_PRAGMA) {
        // SQLite ABI: pArg = char*[3]
        //   azArg[0] —— OUT，错误/结果消息；用 sqlite3_mprintf 分配
        //   azArg[1] —— pragma 名（只读）
        //   azArg[2] —— pragma 值（只读，无参数时为 NULL）
        char** azArg = static_cast<char**>(pArg);
        const char* name  = azArg[1];
        const char* value = azArg[2] ? azArg[2] : "";

        auto setKeyFromRaw = [&](const QByteArray& raw) -> int {
            if (raw.size() != 32) {
                azArg[0] = sqlite3_mprintf(
                    "Adiantum: key must be exactly 32 bytes (got %d)", raw.size());
                return SQLITE_ERROR;
            }
            ctx->deriveSubKeys(reinterpret_cast<const uint8_t*>(raw.constData()));
            ctx->initialized = true;
            return SQLITE_OK;                              // 返回 OK：SQLite 把
                                                           // PRAGMA 当成已处理
        };

        if (qstricmp(name, "hexkey") == 0) {
            return setKeyFromRaw(QByteArray::fromHex(QByteArray(value)));
        }
        if (qstricmp(name, "key") == 0) {
            // raw bytes（注意不能用 strlen——可能含 0x00）
            // 这里假设 PRAGMA key('...') 的字面量已经是 32 字符 ASCII；
            // 严肃实现需要走 SQL 参数绑定路径，详见 §5.4 一期裁剪说明。
            return setKeyFromRaw(QByteArray(value));
        }
        // 注意: textkey 一期不实现，详见 §5.4
        // 任何未知 PRAGMA → SQLITE_NOTFOUND，让 SQLite 走默认解析
        return SQLITE_NOTFOUND;
    }

    // 其他 FileControl 透传给底层 VFS（SQLITE_FCNTL_LOCK_TIMEOUT 等）
    return ctx->pRealFile->pMethods->xFileControl(ctx->pRealFile, op, pArg);
}
```

---

## 五、加密原语实现方案

### 5.1 所需最小密码学原语

| 原语 | 用途 | 推荐来源 | 备注 |
|------|------|---------|------|
| AES-256 ECB | 单块加密/解密 (HBSH 核心) | OpenSSL / libsodium / 微软 BCrypt | 只需单块模式 |
| ChaCha20 (12轮) | XChaCha 流密码 | libsodium (`crypto_core_hchacha20` + `crypto_stream_chacha20_ietf`) | **必须是12轮**, 非20轮 |
| Poly1305 | MAC (tweak hash + message hash) | libsodium (`crypto_onetimeauth_poly1305`) | 标准 |
| NH | Universal Hash (消息摘要) | **需自行实现** | 无成熟库提供, 但算法简单(~50行C代码) |
| Argon2id | textkey KDF | libsodium (`crypto_pwhash_argon2id`) | 或 libargon2 |

### 5.2 推荐方案: 基于 libsodium

libsodium 覆盖了大部分需求:

```
libsodium 提供的功能:
  ✅ crypto_onetimeauth_poly1305          → Poly1305 MAC
  ✅ crypto_stream_chacha20_ietf          → ChaCha20 (需指定 rounds=12)
  ✅ crypto_core_hchaCha20_ietf           → HChaCha (XChaCha 的第一步)
  ⚠️ crypto_pwhash_argon2id               → Argon2id KDF —— salt 强制 16 字节，
                                              与 Go 端 43 字节 salt 不兼容！
                                              详见 §5.4
  ✅ crypto_aead_aes256_gcm_encrypt/decrypt → AES-256 GCM 不能直接当 ECB；ECB
                                              单块需自行实现或调 OpenSSL EVP
  ❌ NH hash                              → 需自实现 (~50 行)
```

**备选方案**: 如果不想引入 libsodium 依赖，可以:
- AES: 使用系统 OpenSSL (SQLiteStudio 可能已链接)
- ChaCha20/Poly1305: 自行实现（RFC 7539 有完整测试向量）
- NH: 必须自实现（算法本身很简单）
- Argon2id: 使用独立 libargon2 或自实现

### 5.3 NH 哈希参考实现要点

```
NH(key[0..klen-1], message[0..mlen-1]):

前提: klen >= mlen + 48

初始化:
  sums[0..3] = 0   // 4 个 uint64 累加器

对每 16 字节消息块 mi (mi = message[i:i+16]):
  取对应 48 字节密钥段 ki = key[i*3 : i*3+48]
  分解为 4 组 (每组 4 个 uint64):
    ki[0..3], ki[4..7], ki[8..11], ki[12~15]

  分解消息块为 4 个 uint64:
    m0, m1, m2, m3

  累加:
    sums[0] += (m0 + ki[0]) * (m2 + ki[2])
    sums[1] += (m0 + ki[4]) * (m2 + ki[6])
    sums[2] += (m0 + ki[8]) * (m2 + ki[10])
    sums[3] += (m0 + ki[12])* (m2 + ki[14])
    sums[0] += (m1 + ki[1]) * (m3 + ki[3])
    sums[1] += (m1 + ki[5]) * (m3 + ki[7])
    sums[2] += (m1 + ki[9]) * (m3 + ki[11])
    sums[3] += (m1 + ki[13])* (m3 + ki[15])

输出: sums[0..3] 作为 32 字节小端序列
```

所有算术均为 uint64，无溢出检查（允许自然回绕）。

> **NH 在 4096 字节块上的应用**: NH 不是直接对整个 4096 字节做一次，而是按
> ≤1024 字节 chunk 切分；每个 chunk 复用同一段 NH key (1072 = 1024 + 48)，
> 输出 32 字节 → `mac.Write(NH(chunk))` 喂给 Poly1305；最后不足 16 字节的尾部
> **零填充到 16 字节倍数后再 NH**。`keyNH` 大小 1072 字节恰好满足
> `mlen <= 1024` 时的下界 `klen >= mlen + 48`。
>
> 早期 anl.md §5.3 伪代码如果照字面 `klen >= mlen + 48` 实现成"全块一次 NH"，
> 对 4096 字节明文会需要 12288 字节 key，与 §2.1.1 的 1072 字节冲突——
> 必须按上述 chunk 化实现。

### 5.4 textkey 一期裁剪、二期方案

| 阶段 | 通道 | 实现方式 | 说明 |
|---|---|---|---|
| 一期 | `hexkey` | `QByteArray::fromHex` → 32 字节 | ★ 业务端 `conn.go` 实际用法 |
| 一期 | `key`    | 直接取前 32 字节 ASCII | 兼容 Go 端罕见用法 |
| 二期 | `textkey` | 引入 `libargon2` (`argon2id_hash_raw`) | 见下 |

**为什么一期不能用 libsodium 实现 textkey**：

Go 实现固定 salt = `"github.com/ncruces/go-sqlite3/vfs/adiantum"`（43 字节）。
Go 的 `argon2.IDKey` 接受任意长 salt；而 libsodium 的 `crypto_pwhash` 强制
`crypto_pwhash_SALTBYTES = 16` 字节，不接受 43 字节。直接调用会失败；
自行截断或哈希到 16 字节后得到的派生密钥与 Go 端**不一致**，textkey
将永远解不开 Go 创建的库。

**二期方案**：

```cmake
# 二期: 加入独立 argon2 库
find_package(argon2 REQUIRED)
target_link_libraries(DbAdiantum PRIVATE argon2::argon2)
```

```cpp
// 二期: textkey → 32 字节
QByteArray adiantumDeriveTextKey(const QByteArray& password) {
    constexpr const char* PEPPER =
        "github.com/ncruces/go-sqlite3/vfs/adiantum";
    QByteArray key(32, '\0');
    int rc = argon2id_hash_raw(
        /*t_cost=*/3,
        /*m_cost=*/64 * 1024,           // 64 MiB
        /*parallelism=*/4,
        password.constData(), password.size(),
        PEPPER, /*saltlen=*/std::strlen(PEPPER),  // 任意长 salt
        key.data(), 32);
    return rc == ARGON2_OK ? key : QByteArray();
}
```

---

## 六、构建配置 (CMakeLists.txt)

```cmake
cmake_minimum_required(VERSION 3.16)
project(DbAdiantum)

option(WITH_DbAdiantum "Build DbAdiantum plugin (Adiantum VFS encryption)" ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
# 密码库依赖 (一期: libsodium 覆盖 ChaCha20/Poly1305；AES 单块走 OpenSSL)
find_package(sodium QUIET)           # ChaCha20/Poly1305
find_package(OpenSSL QUIET)          # AES-256 ECB 单块

set(PROJECT_HEADERS
    dbadiantum_global.h
    dbadiantum.h
    dbadiantuminstance.h
    adiantum_vfs.h
    adiantum_cipher.h
)

set(PROJECT_SOURCES
    dbadiantum.cpp
    dbadiantuminstance.cpp
    adiantum_vfs.cpp
    adiantum_cipher.cpp
    # NH hash 实现 (无成熟库可用，必须自实现 ~100 行)
)

add_library(DbAdiantum SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(DbAdiantum)

target_compile_definitions(DbAdiantum PRIVATE DBADIANTUM_LIBRARY)
target_include_directories(DbAdiantum PRIVATE ${sodium_INCLUDE_DIR})

# ★ 关键: 必须复用核心 SQLite (SQLite::SQLite3 / SQLite::Headers)
#   只有共享同一个 sqlite3 库，sqlite3_vfs_register("adiantum", ...)
#   注册的 VFS 才能被同进程的核心 sqlite3 看到。
#   绝不要在本插件里 vendor 第二份 sqlite3 amalgamation
#   （那是 DbSQLite3MC 的做法，理由是 SQLite3MC 必须用 mc_ 前缀符号隔离）。
target_link_libraries(DbAdiantum
    PRIVATE
        Qt::Core
        coreSQLiteStudio
        SQLite::SQLite3              # 由 cmake/custom_sqlite3.cmake 提供
        SQLite::Headers              # sqlite3.h
        sodium::sodium
        OpenSSL::Crypto              # AES-256 单块 (EVP_aes_256_ecb)
)
```

**与 STD_SQLITE3_DRIVER 配合**：

```cpp
// dbadiantuminstance.h
// 空 prefix / 空 uppercase prefix —— 直接用核心 sqlite3 的标准符号
STD_SQLITE3_DRIVER(AdiantumDriver, "Adiantum", , )
```

顶层 Plugins/CMakeLists.txt 需添加:
```cmake
option(WITH_DbAdiantum "Build DbAdiantum plugin" ON)
# ...
if(WITH_DbAdiantum)
    add_subdirectory(DbAdiantum)
endif()
```

---

## 七、测试策略

### 7.1 单元测试 (按优先级)

| 测试项 | 方法 | 验证目标 |
|--------|------|---------|
| **NH 哈希** | 已知输入/输出向量 | 与 Go 版本 `nh_generic.go` 一致 |
| **Poly1305** | RFC 测试向量 | 标准兼容性 |
| **HBSH 加解密** | 单块 4096 字节 round-trip | 明文 == 解密(加密(明文)) |
| **XChaCha12** | 已知 keystream 测试向量 | 与 Go `xchacha.go` 一致 |
| **密钥派生** | 固定 master key → 检查子密钥输出 | AES/NH/Poly 子密钥正确分割 |
| **VFS 块对齐** | 非 4096 对齐的 read/write | 部分块读写正确性 |
| **延迟 PRAGMA** | 打开无密钥 → PRAGMA set key → 读成功 | 新建加密 DB 流程 |
| **跨边界 I/O** | 读写跨越两个 4096 块的数据 | 多块处理正确性 |

### 7.2 集成测试（交叉兼容性）

```
测试场景 A (互操作性):
  1. Go 程序创建加密数据库 (使用 go-sqlite3/vfs/adiantum)
  2. SQLiteStudio 打开该数据库 → 应能正常读取
  3. SQLiteStudio 修改数据并关闭
  4. Go 程序重新打开 → 应能读到修改后的数据

测试场景 B (反向):
  1. SQLiteStudio 创建新的加密数据库
  2. Go 程序打开 → 应能正常读取

测试场景 C (明文兼容):
  1. SQLiteStudio 以 plain=true 打开普通 .db 文件 → 应正常工作
  2. Go 程序以相同方式打开 → 应正常工作
```

### 7.3 性能基准

对比指标:
- 纯 SQLite3 (无加密) 基线
- Adiantum VFS 加密的吞吐量
- 目标: 加密开销 < 30% (基于内存中 AES-NI + ChaCha12 的效率预估)

---

## 八、风险与挑战

| 风险 | 等级 | 影响 | 缓解措施 |
|------|------|------|---------|
| **加密算法实现 bug** | 高 | 数据损坏/无法读取 | 严格遵循 Go 源码；充分单元测试；保留测试向量 |
| **XChaCha12 rounds 不一致** | 中 | 加密结果不兼容 | Go 版明确用 12 轮，非标准的 20 轮；必须确认 |
| **NH 哈希边缘情况** | 中 | 大块数据出错 | Go 版每 chunk <=1024 字节，最后块零填充到 16 字节倍数 |
| **libsodium 版本/ABI** | 低 | 编译/运行失败 | 可选 vendoring 或使用 CMake FetchContent |
| **并发安全性** | 中 | 多线程崩溃 | VFS 内部加锁；ctx->blockBuf 每线程或 mutex 保护 |
| **WAL 模式兼容性** | 中 | WAL 文件加密异常 | 确保 WAL/journal 文件的 VFS 正确分发密钥 |
| **大文件性能** | 低 | 大 DB 变慢 | 4KiB 块已是对齐好的大小；考虑 mmap 优化(后期) |

---

## 九、实施路线图

### Phase 1: 加密算法纯实现 (预计 3-4 天)
- [ ] 实现 NH hash (C/C++, ~100 行含测试)
- [ ] 实现Poly1305 MAC (或调用 libsodium)
- [ ] 实现 XChaCha12 stream cipher (12 轮)
- [ ] 实现 AES-256 ECB 单块 (调用 OpenSSL/libsodium)
- [ ] 实现 HBSH Encrypt/Decrypt (组合以上原语)
- [ ] 实现子密钥派生 (makeAdiantum)
- [ ] 编写全部已知向量测试

### Phase 2: SQLite VFS 封装 (预计 2-3 天)
- [ ] 实现 AdiantumVFS 注册/注销
- [ ] 实现 xOpen (包装底层 VFS, 创建 AdiantumCtx)
- [ ] 实现 xRead (块对齐读取 + 解密)
- [ ] 实现 xWrite (read-modify-write + 加密)
- [ ] 实现 xTruncate (向上取整到 4096)
- [ ] 实现 xSectorSize, DeviceCharacteristics, ChunkSize, SizeHint
- [ ] 实现 xFileControl (FCNTL_PRAGMA 处理 key/hexkey/textkey)
- [ ] 实现辅助文件的加密判断逻辑
- [ ] 延迟 PRAGMA (hbsh==nil 时的 EOF 返回)

### Phase 3: SQLiteStudio 插件集成 (预计 1-2 天)
- [ ] 创建 DbAdiantum 插件骨架 (参照 DbSQLite3MC)
- [ ] 实现 getLabel(), getOptionsList(), newInstance()
- [ ] 实现 DbAdiantumInstance (重写 openInternal/initAfterOpen/getAttachSql)
- [ ] 配置 CMakeLists.txt (链接 libsodium + coreSQLiteStudio)
- [ ] 注册到顶层 Plugins/CMakeLists.txt
- [ ] 编译并通过加载测试

### Phase 4: 测试与调优 (预计 2 天)
- [ ] 交叉兼容性测试 (Go ↔ C++)
- [ ] 多线程并发测试
- [ ] WAL/Journal 模式测试
- [ ] 大数据库 (>100MB) 性能测试
- [ ] 错误注入测试 (错误密钥、损坏文件等)

---

## 十、结论

**可行性评估: ⭐⭐⭐⭐ 高度可行**

核心技术路径清晰:
1. **加密层**: 从 Go 源码精确移植 HBSH/XChaCha12/AES/NH 算法 (有完整参考实现)
2. **VFS 层**: SQLite 标准 C 接口，包装模式成熟 (类似 SQLCipher 的做法)
3. **插件层**: SQLiteStudio 插件架构完善，DbSQLite3MC 提供完美模板
4. **密钥格式**: 与 demo 代码 `conn.go` 完全兼容 (`hexkey` → 32 字节 raw key)

**最大工作量**: C++ 加密算法的正确移植 (Phase 1+2, ~5-7 天)
**推荐依赖**: libsodium（ChaCha20/Poly1305）+ OpenSSL（AES 单块）
**自实现**: NH universal hash (~100 行 C 代码)
**二期依赖**: libargon2（处理任意长 salt 的 textkey 通道）

---

## 十一、本文档修订记录（相对早期版本）

| # | 修订点 | 位置 | 原因 |
|---|---|---|---|
| 1 | `minAppVersion` 40000 → 30300 | §3.3 | 现役 DbPlugin 全部 30300，40000 会被加载器拒绝 |
| 2 | `getOptionsList` 用 PASSWORD 类型并加 `optPragmas: SQL` | §3.4 | 对齐 DbSqliteMc 三段式；textkey 移到二期 |
| 3 | `openInternal` 重写：预注册 hexkey、明文模式走 `nullptr` VFS、`enableLoadExtension()` 改为 `db_config(DBCONFIG_ENABLE_LOAD_EXTENSION)` | §4.3 | `enableLoadExtension()` 不存在；hexkey 必须在 open_v2 之前注册；plain 模式需禁用 VFS |
| 4 | `initAfterOpen` 移除硬编码 `journal_mode=WAL` 与 `busy_timeout`；新增 `PRAGMA mmap_size=0`；用 `escapeString(hexKey)` | §4.4 | 不能强制改写既有库；mmap 会旁路 xRead；与 MC 风格一致 |
| 5 | `getAttachSql` 处理跨密钥 ATTACH（预注册 + URI 形式 ATTACH） | §4.5 | VFS 加密无法在 SQL 里传 KEY |
| 6 | `xFileControl(SQLITE_FCNTL_PRAGMA)` 改为 `char**` 三元组 ABI；未知 PRAGMA 返回 `SQLITE_NOTFOUND`；新增 `SQLITE_FCNTL_MMAP_SIZE` 拦截 | §4.6 | `sqlite3_pragma_handler*` 结构体不存在；底层 unix VFS 不识别 PRAGMA |
| 7 | `AdiantumVFS` 加入按文件名隔离的密钥注册表（含 WAL/Journal/SHM 后缀回查） | §4.2 | 多库并发会串密钥；WAL/Journal 文件需要主库密钥 |
| 8 | `xRead` 短读必须返回 `SQLITE_IOERR_SHORT_READ`，不是"EOF" | §2.6.1 | SQLite 没有 EOF 返回码；这是延迟 PRAGMA 流程的关键约定 |
| 9 | NH 对 4096 块按 ≤1024 chunk 切分 + Poly1305 合并 | §5.3 末尾 | `keyNH` 1072 字节只够单 chunk，必须按 chunk 切 |
| 10 | textkey 一期裁掉，二期用 `libargon2` | §5.4（新增） | libsodium 强制 16 字节 salt，无法兼容 Go 端 43 字节 salt |
| 11 | CMake 链接 `SQLite::SQLite3 / SQLite::Headers / coreSQLiteStudio`，禁止 vendor 第二份 sqlite3 | §6 | 共享 sqlite3 才能让 `sqlite3_vfs_register` 全局生效 |
| 12 | `STD_SQLITE3_DRIVER(AdiantumDriver, "Adiantum", , )` 用空 prefix | §6 末尾 | 复用核心 sqlite3 的标准符号 |
