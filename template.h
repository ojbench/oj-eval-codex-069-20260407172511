#pragma once

#include <cstddef>
#include <cstdint>

namespace sjtu {

constexpr size_t MEMORY_SIZE = 1024 * 1024; // 1 MB

struct MemoryBlock {
    int address;
    int size;
    bool is_free;
    MemoryBlock* next = nullptr;
};

inline MemoryBlock* head = nullptr; // head of the block list

inline MemoryBlock* splitBlock(MemoryBlock* block, size_t size) {
    if (block->size < static_cast<int>(size + sizeof(MemoryBlock))) {
        return nullptr; // cannot split
    }
    MemoryBlock* new_block = new MemoryBlock;
    new_block->address = block->address + static_cast<int>(sizeof(MemoryBlock)) + static_cast<int>(size);
    new_block->size = block->size - static_cast<int>(size) - static_cast<int>(sizeof(MemoryBlock));
    new_block->is_free = true;
    new_block->next = block->next;

    block->size = static_cast<int>(size);
    block->next = new_block;
    return new_block;
}

inline void mergeBlock(MemoryBlock* block) {
    MemoryBlock* next = block->next;
    if (next == nullptr) return;
    block->size += static_cast<int>(sizeof(MemoryBlock)) + next->size;
    block->next = next->next;
    delete next;
}

inline MemoryBlock* allocate(int size) {
    if (size <= 0) return nullptr;
    // Lazy initialize the memory pool
    if (head == nullptr) {
        head = new MemoryBlock;
        head->address = 0;
        head->size = static_cast<int>(MEMORY_SIZE - sizeof(MemoryBlock));
        head->is_free = true;
        head->next = nullptr;
    }

    MemoryBlock* cur = head;
    while (cur) {
        if (cur->is_free && cur->size >= size) {
            // Split only if leftover can hold a header and at least 1 byte
            if (cur->size >= size + static_cast<int>(sizeof(MemoryBlock)) + 1) {
                splitBlock(cur, static_cast<size_t>(size));
            }
            cur->is_free = false;
            return cur;
        }
        cur = cur->next;
    }
    return nullptr;
}

inline void deallocate(MemoryBlock* ptr) {
    if (ptr == nullptr) return;
    ptr->is_free = true;
    // Merge forward if possible (only backward merge is required by spec)
    if (ptr->next && ptr->next->is_free) {
        mergeBlock(ptr);
    }
}

inline void resetMemory() {
    // Free previous list
    MemoryBlock* temp = head;
    while (temp) {
        MemoryBlock* next = temp->next;
        delete temp;
        temp = next;
    }
    // Initialize a fresh free block
    head = new MemoryBlock;
    head->address = 0;
    head->size = static_cast<int>(MEMORY_SIZE - sizeof(MemoryBlock));
    head->is_free = true;
    head->next = nullptr;
}

} // namespace sjtu

