# Project 1 六人分工与提交方案

## 一、提交目标

本实验按照 `Tutorial for Project1.pdf` 完成 BusTub Project 1：

- Project 1 要求的六个实现文件；
- 代码能够编译；
- 三个模块的测试能够运行；
- 格式、lint 和 clang-tidy 检查能够运行；
- 仓库中有清楚的实验说明和必要的截图；
- 不提交 `build/`、临时文件、日志和无关代码。

实验包含三个模块：

1. Extendible Hash Table；
2. LRU-K Replacer；
3. Buffer Pool Manager Instance。

---

## 二、六人分工

| 成员 | 主要职责 | 负责文件 | 协作对象 |
|---|---|---|---|
| 1号 | Extendible Hash Table 实现 | `src/include/container/hash/extendible_hash_table.h`；`src/container/hash/extendible_hash_table.cpp` | 2号、5号 |
| 2号 | 哈希表测试与边界验证 | `test/container/hash/extendible_hash_table_test.cpp` | 1号、6号 |
| 3号 | LRU-K Replacer 实现 | `src/include/buffer/lru_k_replacer.h`；`src/buffer/lru_k_replacer.cpp` | 4号、5号 |
| 4号 | LRU-K 测试与算法材料 | `test/buffer/lru_k_replacer_test.cpp`；阅读 `src/buffer/clock_replacer.cpp`、`src/buffer/lru_replacer.cpp` | 3号、6号 |
| 5号 | Buffer Pool Manager 实现 | `src/include/buffer/buffer_pool_manager_instance.h`；`src/buffer/buffer_pool_manager_instance.cpp` | 1号、3号 |
| 6号 | 集成测试、仓库整理和最终提交 | `test/buffer/buffer_pool_manager_instance_test.cpp`；根目录 `CMakeLists.txt` | 全体成员 |

每个人都要负责：代码阅读、自己模块的测试、问题记录和最终提交前检查。6号负责汇总，不代替其他成员完成核心实现。

---

## 三、1号：Extendible Hash Table

### 负责内容

实现可扩展哈希表，用于维护 `page_id -> frame_id` 的映射。

### 必须完成

- `Find(K, V)`；
- `Insert(K, V)`；
- `Remove(K)`；
- `GetGlobalDepth()`；
- `GetLocalDepth(dir_index)`；
- `GetNumBuckets()`；
- 桶满时的分裂；
- global depth 达到 local depth 时的目录扩容；
- 分裂后的 key/value 重新分配；
- 使用互斥锁保证并发安全。

### 注意事项

- 不要修改既有函数签名；
- 已存在的 key 应更新 value，而不是重复插入；
- 必须在插入前判断桶是否会溢出；
- global depth 和 local depth 的含义不能混淆；
- 不能用另一个 STL 哈希表替代实验要求的结构。

### 交付

- 实现代码；
- 对应测试通过；
- 一段桶分裂和目录扩容的说明；
- 提交前告知 2号和 5号接口已经稳定。

---

## 四、2号：哈希表测试

### 负责内容

在 [`test/container/hash/extendible_hash_table_test.cpp`](./test/container/hash/extendible_hash_table_test.cpp) 中验证 1号的实现。

### 测试范围

- 空表查询；
- 插入、查询和删除；
- 重复 key 更新 value；
- 删除不存在的 key；
- 单次桶分裂；
- 多次桶分裂；
- global depth 增长；
- local depth 检查；
- 大量随机 key；
- 多线程查询和插入；
- 分裂后所有 key 仍可查询。

### 交付

- 测试代码；
- 每个失败测试的最小复现数据；
- 测试结果截图；
- 测试清单和已知限制。

测试不能通过降低断言要求或删除原有测试来“修复”。

---

## 五、3号：LRU-K Replacer

### 负责内容

实现 LRU-K 页面替换器。

### 必须完成

- `Evict(frame_id_t *)`；
- `RecordAccess(frame_id_t)`；
- `Remove(frame_id_t)`；
- `SetEvictable(frame_id_t, bool)`；
- `Size()`；
- 访问时间记录；
- backward k-distance 计算；
- 少于 k 次访问的 frame 按无穷大距离处理；
- 多个无穷大距离 frame 按最早访问时间淘汰；
- 所有共享状态的互斥保护。

### 注意事项

- 访问历史和可淘汰状态是两套不同状态；
- 不可淘汰的 frame 不能被 `Evict` 选中；
- `SetEvictable` 重复设置相同状态时不能重复修改 size；
- `Remove` 后必须清除该 frame 的访问历史；
- 需要检查 frame_id 越界。

### 交付

- 实现代码；
- LRU-K 测试通过；
- 说明 k=2 时不同访问序列的淘汰顺序；
- 与 4号确认 Buffer Pool 调用方式。

