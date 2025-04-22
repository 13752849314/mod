//
// Created by admin on 25-3-30.
//

#ifndef SHARED_PTR_H
#define SHARED_PTR_H
#include <atomic>

/**
 * 1. 不考虑删除器和空间配置器
 * 2. 不考虑弱引用
 * 3. 考虑引用计数的线程安全
 */
template <typename T>
class shared_ptr
{
private:
    T *ptr; // 指向管理的对象
    std::atomic<std::size_t> *ref_count; // 原子引用计数

    void release() const;

public:
    shared_ptr(); // 默认构造函数
    // explicit关键字，防止隐式类型转换
    explicit shared_ptr(T *p); // 构造函数

    ~shared_ptr(); // 析构函数

    shared_ptr(const shared_ptr<T> &other); // 拷贝构造函数

    shared_ptr<T> &operator=(const shared_ptr<T> &other); // 拷贝赋值运算符

    shared_ptr(shared_ptr<T> &&other) noexcept; // 移动构造函数

    shared_ptr<T> &operator=(shared_ptr<T> &&other) noexcept; // 移动赋值运算符

    T &operator*() const; // 解引用运算符

    T *operator->() const; // 箭头运算符

    std::size_t use_count() const; // 获取引用计数

    T *get() const; // 获取原始指针

    void reset(T* p=nullptr); // 重置指针

};

void test();

#endif //SHARED_PTR_H
