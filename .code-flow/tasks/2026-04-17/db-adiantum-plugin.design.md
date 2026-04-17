# 设计简报：SQLiteStudio Adiantum VFS 加密插件

## 目标

- 为 SQLiteStudio 实现 Adiantum VFS 加密插件（DbAdiantum）
- 支持打开和创建使用 ncruces/go-sqlite3/vfs/adiantum 加密的 SQLite 数据库
- 实现与 Go 版本完全兼容的 HBSH (XChaCha12 + AES-256) 加密算法

## 非目标

- 不实现 textkey/口令加密通道（一期仅支持 hexkey）（原因：libsodium 强制 16 字节 salt，与 Go 端 43 字节 salt 不兼容）
- 不实现 SQLCipher 类型的页级 PRAGMA 加密（Adiantum 是文件层 VFS 加密）

## 技术决策

- **加密库**: libsodium（ChaCha20/Poly1305）+ OpenSSL（AES-256 ECB 单块）+ 自实现 NH hash
- **VFS 名称**: `adiantum`
- **密钥格式**: 64 字符十六进制（解码后 32 字节）
- **插件类型**: DbPlugin（参照 DbSQLite3MC 模板）
- **minAppVersion**: 30300（与现役 DbPlugin 一致）

## 技术方案

### 变更范围

| 模块 | 变更 |
|------|------|
| `Plugins/DbAdiantum/` | 新增插件目录 |
| `SQLiteStudio3/coreSQLiteStudio/` | 无变更（复用现有 sqlite3） |
| `Plugins/CMakeLists.txt` | 添加 DbAdiantum 子目录 |

### 插件文件结构

```
Plugins/DbAdiantum/
├── CMakeLists.txt
├── dbadiantum_global.h
├── dbadiantum.json
├── dbadiantum.h / .cpp
├── dbadiantuminstance.h / .cpp
├── adiantum_vfs.h / .cpp
└── adiantum_cipher.h / .cpp
```

### 核心类继承体系

```
QObject
  └── GenericPlugin ──── virtual public Plugin
        │
        └── DbPluginStdFileBase ──── public DbPlugin
              │
              └── DbAdiantum
                    │
                    └── AbstractDb3<AdiantumDriver> ──── public AbstractDb
                          │
                          └── DbAdiantumInstance
```

### 关键实现要点

1. **openInternal() 重写**:
   - hexkey 预注册到 VFS 全局 keymap（xOpen 之前）
   - open_v2 第 4 参数传入 `"adiantum"` VFS 名称

2. **initAfterOpen() 重写**:
   - 执行 `PRAGMA mmap_size = 0`（防止 mmap 旁路 VFS）
   - 执行 `PRAGMA hexkey('...')` 设置密钥
   - 执行用户自定义 PRAGMA（journal_mode 等由用户填写，不再硬编码）

3. **VFS xFileControl 处理**:
   - 拦截 `SQLITE_FCNTL_PRAGMA`：处理 key/hexkey/textkey
   - 拦截 `SQLITE_FCNTL_MMAP_SIZE`：强制清零

4. **密钥查表规则**:
   - 主库路径为 key，WAL/Journal 文件自动去除后缀回查主库 ctx
   - 使用 std::unordered_map 按文件名隔离密钥

### 加密算法栈

```
Adiantum = HBSH(StreamCipher=XChaCha12, BlockCipher=AES-256, Hash=NH+Poly1305)
  ├── XChaCha12 (12 轮，非 20 轮)
  ├── AES-256 ECB 单块
  └── NH + Poly1305（消息摘要）
```

## Go 实现参考

### 参考源码

| 文件 | 用途 | 来源 |
|------|------|------|
| `vfs/adiantum/adiantum.go` | VFS 封装、hbshFile | github.com/ncruces/go-sqlite3 |
| `vfs/adiantum/hbsh.go` | HBSH Encrypt/Decrypt | github.com/ncruces/go-sqlite3 |
| `lukechampine.com/adiantum` | Adiantum 主库 | lukechampine.com/adiantum |
| `lukechampine.com/adiantum/nh` | NH hash | lukechampine.com/adiantum |
| `lukechampine.com/adiantum/hbsh` | HBSH 原语 | lukechampine.com/adiantum |