---

## 六、4号：LRU-K 测试与算法说明

### 负责内容

在 [`test/buffer/lru_k_replacer_test.cpp`](./test/buffer/lru_k_replacer_test.cpp) 中测试 LRU-K，并整理 CLOCK、LRU、LRU-2、LRU-K 的区别。

### 测试范围

- 初始 `Size()`；
- 没有可淘汰 frame 时 `Evict`；
- `RecordAccess`；
- 访问次数少于 k 次；
- 多个无穷大距离 frame；
- 达到 k 次访问后的距离比较；
- `SetEvictable` 状态变化；
- `Remove` 后历史清理；
- 淘汰后 size 更新；
- 并发调用替换器接口。

### 仓库材料

可以在说明文档中加入以下简表：

| 算法 | 淘汰依据 | 主要特点 |
|---|---|---|
| LRU | 最近一次访问 | 简单，但容易受顺序扫描影响 |
| CLOCK | reference bit 和时钟指针 | 开销低，是 LRU 的近似 |
| LRU-2 | 倒数第 2 次访问 | 能识别较稳定的热点 |
| LRU-K | 倒数第 K 次访问 | 访问历史更充分，但实现更复杂 |

补充材料只用于理解和说明，不要把 CLOCK 或普通 LRU 当成 Project 1 的实现目标。

### 交付

- 测试代码；
- 算法对比文字；
- 测试截图；
- 与 3号一起确认测试结果。

---

## 七、5号：Buffer Pool Manager Instance

### 负责内容

实现页面在磁盘和内存之间的调度。

### 主要接口

- `FetchPgImp(page_id)`；
- `UnpinPgImp(page_id, is_dirty)`；
- `FlushPgImp(page_id)`；
- `NewPgImp(page_id)`；
- `DeletePgImp(page_id)`；
- `FlushAllPgsImp()` 或头文件中对应的实现函数。

### 处理要求

#### Fetch

1. 先查找 page table；
2. 页面已在内存时增加 pin count 并记录访问；
3. 否则先从 free list 找 frame；
4. free list 为空时调用 LRU-K 淘汰；
5. 被淘汰页面为 dirty 时先写回磁盘；
6. 更新 page table、Page 元数据和 replacer 状态；
7. 所有 frame 都被 pin 时返回 `nullptr`。

#### New

1. 分配新的 page_id；
2. 找到空闲或可淘汰 frame；
3. 必要时写回旧 dirty 页面；
4. 初始化新 Page；
5. 建立映射并设置正确的 pin count。

#### Unpin

1. 查找页面；
2. 防止 pin count 减到负数；
3. 更新 dirty 状态；
4. pin count 变为 0 时设置为可淘汰。

#### Flush

- 页面不存在时返回失败；
- 不论 pin count 是否为 0，都允许刷新；
- 写盘成功后清除 dirty 状态。

#### Delete

- pinned 页面不能删除；
- 从 page table 删除；
- 从 replacer 移除；
- 重置 Page；
- 把 frame 放回 free list；
- 调用 `DeallocatePage()`。

### 交付

- 实现代码；
- Buffer Pool 测试通过；
- 验证 dirty 页面淘汰前会写盘；
- 验证 pinned 页面不会被淘汰；
- 验证删除后的 frame 可以重新使用。

---

## 八、6号：集成、截图和最终提交

### 负责内容

- 确认三个模块接口能够连接；
- 运行单模块和完整测试；
- 收集各成员测试结果；
- 整理说明文档和截图；
- 检查 Git 仓库；
- 生成 Project 1 提交包。

### 集成重点

重点检查以下状态变化：

```text
Fetch -> page table -> free list/LRU-K -> dirty flush -> disk read -> pin
Unpin -> pin count 减少 -> dirty 更新 -> pin count 为 0 -> 可淘汰
Delete -> 检查 pin count -> 移除映射 -> 移除替换器状态 -> 回收 frame
```

### 负责测试

Buffer Pool 测试文件：

[`test/buffer/buffer_pool_manager_instance_test.cpp`](./test/buffer/buffer_pool_manager_instance_test.cpp)

重点检查：

- New、Fetch、Unpin、Flush、Delete；
- free list 耗尽；
- LRU-K 淘汰；
- dirty 页面写回；
- pinned 页面无法淘汰；
- 页面删除后 frame 复用；
- 多线程访问。

---

## 九、构建、测试和检查命令

以下命令以当前仓库路径为准。先进入：

```bash
cd /home/lixep/DBSP_6/Project1
mkdir -p build
cd build
cmake ..
```

### 单模块测试

