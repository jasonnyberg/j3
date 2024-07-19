#include <gtest/gtest.h>

#include "alloc.h"


// add a test that displays the size of a compressed pointer
TEST(CompressedPointerTest, SizeOfCompressedPointer) {
    EXPECT_EQ(sizeof(int *), 8);
    EXPECT_EQ(sizeof(CompressedPointer<int>), 4);
}

TEST(CompressedPointerTest, DefaultConstructor) {
    CompressedPointer<int> ptr;
    EXPECT_FALSE(ptr.valid());
    EXPECT_FALSE(ptr);
}

TEST(CompressedPointerTest, ConstructorWithIndexAndOffset) {
    CompressedPointer<int> ptr(1, 2);
    EXPECT_FALSE(ptr.valid());
    EXPECT_TRUE(ptr);
}

TEST(CompressedPointerTest, Dereference) {
    CompressedPointer<int> ptr = CompressedPointer<int>::allocate();
    *ptr                       = 42;
    EXPECT_TRUE(ptr.valid());
    EXPECT_EQ(*ptr, 42);
}

TEST(CompressedPointerTest, ArrowOperator) {
    struct TestStruct {
        int value;
    };

    CompressedPointer<TestStruct> ptr = CompressedPointer<TestStruct>::allocate();
    ptr->value                        = 42;
    EXPECT_EQ(ptr->value, 42);
}

TEST(CompressedPointerTest, BoolConversion) {
    CompressedPointer<int> ptr1;
    EXPECT_FALSE(ptr1);

    CompressedPointer<int> ptr2 = CompressedPointer<int>::allocate();
    EXPECT_TRUE(ptr2);
}

TEST(CompressedPointerTest, Allocate) {
    CompressedPointer<int> ptr = CompressedPointer<int>::allocate();
    EXPECT_TRUE(ptr);
}

TEST(CompressedPointerTest, Deallocate) {
    CompressedPointer<int> ptr = CompressedPointer<int>::allocate();
    ptr.deallocate();
    EXPECT_FALSE(ptr);
}

TEST(CompressedPointerTest, FromRawPtr) {
    CompressedPointer<int> ptr = CompressedPointer<int>::allocate();
    *ptr                       = 42;
    int *rawPtr                = ptr; // ptr.operator->();
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(*rawPtr, 42);
    ptr.deallocate();
}

TEST(AllocatorTest, Allocate) {
    Allocator<int>                allocator;
    std::pair<uint16_t, uint16_t> result = allocator.allocate();
    EXPECT_NE(result.first + result.second, 0);
}

TEST(AllocatorTest, Deallocate) {
    Allocator<int>                allocator;
    std::pair<uint16_t, uint16_t> result = allocator.allocate();
    allocator.deallocate(result.first, result.second);
}

TEST(AllocatorTest, GetPtr) {
    Allocator<int>                allocator;
    std::pair<uint16_t, uint16_t> result = allocator.allocate();
    int                          *ptr    = allocator.getPtr(result.first, result.second);
    *ptr                                 = 42;
    EXPECT_EQ(*ptr, 42);
    allocator.deallocate(result.first, result.second);
}

TEST(AllocatorTest, GetSlabInfo) {
    Allocator<int>                allocator;
    std::pair<uint16_t, uint16_t> result   = allocator.allocate();
    int                          *rawPtr   = allocator.getPtr(result.first, result.second);
    std::pair<uint16_t, uint16_t> slabInfo = allocator.getSlabInfo(rawPtr);
    EXPECT_EQ(slabInfo.first, result.first);
    EXPECT_EQ(slabInfo.second, result.second);
    allocator.deallocate(result.first, result.second);
}

TEST(AllocatorTest, AllocateAllElements) {
    Allocator<int> allocator;

    // Allocate all elements in the allocator
    std::vector<std::pair<uint16_t, uint16_t>> allocations;
    for (size_t i = 0; i < NUM_SLABS * SLAB_SIZE - 1; ++i) {
        std::pair<uint16_t, uint16_t> allocation = allocator.allocate();
        ASSERT_NE(allocation.first + allocation.second, 0);
        allocations.push_back(allocation);
        if (!(i % 10000)) { std::cout << "Allocated " << i << " elements" << std::endl; }
    }

    // Try to allocate one more element (should return null)
    std::pair<uint16_t, uint16_t> nullAllocation = allocator.allocate();
    EXPECT_EQ(nullAllocation.first, 0);
    EXPECT_EQ(nullAllocation.second, 0);

    // Deallocate all previously allocated elements
    for (const auto &allocation : allocations) { allocator.deallocate(allocation.first, allocation.second); }
}

struct ComplexItem {
    int         value;
    std::string name;

    ComplexItem(int v, const std::string &n) : value(v), name(n) {}
    ComplexItem() : value(0), name("") {}

    static void *operator new(std::size_t size) {
        void *ptr = CompressedPointer<ComplexItem>::allocate();
        if (!ptr) { throw std::bad_alloc(); }
        return ptr;
    }

    static void operator delete(void *ptr, std::size_t size) noexcept {
        CompressedPointer<ComplexItem> cp = CompressedPointer<ComplexItem>::fromRawPtr(static_cast<ComplexItem *>(ptr));
        cp.deallocate();
    }
};

TEST(CompressedPointerTest, ComplexItemTest) {
    CompressedPointer<ComplexItem> ptr = CompressedPointer<ComplexItem>::allocate();
    ptr->value                         = 42;
    ptr->name                          = "Hello, World!";
    EXPECT_EQ(ptr->value, 42);
    EXPECT_EQ(ptr->name, "Hello, World!");
}

TEST(CompressedPointerTest, CastTest) {
    CompressedPointer<ComplexItem> ptr  = CompressedPointer<ComplexItem>::allocate();
    ptr->value                          = 42;
    ptr->name                           = "Hello, World!";
    ComplexItem                   *ci   = ptr;
    CompressedPointer<ComplexItem> ptr2 = ci;
    EXPECT_EQ(ptr2->value, 42);
    EXPECT_EQ(ptr2->name, "Hello, World!");
}

TEST(CompressedPointerTest, ComplexItemFromRawPtr) {
    CompressedPointer<ComplexItem> ptr = CompressedPointer<ComplexItem>::allocate();
    *ptr                               = ComplexItem(42, "Hello, World!");
    ComplexItem *rawPtr                = ptr.operator->();
    EXPECT_EQ(rawPtr->value, 42);
    EXPECT_EQ(rawPtr->name, "Hello, World!");
    ptr.deallocate();
}

TEST(CompressedPointerTest, ComplexItemNewDelete) {
    ComplexItem *ptr = new ComplexItem(42, "hello world!");
    EXPECT_EQ(ptr->value, 42);
    EXPECT_EQ(ptr->name, "hello world!");
    CompressedPointer<ComplexItem> cp = ptr;
    EXPECT_EQ(cp->value, 42);
    EXPECT_EQ(cp->name, "hello world!");
    EXPECT_TRUE(cp.valid());
    delete ptr;
    EXPECT_FALSE(cp.valid());
}

TEST(CompressedPointerTest, ComplexItemAllocation) {
    CompressedPointer<ComplexItem> cp = new ComplexItem(42, "hello world!");
    EXPECT_EQ(cp->value, 42);
    EXPECT_EQ(cp->name, "hello world!");
    EXPECT_TRUE(cp.valid());
    delete cp;
    EXPECT_FALSE(cp.valid());
}