### DSN 格式规范

**加密模式** (`conn.go:47`):
```
file:%s?vfs=adiantum&_pragma=hexkey('%s')&_pragma=busy_timeout(5000)
```

**明文模式** (`conn.go:40`):
```
file:%s?_pragma=busy_timeout(5000)
```

**关键参数顺序**: `vfs=adiantum` 必须在 `_pragma=hexkey()` 之前

### 密钥验证规范

```go
// conn.go:19
var hex64Re = regexp.MustCompile(`^[0-9a-fA-F]{64}$`)
```

- 必须是 64 个十六进制字符（0-9, a-f, A-F）
- 解码后为 32 字节原始密钥

## HBSH 加密算法详解

### 算法栈

```
Adiantum = HBSH(StreamCipher=XChaCha12, BlockCipher=AES-256, Hash=NH+Poly1305)
```

### 子密钥派生 (makeAdiantum)

```go
// adiantum.go:makeAdiantum
func makeAdiantum(key []byte, chachaRounds int) (stream, block, hash) {
    stream := &chachaStream{key, chachaRounds}
    keyBuf := make([]byte, 32+16+16+1072)  // 1136 bytes
    stream.XORKeyStream(keyBuf, nil)  // 全零 nonce 生成 keystream

    // 分割: [32]AES密钥 + [16]Poly1305_T + [16]Poly1305_M + [1072]NH密钥
}
```

### HBSH Encrypt

```
输入: block[N], tweak[8]
PL = block[0:N-16]      // 前 N-16 字节
PR = block[N-16:N]      // 后 16 字节

PM = PR ⊕ Hash(T, PL)           // GF(2^128) 加法
CM = AES256_ECB_Encrypt(PM)     // AES 单块加密
CL = CM ⊕ XChaCha12(CM, nonce)  // nonce=[CM[16:], tweak, 0x01]
CR = CM ⊖ Hash(T, CL)           // GF(2^128) 减法

输出: CL || CR
```

### HBSH Decrypt (精确逆过程)

```
输入: block[N], tweak[8]
CL = block[0:N-16]
CR = block[N-16:N]

CM = CR ⊕ Hash(T, CL)           // GF(2^128) 加法
PL = CM ⊕ XChaCha12(CM, nonce) // nonce=[CM[16:], tweak, 0x01]
PM = AES256_ECB_Decrypt(CM)     // AES 单块解密
PR = PM ⊖ Hash(T, PL)           // GF(2^128) 减法

输出: PL || PR
```

### GF(2^128) 运算

```go
// bits.Add64 / bits.Sub64 实现
func blockAdd(x, y []byte) []byte {
    x1 := binary.LittleEndian.Uint64(x[:8])
    x2 := binary.LittleEndian.Uint64(x[8:16])
    y1 := binary.LittleEndian.Uint64(y[:8])
    y2 := binary.LittleEndian.Uint64(y[8:16])
    r1, c := bits.Add64(x1, y1, 0)
    r2, _ := bits.Add64(x2, y2, c)
    return x
}
```

### NH Hash (自实现)

```go
// nh/nh_generic.go
func sum(out *[32]byte, m []byte, key []byte) {
    var k [16]uint32
    var sums [4]uint64

    for len(m) >= 16 && len(key) >= 16 {
        // 密钥滑动窗口
        k[0]=k[4]; k[1]=k[5]; ... k[15]=binary.LittleEndian.Uint32(key[12:16])

        // 消息分解
        m0 := binary.LittleEndian.Uint32(m[0:4])
        m1 := binary.LittleEndian.Uint32(m[4:8])
        m2 := binary.LittleEndian.Uint32(m[8:12])
        m3 := binary.LittleEndian.Uint32(m[12:16])

        // 累加乘积
        sums[0] += uint64(m0+k[0]) * uint64(m2+k[2])
        sums[1] += uint64(m0+k[4]) * uint64(m2+k[6])
        sums[2] += uint64(m0+k[8]) * uint64(m2+k[10])
        sums[3] += uint64(m0+k[12]) * uint64(m2+k[14])
        sums[0] += uint64(m1+k[1]) * uint64(m3+k[3])
        sums[1] += uint64(m1+k[5]) * uint64(m3+k[7])
        sums[2] += uint64(m1+k[9]) * uint64(m3+k[11])
        sums[3] += uint64(m1+k[13]) * uint64(m3+k[15])

        key = key[16:]; m = m[16:]
    }
}
```

