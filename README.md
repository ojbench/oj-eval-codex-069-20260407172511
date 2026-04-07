# Problem 069 - Wutong的内存管理器

**ACMOJ Problem ID**: 2917

## Table of Contents

- [Problem 069 - Wutong的内存管理器](#problem-069-wutong的内存管理器)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
    - [Background](#background)
  - [Assignment Description](#assignment-description)
    - [Grade Composition](#grade-composition)
  - [Assignment Requirements](#assignment-requirements)
    - [Input Format](#input-format)
    - [Output Format](#output-format)
    - [Samples](#samples)
    - [Data Constraints](#data-constraints)
  - [Per-Testcase Resource Limits](#per-testcase-resource-limits)
  - [Test Data](#test-data)
  - [Submission Requirements](#submission-requirements)
    - [OJ Git Repository Compilation Process](#oj-git-repository-compilation-process)
    - [Git Configuration Requirements](#git-configuration-requirements)
    - [Submission Guidelines](#submission-guidelines)
    - [Evaluation Notes](#evaluation-notes)
    - [Academic Integrity](#academic-integrity)

## Introduction

### Background

Implement simple memory manager with linked list of blocks. Support alloc, free, coalescing.

## Assignment Description

### Grade Composition

| Grading Component | Percentage |
| :--: | :--: |
| Pass **2917. Wutong的内存管理器** | 80% |
| Code Review | 20% |

Here are several points that need clarification:

- In the Code Review, we will **strictly examine your code style and repository organization structure, etc.**

- This assignment provides some sample data for testing, stored in the `/workspace/data/069/data_test/` directory. Note that these are not the test cases on the Online Judge. Passing all local test cases does not guarantee that you will pass the OJ tests.

- Besides the provided sample data, we also encourage you to design your own test data based on your program logic to assist debugging.

## Assignment Requirements

### Problem Description

**Update**:如果你在使用 **dev-c++** 且不会多文件编译，可以使用下发文件中的 **merged.cpp** ,提交时 **仅提交头文件部分**

**Update**:下发文件中的test.cpp即为OJ上所有的测试点

`new` 和 `delete` 为程序提供了在运行时动态管理内存的强大能力，但其背后隐藏着内存管理器的复杂工作。一个高效的内存管理器必须解决两个核心问题：

1.**大小追踪**：当程序调用 `delete ptr;` 时，并未提供待释放内存的大小。内存管理器必须有办法记录每个已分配内存块的尺寸，以便能精确回收。

2.**内存碎片化**：频繁的内存分配与释放，会导致内存池中出现大量不连续的小块空闲空间（即**内存碎片**）。这可能导致尽管总空闲内存充足，却没有一块足够大的 *连续* 空间来满足新的分配请求。

在本题中，你将通过维护一个**内存块链表**，来**模拟**一个简易内存管理器的核心逻辑。

> 提示：你可以通过实现简单的部分获取部分分！你可以观察数据点的要求来决定实现的方法。

## 内存块的模拟

在真实的内存管理器中，整个内存池被组织成一系列的内存块，每个块都由两部分构成：**头部（Header）**和紧随其后的**数据区（Data）**。

为了模拟这一过程，我们使用 `MemoryBlock` 结构体来代表内存中的一个块。这个结构体本身就扮演了**头部（Header）**的角色，其成员变量描述了对应**数据区**的信息：

- `address`: 数据区在概念内存池中的起始地址。
- `size`: 数据区的大小（用户可用的字节数）。
- `is_free`: 标记该块当前是否空闲。

与现实中通过指针运算计算下一个块位置不同，我们使用一个显式的 `next` 指针来将所有内存块组织成一个**单向链表**。头指针 `head` 指向内存池的第一个块。

### 运行逻辑

1. **初始化内存池**：调用 `resetMemory` 函数时，整个 `1MB` 的内存池被视为一个单一的空闲块，由链表的第一个节点表示。
2. **内存分配**：当用户请求分配内存时，`allocate` 函数会遍历链表，寻找第一个空闲且大小足够的块。如果找到一个合适的块，就将其标记为已分配，并根据需要拆分成两个块。
3. **内存释放**：当用户释放内存时，`deallocate` 函数会将对应块标记为空闲。如果该块的下一个块也是空闲的，则将它们合并成一个更大的空闲块，以减少碎片化。

### 分配策略 (30分 + 40分)

当有新的数据需要记录到内存中时，我们需要一个确定的方案来插入它，这就是**分配策略**。为了简单起见，我们会采用**首次适配（First Fit）**的分配策略，即顺序**遍历**内存块链表，找到第一个空闲且大小足够的块进行分配。

当有内存请求时，如果找到的空闲块比请求的大小大得多，为了节约空间，可以将其**拆分（Split）**成两个块：一个用于满足当前请求（大小正好），另一个作为新的、较小的空闲块保留在链表中。

### 释放策略 (20分)

当用户不再需要某块内存时，会调用 `deallocate` 函数将其释放。释放操作的核心是将该块标记为空闲，以便未来的分配请求可以重用这部分内存。

### 块合并 (10分)

如果我们在释放时，只是简单地将块标记为空闲，那么随着时间的推移，链表中会出现大量相邻的、细碎的空闲块，这就是**内存碎片化**问题。

为了解决这个问题，我们需要实现**块合并**。当释放一个内存块时，如果它的**下一个**块（通过 `next` 指针访问）也是空闲的，那么我们可以将这两个块合并成一个更大的空闲块。

对于此次任务，你只需要实现**向后合并**，即在释放一个块时，只检查并合并其**后面**的相邻空闲块。


## 头文件模板(template.h)

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace sjtu {

constexpr size_t MEMORY_SIZE = 1024 * 1024; // 1 MB

// 内存块结构体
struct MemoryBlock{
    int address;
    int size;
    bool is_free;

    // 链表指针
    MemoryBlock* next = nullptr;
};

// 全局变量
// TODO 你可以自由添加其他变量
inline MemoryBlock* head = nullptr; // 内存块链表头指针

// ------- 辅助函数 ------- //

/**
 * @brief 将一个大的内存块拆分为两块。
 * @param block 要拆分的内存块
 * @param size 拆分后原内存块的大小
 * @return 拆分后新块的指针（如果拆分失败,返回nullptr）
 */
inline MemoryBlock* splitBlock(MemoryBlock* block, size_t size) {
    if (block->size < size + sizeof(MemoryBlock)) {
        return nullptr; // 无法拆分
    }
    // 创建新的内存块
    MemoryBlock* new_block = new MemoryBlock;
    new_block->address = block->address + sizeof(MemoryBlock) + size;
    new_block->size = block->size - size - sizeof(MemoryBlock);
    new_block->is_free = true;
    new_block->next = block->next;
    // 更新原内存块
    block->size = size;
    block->next = new_block;

    return new_block;
}

/**
 * @brief 合并相邻的内存块。
 * @param block 要合并的内存块
*/
inline void mergeBlock(MemoryBlock* block){
    MemoryBlock* next = block->next;
    if (next == nullptr) {
        return;
    }
    block->size += sizeof(MemoryBlock) + next->size;
    block->next = next->next;
    delete next;
}

// ------ 以下是你要实现的内存分配和释放函数 ------ //

/**
 * @brief 分配指定大小的内存块。
 * @param size 要分配的内存大小（以字节为单位）
 * @return 指向分配内存块的指针（如果内存不足,返回nullptr）
 */
inline MemoryBlock* allocate(int size){
    // TODO 请自行实现内存分配的逻辑
    return nullptr;
}

/**
 * @brief 释放之前分配的内存块。请自行实现并块操作
 * @param ptr 指向要释放的内存块的指针
 */
inline void deallocate(MemoryBlock* ptr){
    // TODO 请自行实现内存分配的逻辑
}
/**
 * @brief 重置内存管理器，初始化内存块链表。如果你在实现中使用了额外的全局变量，请在此函数中进行初始化。
 */
inline void resetMemory(){
    // 释放之前的链表
    MemoryBlock* temp = head;
    while (temp) {
        MemoryBlock* next = temp->next;
        delete temp;
        temp = next;
    }
    
    // 初始化新的内存块链表
    head = new MemoryBlock;
    head->address = 0;
    head->size = MEMORY_SIZE - sizeof(MemoryBlock);
    head->is_free = true;
}

} // namespace sjtu

```

### Input Format

See the problem description above.

### Output Format

See the problem description above.

### Samples

No sample data provided for this problem.

### Data Constraints

你需要填充下发的头文件模板。下发文件中的 `test.cpp` 是一份测试样例。

[attachment]Pointer.zip[/attachment]

| 测试点编号 | 要求功能 | 分数 |
| :--- | :--- | :--- |
| 1, 2, 3 | 实现 `allocate` (只分配一次) | 30 |
| 4, 5, 6, 7 | `allocate` (分配多次) | 40 |
| 8, 9 | `deallocate` (单块重用) | 20 |
| 10 | `deallocate`的块合并 | 10 |

### 说明

- 所有内存操作均合法。
- `allocate` 的 `size` 参数指的是用户请求的数据区大小，不包含 `MemoryBlock` 头部本身的大小。
- 实现中只需调用提供的工具函数，不必对链表进行更多的修改

## Per-Testcase Resource Limits

- **Time Limit (per test case)**: 1000 ms
- **Memory Limit (per test case)**: 256 MiB
- **Disk Usage**: No disk usage is permitted.

## Test Data

The test data for this problem is located at `/workspace/data/069/data_test/`.

Attachments and starter files for this problem are located at `/workspace/data/069/`.

## Submission Requirements

### OJ Git Repository Compilation Process

For Git compilation, we will first clone the repository using a command similar to:
```bash
git clone <repo_url> . --depth 1 --recurse-submodules --shallow-submodules --no-local
```

Then we check if there is a `CMakeLists.txt` file. If it exists, we run (if not, a warning message will be displayed):
```bash
cmake .
```

Finally, we check if there is any of `GNUmakefile`/`makefile`/`Makefile` (if cmake was run previously, this will be the generated Makefile). If it exists, we run (if not, a warning message will be displayed):
```bash
make
```

After this process is complete, we will use the `code` file in the project root directory as the compilation result.

A `CMakeLists.txt` file is provided in the project. You can use or modify it as needed. The local environment has gcc-13 and g++-13 available.

### Git Configuration Requirements

**IMPORTANT**: You must create a `.gitignore` file in your project root directory to avoid OJ evaluation conflicts.

The `.gitignore` file should include at least the following entries:

```gitignore
CMakeFiles/
CMakeCache.txt
```

### Submission Guidelines

- The submitted code must be able to compile successfully through the above compilation process
- The compiled executable file name must be `code`
- The program needs to be able to read data from standard input and write results to standard output
- Please ensure the code runs correctly within the given time and space limits
- **You must use C++ or C language** to implement this assignment

### Evaluation Notes

- The evaluation system will test your program using the provided test data
- The program output must exactly match the expected output (including format)
- Exceeding time or memory limits will be judged as the corresponding error type
- Please pay attention to the overall time performance of your code and the time complexity of each part of your algorithm.

### Academic Integrity

If any violations are found during evaluation or code review (including but not limited to using unconventional methods to pass test cases), your final score may be significantly reduced or become **0 points**.
