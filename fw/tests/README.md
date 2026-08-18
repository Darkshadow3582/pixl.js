# fw/tests — 宿主机单元测试

在开发机（macOS/Linux）上直接编译运行的单元测试，不依赖 nRF52 SDK、交叉编译工具链或硬件。
覆盖固件中可脱离硬件测试的纯逻辑模块。

## 运行

```bash
make test     # 编译并运行全部测试
make          # 只编译
make clean
```

依赖：任意 C 编译器 + OpenSSL 开发头文件（`nrf_crypto` mock 使用 libcrypto 实现 AES-128-CTR 与 HMAC-SHA256）。
macOS 自带环境通常可直接运行；Ubuntu 需要 `libssl-dev`。

## 覆盖的模块

| 测试 | 被测代码 | 内容 |
|---|---|---|
| test_ntag_store | `src/ntag/ntag_store.c` | 默认 NTAG215 数据、UID/BCC 规则、随机 UUID |
| test_ntag_emu | `src/ntag/ntag_emu_v2.c` | NFC T2T 命令状态机（READ/WRITE/GET_VERSION/SECTOR_SELECT/PWD_AUTH、只读保护、脏标记事件） |
| test_vfs_meta | `src/mod/vfs/vfs_meta.c` | 元数据编码/解码往返 |
| test_amiitool | `components/amiitool` | DRBG 确定性、keygen、密钥加载校验、amiibo 加解密 pack/unpack 往返、篡改检测 |
| test_amiibo_helper | `src/amiibo_helper.c` | CRC16-MCRF4XX、UUID/密码替换规则、amiibo 生成与重签 |
| test_switch_read | `ntag_emu_v2.c` + `amiibo_helper.c` | 重放主机读取 amiibo 的 NFC 命令序列（GET_VERSION → READ page3 → PWD_AUTH(UID 派生密码) → FAST_READ 三段 0-59/60-119/120-134），并验证全标签顺序扫描 |

## 结构

- `test_framework.h`：零依赖的最小断言框架
- `mocks/`：nRF SDK 头文件的宿主机替身（nrf_log、sdk_errors、app_scheduler、boards 等），
  以及带捕获功能的 `hal_nfc_t2t_mock`（可向固件注入 NFC 读卡器命令）和
  基于 OpenSSL 的 `nrf_crypto_mock`
- CI：`.github/workflows/pixl.js-fw.yml` 中的 `unit-test` job 在每次 push/PR 时运行

新增测试：在 `fw/tests/test_xxx.c` 写测试，在 `Makefile` 里加上对应的对象列表和链接规则。