### NH + Poly1305 组合 (HashNHPoly1305)

```go
// 1. Tweak Poly1305
tweakBuf[0:8] = 8 * len(msg) (uint64 LE)
poly1305.Sum(outT, tweakBuf + tweak, keyT)

// 2. NH + Poly1305 消息摘要 (按 1024 字节分块)
mac := poly1305.New(keyM)
for len(msg) >= 1024 {
    nh.Sum(outNH, msg[:1024], keyNH)
    mac.Write(outNH)
    msg = msg[1024:]
}
// 末尾不足 1024 字节：零填充到 16 字节倍数后 NH
if len(msg) > 0 { /* pad and NH */ }
mac.Sum(outM)

// 3. GF(2^128) 加法
result = addHashes(outT, outM)
```

### XChaCha12 Nonce 构造

```go
// nonce[24] = [CM[16:], tweak[8], 0x01]
nonceBuf := make([]byte, 24)
copy(nonceBuf, cm[16:])        // CM 的后 16 字节
copy(nonceBuf[16:], tweak)     // 8 字节 tweak
nonceBuf[23] = 1               // 固定值
```

## C++ 实现方案

### 加密库对应

| 功能 | Go 实现 | C++ 实现 |
|------|---------|----------|
| XChaCha12 | `xchacha.XORKeyStream(rounds=12)` | `crypto_stream_chacha20_ietf` + `crypto_core_hchacha20` |
| AES-256 ECB | `aes.NewCipher` | `EVP_aes_256_ecb` + `EVP_CIPHER_CTX` |
| Poly1305 | `poly1305.Sum` | `crypto_onetimeauth_poly1305` |
| NH hash | `nh.Sum` (自实现) | **自实现** (~60行) |
| GF(2^128) | `bits.Add64/Sub64` | `_addcarry_u64` / `_subborrow_u64` |

### 块大小常量

```cpp
const size_t TweakSize = 8;      // 字节
const size_t BlockSize = 4096;  // 字节 (SQLite 默认页大小)
const size_t NonceSize = 24;     // 字节
const size_t KeySize = 32;       // 字节 (256-bit)
const size_t KeyNHSize = 1072;  // 字节 (1024 + 48)
```

### 关键 C++ 代码片段

#### NH Hash 自实现

```cpp
void nh_sum(uint8_t out[32], const uint8_t* msg, size_t msgLen, const uint8_t* key) {
    uint32_t k[16];
    uint64_t sums[4] = {0};

    while (msgLen >= 16 && msgLen >= 16) {
        // 加载密钥滑动窗口
        for (int i = 4; i < 16; i++) {
            k[i] = *reinterpret_cast<const uint32_t*>(key);
            key += 4;
        }

        // 加载消息块
        uint32_t m0 = *reinterpret_cast<const uint32_t*>(msg);
        uint32_t m1 = *(reinterpret_cast<const uint32_t*>(msg) + 1);
        uint32_t m2 = *(reinterpret_cast<const uint32_t*>(msg) + 2);
        uint32_t m3 = *(reinterpret_cast<const uint32_t*>(msg) + 3);

        // 累加乘积
        sums[0] += uint64_t(m0 + k[0]) * uint64_t(m2 + k[2]);
        sums[1] += uint64_t(m0 + k[4]) * uint64_t(m2 + k[6]);
        sums[2] += uint64_t(m0 + k[8]) * uint64_t(m2 + k[10]);
        sums[3] += uint64_t(m0 + k[12]) * uint64_t(m2 + k[14]);
        sums[0] += uint64_t(m1 + k[1]) * uint64_t(m3 + k[3]);
        sums[1] += uint64_t(m1 + k[5]) * uint64_t(m3 + k[7]);
        sums[2] += uint64_t(m1 + k[9]) * uint64_t(m3 + k[11]);
        sums[3] += uint64_t(m1 + k[13]) * uint64_t(m3 + k[15]);

        msg += 16; msgLen -= 16;
    }

    // 写入输出
    for (int i = 0; i < 4; i++) {
        *reinterpret_cast<uint64_t*>(out + i*8) = sums[i];
    }
}
```

