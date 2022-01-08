#include <algorithm>
#include <iostream>
#include <memory>
#include <list>
#include <type_traits>
#include <utility>

//with respect to example http://coliru.stacked-crooked.com/a/cfd0c5c5021596ad

template <typename T>
class HeapAllocator
{
    union node
	{
        node* next;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    };
    node *list = nullptr;
    void clear() noexcept
	{
        auto p = list;
        while (p)
		{
            auto tmp = p;
            p = p->next;
            delete tmp;
        }
        list = nullptr;
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    using propagate_on_container_move_assignment = std::true_type;
    HeapAllocator() noexcept = default;
    HeapAllocator(const HeapAllocator&) noexcept {}
    template <typename U>
    HeapAllocator(const HeapAllocator<U>&) noexcept {}
    HeapAllocator(HeapAllocator&& other) noexcept :  list(other.list)
	{
        other.list = nullptr;
    }
    HeapAllocator& operator = (const HeapAllocator&) noexcept
	{
        return *this;
    }
    HeapAllocator& operator = (HeapAllocator&& other) noexcept
	{
        clear();
        list = other.list;
        other.list = nullptr;
        return *this;
    }
    ~HeapAllocator() noexcept { clear(); }
    T* allocate(size_type n)
	{
        if (n == 1)
		{
            auto ptr = list;
            if (ptr) list = list->next;
			else ptr = new node;
            return reinterpret_cast<T*>(ptr);
        }
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    void deallocate(T* ptr, size_type n) noexcept
	{
        if (n == 1)
		{
            auto node_ptr = reinterpret_cast<node*>(ptr);
            node_ptr->next = list;
            list = node_ptr;
        }
		else ::operator delete(ptr);
    }
};

template <typename T, typename U>
inline bool operator == (const HeapAllocator<T>&, const HeapAllocator<U>&)
{
    return true;
}

template <typename T, typename U>
inline bool operator != (const HeapAllocator<T>&, const HeapAllocator<U>&)
{
    return false;
}
