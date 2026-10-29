#pragma once

#include <iostream>
#include <new>

namespace SimpleAlloc {
    template <typename T>
    class MySimpleAllocator {
    public:
        // 1. Обязательные типы данных
        using value_type = T;

        // 2. Дефолтный конструктор и конструктор копирования
        MySimpleAllocator() noexcept = default;

        template <typename U>
        MySimpleAllocator(const MySimpleAllocator<U>&) noexcept {}

        // 3. Выделение памяти под n элементов типа T
        T* allocate(size_t n) {
            if (n == 0) return nullptr;

            // Выводим отладочную информацию в консоль
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator] Memory allocation for " << n
                << " element(s) with a size of " << sizeof(T) << " byte all .\n";
#endif // PRINT_TECH_ALLOCATION

            // Выделяем сырую память
            void* ptr = ::operator new(n * sizeof(T));
            return static_cast<T*>(ptr);
        }

        // 4. Освобождение памяти
        void deallocate(T* p, size_t n) noexcept {
            if (p == nullptr) return;
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator] Freeing up memory from " << n
                << " element(s).\n";
#endif // PRINT_TECH_ALLOCATION
            ::operator delete(p);
        }

        // 5. Операторы сравнения (обязательны для аллокаторов)
        // Если аллокаторы равны, значит память, выделенная одним, может быть освобождена другим.
        template <typename U>
        bool operator==(const MySimpleAllocator<U>&) const noexcept { return true; }

        template <typename U>
        bool operator!=(const MySimpleAllocator<U>&) const noexcept { return false; }
    };
}

namespace PmrAllocator {
    class MyMemoryResource {
    public:
        // Выделяет сырые байты с учетом выравнивания (alignment)
        virtual void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Resource] Physical isolation " << bytes
                << " byte (alignment): " << alignment << ").\n";
#endif // PRINT_TECH_ALLOCATION
            // Используем современный оператор выделения с выравниванием (C++17)
            return ::operator new(bytes, std::align_val_t(alignment));
        }

        // Освобождает сырые байты
        virtual void deallocate(void* ptr, size_t bytes, size_t alignment = alignof(std::max_align_t)) noexcept {
            if (!ptr) return;
#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Resource] Physical liberation " << bytes << " byte.\n";
#endif // PRINT_TECH_ALLOCATION
            ::operator delete(ptr, std::align_val_t(alignment));
        }

        virtual ~MyMemoryResource() = default;

        // Сравнение ресурсов (два разных ресурса не могут освобождать память друг друга)
        bool is_equal(const MyMemoryResource& other) const noexcept {
            return this == &other;
        }
    };

    static MyMemoryResource default_res;

    template <typename T>
    class MyPmrAllocator {
    private:
        MyMemoryResource* res_ = nullptr; // Ссылка на настоящий источник памяти

    public:
        using value_type = T;
        using is_always_equal = std::false_type;

        MyPmrAllocator() noexcept {
            // Создаем ленивую инициализацию статического ресурса по умолчанию
            res_ = &default_res;
        }

        // Конструктор обязательно принимает ресурс
        explicit MyPmrAllocator(MyMemoryResource* res) noexcept : res_(res) {
            if (!res_)
                res_ = &default_res;
        }

        // Конструктор копирования для разных типов (КРИТИЧЕСКИ ВАЖЕН ДЛЯ REBIND)
        // Когда int-аллокатор копируется в int*-аллокатор, мы просто передаем указатель на тот же ресурс
        template <typename U>
        MyPmrAllocator(const MyPmrAllocator<U>& other) noexcept : res_(other.resource()) {}

        // ЯВНО РАЗРЕШАЕМ КОПИРОВАНИЕ (Решает ошибку C2280)
        MyPmrAllocator(const MyPmrAllocator& other) noexcept : res_(other.res_) {}

        MyPmrAllocator& operator=(const MyPmrAllocator& other) noexcept {
            if (this != &other) {
                res_ = other.res_;
            }
            return *this;
        }
        MyPmrAllocator(MyPmrAllocator&& other) noexcept : res_(other.res_) {
            other.res_ = &default_res; // Безопасно перенаправляем старый аллокатор на дефолтный ресурс
        }
        // Оператор перемещающего присваивания
        MyPmrAllocator& operator=(MyPmrAllocator&& other) noexcept {
            if (this != &other) {
                res_ = other.res_;
                other.res_ = &default_res; // Сброс источника
            }
            return *this;
        }

        // Вспомогательный метод, чтобы другие типы аллокатора могли достать ресурс
        MyMemoryResource* resource() const noexcept { return res_; }

        T* allocate(size_t n) {
            if (n == 0) return nullptr;

#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator <" << typeid(T).name() << ">] I am redirecting the request to "
                << n << " elements in MyMemoryResource.\n";
#endif // PRINT_TECH_ALLOCATION

            // Переводим штуки (n) в байты и отдаем ресурсу
            size_t total_bytes = n * sizeof(T);
            void* raw_ptr = res_->allocate(total_bytes, alignof(T));

            return static_cast<T*>(raw_ptr);
        }

        void deallocate(T* p, size_t n) noexcept {
            if (!p) return;

#ifdef PRINT_TECH_ALLOCATION
            std::cout << "[Allocator <" << typeid(T).name() << ">] Redirecting the release. "
                << n << " elements in MyMemoryResource.\n";
#endif // PRINT_TECH_ALLOCATION

            size_t total_bytes = n * sizeof(T);
            res_->deallocate(p, total_bytes, alignof(T));
        }

        template <typename U, typename... Args>
        void construct(U* p, Args&&... args) {
            // Проверяем, умеет ли тип U принимать аллокатор при конструировании
            if constexpr (std::uses_allocator_v<U, MyPmrAllocator>) {
                // Если да (например, это вложенный pmr-контейнер или pmr::string),
                // конструируем его, передавая наш текущий аллокатор (*this) в конец или в начало аргументов
                std::allocator_traits<MyPmrAllocator>::construct(*this, p, std::forward<Args>(args)...);
            }
            else {
                // Если тип U — это обычный тип (int, double, обычный класс),
                // используем стандартный placement new, как в вашем коде
                ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
            }
        }

        // 2. Метод уничтожения одного элемента
        template <typename U>
        void destroy(U* p) noexcept {
            p->~U(); // Вызываем деструктор напрямую
        }

        // Вспомогательный метод для диапазона (опционально, но полезно)
        template <typename ForwardIterator>
        void destroy(ForwardIterator first, ForwardIterator last) noexcept {
            while (first != last) {
                destroy(std::addressof(*first));
                ++first;
            }
        }

        // Операторы сравнения теперь проверяют, на один ли ресурс они указывают
        template <typename U>
        bool operator==(const MyPmrAllocator<U>& other) const noexcept {
            return res_->is_equal(*other.resource());
        }

        template <typename U>
        bool operator!=(const MyPmrAllocator<U>& other) const noexcept {
            return !(*this == other);
        }
    };
}