```bash
make extendible_hash_table_test -j$(nproc)
./test/extendible_hash_table_test

make lru_k_replacer_test -j$(nproc)
./test/lru_k_replacer_test

make buffer_pool_manager_instance_test -j$(nproc)
./test/buffer_pool_manager_instance_test
```

当前仓库的 CMake 会把测试程序生成到：

```text
Project1/build/test/
```

### 完整测试和代码检查

```bash
make check-tests
make format
make check-format
make check-lint
make check-clang-tidy-p1
```

`make format` 会修改格式；修改后还应运行 `make check-format` 确认格式已经统一。所有命令的实际结果都要如实记录，不能把未运行的命令写成“通过”。

### 生成提交包

当前仓库的 `CMakeLists.txt` 提供 `submit-p1`：

```bash
make submit-p1
```

提交包会包含 Tutorial 指定的六个 Project 1 实现文件：

```text
src/include/container/hash/extendible_hash_table.h
src/container/hash/extendible_hash_table.cpp
src/include/buffer/lru_k_replacer.h
src/buffer/lru_k_replacer.cpp
src/include/buffer/buffer_pool_manager_instance.h
src/buffer/buffer_pool_manager_instance.cpp
```

生成的 `project1-submission.zip` 位于 `Project1/` 根目录。提交前检查压缩包内容。

---

## 十、截图和图片要求

如果老师要求提交截图，建议保存到：

```text
Project1/docs/images/
```

建议准备以下图片：

| 文件名 | 内容 | 负责人 |
|---|---|---|
| `01-build-success.png` | `cmake` 和目标编译成功 | 6号 |
| `02-hash-table-test.png` | Extendible Hash Table 测试通过 | 2号 |
| `03-lru-k-test.png` | LRU-K 测试通过 | 4号 |
| `04-buffer-pool-test.png` | Buffer Pool 测试通过 | 5号 |
| `05-checks.png` | `check-format`、`check-lint`、`check-clang-tidy-p1` 结果 | 6号 |
| `06-submit-package.png` | 提交包生成和文件清单 | 6号 |
| `07-git-status.png` | 最终 Git 状态 | 6号 |

截图必须能看清：

- 执行的命令；
- 测试名称；
- `PASSED`、成功退出或检查成功信息；
- 终端路径；
- 必要的错误信息。

不能只截取命令输入部分，也不能通过裁剪隐藏失败信息。

每张图片建议在 `Project1/docs/images/README.md` 中记录：

```text
图片名称：
执行命令：
对应代码或测试文件：
证明的实验要求：
结果：
```

截图是辅助证据，不能替代源代码、测试代码和实际提交包。

---

## 十一、时间安排

### 第一天：准备

- 全员阅读 Tutorial；
- 1号和2号确认哈希表接口；
- 3号和4号确认 LRU-K 状态；
- 5号阅读 Buffer Pool 生命周期；
- 6号建立测试和提交清单。

### 第二至三天：并行实现

- 1号实现哈希表；
- 2号准备哈希表测试；
- 3号实现 LRU-K；
- 4号准备 LRU-K 测试；
- 5号实现 Buffer Pool；
- 6号准备集成测试。

### 第四至五天：模块和集成测试

- 先分别通过哈希表和 LRU-K 测试；
- 再运行 Buffer Pool 测试；
- 修复接口、dirty 状态、pin count 和淘汰问题。

### 第六天：检查

- 运行完整测试；
- 运行 format、lint、clang-tidy；
- 修复检查错误；
- 收集截图。

### 第七天：提交

- 生成 `project1-submission.zip`；
- 检查压缩包内容；
- 删除临时文件和编译产物；
- 检查 Git 状态；
- 提交最终仓库。

---

## 十二、最终验收清单

### 代码

- [ ] 六个 Project 1 实现文件均已完成；
- [ ] 没有修改规定的函数签名；
- [ ] 哈希表支持查找、插入、更新、删除、分裂和扩容；
- [ ] LRU-K 正确记录访问历史和可淘汰状态；
- [ ] Buffer Pool 正确处理 Fetch、New、Unpin、Flush、Delete；
- [ ] dirty 页面复用前会写回；
- [ ] pinned 页面不会被淘汰；
- [ ] 共享状态有适当的锁保护。

### 测试和检查

- [ ] 三个模块的单元测试通过；
- [ ] `make check-tests` 已运行；
- [ ] `make check-format` 通过；
- [ ] `make check-lint` 已运行；
- [ ] `make check-clang-tidy-p1` 已运行；
- [ ] 测试失败和已知限制已经如实记录。

### 提交内容

- [ ] 提交包内容与 `P1_FILES` 一致；
- [ ] 截图和图片说明齐全；
- [ ] 没有提交 `build/`、日志、临时文件和无关图片；
- [ ] 最终仓库路径和文件名正确；
- [ ] 最终提交前由 6号完成一次全量检查。