#### HBSH Encrypt

```cpp
void hbsh_encrypt(uint8_t block[4096], const uint8_t tweak[8]) {
    // PL = block[0:N-16], PR = block[N-16:N]
    // PM = PR ⊕ Hash(T, PL)
    // CM = AES256_ECB_Encrypt(PM)
    // CL = CM ⊕ XChaCha12(CM, nonce)
    // CR = CM ⊖ Hash(T, CL)

    // nonce 构造: [CM[16:], tweak, 0x01]
    uint8_t nonce[24];
    memcpy(nonce, block + 16, 16);
    memcpy(nonce + 16, tweak, 8);
    nonce[23] = 0x01;
}
```

### 自实现工作量估算

| 组件 | 代码量 | 复杂度 |
|------|--------|--------|
| NH hash | ~60 行 | 低 |
| HBSH Encrypt/Decrypt | ~40 行 | 低 |
| GF(2^128) 运算 | ~20 行 | 低 |
| 子密钥派生 | ~30 行 | 低 |
| VFS 封装 | ~200 行 | 中 |
| **总计** | **~350 行** | - |

## 并发安全处理

```go
// conn.go:60
db.SetMaxOpenConns(1)  // Go 通过限制连接数规避并发
```

C++ 实现需要在全局密钥注册表中使用 `std::mutex` 保护：

```cpp
static std::mutex s_keyMutex;
static std::unordered_map<std::string, std::shared_ptr<AdiantumCtx>> s_keyByPath;

void registerKey(const std::string& path, const uint8_t key[32]) {
    std::lock_guard<std::mutex> lock(s_keyMutex);
    s_keyByPath[path] = std::make_shared<AdiantumCtx>(key);
}

void unregisterKey(const std::string& path) {
    std::lock_guard<std::mutex> lock(s_keyMutex);
    s_keyByPath.erase(path);
}
```

## 约束条件

- 必须复用核心 SQLite（SQLite::SQLite3），禁止 vendor 第二份 sqlite3 amalgamation
- AdiantumDriver 使用空 prefix，直接用核心 sqlite3 标准符号
- xRead 在 hbsh==nil 且读头 100 字节时返回 SQLITE_IOERR_SHORT_READ（延迟设密钥流程）
- PRAGMA hexkey('...') 必须在 open_v2 之前预注册到 VFS keymap
- DSN 参数顺序：vfs=adiantum 在前，_pragma=hexkey() 在后
- 密钥验证正则：`^[0-9a-fA-F]{64}$`
- NH hash 按 1024 字节分块，末尾不足部分零填充到 16 字节倍数
- XChaCha12 必须使用 12 轮（非 20 轮）

## 验收标准

- [ ] 插件能正确编译并被 SQLiteStudio 加载
- [ ] 能打开 Go 端创建的 Adiantum 加密数据库
- [ ] 能创建新的 Adiantum 加密数据库
- [ ] 明文模式（plain=true）能打开普通未加密数据库
- [ ] WAL 模式文件正确加密
- [ ] ATTACH 其他 Adiantum 加密库能正常工作
- [ ] 单元测试覆盖：NH hash、HBSH 加解密、XChaCha12
- [ ] 与 Go 端交叉兼容性测试通过

## 与 Go 实现的关键差异（实现时需注意）

| 差异点 | Go 实现 | C++ 设计 | 注意事项 |
|--------|---------|----------|----------|
| mmap 处理 | 驱动透明处理 | 必须显式 `PRAGMA mmap_size=0` | C++ 必须禁用 mmap |
| VFS 注册 | import 时自动注册 | 插件初始化时显式注册 | 需在 Plugin::init() 中调用 sqlite3_vfs_register |
| 并发安全 | SetMaxOpenConns(1) | 全局注册表需加锁 | 多库并发时密钥隔离 |
| journal_mode | 硬编码 busy_timeout | 用户自定义 | 设计更灵活 |
