// myDeque_Final.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.
#pragma once

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include <concepts>
#include <compare>
#include <algorithm>

#include <my_memory/my_allocator.hpp>
#include <myDeque/my_deque_buff.hpp>
#include <myDeque/deque_iterator.hpp>

namespace Deque {

    template <typename T> void construct(T* p, const T& rhs) {
        new (p) T(rhs);
    }
    template <typename T, typename ... Args> void construct(T* p, Args ... args) {
        new (p) T(std::forward<Args>(args)...);
    }
    template <class T>
    void destroy(T* p) {
        p->~T();
    }
    template <typename DequeIter>
    void destroy(DequeIter first, DequeIter last) {
        while (first != last) {
            destroy(&*first);
            ++first;
        }
    }

    template <typename T, typename Allocator = std::allocator<T>, size_t CHUNK_SIZE = 8>
    class MyDeque : private MyDequeBuff<T, Allocator, CHUNK_SIZE> {
    public:
        // Обязательные типы для STL-совместимости контейнера
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;

        using iterator = DequeIterator<T, CHUNK_SIZE, false>;
        using const_iterator = DequeIterator<T, CHUNK_SIZE, true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using Base = MyDequeBuff<T, Allocator, CHUNK_SIZE>;

        using Base::allocator_type;
        using Base::alloc_;
        using Base::map_;
        using Base::size_map_;
        using Base::size_elem_map_;
        using Base::first_block_;
        using Base::last_block_;
        using Base::allocate_block;
        using Base::deallocate_block;
        using Base::traits_type;
        using Base::map_traits_type;

        using Base::getSizeMap;
        using Base::getSizeElemMap;
        using Base::getMap;
        using Base::getFirstBlock;
        using Base::getLastBlock;

    private:
        size_type size_ = 0;
        size_type capasity_ = 0;
        float capac_size_ratio_ = 2.f;

        size_type last_index_i = 0;
        size_type last_index_j = 0;

        iterator start;
        iterator finish;

        //private:
        void init_blank_memory(size_t count);
        void realocate(size_t newCapasity);
        void constructDeque(const T& value = T());
    public:
        MyDeque(const Allocator& alloc = Allocator());
        explicit MyDeque(size_t count, const T& value = T(), const Allocator& alloc = Allocator());
        MyDeque(const MyDeque& rhs);
        MyDeque(MyDeque&& rhs) noexcept(std::is_nothrow_move_constructible_v<Allocator>);
        MyDeque(std::initializer_list<T> init, const Allocator& alloc = Allocator());

        template <typename InputIterator,
            typename = std::enable_if_t<!std::is_integral_v<InputIterator>>> // Защита от вызова MyDeque(5, 10)
            MyDeque(InputIterator first_it, InputIterator last_it, const Allocator& alloc = Allocator());

        ~MyDeque() {
            destroy(start, finish);
        }

        MyDeque& operator=(const MyDeque& rhs);
        MyDeque& operator=(MyDeque&& rhs)  noexcept(//false)
            (typename std::allocator_traits<Allocator>::propagate_on_container_move_assignment() &&
                std::is_nothrow_move_assignable_v<Allocator>) ||
            typename std::allocator_traits<Allocator>::is_always_equal());

        void push_back(const T& value);
        template <typename InputIterator>
        requires std::input_iterator<InputIterator>
            void push_back(InputIterator first, InputIterator last);
        void pop_back();

        void push_front(const T& value);
        template <typename InputIterator>
        requires std::input_iterator<InputIterator>
            void push_front(InputIterator first, InputIterator last);
        void pop_front();

        reference front();
        // Константная версия (только для чтения)
        const_reference front() const;
        reference back();
        // Константная версия (только для чтения)
        const_reference back() const;

        iterator insert(const_iterator position, const T& value);
        template <typename InputIterator>
            requires std::input_iterator<InputIterator>
        iterator insert(const_iterator position, InputIterator first, InputIterator last);
        template <typename... Args>
        reference emplace_back(Args&&... args);
        template <typename... Args>
        reference emplace_front(Args&&... args);
        template <typename... Args>
        iterator emplace(const_iterator position, Args&&... args);
        
        iterator erase(const_iterator position);
        iterator erase(const_iterator first, const_iterator last);

        void reserve(size_t new_capacity);
        void resize(size_t new_size, const T& value = T());
        void clear() noexcept;

        size_type size() const noexcept {return size_;}
        size_type capasity() const noexcept { return capasity_;}

        T& operator[](size_t index) { return *(start + index); }
        const T& operator[](size_t index) const { return *(start + index); }

        iterator begin() noexcept { return start; }
        iterator end() noexcept { return finish; }

        const_iterator begin() const noexcept { return start; }
        const_iterator end() const noexcept { return finish; }
        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        // Обратные итераторы
        reverse_iterator rbegin() noexcept { return reverse_iterator(finish); }
        reverse_iterator rend() noexcept { return reverse_iterator(start); }

        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(finish); }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator(start); }

        const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(finish); }
        const_reverse_iterator crend() const noexcept { return const_reverse_iterator(start); }
    };

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::init_blank_memory(size_t count) {
        size_t new_size_map = (count / CHUNK_SIZE) < 5 ? (count / CHUNK_SIZE) + 3 : (count / CHUNK_SIZE) * 2;

        // 1. Создаем временный буфер (выделяется память только под карту map_)
        MyDequeBuff<T, Allocator, CHUNK_SIZE> new_buf(new_size_map, this->alloc_);

        size_t new_capas = new_size_map * CHUNK_SIZE;
        size_t first_point = new_capas / 2 - count / 2;
        size_t last_point = first_point + count;

        size_t first_index_i = first_point / CHUNK_SIZE;
        size_t first_index_j = first_point % CHUNK_SIZE;
        size_t last_index_i = last_point / CHUNK_SIZE;
        size_t last_index_j = last_point % CHUNK_SIZE;
        size_t max_allocated_block = (last_index_j == 0) ? last_index_i : last_index_i + 1;

        new_buf.first_block_ = first_index_i;
        new_buf.last_block_ = max_allocated_block - 1;

        // 2. ВЫДЕЛЯЕМ СЫРЫЕ БЛОКИ (ЧАНКИ). Это увеличит allocate_calls!
        size_t allocated_up_to = first_index_i;
        try {
            for (size_t i = first_index_i; i < max_allocated_block; ++i) {
                new_buf.map_[i] = new_buf.allocate_block(); // <--- ТУТ ВЫЗОВЕТСЯ ALLOCATE
                allocated_up_to = i + 1;
            }
        }
        catch (...) {
            for (size_t i = first_index_i; i < allocated_up_to; ++i) {
                if (new_buf.map_[i] != nullptr) {
                    new_buf.deallocate_block(new_buf.map_[i]);
                }
            }
            throw;
        }
        // 3. БЕЗОПАСНЫЙ РУЧНОЙ ПЕРЕНОС В *THIS
        this->map_ = new_buf.map_;
        this->size_map_ = new_buf.size_map_;
        this->first_block_ = new_buf.first_block_;
        this->last_block_ = new_buf.last_block_;

        // Обязательно зануляем временный объект, чтобы его деструктор не стёр память
        new_buf.map_ = nullptr;
        new_buf.size_map_ = 0;
        new_buf.first_block_ = 0;
        new_buf.last_block_ = 0;

        capasity_ = new_capas;
        size_ = count;

        // 4. Настраиваем итераторы на СВОЮ новую карту
        start.setBlock(&map_[first_index_i], first_index_i, first_index_j);
        finish.setBlock(&map_[last_index_i], last_index_i, last_index_j);
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
    }
    
    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::realocate(size_t newCapasity) {
        if (newCapasity == 0) {
            newCapasity = CHUNK_SIZE * 3;
        }

        size_t new_size_map = (newCapasity % CHUNK_SIZE) == 0 ? (newCapasity / CHUNK_SIZE) : (newCapasity / CHUNK_SIZE) + 1;
        size_t newCapas = new_size_map * CHUNK_SIZE;

        if (capasity_ > 0 && newCapas <= capasity_) return;

        // 1. Выделяем СЫРУЮ новую карту указателей через аллокатор.
        // Если тут вылетит bad_alloc — текущий контейнер останется полностью нетронутым!
        // Строгая гарантия на этапе выделения карты соблюдена.
        typename Base::map_allocator_type map_alloc(this->alloc_);
        T** new_raw_map = Base::map_traits_type::allocate(map_alloc, new_size_map);

        // Зануляем новую карту
        for (size_t i = 0; i < new_size_map; ++i) {
            new_raw_map[i] = nullptr;
        }

        size_t new_first_block = (newCapas / 2 - size_ / 2) / CHUNK_SIZE;
        size_t new_last_block = new_first_block + (last_block_ - first_block_);

        // 2. Переносим указатели на старые чанки в новую карту
        if (map_ != nullptr) {
            for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                new_raw_map[new_i] = map_[old_i];
            }
        }
        else {
            // Если дек был пуст (например, после move), выделяем один чанк.
            // Если allocate_block() бросит исключение — мы поймаем его, очистим сырую карту 
            // и пробросим ошибку. Текущий контейнер останется нетронутым (Строгая гарантия!).
            try {
                new_raw_map[new_first_block] = allocate_block();
            }
            catch (...) {
                Base::map_traits_type::deallocate(map_alloc, new_raw_map, new_size_map);
                throw;
            }
            new_last_block = new_first_block;
        }

        // --- ТОЧКА НЕВОЗВРАТА (Все опасные операции выделения памяти завершились успехом) ---
        size_t old_first_block = first_block_;
        size_t old_capasity = capasity_;

        // Освобождаем исключительно старый массив указателей T** (сами чанки мы бережно перенесли)
        if (this->map_ != nullptr) {
            Base::map_traits_type::deallocate(map_alloc, this->map_, this->size_map_);
        }

        // Присваиваем новые указатели и размеры напрямую (noexcept операции)
        this->map_ = new_raw_map;
        this->size_map_ = new_size_map;
        this->first_block_ = new_first_block;
        this->last_block_ = new_last_block;
        capasity_ = newCapas;

        // Вычисляем корректное смещение итераторов
        size_t block_offset = new_first_block - old_first_block;
        if (old_capasity == 0) block_offset = 0;

        auto nfi = start.first_index_i + block_offset;
        auto nfj = start.first_index_j;
        last_index_i = finish.first_index_i + block_offset;
        last_index_j = finish.first_index_j;

        // Перевыставляем блоки, обновляя скрытые proxy-структуры MSVC Iterator Debugging
        start.setBlock(&map_[nfi], nfi, nfj);
        finish.setBlock(&map_[last_index_i], last_index_i, last_index_j);
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::realocate(size_t newCapasity) {
        if (newCapasity == 0 && capasity_ == 0) {
            newCapasity = CHUNK_SIZE * 3;
        }

        size_t new_size_map = (newCapasity % CHUNK_SIZE) == 0 ? (newCapasity / CHUNK_SIZE) : (newCapasity / CHUNK_SIZE) + 1;
        size_t newCapas = new_size_map * CHUNK_SIZE;

        if (capasity_ > 0 && newCapas <= capasity_) return;

        Base new_buf(new_size_map, this->alloc_);

        size_t new_first_block = (newCapas / 2 - size_ / 2) / CHUNK_SIZE;
        size_t new_last_block = new_first_block + (last_block_ - first_block_);

        // 2. КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Копируем блоки только если старая карта существовала!
        if (map_ != nullptr) {
            for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                new_buf.map_[new_i] = map_[old_i];
                map_[old_i] = nullptr; // Зануляем в старой карте
            }
        }
        else {
            // ЕСЛИ СТАРЫЙ ДЕК БЫЛ ПУСТ (после move):
            // Сразу выделим центральный чанк, чтобы итераторы start и finish инициализировались валидными адресами!
            new_buf.map_[new_first_block] = new_buf.allocate_block();
        }
        new_buf.first_block_ = new_first_block;
        new_buf.last_block_ = new_last_block;

        size_t block_offset = new_first_block - first_block_;

        // 3. КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Вместо std::swap всего класса (который вызывает оператор=),
        // мы вручную и гарантированно без исключений меняем ТОЛЬКО указатели и размеры.
        // Аллокатор при этом остается на месте и не трогается!
        std::swap(this->map_, new_buf.map_);
        std::swap(this->size_map_, new_buf.size_map_);
        std::swap(this->first_block_, new_buf.first_block_);
        std::swap(this->last_block_, new_buf.last_block_);

        capasity_ = newCapas;

        //auto 
        auto nfi = start.first_index_i + block_offset;
        auto nfj = start.first_index_j;
        last_index_i = finish.first_index_i + block_offset;
        last_index_j = finish.first_index_j;
        start.setBlock(&map_[nfi], nfi, nfj);
        finish.setBlock(&map_[last_index_i], last_index_i, last_index_j);
    }
    
    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::constructDeque(const T& value ) {
        size_t first_point = capasity_ / 2 - size_ / 2;
        size_t last_point = first_point + size_;
        size_t first_index_i = first_point / CHUNK_SIZE;
        size_t first_index_j = first_point % CHUNK_SIZE;
        last_index_i = (last_point / CHUNK_SIZE);
        last_index_j = last_point % CHUNK_SIZE;
        size_t max_allocated_block = (last_index_j == 0) ? last_index_i : last_index_i + 1;

#ifdef PRINT_TECH_1
        std::cout << "first_point = " << first_point << ", " << "last_point = " << last_point << ", "
            << "first_index_i = " << first_index_i << ", " << "first_index_j = " << first_index_j << ", "
            << "last_index_i = " << last_index_i << ", " << "last_index_j = " << last_index_j << "\n";
#endif // PRINT_TECH_1

        first_block_ = first_index_i;
        last_block_ = max_allocated_block - 1;
        // 1. ЗАЩИТА №1: Выделение сырых блоков памяти
        size_t allocated_up_to = first_index_i; // Запоминаем, до какого индекса выделили память
        try {
            for (size_t i = first_index_i; i < max_allocated_block; ++i) {
                map_[i] = allocate_block();
                allocated_up_to = i + 1; // Успешно выделили текущий блок
            }
        }
        catch (...) {
            // Если allocate_block() выбросил исключение, чистим только то, что успели выделить
            for (size_t i = first_index_i; i < allocated_up_to; ++i) {
                if (map_[i] != nullptr) {
                    deallocate_block(map_[i]);
                    map_[i] = nullptr;
                }
            }
            throw; // Перевыбрасываем исключение bad_alloc дальше пользователю
        }
        start.setBlock(&map_[first_index_i], first_index_i, first_index_j);
        finish.setBlock(&map_[last_index_i], last_index_i, last_index_j);

        // 2. ЗАЗАЩИТА №2: Заполняем блоки объектами T
        auto it = start;
        try {
            while (it != finish) {
                traits_type::construct(this->alloc_, it.current, value);
                ++it;
            }
        }
        catch (...) {
            // Если упал конструктор самого объекта T, уничтожаем созданные объекты
            destroy(start, it);

            // И очищаем ВСЕ выделенные на шаге 1 сырые блоки памяти
            for (size_t i = first_index_i; i < max_allocated_block; ++i) {
                if (map_[i] != nullptr) {
                    deallocate_block(map_[i]);
                    map_[i] = nullptr;
                }
            }
            throw;
        }
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque (const Allocator& alloc) : Base(3, alloc), size_(0), capasity_(size_map_* CHUNK_SIZE) {
        constructDeque(T());
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque (size_t count, const T& value, const Allocator& alloc) : Base(((count / CHUNK_SIZE) < 0 ? 3 :
        (count / CHUNK_SIZE) < 5 ? (count / CHUNK_SIZE) + 3 : (count / CHUNK_SIZE) * 2), alloc),
        size_(count), capasity_(size_map_* CHUNK_SIZE)
    {
        constructDeque(value);
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque(const MyDeque& rhs) : MyDequeBuff<T, Allocator, CHUNK_SIZE>(0, rhs.alloc_)
    {
        if (rhs.size_ == 0) {
            init_blank_memory(0);
            return;
        }
        // Теперь этот вызов создаст полностью независимую структуру памяти в куче
        init_blank_memory(rhs.size_);
        // Безопасно копируем объекты T в изолированные новые чанки
        auto it_dest = start;
        auto it_src = rhs.start;
        try {
            while (it_src != rhs.finish) {
                traits_type::construct(this->alloc_, it_dest.current, *it_src);
                ++it_dest;
                ++it_src;
            }
        }
        catch (...) {
            destroy(start, it_dest);
            // Вызывать вручную deallocate_block здесь НЕ НАДО! 
            // Деструктор базового класса MyDequeBuff сделает это автоматически при раскрутке стека.
            size_ = 0;
            throw;
        }
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque(MyDeque&& rhs) noexcept(std::is_nothrow_move_constructible_v<Allocator>)
        try : Base(std::move(rhs)), capasity_(rhs.capasity_),
        size_(rhs.size_), capac_size_ratio_(rhs.capac_size_ratio_), last_index_i(rhs.last_index_i),
        last_index_j(rhs.last_index_j), start(rhs.start), finish(rhs.finish)
    {
        // Обязательно сбрасываем rhs в валидное пустое состояние,
        // чтобы его деструктор случайно не удалил наши новые блоки
        rhs.capasity_ = 0;
        rhs.size_ = 0;
        size_t center_i = rhs.size_map_ / 2;
        rhs.start.setBlock(&rhs.map_[center_i], center_i, 0);
        rhs.finish = rhs.start;
        rhs.first_block_ = center_i;
        rhs.last_block_ = center_i;
    }
    catch (...) {
        throw;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>& MyDeque<T, Allocator, CHUNK_SIZE>::operator=(const MyDeque& rhs) {
        if (this == &rhs) {
            return *this;
        }
        // Проверяем: могут ли наши аллокаторы освобождать память друг друга?
        if constexpr (traits_type::propagate_on_container_copy_assignment::value) {
            // Если стандарт разрешает скопировать аллокатор:
            MyDeque tmp(rhs);
            *this = std::move(tmp);
        }
        else if (this->alloc_ == rhs.alloc_) {
            // Если аллокаторы разные по сути, но эквивалентны (работают с одним ресурсом)
            MyDeque tmp(rhs);
            *this = std::move(tmp);
        }
        else {
            // 1. Создаем временный дек, используя НАШ текущий аллокатор
            MyDeque tmp(rhs.begin(), rhs.end(), this->alloc_);
            // 2. Обмениваемся данными через noexcept перемещение
            *this = std::move(tmp);
        }

        return *this;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>& MyDeque<T, Allocator, CHUNK_SIZE>::operator=(MyDeque&& rhs) noexcept(//false)
        (typename std::allocator_traits<Allocator>::propagate_on_container_move_assignment() &&
            std::is_nothrow_move_assignable_v<Allocator>) ||
        typename std::allocator_traits<Allocator>::is_always_equal())
    {
        if (this != &rhs) {
            // Если тут вылетит исключение из-за tmp_alloc, метод прервется.
            // Но так как память буфера еще НЕ была удалена, текущий дек (this) 
            // останется в своем ИСХОДНОМ рабочем состоянии со всеми старыми данными!
            Base::operator=(std::move(rhs));

            // --- ТОЧКА НЕВОЗВРАТА ---
            destroy(start, finish);

            capasity_ = rhs.capasity_;
            size_ = rhs.size_;
            capac_size_ratio_ = rhs.capac_size_ratio_;
            last_index_i = rhs.last_index_i;
            last_index_j = rhs.last_index_j;

            start = rhs.start;
            finish = rhs.finish;

            rhs.capasity_ = 0;
            rhs.size_ = 0;
            rhs.last_index_i = 0;
            rhs.last_index_j = 0;
            rhs.start.setBlock(nullptr, 0, 0);
            rhs.finish = rhs.start;
        }
        return *this;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque(std::initializer_list<T> init, const Allocator& alloc)
        : MyDeque(init.begin(), init.end(), alloc) { /*Просто перенаправляем итераторы списка инициализации в предыдущий конструктор*/
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator, typename> // Защита от вызова MyDeque(5, 10)
        MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque(InputIterator first_it, InputIterator last_it, const Allocator& alloc)
        : Base(0, alloc)
    {
        // Вычисляем расстояние, если категория итераторов это позволяет (например, Random Access)
        size_t dist = 0;
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
            typename std::iterator_traits<InputIterator>::iterator_category>) {
            dist = std::distance(first_it, last_it);
        }
        // Инициализируем пустую структуру
        init_blank_memory(dist);
        auto it_dest = start;
        try {
            while (first_it != last_it) {
                // Если мы изначально не знали точный размер (например, читали из std::list),
                // и вышли за пределы выделенного в init_blank_memory объема памяти
                if (it_dest == finish) {
                    // Если память кончилась, просто используем наш безопасный push_back!
                    push_back(*first_it);
                    it_dest = finish; // Обновляем рабочую позицию
                }
                else {
                    traits_type::construct(this->alloc_, it_dest.current, *first_it);
                    ++it_dest;
                }
                ++first_it;
            }
            // Если элементов оказалось меньше, чем мы зарезервировали по dist,
            // корректируем финишный итератор под реальное количество
            finish = it_dest;
            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
        }
        catch (...) {
            destroy(start, it_dest);
            for (size_t i = first_block_; i <= last_block_; ++i) {
                if (map_[i] != nullptr) {
                    deallocate_block(map_[i]);
                    map_[i] = nullptr;
                }
            }
            size_ = 0;
            throw;
        }
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator, typename>
    MyDeque<T, Allocator, CHUNK_SIZE>::MyDeque(InputIterator first_it, InputIterator last_it, const Allocator& alloc)
        : Base(0, alloc), size_(0), capasity_(0) // Явно сбрасываем в 0
    {
        // 1. Проверяем, итератор произвольного доступа или нет
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
            typename std::iterator_traits<InputIterator>::iterator_category>)
        {
            size_t dist = std::distance(first_it, last_it);
            init_blank_memory(dist); // Выделит точный объем памяти и выставит size_ = dist

            iterator it_dest = start;
            try {
                while (first_it != last_it) {
                    traits_type::construct(this->alloc_, it_dest.current, *first_it);
                    ++it_dest;
                    ++first_it;
                }
                // Синхронизируем финиш на случай, если всё заполнили штатно
                finish = it_dest;
                last_index_i = finish.first_index_i;
                last_index_j = finish.first_index_j;
            }
            catch (...) {
                destroy(start, it_dest);
                size_ = 0;
                throw;
            }
        }
        else
        {
            // 2. Для bidirectional / input итераторов (например, std::list)
            // Инициализируем полностью пустой дек (как дефолтный конструктор)
            init_blank_memory(0);
            size_ = 0; // push_back сам будет инкрементировать это поле

            try {
                while (first_it != last_it) {
                    // Наш безопасный push_back сам выделит чанки, 
                    // продвинет finish и корректно увеличит size_!
                    push_back(*first_it);
                    ++first_it;
                }
            }
            catch (...) {
                destroy(start, finish);
                size_ = 0;
                throw;
            }
        }
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::push_back(const T& value) {
        // 1. Проверяем: если массив пустой ИЛИ финиш вышел за пределы текущей карты
        if (size_map_ == 0 || finish.first_index_i >= size_map_) {
            realocate(capasity_ == 0 ? CHUNK_SIZE * 3 : capasity_ * 2);
        }
        // 2. Если финиш стоит на границе чанка, проверяем, нужно ли выделить следующий блок
        if (finish.first_index_j == 0) {
            size_t current_block_idx = finish.first_index_i;
            if (map_[current_block_idx] == nullptr) {
                map_[current_block_idx] = allocate_block();
                if (current_block_idx > last_block_) {
                    last_block_ = current_block_idx; // Обновляем правую границу в MyDequeBuff
                }
                // Перевыставляем блок, так как теперь память под ним гарантированно есть
                finish.setBlock(&map_[current_block_idx], current_block_idx, finish.first_index_j);
            }
        }
        // 3. Место под элемент гарантированно валидно, конструируем объект T
        traits_type::construct(this->alloc_, finish.current, value);
        // 4. Двигаем итератор и обновляем размеры
        ++size_;
        ++finish;
        // Синхронизируем служебные индексы
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
    }*/

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::push_back(const T& value) {
        // 1. Проверяем границы карты
        if (size_map_ == 0 || finish.first_index_i >= size_map_) {
            realocate(capasity_ == 0 ? CHUNK_SIZE * 3 : capasity_ * 2);
        }

        size_t current_block_idx = finish.first_index_i;
        bool allocated_new_chunk = false;

        // 2. Выделяем физический чанк, если это необходимо
        if (finish.first_index_j == 0 && map_[current_block_idx] == nullptr) {
            map_[current_block_idx] = allocate_block();
            allocated_new_chunk = true;

            // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: 
            // Синхронизируем finish с новым выделенным блоком ПЕРЕД конструированием объекта.
            // Теперь внутренние поля finish.first, finish.last и finish.current инициализируются!
            finish.setBlock(&map_[current_block_idx], current_block_idx, 0);
        }

        // 3. Конструируем объект
        try {
            // Теперь можно безопасно использовать finish.current
            traits_type::construct(this->alloc_, finish.current, value);
        }
        catch (...) {
            if (allocated_new_chunk) {
                deallocate_block(map_[current_block_idx]);
                map_[current_block_idx] = nullptr;
                // Возвращаем finish в состояние "невыделенного блока" в случае отката
                finish.setBlock(&map_[current_block_idx], current_block_idx, 0);
            }
            throw;
        }

        // --- ТОЧКА НЕВОЗВРАТА ---
        if (allocated_new_chunk && current_block_idx > last_block_) {
            last_block_ = current_block_idx;
        }

        // 4. Безопасно двигаем итераторы
        ++finish;
        ++size_;
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::push_back(const T& value) {
        // Сценарий А: Требуется расширение карты указателей (массива чанков)
        if (size_map_ == 0 || finish.first_index_i >= size_map_) {
            size_type newCapasity = (capasity_ == 0) ? CHUNK_SIZE * 3 : capasity_ * 2;
            size_t new_size_map = (newCapasity % CHUNK_SIZE) == 0 ? (newCapasity / CHUNK_SIZE) : (newCapasity / CHUNK_SIZE) + 1;
            size_t newCapas = new_size_map * CHUNK_SIZE;

            // 1. Создаем локальный буфер новой карты "на стороне". 
            // Если вылетит bad_alloc, текущий дек вообще не изменится.
            Base new_buf(new_size_map, this->alloc_);

            size_t new_first_block = (newCapas / 2 - size_ / 2) / CHUNK_SIZE;
            size_t new_last_block = new_first_block + (last_block_ - first_block_);

            // Переносим указатели на старые чанки во временную карту
            if (map_ != nullptr) {
                for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                    new_buf.map_[new_i] = map_[old_i];
                }
            }

            // Вычисляем, где в НОВОЙ карте будет находиться наш элемент
            size_t block_offset = new_first_block - first_block_;
            size_t target_block_idx = finish.first_index_i + block_offset;

            bool allocated_new_chunk = false;

            // 2. Если для нового элемента нужен новый чанк, выделяем его во ВРЕМЕННОЙ карте
            if (finish.first_index_j == 0 && new_buf.map_[target_block_idx] == nullptr) {
                new_buf.map_[target_block_idx] = new_buf.allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            // 3. Пытаемся сконструировать объект T в памяти этого чанка
            try {
                T* raw_ptr = new_buf.map_[target_block_idx] + finish.first_index_j;
                traits_type::construct(this->alloc_, raw_ptr, value); // Может бросить исключение конструктора T
            }
            catch (...) {
                // Если конструктор T упал, чистим только то, что успели выделить локально
                if (allocated_new_chunk) {
                    new_buf.deallocate_block(new_buf.map_[target_block_idx]);
                }
                // Старую карту в new_buf мы НЕ зануляли, поэтому при выходе из catch 
                // деструктор new_buf её не тронет. Текущий контейнер остался девственно чист!
                throw;
            }

            // --- ТОЧКА НЕВОЗВРАТА (Все опасные операции завершились успехом!) ---

            // Чтобы деструктор new_buf не удалил наши СТАРЫЕ чанки, которые мы туда скопировали,
            // мы очищаем старую карту map_ в текущем контейнере (зануляем ячейки)
            if (map_ != nullptr) {
                for (size_t old_i = first_block_; old_i <= last_block_; ++old_i) {
                    map_[old_i] = nullptr;
                }
            }

            // Переносим данные из буфера в текущий класс через безопасный swap плоских полей
            std::swap(this->map_, new_buf.map_);
            std::swap(this->size_map_, new_buf.size_map_);

            this->first_block_ = new_first_block;
            this->last_block_ = allocated_new_chunk ? std::max(new_last_block, target_block_idx) : new_last_block;
            this->capasity_ = newCapas;

            // Корректируем индексы итераторов
            size_t nfi = start.first_index_i + block_offset;
            size_t nfj = start.first_index_j;
            last_index_i = finish.first_index_i + block_offset;
            last_index_j = finish.first_index_j;

            // Перевыставляем итераторы легально для MSVC Debugging
            start.setBlock(&map_[nfi], nfi, nfj);
            finish.setBlock(&map_[last_index_i], last_index_i, last_index_j);
        }
        // Сценарий Б: Карта большая, реаллокация самой карты НЕ нужна, максимум — выделить один чанк
        else {
            size_t current_block_idx = finish.first_index_i;
            bool allocated_new_chunk = false;

            if (finish.first_index_j == 0 && map_[current_block_idx] == nullptr) {
                map_[current_block_idx] = allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            try {
                T* raw_ptr = map_[current_block_idx] + finish.first_index_j;
                traits_type::construct(this->alloc_, raw_ptr, value);
            }
            catch (...) {
                if (allocated_new_chunk) {
                    deallocate_block(map_[current_block_idx]);
                    map_[current_block_idx] = nullptr;
                }
                throw;
            }

            if (allocated_new_chunk) {
                finish.setBlock(&map_[current_block_idx], current_block_idx, 0);
                if (current_block_idx > last_block_) {
                    last_block_ = current_block_idx;
                }
            }
        }

        // Общий успешный финал для обоих сценариев
        ++finish;
        ++size_;
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
    requires std::input_iterator<InputIterator>
        void MyDeque<T, Allocator, CHUNK_SIZE>::push_back(InputIterator first, InputIterator last) {
        if (first == last) return;
        difference_type dist = std::distance(first, last);
        difference_type free_space_block = CHUNK_SIZE - finish.first_index_j;
        difference_type free_space_back = (size_map_ - finish.first_index_i - 1) * CHUNK_SIZE + free_space_block;

        // Сценарий 1: Элементы гарантированно помещаются в текущий чанк (Реаллокация и новые блоки не нужны)
        if (finish.first_index_j != 0 && dist <= free_space_block) {
            size_t curr_i = finish.first_index_i;
            size_t curr_j = finish.first_index_j;
            size_t constructed_count = 0;
            try {
                while (first != last) {
                    T* raw_ptr = map_[curr_i] + curr_j;
                    traits_type::construct(this->alloc_, raw_ptr, *first);
                    ++curr_j;
                    ++constructed_count;
                    ++first;
                }
            }
            catch (...) {
                while (constructed_count > 0) {
                    --curr_j;
                    destroy(map_[curr_i] + curr_j);
                    --constructed_count;
                }
                throw;
            }
            finish.setBlock(&map_[curr_i], curr_i, curr_j);
            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
            size_ += dist;
        }
        // Сценарий 2: Карты map_ хватает, но нужно довыделить новые физические чанки
        else if (dist <= free_space_back) {
            // Безопасно вычисляем индекс конечного блока БЕЗ использования operator+ у итератора
            size_t total_j = finish.first_index_j + dist;
            size_t end_block_needed = finish.first_index_i + (total_j / CHUNK_SIZE);
            if (total_j % CHUNK_SIZE == 0 && end_block_needed > finish.first_index_i) {
                --end_block_needed;
            }

            // Выделяем блоки
            for (size_t i = finish.first_index_i; i <= end_block_needed; ++i) {
                if (map_[i] == nullptr) {
                    map_[i] = allocate_block();
                    if (i > last_block_) last_block_ = i;
                }
            }

            size_t curr_i = finish.first_index_i;
            size_t curr_j = finish.first_index_j;
            size_t constructed_count = 0;
            try {
                while (first != last) {
                    if (curr_j == CHUNK_SIZE) {
                        curr_j = 0;
                        ++curr_i;
                    }
                    T* raw_ptr = map_[curr_i] + curr_j;
                    traits_type::construct(this->alloc_, raw_ptr, *first);
                    ++curr_j;
                    ++constructed_count;
                    ++first;
                }
            }
            catch (...) {
                for (size_t c = 0; c < constructed_count; ++c) {
                    if (curr_j == 0) {
                        curr_j = CHUNK_SIZE;
                        --curr_i;
                    }
                    --curr_j;
                    destroy(map_[curr_i] + curr_j);
                }
                throw;
            }

            finish.setBlock(&map_[curr_i], curr_i, curr_j);
            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
            size_ += dist;
        }
        // Сценарий 3: Свободного места в карте не хватает вообще, требуется глобальный realocate
        else {
            size_type needed_capasity = (dist - free_space_back + capasity_) * 2;
            realocate(needed_capasity);

            // Повторяем логику Сценария 2 на расширенной карте
            size_t total_j = finish.first_index_j + dist;
            size_t end_block_needed = finish.first_index_i + (total_j / CHUNK_SIZE);
            if (total_j % CHUNK_SIZE == 0 && end_block_needed > finish.first_index_i) {
                --end_block_needed;
            }

            for (size_t i = finish.first_index_i; i <= end_block_needed; ++i) {
                if (map_[i] == nullptr) {
                    map_[i] = allocate_block();
                    if (i > last_block_) last_block_ = i;
                }
            }

            size_t curr_i = finish.first_index_i;
            size_t curr_j = finish.first_index_j;
            size_t constructed_count = 0;
            try {
                while (first != last) {
                    if (curr_j == CHUNK_SIZE) {
                        curr_j = 0;
                        ++curr_i;
                    }
                    T* raw_ptr = map_[curr_i] + curr_j;
                    traits_type::construct(this->alloc_, raw_ptr, *first);
                    ++curr_j;
                    ++constructed_count;
                    ++first;
                }
            }
            catch (...) {
                for (size_t c = 0; c < constructed_count; ++c) {
                    if (curr_j == 0) {
                        curr_j = CHUNK_SIZE;
                        --curr_i;
                    }
                    --curr_j;
                    destroy(map_[curr_i] + curr_j);
                }
                throw;
            }

            finish.setBlock(&map_[curr_i], curr_i, curr_j);
            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
            size_ += dist;
        }
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
    requires std::input_iterator<InputIterator>
        void MyDeque<T, Allocator, CHUNK_SIZE>::push_back(InputIterator first, InputIterator last) {
        if (first == last) return;
        difference_type dist = std::distance(first, last);
        difference_type free_space_block = CHUNK_SIZE - finish.first_index_j;
        difference_type free_space_back = (size_map_ - finish.first_index_i - 1) * CHUNK_SIZE + free_space_block;
        //std::cout << "\nfree_space_block = " << free_space_block << " " << "free_space_back = " << free_space_back << "\n";
        //if (finish.first_index_j != 0) {
            if ((finish.first_index_j != 0) && (dist <= free_space_block)) {
                iterator current_finish = finish;
                try {
                    while (first != last) {
                        // Конструируем элемент в сырой памяти, куда указывает текущий виртуальный finish
                        traits_type::construct(this->alloc_, current_finish.current, *first);
                        ++current_finish;
                        ++first;
                    }
                }
                catch (...) {
                    // Откат: уничтожаем всё, что успели создать в этом вызове
                    while (finish != current_finish) {
                        --current_finish;
                        destroy(current_finish.current);
                    }
                    throw; // Пробрасываем исключение дальше
                }
                finish = current_finish;
                last_index_i = finish.first_index_i;
                last_index_j = finish.first_index_j;
                size_ += dist;
            }
            else if ((finish.first_index_j != 0) && (dist <= free_space_back) && (dist > free_space_block)) {
                size_type end_block_needed = (finish + dist).first_index_i;
                for (size_t i = finish.first_index_i; i <= end_block_needed; ++i) {
                    if (map_[i] == nullptr) {
                        map_[i] = allocate_block();
                        //if (i > last_block_) last_block_ = i;
                    }
                }

                iterator current_finish;
                current_finish.setBlock(&map_[finish.first_index_i], finish.first_index_i, finish.first_index_j);
                try {
                    while (first != last) {
                        // Конструируем элемент в сырой памяти, куда указывает текущий виртуальный finish
                        traits_type::construct(this->alloc_, current_finish.current, *first);
                        ++current_finish;
                        ++first;
                    }
                }
                catch (...) {
                    // Откат: уничтожаем всё, что успели создать в этом вызове
                    while (finish != current_finish) {
                        --current_finish;
                        destroy(current_finish.current);
                    }
                    throw; // Пробрасываем исключение дальше
                }

                // Если всё прошло успешно, обновляем состояние дека
                finish = current_finish;
                last_index_i = finish.first_index_i;
                last_index_j = finish.first_index_j;
                size_ += dist;
            }
            else if ((finish.first_index_j == 0) && (dist <= free_space_back)) {
                // 1. Правильно вычисляем конечный блок БЕЗ использования арифметики итераторов
                size_t total_j = finish.first_index_j + dist;
                size_t end_block_needed = finish.first_index_i + (total_j / CHUNK_SIZE);
                if (total_j % CHUNK_SIZE == 0 && end_block_needed > finish.first_index_i) {
                    --end_block_needed;
                }

                // 2. Выделяем чанки и ОБЯЗАТЕЛЬНО обновляем границы в базовом буфере
                for (size_t i = finish.first_index_i; i <= end_block_needed; ++i) {
                    if (map_[i] == nullptr) {
                        map_[i] = allocate_block();
                    }
                }
                // КРИТИЧЕСКИ ВАЖНО: Обновляем правую границу выделенных блоков,
                // чтобы деструктор и итераторы знали, до какого момента память легальна!
                if (end_block_needed > last_block_) {
                    last_block_ = end_block_needed;
                }

                // 3. Переходим на абсолютно безопасные плоские индексы для заполнения
                size_t curr_i = finish.first_index_i;
                size_t curr_j = finish.first_index_j;
                size_t constructed_count = 0;

                try {
                    while (first != last) {
                        // Если мы заполнили текущий чанк полностью, переходим на начало следующего чанка
                        if (curr_j == CHUNK_SIZE) {
                            curr_j = 0;
                            ++curr_i;
                        }

                        // Конструируем элемент по прямому и честному адресу памяти в чанке
                        T* raw_ptr = map_[curr_i] + curr_j;
                        traits_type::construct(this->alloc_, raw_ptr, *first);

                        ++curr_j;
                        ++constructed_count;
                        ++first;
                    }
                }
                catch (...) {
                    // Откат: если упал конструктор T, строго назад уничтожаем всё, что создали в этом вызове
                    for (size_t c = 0; c < constructed_count; ++c) {
                        if (curr_j == 0) {
                            curr_j = CHUNK_SIZE;
                            --curr_i;
                        }
                        --curr_j;
                        destroy(map_[curr_i] + curr_j);
                    }
                    throw;
                }

                // 4. Если всё прошло успешно, ОДИН РАЗ обновляем состояние итератора finish контейнера
                finish.setBlock(&map_[curr_i], curr_i, curr_j);
                last_index_i = finish.first_index_i;
                last_index_j = finish.first_index_j;
                size_ += dist;
            }
            else {
            // 1. Математически точный расчет новой карты: она должна вмещать старые блоки + новые элементы
            size_t old_num_blocks = last_block_ - first_block_ + 1;
            size_t additional_blocks_needed = (finish.first_index_j + dist) / CHUNK_SIZE + 2;

            size_t new_size_map = (size_map_ == 0) ? (dist / CHUNK_SIZE) + 5 : (size_map_ * 2) + additional_blocks_needed;
            size_t newCapas = new_size_map * CHUNK_SIZE;

            // Создаем новый временный буфер
            Base new_buf(new_size_map, this->alloc_);

            // Центрируем старые блоки с запасом
            size_t new_first_block = (new_size_map - old_num_blocks) / 2;
            size_t new_last_block = new_first_block + old_num_blocks - 1;

            // Фиксируем разницу индексов ДО каких-либо изменений
            size_t block_offset = new_first_block - first_block_;

            // Копируем указатели на существующие чанки в новую карту
            if (map_ != nullptr) {
                for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                    new_buf.map_[new_i] = map_[old_i];
                }
            }

            // Точный расчет индексов, где начнется вставка новых элементов во временной карте
            size_t curr_i = finish.first_index_i + block_offset;
            size_t curr_j = finish.first_index_j;

            // Вычисляем конечный блок, который потребуется для вставки dist элементов
            size_t total_j = curr_j + dist;
            size_t end_block_needed = curr_i + (total_j / CHUNK_SIZE);
            if (total_j % CHUNK_SIZE == 0 && end_block_needed > curr_i) {
                --end_block_needed;
            }

            // Массив для отслеживания выделенных чанков на случай исключения
            std::vector<size_t> allocated_chunk_indices;
            allocated_chunk_indices.reserve(end_block_needed - curr_i + 1);

            try {
                // 2. Выделяем новые чанки во временной карте (теперь гарантированно БЕЗ выхода за границы)
                for (size_t i = curr_i; i <= end_block_needed; ++i) {
                    if (new_buf.map_[i] == nullptr) {
                        new_buf.map_[i] = new_buf.allocate_block();
                        allocated_chunk_indices.push_back(i); // Запоминаем для отката
                    }
                }

                // 3. Копируем/Конструируем элементы БЕЗ использования сложного итератора во временную карту new_buf.map_
                size_t constructed_count = 0;
                size_t insert_i = curr_i;
                size_t insert_j = curr_j;

                try {
                    while (first != last) {
                        if (insert_j == CHUNK_SIZE) {
                            insert_j = 0;
                            ++insert_i;
                        }
                        T* raw_ptr = new_buf.map_[insert_i] + insert_j;
                        traits_type::construct(this->alloc_, raw_ptr, *first);
                        ++insert_j;
                        ++constructed_count;
                        ++first;
                    }
                }
                catch (...) {
                    // Если упал конструктор T, уничтожаем только то, что успели создать в этом вызове
                    for (size_t c = 0; c < constructed_count; ++c) {
                        if (insert_j == 0) {
                            insert_j = CHUNK_SIZE;
                            --insert_i;
                        }
                        --insert_j;
                        destroy(new_buf.map_[insert_i] + insert_j);
                    }
                    throw;
                }

                // Запоминаем финальные индексы для итератора finish
                curr_i = insert_i;
                curr_j = insert_j;
            }
            catch (...) {
                // ОТКАТ ПАМЯТИ В СЛУЧАЕ ОШИБКИ
                for (size_t idx : allocated_chunk_indices) {
                    new_buf.deallocate_block(new_buf.map_[idx]);
                    new_buf.map_[idx] = nullptr;
                }
                throw;
            }

            // --- ТОЧКА НЕВОЗВРАТА (Все опасные операции завершились успехом!) ---

            // 4. Очищаем старую карту, чтобы деструктор `new_buf` не уничтожил наши старые рабочие чанки
            if (map_ != nullptr) {
                for (size_t old_i = first_block_; old_i <= last_block_; ++old_i) {
                    map_[old_i] = nullptr;
                }
            }

            // 5. Обновляем состояние контейнера (Безопасный swap)
            std::swap(this->map_, new_buf.map_);
            std::swap(this->size_map_, new_buf.size_map_);

            // Корректируем границы блоков с учетом добавленных чанков
            this->first_block_ = new_first_block;
            this->last_block_ = std::max(new_last_block, end_block_needed);
            this->capasity_ = newCapas;

            // 6. Корректируем итераторы самого контейнера легальным способом
            size_t new_start_i = start.first_index_i + block_offset;

            start.setBlock(&map_[new_start_i], new_start_i, start.first_index_j);
            finish.setBlock(&map_[curr_i], curr_i, curr_j);

            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
            size_ += dist;
            }

    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::pop_back() {
        if (size_ == 0) {
            size_t center_i = size_map_ / 2;
            start.setBlock(&map_[center_i], center_i, 0);
            finish = start;
            first_block_ = center_i;
            last_block_ = center_i;
            throw std::runtime_error("Pop from empty deque");
        }
        --finish;
        --size_;

        destroy(finish.current);

        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
        if (finish.first_index_j == 0) {
            --last_block_; // Обновляем границу буфера
        }
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::push_front(const T& value) {

        // Сценарий А: Мы уперлись в самый левый край карты (start.first_index_i == 0) 
        // ИЛИ старт на j == 0 и предыдущий блок физически некуда вставить, так как карта кончилась.
        if (size_map_ == 0 || (start.first_index_i == 0 && start.first_index_j == 0)) {
            size_type newCapasity = (capasity_ == 0) ? CHUNK_SIZE * 3 : capasity_ * 2;
            size_t new_size_map = (newCapasity % CHUNK_SIZE) == 0 ? (newCapasity / CHUNK_SIZE) : (newCapasity / CHUNK_SIZE) + 1;
            size_t newCapas = new_size_map * CHUNK_SIZE;

            // 1. Выделяем новую карту во временном буфере "на стороне"
            Base new_buf(new_size_map, this->alloc_); // Обязательно передаем текущий аллокатор!

            // Важно: так как мы добавляем элемент В НАЧАЛО, смещение блоков должно учитывать, 
            // чтобы слева оставалось достаточно места. Центрируем старые блоки в новой карте.
            size_t new_first_block = (newCapas / 2 - size_ / 2) / CHUNK_SIZE;
            size_t new_last_block = new_first_block + (last_block_ - first_block_);

            // Переносим указатели на старые чанки
            if (map_ != nullptr) {
                for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                    new_buf.map_[new_i] = map_[old_i];
                }
            }

            // Вычисляем, где логически находился старт в новой карте
            size_t block_offset = new_first_block - first_block_;
            size_t current_start_i = start.first_index_i + block_offset;
            size_t current_start_j = start.first_index_j;

            // Определяем целевые индексы для НОВОГО элемента (на 1 позицию левее текущего старта)
            size_t target_block_idx = current_start_i;
            size_t target_index_j = 0;

            if (current_start_j == 0) {
                target_block_idx = current_start_i - 1;
                target_index_j = CHUNK_SIZE - 1;
            }
            else {
                target_index_j = current_start_j - 1;
            }

            bool allocated_new_chunk = false;
            // 2. Если целевой чанк еще не выделен, выделяем его локально во временной карте
            if (new_buf.map_[target_block_idx] == nullptr) {
                new_buf.map_[target_block_idx] = new_buf.allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            // 3. Пытаемся сконструировать объект T в сырой памяти нового чанка
            try {
                T* raw_ptr = new_buf.map_[target_block_idx] + target_index_j;
                traits_type::construct(this->alloc_, raw_ptr, value); // Может бросить исключение конструктора T
            }
            catch (...) {
                if (allocated_new_chunk) {
                    new_buf.deallocate_block(new_buf.map_[target_block_idx]);
                }
                throw; // Откат: старая карта не тронута, capacity не изменилась
            }

            // --- ТОЧКА НЕВОЗВРАТА (Все опасные операции завершились успехом!) ---
            if (map_ != nullptr) {
                for (size_t old_i = first_block_; old_i <= last_block_; ++old_i) {
                    map_[old_i] = nullptr; // Зануляем, чтобы деструктор new_buf не стёр живые данные
                }
            }

            // Безопасный обмен плоскими полями
            std::swap(this->map_, new_buf.map_);
            std::swap(this->size_map_, new_buf.size_map_);

            this->first_block_ = allocated_new_chunk ? std::min(new_first_block, target_block_idx) : new_first_block;
            this->last_block_ = new_last_block;
            this->capasity_ = newCapas;

            // Вычисляем новые финальные позиции итераторов
            size_t nfi_finish = finish.first_index_i + block_offset;
            size_t nfj_finish = finish.first_index_j;

            // Принудительно и легально обновляем прокси-связи MSVC
            start.setBlock(&map_[target_block_idx], target_block_idx, target_index_j);
            finish.setBlock(&map_[nfi_finish], nfi_finish, nfj_finish);
        }
        // Сценарий Б: Реаллокация карты не нужна, на левом краю карты есть свободные блоки
        else {
            size_t target_block_idx = start.first_index_i;
            size_t target_index_j = 0;

            if (start.first_index_j == 0) {
                target_block_idx = start.first_index_i - 1;
                target_index_j = CHUNK_SIZE - 1;
            }
            else {
                target_index_j = start.first_index_j - 1;
            }

            bool allocated_new_chunk = false;
            if (map_[target_block_idx] == nullptr) {
                map_[target_block_idx] = allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            try {
                T* raw_ptr = map_[target_block_idx] + target_index_j;
                traits_type::construct(this->alloc_, raw_ptr, value);
            }
            catch (...) {
                if (allocated_new_chunk) {
                    deallocate_block(map_[target_block_idx]);
                    map_[target_block_idx] = nullptr;
                }
                throw;
            }

            if (allocated_new_chunk && target_block_idx < first_block_) {
                first_block_ = target_block_idx;
            }

            // Только теперь безопасно сдвигаем старт на новую позицию
            start.setBlock(&map_[target_block_idx], target_block_idx, target_index_j);
        }

        // Общий успешный финал для обоих сценариев
        ++size_;
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
    requires std::input_iterator<InputIterator>
        void MyDeque<T, Allocator, CHUNK_SIZE>::push_front(InputIterator first, InputIterator last) {
        if (first == last) return;
        difference_type dist = std::distance(first, last);
        difference_type free_space_front = (start.first_index_i) * CHUNK_SIZE + start.first_index_j;
        if (dist > free_space_front) {
            size_type needed_capasity = (dist - free_space_front + capasity_) * 2;
            realocate(needed_capasity);
        }
        iterator newStart = start - dist;
        for (ptrdiff_t i = static_cast<ptrdiff_t>(start.first_index_i);
            i >= static_cast<ptrdiff_t>(newStart.first_index_i); --i)
        {
            if (map_[i] == nullptr) {
                map_[i] = allocate_block();
                if (static_cast<size_t>(i) < first_block_) {
                    first_block_ = static_cast<size_t>(i);
                }
            }
        }
        newStart.setBlock(&map_[newStart.first_index_i], newStart.first_index_i, newStart.first_index_j);
        iterator itherNow = newStart;
        try {
            while (first != last) {
                traits_type::construct(this->alloc_, itherNow.current, *first);
                ++itherNow;
                ++first;
            }
        }
        catch (...) {
            // Откат: уничтожаем всё, что успели создать в этом вызове
            while (itherNow != newStart) {
                --itherNow;
                destroy(itherNow.current);
            }
            throw;
        }
        start = newStart;
        size_ += dist;
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
    requires std::input_iterator<InputIterator>
    void MyDeque<T, Allocator, CHUNK_SIZE>::push_front(InputIterator first, InputIterator last) {
        if (first == last) return;
        difference_type dist = std::distance(first, last);
        difference_type free_space_block = start.first_index_j;
        difference_type free_space_front = (start.first_index_i) * CHUNK_SIZE + free_space_block;
        if (dist <= free_space_block) {
            iterator newStart = start - dist;
            if (size_ == 0) {
                ++newStart;
            }
            iterator itherNow = newStart;
            try {
                while (first != last) {
                    traits_type::construct(this->alloc_, itherNow.current, *first);
                    ++itherNow;
                    ++first;
                }
            }
            catch (...) {
                // Откат: уничтожаем всё, что успели создать в этом вызове
                while (itherNow != newStart) {
                    --itherNow;
                    destroy(itherNow.current);
                }
                throw;
            }
            start = newStart;
            size_ += dist;
        }
        else if ((dist <= free_space_front) && (dist > free_space_block)) {
            size_t frontNewI = start.first_index_i;
            size_t frontNewJ = start.first_index_j;
            if (dist <= frontNewJ) {
                frontNewJ -= dist;
            }
            else {
                size_t rem = dist - frontNewJ;
                frontNewI -= (rem / CHUNK_SIZE);
                if (rem % CHUNK_SIZE == 0) {
                    frontNewJ = 0;
                }
                else {
                    --frontNewI;
                    frontNewJ = CHUNK_SIZE - (rem % CHUNK_SIZE);
                }
            }
            //std::cout << frontNewI << " " << frontNewJ << "\n";

            if (size_ == 0) {
                ++frontNewJ;
                if (frontNewJ == CHUNK_SIZE) {
                    frontNewJ = 0;
                    ++frontNewI;
                }
            }
            // 2. Выделяем чанки и ОБЯЗАТЕЛЬНО обновляем границы в базовом буфере
            for (size_t i = start.first_index_i - 1; i >= frontNewI; --i) {
                if (map_[i] == nullptr) {
                    map_[i] = allocate_block();
                }
                if (i == 0) break;
            }
            // КРИТИЧЕСКИ ВАЖНО: Обновляем правую границу выделенных блоков,
            // чтобы деструктор и итераторы знали, до какого момента память легальна!
            size_t old_first_block = first_block_;
            if (frontNewI < first_block_) {
                first_block_ = frontNewI;
            }
            // 3. Переходим на абсолютно безопасные плоские индексы для заполнения

            size_t curr_i = frontNewI;
            size_t curr_j = frontNewJ;
            size_t constructed_count = 0;
            
            try {
                while (first != last) {
                    if (curr_j == CHUNK_SIZE) {
                        curr_j = 0;
                        ++curr_i;
                    }
                    // Конструируем элемент в сырой памяти
                    T* raw_ptr = map_[curr_i] + curr_j;
                    traits_type::construct(this->alloc_, raw_ptr, *first);

                    ++curr_j;
                    ++constructed_count;
                    ++first;
                }
            }
            catch (...) {
                // Откат: бережно удаляем только то, что успели сконструировать в этом вызове
                for (size_t c = 0; c < constructed_count; ++c) {
                    if (curr_j == 0) {
                        curr_j = CHUNK_SIZE;
                        --curr_i;
                    }
                    --curr_j;
                    destroy(map_[curr_i] + curr_j);
                }
                first_block_ = old_first_block;
                throw;
            }
            // 4. Успешный финал: сдвигаем официальный старт контейнера влево
            start.setBlock(&map_[frontNewI], frontNewI, frontNewJ);
            size_ += dist;
        }
        else {
            // Вычисляем новую емкость с запасом
            size_type newCapacity = ((capasity_ == 0) ? CHUNK_SIZE * 3 : capasity_ * 2) + dist;
            size_t new_size_map = ((newCapacity % CHUNK_SIZE) == 0 ? (newCapacity / CHUNK_SIZE) : (newCapacity / CHUNK_SIZE) + 1);
            size_t newCapas = new_size_map * CHUNK_SIZE;

            // 1. Создаем новый временный буфер (карту) на стороне
            Base new_buf(new_size_map, this->alloc_);

            // Вычисляем, сколько блоков занимали старые элементы
            size_t old_num_blocks = (map_ != nullptr) ? (last_block_ - first_block_ + 1) : 0;

            // КРИТИЧЕСКОЕ ОТЛИЧИЕ ОТ PUSH_BACK:
            // Центрируем старые блоки, но со смещением вправо, чтобы гарантировать свободные блоки слева под dist
            size_t min_blocks_for_dist = (dist / CHUNK_SIZE) + 2;
            size_t new_first_block = (new_size_map > old_num_blocks + min_blocks_for_dist)
                ? (new_size_map - old_num_blocks) / 2 + (min_blocks_for_dist / 2)
                : new_size_map - old_num_blocks - 1;
            size_t new_last_block = new_first_block + old_num_blocks - 1;

            // Фиксируем смещение старых блоков относительно новой карты
            size_t block_offset = new_first_block - first_block_;

            // Копируем указатели на существующие чанки в новую временную карту
            if (map_ != nullptr) {
                for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                    new_buf.map_[new_i] = map_[old_i];
                }
            }

            // Вычисляем, где находился бы СТАРЫЙ старт в НОВОЙ карте
            size_t old_start_i_in_new = start.first_index_i + block_offset;
            size_t old_start_j_in_new = start.first_index_j;

            // Считаем точные координаты, где должен начаться НОВЫЙ старт контейнера (отступаем на dist влево)
            size_t frontNewI = old_start_i_in_new;
            size_t frontNewJ = old_start_j_in_new;

            if (dist <= frontNewJ) {
                frontNewJ -= dist;
            }
            else {
                size_t rem = dist - frontNewJ;
                frontNewI -= (rem / CHUNK_SIZE);
                if (rem % CHUNK_SIZE == 0) {
                    frontNewJ = 0;
                }
                else {
                    --frontNewI;
                    frontNewJ = CHUNK_SIZE - (rem % CHUNK_SIZE);
                }
            }

            if (size_ == 0) {
                ++frontNewJ;
                if (frontNewJ == CHUNK_SIZE) {
                    frontNewJ = 0;
                    ++frontNewI;
                }
            }

            // Вектор для трекинга выделенных блоков на случай rollback
            std::vector<size_t> allocated_chunk_indices;
            allocated_chunk_indices.reserve(old_start_i_in_new - frontNewI + 1);

            try {
                // 2. Выделяем новые чанки во временной карте слева под новые элементы
                for (size_t i = frontNewI; i <= old_start_i_in_new; ++i) {
                    if (new_buf.map_[i] == nullptr) {
                        new_buf.map_[i] = new_buf.allocate_block();
                        allocated_chunk_indices.push_back(i);
                    }
                }

                // 3. Конструируем новые элементы СЛЕВА НАПРАВО (от frontNewI/J до старого старта)
                if (size_ == 0) {
                    ++frontNewJ;
                    if (frontNewJ == CHUNK_SIZE) {
                        frontNewJ = 0;
                        ++frontNewI;
                    }
                }
                size_t curr_i = frontNewI;
                size_t curr_j = frontNewJ;
                size_t constructed_count = 0;

                try {
                    while (first != last) {
                        if (curr_j == CHUNK_SIZE) {
                            curr_j = 0;
                            ++curr_i;
                        }
                        T* raw_ptr = new_buf.map_[curr_i] + curr_j;
                        traits_type::construct(this->alloc_, raw_ptr, *first);

                        ++curr_j;
                        ++constructed_count;
                        ++first;
                    }
                }
                catch (...) {
                    // Если упал конструктор T, уничтожаем только то, что создали в текущем цикле
                    for (size_t c = 0; c < constructed_count; ++c) {
                        if (curr_j == 0) {
                            curr_j = CHUNK_SIZE;
                            --curr_i;
                        }
                        --curr_j;
                        destroy(new_buf.map_[curr_i] + curr_j);
                    }
                    throw;
                }
            }
            catch (...) {
                // Если упал аллокатор чанков или конструктор T, полностью зачищаем созданные блоки
                for (size_t idx : allocated_chunk_indices) {
                    new_buf.deallocate_block(new_buf.map_[idx]);
                    new_buf.map_[idx] = nullptr;
                }
                throw;
            }

            // --- ТОЧКА НЕВОЗВРАТА (Все опасные операции завершились успехом!) ---

            // 4. Очищаем старую карту в контейнере, чтобы деструктор `new_buf` не удалил чанки
            if (map_ != nullptr) {
                for (size_t old_i = first_block_; old_i <= last_block_; ++old_i) {
                    map_[old_i] = nullptr;
                }
            }

            // 5. Меняем карты местами (noexcept транзакция)
            std::swap(this->map_, new_buf.map_);
            std::swap(this->size_map_, new_buf.size_map_);

            // Синхронизируем новые границы блоков контейнера
            this->first_block_ = std::min(new_first_block, frontNewI);
            this->last_block_ = new_last_block;
            this->capasity_ = newCapas;

            // 6. Вычисляем новое положение итератора finish контейнера
            size_t new_finish_i = finish.first_index_i + block_offset;

            // Перевыставляем итераторы легально для MSVC Iterator Debugging
            start.setBlock(&map_[frontNewI], frontNewI, frontNewJ);
            finish.setBlock(&map_[new_finish_i], new_finish_i, finish.first_index_j);

            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
            size_ += dist;
        }
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::pop_front() {
        if (size_ == 0) {
            size_t center_i = size_map_ / 2;
            start.setBlock(&map_[center_i], center_i, 0);
            finish = start;
            first_block_ = center_i;
            last_block_ = center_i;
            throw std::runtime_error("Pop from empty deque");
        }
        destroy(start.current);
        --size_;
        ++start;

        if (start.first_index_j == 0) {
            first_block_ = start.first_index_i;
        }
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::reference MyDeque<T, Allocator, CHUNK_SIZE>::front() {
        if (size_ == 0) {
            throw std::runtime_error("Deque is empty");
        }
        return *start;
    }
    
    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::const_reference MyDeque<T, Allocator, CHUNK_SIZE>::front() const {
        if (size_ == 0) {
            throw std::runtime_error("Deque is empty");
        }
        return *start;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::reference MyDeque<T, Allocator, CHUNK_SIZE>::back() {
        if (size_ == 0) {
            throw std::runtime_error("Deque is empty");
        }
        // Так как finish указывает ЗА последний элемент, 
        // последний живой элемент находится на (finish - 1)
        return *(finish - 1);
    }
    
    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::const_reference MyDeque<T, Allocator, CHUNK_SIZE>::back() const {
        if (size_ == 0) {
            throw std::runtime_error("Deque is empty");
        }
        return *(finish - 1);
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::insert(const_iterator position, const T& value) {
        difference_type index = position - cbegin();
        if (index == 0) {
            push_front(value);
            return start;
        }
        else if (index == static_cast<difference_type>(size_)) {
            push_back(value);
            return finish - 1; // элемент встал перед прошлым finish
        }

        difference_type distStart = index;
        difference_type distFinish = static_cast<difference_type>(size_) - index;

        if (distStart < distFinish) {
            // Элемент ближе к началу: готовим место через push_front
            // Копируем самый первый элемент вперед
            push_front(front());

            // Теперь старт сместился влево на 1 элемент. 
            // Старые элементы теперь находятся со смещением +1.
            iterator old_start = start + 1;
            iterator target_pos = start + (index + 1); // Позиция, куда должна встать копия старого элемента

            size_type constructed_count = 0;
            iterator curr = old_start;
            try {
                // Конструированием (а не присваиванием) сдвигаем элементы влево
                while (curr != target_pos) {
                    // Конструируем копию следующего элемента на месте текущего
                    traits_type::construct(this->alloc_, curr.current, *(curr + 1));
                    ++curr;
                    ++constructed_count;
                }

                // Все элементы успешно сдвинуты. Теперь безопасно перезаписываем целевой элемент.
                // (Используем оператор присваивания, так как на этой позиции объект уже существовал)
                *target_pos = value;
            }
            catch (...) {
                // ОТКАТ: Если сдвиг или финальное присваивание выбросили исключение
                // 1. Уничтожаем все созданные дубликаты
                iterator destroy_it = old_start;
                for (size_type i = 0; i < constructed_count; ++i) {
                    destroy(destroy_it.current);
                    ++destroy_it;
                }
                // 2. Убираем временный элемент из начала, возвращая структуру памяти в исходное состояние
                pop_front();
                throw; // Пробрасываем исключение дальше (Строгая гарантия выполнена)
            }

            return target_pos;
        }
        else {
            // Элемент ближе к концу: готовим место через push_back
            push_back(back());

            iterator old_finish = finish - 1;
            iterator target_pos = start + index; // Исходная позиция вставки

            size_type constructed_count = 0;
            iterator curr = old_finish - 1;
            try {
                // Сдвигаем элементы вправо с конца к позиции вставки
                while (curr != target_pos) {
                    traits_type::construct(this->alloc_, curr.current, *(curr - 1));
                    --curr;
                    ++constructed_count;
                }

                // Перезаписываем элемент на целевой позиции
                *target_pos = value;
            }
            catch (...) {
                // ОТКАТ: Если что-то упало в процессе сдвига вправо
                iterator destroy_it = old_finish - 1;
                for (size_type i = 0; i < constructed_count; ++i) {
                    destroy(destroy_it.current);
                    --destroy_it;
                }
                pop_back();
                throw;
            }

            return target_pos;
        }
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
        requires std::input_iterator<InputIterator>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::insert(const_iterator position, InputIterator first, InputIterator last) {
        if (first == last) {
            return start + (position - cbegin());
        }
        difference_type dist = std::distance(first, last);
        difference_type index = position - cbegin();

        difference_type distStart = index;
        difference_type distFinish = static_cast<difference_type>(size_) - index;

        if (distStart < distFinish) {
            for (difference_type i = 0; i < dist; ++i) {
                push_front(T());
            }

            iterator old_start = start + dist;
            iterator pos = old_start + index;

            std::copy(old_start, pos, start);
            std::copy(first, last, pos - dist);

            return pos - dist;
        }
        else {
            for (difference_type i = 0; i < dist; ++i) {
                push_back(T());
            }
            iterator pos = start + index;
            iterator old_finish = finish - dist;

            std::copy_backward(pos, old_finish, finish);
            std::copy(first, last, pos);

            return pos;
        }
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
        requires std::input_iterator<InputIterator>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::insert(const_iterator position, InputIterator first, InputIterator last) {
        // 1. Если диапазон пуст, вставлять нечего. 
        // Возвращаем валидный неконстантный итератор на позицию.
        if (first == last) {
            return start + (position - cbegin());
        }

        // Запоминаем индекс вставки, так как position — константный итератор для внешнего использования
        difference_type insert_index = position - cbegin();

        // 2. Создаем временный дек с ТЕМ ЖЕ аллокатором, что и у текущего контейнера
        MyDeque tmp(this->alloc_);
        tmp.reserve(this->capasity_);
        // Заранее резервируем суммарный объем памяти, если категория итераторов позволяет узнать dist.
        // Это минимизирует количество реаллокаций во временном контейнере.
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
            typename std::iterator_traits<InputIterator>::iterator_category>) {
            difference_type dist = std::distance(first, last);
            tmp.reserve(size_ + dist);
        }

        try {
            // Этап А: Копируем элементы ДО точки вставки
            for (difference_type i = 0; i < insert_index; ++i) {
                tmp.push_back(*(start + i));
            }

            // Этап Б: Копируем элементы из вставляемого диапазона [first, last)
            while (first != last) {
                tmp.push_back(*first);
                ++first;
            }

            // Этап В: Копируем элементы ПОСЛЕ точки вставки
            for (size_type i = insert_index; i < size_; ++i) {
                tmp.push_back(*(start + i));
            }
        }
        catch (...) {
            // Если что-то упало (std::bad_alloc или конструктор копирования T), 
            // деструктор `tmp` автоматически очистит всё, что успел выделить.
            // Исходный дек (*this) гарантированно не изменился.
            throw;
        }

        // --- ТОЧКА НЕВОЗВРАТА ---
        // Все элементы успешно скопированы во временный буфер.
        // Обмениваем низкоуровневые буферы (карты указателей) из MyDequeBuff
        std::swap(this->map_, tmp.map_);
        std::swap(this->size_map_, tmp.size_map_);
        std::swap(this->size_elem_map_, tmp.size_elem_map_);
        std::swap(this->first_block_, tmp.first_block_);
        std::swap(this->last_block_, tmp.last_block_);

        // Обмениваем высокоуровневые поля MyDeque
        std::swap(this->size_, tmp.size_);
        std::swap(this->capasity_, tmp.capasity_);
        std::swap(this->capac_size_ratio_, tmp.capac_size_ratio_);
        std::swap(this->last_index_i, tmp.last_index_i);
        std::swap(this->last_index_j, tmp.last_index_j);
        std::swap(this->start, tmp.start);
        std::swap(this->finish, tmp.finish);

        // ВАЖНО: Аллокаторы (this->alloc_ и tmp.alloc_) остаются на своих местах и НЕ перемещаются.
        // Теперь деструктор `tmp` автоматически и полностью безопасно уничтожит СТАРЫЕ данные 
        // вашего контейнера, так как мы подменили его внутренности.
        // Возвращаем итератор, указывающий на первый из вставленных элементов
        return start + insert_index;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename... Args>
    MyDeque<T, Allocator, CHUNK_SIZE>::reference MyDeque<T, Allocator, CHUNK_SIZE>::emplace_back(Args&&... args) {
        // Сценарий А: Карта чанков заполнена, требуется её расширение
        if (size_map_ == 0 || finish.first_index_i >= size_map_) {
            size_type newCapacity = (capasity_ == 0) ? CHUNK_SIZE * 3 : capasity_ * 2;
            size_t new_size_map = (newCapacity % CHUNK_SIZE) == 0 ? (newCapacity / CHUNK_SIZE) : (newCapacity / CHUNK_SIZE) + 1;
            size_t newCapas = new_size_map * CHUNK_SIZE;

            // Создаем временный буфер для новой карты
            Base new_buf(new_size_map, this->alloc_);

            size_t new_first_block = (newCapas / 2 - size_ / 2) / CHUNK_SIZE;
            size_t new_last_block = new_first_block + (last_block_ - first_block_);

            if (map_ != nullptr) {
                for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                    new_buf.map_[new_i] = map_[old_i];
                }
            }

            size_t block_offset = new_first_block - first_block_;
            size_t target_block_idx = finish.first_index_i + block_offset;
            bool allocated_new_chunk = false;

            if (finish.first_index_j == 0 && new_buf.map_[target_block_idx] == nullptr) {
                new_buf.map_[target_block_idx] = new_buf.allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            try {
                T* raw_ptr = new_buf.map_[target_block_idx] + finish.first_index_j;
                // Конструируем объект прямо в куче, используя переданные аргументы
                traits_type::construct(this->alloc_, raw_ptr, std::forward<Args>(args)...);
            }
            catch (...) {
                if (allocated_new_chunk) {
                    new_buf.deallocate_block(new_buf.map_[target_block_idx]);
                }
                throw; // Откат: исходный контейнер не тронут
            }

            // --- ТОЧКА НЕВОЗВРАТА ---
            if (map_ != nullptr) {
                for (size_t old_i = first_block_; old_i <= last_block_; ++old_i) {
                    map_[old_i] = nullptr;
                }
            }

            std::swap(this->map_, new_buf.map_);
            std::swap(this->size_map_, new_buf.size_map_);

            this->first_block_ = new_first_block;
            this->last_block_ = allocated_new_chunk ? std::max(new_last_block, target_block_idx) : new_last_block;
            this->capasity_ = newCapas;

            size_t nfi = start.first_index_i + block_offset;
            size_t nfj = start.first_index_j;
            size_t nli = finish.first_index_i + block_offset;
            size_t nlj = finish.first_index_j;

            start.setBlock(&map_[nfi], nfi, nfj);
            finish.setBlock(&map_[nli], nli, nlj);
        }
        // Сценарий Б: Реаллокация карты не нужна, работаем в текущей карте
        else {
            size_t current_block_idx = finish.first_index_i;
            bool allocated_new_chunk = false;

            if (finish.first_index_j == 0 && map_[current_block_idx] == nullptr) {
                map_[current_block_idx] = allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            try {
                T* raw_ptr = map_[current_block_idx] + finish.first_index_j;
                traits_type::construct(this->alloc_, raw_ptr, std::forward<Args>(args)...);
            }
            catch (...) {
                if (allocated_new_chunk) {
                    deallocate_block(map_[current_block_idx]);
                    map_[current_block_idx] = nullptr;
                }
                throw; // Откат
            }

            if (allocated_new_chunk) {
                finish.setBlock(&map_[current_block_idx], current_block_idx, 0);
                if (current_block_idx > last_block_) {
                    last_block_ = current_block_idx;
                }
            }
        }

        // Общий успешный финал
        T* result_ptr = finish.current;
        ++finish;
        ++size_;
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;

        return *result_ptr;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename... Args>
    MyDeque<T, Allocator, CHUNK_SIZE>::reference MyDeque<T, Allocator, CHUNK_SIZE>::emplace_front(Args&&... args) {
        // Сценарий А: Достигнут левый край карты
        if (size_map_ == 0 || (start.first_index_i == 0 && start.first_index_j == 0)) {
            size_type newCapacity = (capasity_ == 0) ? CHUNK_SIZE * 3 : capasity_ * 2;
            size_t new_size_map = (newCapacity % CHUNK_SIZE) == 0 ? (newCapacity / CHUNK_SIZE) : (newCapacity / CHUNK_SIZE) + 1;
            size_t newCapas = new_size_map * CHUNK_SIZE;

            Base new_buf(new_size_map, this->alloc_);

            size_t new_first_block = (newCapas / 2 - size_ / 2) / CHUNK_SIZE;
            size_t new_last_block = new_first_block + (last_block_ - first_block_);

            if (map_ != nullptr) {
                for (size_t old_i = first_block_, new_i = new_first_block; old_i <= last_block_; ++old_i, ++new_i) {
                    new_buf.map_[new_i] = map_[old_i];
                }
            }

            size_t block_offset = new_first_block - first_block_;
            size_t current_start_i = start.first_index_i + block_offset;
            size_t current_start_j = start.first_index_j;

            size_t target_block_idx = current_start_i;
            size_t target_index_j = 0;

            if (current_start_j == 0) {
                target_block_idx = current_start_i - 1;
                target_index_j = CHUNK_SIZE - 1;
            }
            else {
                target_index_j = current_start_j - 1;
            }

            bool allocated_new_chunk = false;
            if (new_buf.map_[target_block_idx] == nullptr) {
                new_buf.map_[target_block_idx] = new_buf.allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            try {
                T* raw_ptr = new_buf.map_[target_block_idx] + target_index_j;
                traits_type::construct(this->alloc_, raw_ptr, std::forward<Args>(args)...);
            }
            catch (...) {
                if (allocated_new_chunk) {
                    new_buf.deallocate_block(new_buf.map_[target_block_idx]);
                }
                throw;
            }

            // --- ТОЧКА НЕВОЗВРАТА ---
            if (map_ != nullptr) {
                for (size_t old_i = first_block_; old_i <= last_block_; ++old_i) {
                    map_[old_i] = nullptr;
                }
            }

            std::swap(this->map_, new_buf.map_);
            std::swap(this->size_map_, new_buf.size_map_);

            this->first_block_ = allocated_new_chunk ? std::min(new_first_block, target_block_idx) : new_first_block;
            this->last_block_ = new_last_block;
            this->capasity_ = newCapas;

            size_t nfi_finish = finish.first_index_i + block_offset;
            size_t nfj_finish = finish.first_index_j;

            start.setBlock(&map_[target_block_idx], target_block_idx, target_index_j);
            finish.setBlock(&map_[nfi_finish], nfi_finish, nfj_finish);
        }
        // Сценарий Б: Реаллокация карты не требуется
        else {
            size_t target_block_idx = start.first_index_i;
            size_t target_index_j = 0;

            if (start.first_index_j == 0) {
                target_block_idx = start.first_index_i - 1;
                target_index_j = CHUNK_SIZE - 1;
            }
            else {
                target_index_j = start.first_index_j - 1;
            }

            bool allocated_new_chunk = false;
            if (map_[target_block_idx] == nullptr) {
                map_[target_block_idx] = allocate_block(); // Может бросить bad_alloc
                allocated_new_chunk = true;
            }

            try {
                T* raw_ptr = map_[target_block_idx] + target_index_j;
                traits_type::construct(this->alloc_, raw_ptr, std::forward<Args>(args)...);
            }
            catch (...) {
                if (allocated_new_chunk) {
                    deallocate_block(map_[target_block_idx]);
                    map_[target_block_idx] = nullptr;
                }
                throw;
            }

            if (allocated_new_chunk && target_block_idx < first_block_) {
                first_block_ = target_block_idx;
            }

            start.setBlock(&map_[target_block_idx], target_block_idx, target_index_j);
        }

        ++size_;
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;

        return *start;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename... Args>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::emplace(const_iterator position, Args&&... args) {
        difference_type insert_index = position - cbegin();

        // 1. Быстрые пути, если вставляем в самые края (строгая гарантия обеспечивается внутри них)
        if (insert_index == 0) {
            emplace_front(std::forward<Args>(args)...);
            return start;
        }
        if (insert_index == static_cast<difference_type>(size_)) {
            emplace_back(std::forward<Args>(args)...);
            return finish - 1;
        }

        // 2. Создаем изолированный временный дек с тем же аллокатором
        MyDeque tmp(this->alloc_);
        tmp.reserve(size_ + 1); // Защищаем tmp от лишних внутренних реаллокаций

        try {
            // Этап А: Переносим элементы ДО точки вставки
            for (difference_type i = 0; i < insert_index; ++i) {
                tmp.push_back(*(start + i));
            }

            // Этап Б: Конструируем НОВЫЙ элемент непосредственно в tmp (ровно 1 раз)
            tmp.emplace_back(std::forward<Args>(args)...);

            // Этап В: Переносим элементы ПОСЛЕ точки вставки
            for (size_type i = insert_index; i < size_; ++i) {
                tmp.push_back(*(start + i));
            }
        }
        catch (...) {
            // Любая ошибка на этапах копирования или конструирования T 
            // приведет к уничтожению tmp. Исходный дек (*this) не изменен.
            throw;
        }

        // --- АБСОЛЮТНАЯ ТОЧКА НЕВОЗВРАТА (`noexcept` рокировка ресурсов) ---
        // Обмениваем низкоуровневые буферы (MyDequeBuff)
        std::swap(this->map_, tmp.map_);
        std::swap(this->size_map_, tmp.size_map_);
        std::swap(this->size_elem_map_, tmp.size_elem_map_);
        std::swap(this->first_block_, tmp.first_block_);
        std::swap(this->last_block_, tmp.last_block_);

        // Обмениваем высокоуровневые поля MyDeque
        std::swap(this->size_, tmp.size_);
        std::swap(this->capasity_, tmp.capasity_);
        std::swap(this->capac_size_ratio_, tmp.capac_size_ratio_);
        std::swap(this->last_index_i, tmp.last_index_i);
        std::swap(this->last_index_j, tmp.last_index_j);
        std::swap(this->start, tmp.start);
        std::swap(this->finish, tmp.finish);

        // В этот момент `tmp` содержит старый буфер текущего дека.
        // При выходе из метода деструктор `tmp` очистит старую память.

        return start + insert_index;
    }

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::erase(const_iterator position) {
        if (size_ == 0) {
            return begin();
        }
        difference_type index = position - cbegin();

        // 1. Быстрые пути для краев контейнера (pop_front/pop_back гарантированно атомарны)
        if (index == 0) {
            pop_front();
            return start;
        }
        if (index == static_cast<difference_type>(size_ - 1)) {
            pop_back();
            return finish; // возвращает новый finish (прошлый finish - 1 уничтожен)
        }

        iterator pos = start + index;

        // Если перемещение типа T гарантированно не бросает исключений, 
        // мы можем выполнить быстрый noexcept сдвиг без оверхеда на транзакции.
        if constexpr (std::is_nothrow_move_assignable_v<T>) {
            if (index < static_cast<difference_type>(size_ / 2)) {
                std::move_backward(start, pos, pos + 1);
                destroy(start.current);
                ++start;
            }
            else {
                std::move(pos + 1, finish, pos);
                --finish;
                destroy(finish.current);
            }
            --size_;
            return start + index;
        }
        // ТРАНЗАКЦИОННЫЙ БЛОК (для типов T, чьи операторы могут выбросить исключение)
        else {
            // Сохраняем удаляемое значение на случай отката (может выбросить исключение, контейнер еще не изменен)
            T saved_value = *pos;

            if (index < static_cast<difference_type>(size_ / 2)) {
                // Элемент ближе к началу: сдвигаем левую часть [start, pos) ВПРАВО
                iterator curr = pos;
                size_type moved_count = 0;
                try {
                    while (curr != start) {
                        *curr = std::move(*(curr - 1));
                        --curr;
                        ++moved_count;
                    }
                }
                catch (...) {
                    // ОТКАТ: возвращаем элементы на свои места слева направо
                    iterator rollback_it = start + (index - moved_count);
                    for (size_type i = 0; i < moved_count; ++i) {
                        *rollback_it = std::move(*(rollback_it + 1));
                        ++rollback_it;
                    }
                    // Восстанавливаем исходное значение удаляемого элемента
                    *pos = std::move(saved_value);
                    throw;
                }

                // Транзакция сдвига завершена успешно. Уничтожаем дубликат в начале.
                destroy(start.current);
                ++start;
                --size_;
                return start + index;
            }
            else {
                // Элемент ближе к концу: сдвигаем правую часть [pos + 1, finish) ВЛЕВО
                iterator curr = pos;
                size_type moved_count = 0;
                try {
                    while (curr + 1 != finish) {
                        *curr = std::move(*(curr + 1));
                        ++curr;
                        ++moved_count;
                    }
                }
                catch (...) {
                    // ОТКАТ: возвращаем элементы на свои места справа налево
                    iterator rollback_it = finish - 1;
                    for (size_type i = 0; i < moved_count; ++i) {
                        *rollback_it = std::move(*(rollback_it - 1));
                        --rollback_it;
                    }
                    // Восстанавливаем исходное значение удаляемого элемента
                    *pos = std::move(saved_value);
                    throw;
                }

                // Транзакция сдвига завершена успешно. Уничтожаем дубликат в конце.
                --finish;
                destroy(finish.current);
                --size_;
                return start + index;
            }
        }
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::erase(const_iterator position) {
        difference_type index = position - cbegin();

        // Если удаляем самый первый или самый последний элемент — вызываем быстрые pop_front/pop_back
        if (index == 0) {
            pop_front();
            return start;
        }
        else if (index == static_cast<difference_type>(size_ - 1)) {
            pop_back();
            return finish;
        }

        iterator pos = start + index;

        // Оптимизация: сдвигаем ту часть элементов, которая меньше
        if (index < static_cast<difference_type>(size_ / 2)) {
            // Элемент ближе к началу: сдвигаем левую часть ВПРАВО
            // Перемещаем элементы из диапазона [start, pos) на одну позицию вправо
            std::move_backward(start, pos, pos + 1);

            // Теперь самый первый элемент (на старом start) дублирован, его нужно уничтожить
            destroy(start.current);

            // Сдвигаем границу начала дека
            ++start;
            --size_;

            // Возвращаем итератор на элемент, который теперь встал на место удаленного
            return start + index;
        }
        else {
            // Элемент ближе к концу: сдвигаем правую часть ВЛЕВО
            // Перемещаем элементы из диапазона [pos + 1, finish) на одну позицию влево
            std::move(pos + 1, finish, pos);

            // Теперь самый последний элемент (на старом finish - 1) дублирован, уничтожаем его
            --finish;
            destroy(finish.current);
            --size_;

            return start + index;
        }
    }
    */

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    typename MyDeque<T, Allocator, CHUNK_SIZE>::iterator
        MyDeque<T, Allocator, CHUNK_SIZE>::erase(const_iterator first, const_iterator last) {
        if (first == last) {
            return start + (first - cbegin());
        }

        difference_type index_first = first - cbegin();
        difference_type count = last - first;

        // Если удаляется вообще весь дек целиком (clear гарантированно не бросает исключений)
        if (count == static_cast<difference_type>(size_)) {
            clear();
            return finish;
        }

        difference_type elements_before = index_first;
        difference_type elements_after = static_cast<difference_type>(size_) - (index_first + count);

        iterator pos_first = start + index_first;
        iterator pos_last = pos_first + count;

        // Сценарий А: Быстрая noexcept ветка для типов с безопасным перемещением
        if constexpr (std::is_nothrow_move_assignable_v<T>) {
            if (elements_before < elements_after) {
                std::move_backward(start, pos_first, pos_last);
                iterator it_to_destroy = start;
                for (difference_type i = 0; i < count; ++i) {
                    destroy(it_to_destroy.current);
                    ++it_to_destroy;
                }
                start += count;
            }
            else {
                std::move(pos_last, finish, pos_first);
                iterator new_finish = finish - count;
                iterator it_to_destroy = new_finish;
                while (it_to_destroy != finish) {
                    destroy(it_to_destroy.current);
                    ++it_to_destroy;
                }
                finish = new_finish;
            }
            size_ -= count;
            return start + index_first;
        }
        // Сценарий Б: Транзакционный блок для типов T, чьи операции могут выбросить исключение
        else {
            // Резервируем и сохраняем удаляемые элементы. 
            // Если тут выбросится bad_alloc, контейнер еще не изменен — строгая гарантия соблюдена.
            std::vector<T> saved_elements;
            saved_elements.reserve(count);
            for (auto it = pos_first; it != pos_last; ++it) {
                saved_elements.push_back(*it);
            }

            if (elements_before < elements_after) {
                // Сдвигаем отрезок [start, pos_first) ВПРАВО на величину count
                iterator curr_src = pos_first;
                iterator curr_dst = pos_last;
                size_type moved_count = 0;

                try {
                    while (curr_src != start) {
                        --curr_src;
                        --curr_dst;
                        *curr_dst = std::move(*curr_src);
                        ++moved_count;
                    }
                }
                catch (...) {
                    // ОТКАТ: Возвращаем сдвинутые элементы обратно ВЛЕВО
                    iterator rollback_src = pos_last - moved_count;
                    iterator rollback_dst = pos_first - moved_count;
                    for (size_type i = 0; i < moved_count; ++i) {
                        *rollback_dst = std::move(*rollback_src);
                        ++rollback_src;
                        ++rollback_dst;
                    }
                    // Восстанавливаем изначальные удаляемые элементы на их законное место
                    for (difference_type i = 0; i < count; ++i) {
                        *(pos_first + i) = std::move(saved_elements[i]);
                    }
                    throw;
                }

                // Транзакция успешна. Уничтожаем ставшие ненужными дубликаты в начале дека
                iterator it_to_destroy = start;
                for (difference_type i = 0; i < count; ++i) {
                    destroy(it_to_destroy.current);
                    ++it_to_destroy;
                }
                start += count;
            }
            else {
                // Сдвигаем отрезок [pos_last, finish) ВЛЕВО на величину count
                iterator curr_src = pos_last;
                iterator curr_dst = pos_first;
                size_type moved_count = 0;

                try {
                    while (curr_src != finish) {
                        *curr_dst = std::move(*curr_src);
                        ++curr_src;
                        ++curr_dst;
                        ++moved_count;
                    }
                }
                catch (...) {
                    // ОТКАТ: Возвращаем сдвинутые элементы обратно ВПРАВО (идем с конца отрезка)
                    iterator rollback_src = pos_first + moved_count;
                    iterator rollback_dst = pos_last + moved_count;
                    for (size_type i = 0; i < moved_count; ++i) {
                        --rollback_src;
                        --rollback_dst;
                        *rollback_dst = std::move(*rollback_src);
                    }
                    // Восстанавливаем изначальные удаляемые элементы из транзакционного буфера
                    for (difference_type i = 0; i < count; ++i) {
                        *(pos_first + i) = std::move(saved_elements[i]);
                    }
                    throw;
                }

                // Транзакция успешна. Уничтожаем освободившиеся элементы в конце дека
                iterator new_finish = finish - count;
                iterator it_to_destroy = new_finish;
                while (it_to_destroy != finish) {
                    destroy(it_to_destroy.current);
                    ++it_to_destroy;
                }
                finish = new_finish;
            }

            size_ -= count;
            return start + index_first;
        }
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    MyDeque<T, Allocator, CHUNK_SIZE>::iterator MyDeque<T, Allocator, CHUNK_SIZE>::erase(const_iterator first, const_iterator last) {
        if (first == last) {
            return start + (first - cbegin());
        }

        difference_type index_first = first - cbegin();
        difference_type count = last - first;

        // Если удаляется вообще весь дек целиком
        if (count == static_cast<difference_type>(size_)) {
            clear();
            return finish;
        }

        difference_type elements_before = index_first;
        difference_type elements_after = static_cast<difference_type>(size_) - (index_first + count);

        iterator pos_first = start + index_first;
        iterator pos_last = pos_first + count;

        if (elements_before < elements_after) {
            // Быстрее сдвинуть элементы ДО удаляемого диапазона ВПРАВО
            // Сдвигаем отрезок [start, pos_first) вправо на величину count
            std::move_backward(start, pos_first, pos_last);

            // Уничтожаем освободившиеся элементы в начале дека (старые элементы от start до start + count)
            iterator it_to_destroy = start;
            for (difference_type i = 0; i < count; ++i) {
                destroy(it_to_destroy.current);
                ++it_to_destroy;
            }

            // Сдвигаем указатель начала и обновляем размер
            start += count;
            size_ -= count;

            return start + index_first;
        }
        else {
            // Быстрее сдвинуть элементы ПОСЛЕ удаляемого диапазона ВЛЕВО
            // Сдвигаем отрезок [pos_last, finish) влево на величину count
            std::move(pos_last, finish, pos_first);

            // Уничтожаем освободившиеся элементы в конце дека (от нового finish до старого finish)
            iterator new_finish = finish - count;
            iterator it_to_destroy = new_finish;
            while (it_to_destroy != finish) {
                destroy(it_to_destroy.current);
                ++it_to_destroy;
            }

            // Сдвигаем указатель конца и обновляем размер
            finish = new_finish;
            size_ -= count;

            return start + index_first;
        }
    }*/

    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::reserve(size_t new_capacity) {
        if (new_capacity > capasity_) {
            realocate(new_capacity);
        }
    }
    
    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::resize(size_t new_size, const T& value) {
        if (new_size > size_) {
            // Запоминаем исходное состояние контейнера для возможного отката
            size_type old_size = size_;

            // Делаем reserve заранее (если бросит bad_alloc — дек еще не изменен, гарантия соблюдена)
            reserve(new_size);

            try {
                // Пытаемся поштучно добавить элементы
                while (size_ < new_size) {
                    push_back(value);
                }
            }
            catch (...) {
                // ОТКАТ: Если хоть один конструктор T упал, 
                // мы обязаны вернуть дек строго в исходное состояние.
                while (size_ > old_size) {
                    pop_back(); // pop_back гарантированно noexcept
                }
                throw; // Пробрасываем исключение пользователю
            }
        }
        else {
            // Сценарий Б: Уменьшение размера (Вызовы деструкторов всегда noexcept)
            iterator new_finish = start + new_size;

            // Уничтожаем элементы от нового конца до старого finish
            destroy(new_finish, finish);

            // Перевыставляем finish на новую позицию
            finish = new_finish;
            size_ = new_size;

            // Синхронизируем служебные переменные
            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;
            last_block_ = finish.first_index_i;
        }
    }

    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::resize(size_t new_size, const T& value) {
        if (new_size > size_) {
            // Вы можете сделать reserve заранее, чтобы realocate вызвался один раз, 
            // а не дергался по ходу push_back
            reserve(new_size);

            while (size_ < new_size) {
                push_back(value);
            }
        }
        else {
            // Сценарий Б: Уменьшение размера
                // Вариант 2: Более быстрый (вызывает деструкторы массово)
                // 1. Вычисляем итератор, который станет новым концом контейнера
            DequeIterator<T, CHUNK_SIZE> new_finish = start + new_size;

            // 2. Уничтожаем только те элементы, которые идут от нового конца до старого finish
            destroy(new_finish, finish);

            // 3. Перевыставляем finish на новую позицию
            finish = new_finish;
            size_ = new_size;

            // 4. Синхронизируем ваши служебные переменные
            last_index_i = finish.first_index_i;
            last_index_j = finish.first_index_j;

            // 5. Опционально: Корректируем last_block_, если итератор ушел на предыдущие блоки
            last_block_ = finish.first_index_i;
        }
    }*/
    
    template <typename T, typename Allocator, size_t CHUNK_SIZE>
    void MyDeque<T, Allocator, CHUNK_SIZE>::clear() noexcept {
        // 1. Вызываем деструкторы для всех реально созданных объектов T.
        // Операция гарантированно не бросает исключений (noexcept).
        destroy(start, finish);

        // 2. Сбрасываем счетчик количества элементов
        size_ = 0;

        // 3. Выравниваем итераторы start и finish ровно по центру текущей карты map_.
        // Это гарантирует, что после очистки контейнер сможет одинаково эффективно 
        // расти как в начало (push_front), так и в конец (push_back).
        size_t center_i = size_map_ / 2;
        start.setBlock(&map_[center_i], center_i, 0);
        finish = start;

        // 4. Синхронизируем служебные индексы
        last_index_i = center_i;
        last_index_j = 0;

        // Границы first_block_ и last_block_ в MyDequeBuff тоже сжимаются к центру,
        // так как живых элементов больше нет, хотя физически память чанков остается выделенной.
        first_block_ = center_i;
        last_block_ = center_i;
    }
}

// TODO: установите здесь ссылки на дополнительные заголовки, требующиеся для программы.
  //template <typename T, typename Allocator, size_t CHUNK_SIZE>
    //MyDeque<T, Allocator, CHUNK_SIZE>& MyDeque<T, Allocator, CHUNK_SIZE>::operator=(MyDeque&& rhs) noexcept(//false)
    //    (typename std::allocator_traits<Allocator>::propagate_on_container_move_assignment() && 
    //        std::is_nothrow_move_assignable_v<Allocator>) ||
    //    typename std::allocator_traits<Allocator>::is_always_equal())
    //{
    //    if (this != &rhs) {
    //        try {
    //            // 1. СНАЧАЛА перемещаем базовый буфер. 
    //            // Именно здесь вылетит std::invalid_argument из аллокатора!
    //            Base::operator=(std::move(rhs));
    //        }
    //        catch (...) {
    //            // СЮДА МЫ ТЕПЕРЬ СТРОГО ЗАЙДЕМ!
    //            std::cout << "\n[SUCCESS] catch (...) exept MyDequeBuff operator=\n";
    //            // Переводим текущий дек в безопасное пустое состояние,
    //            // чтобы его деструктор в конце теста не вызвал Access Violation / Double Free!
    //            size_ = 0;
    //            capasity_ = 0;
    //            last_index_i = 0;
    //            last_index_j = 0;
    //            start.setBlock(nullptr, 0, 0);
    //            finish = start;
    //            throw; // Пробрасываем исключение дальше в Google Test
    //        }
    //        // --- ТОЧКА НЕВОЗВРАТА (Сюда дойдем, только если база успешно переместилась) ---
    //        // 2. Уничтожаем объекты текущего контейнера ТОЛЬКО СЕЙЧАС.
    //        destroy(start, finish);
    //        // 3. Обновляем плоские переменные и итераторы
    //        capasity_ = rhs.capasity_;
    //        size_ = rhs.size_;
    //        capac_size_ratio_ = rhs.capac_size_ratio_;
    //        last_index_i = rhs.last_index_i;
    //        last_index_j = rhs.last_index_j;
    //        start = rhs.start;
    //        finish = rhs.finish;
    //        // 4. Безопасно зануляем источник (rhs)
    //        rhs.capasity_ = 0;
    //        rhs.size_ = 0;
    //        rhs.last_index_i = 0;
    //        rhs.last_index_j = 0;
    //        rhs.start.setBlock(nullptr, 0, 0);
    //        rhs.finish = rhs.start;
    //    }
    //    return *this;
    //}


    /*template <typename T, typename Allocator, size_t CHUNK_SIZE>
    template <typename InputIterator>
    requires std::input_iterator<InputIterator>
        void MyDeque<T, Allocator, CHUNK_SIZE>::push_back(InputIterator first, InputIterator last) {
        if (first == last) return;
        difference_type dist = std::distance(first, last);
        difference_type free_space_back = (size_map_ - finish.first_index_i - 1) * CHUNK_SIZE + CHUNK_SIZE - finish.first_index_j;
        //std::cout << "free_space_back = " << free_space_back << "\n";
        if (dist > free_space_back) {
            size_type needed_capasity = (dist - free_space_back + capasity_) * 2;
            realocate(needed_capasity);
        }
        size_type end_block_needed = (finish + dist).first_index_i;
        for (size_t i = finish.first_index_i; i <= end_block_needed; ++i) {
            if (map_[i] == nullptr) {
                map_[i] = allocate_block();
                if (i > last_block_) last_block_ = i;
            }
        }
        finish.setBlock(&map_[finish.first_index_i], finish.first_index_i, finish.first_index_j);
        iterator current_finish = finish;
        try {
            while (first != last) {
                // Конструируем элемент в сырой памяти, куда указывает текущий виртуальный finish
                traits_type::construct(this->alloc_, current_finish.current, *first);
                ++current_finish;
                ++first;
            }
        }
        catch (...) {
            // Откат: уничтожаем всё, что успели создать в этом вызове
            while (finish != current_finish) {
                --current_finish;
                destroy(current_finish.current);
            }
            throw; // Пробрасываем исключение дальше
        }

        // Если всё прошло успешно, обновляем состояние дека
        finish = current_finish;
        last_index_i = finish.first_index_i;
        last_index_j = finish.first_index_j;
        size_ += dist;
    }*/

/*

*/

/*
* iterator insert(const_iterator position, const T& value) {
            difference_type index = position - cbegin();
            if (index == 0) {
                push_front(value);
                return start;
            }
            else if (index == static_cast<difference_type>(size_)) {
                push_back(value);
                return finish - 1; // элемент встал перед прошлым finish
            }
            difference_type distStart = index;
            difference_type distFinish = static_cast<difference_type>(size_) - index;

            iterator pos = start + index;
            if (distStart < distFinish) {
                push_front(front());

                pos = start + (index + 1);
                iterator old_start = start + 1;

                std::copy(old_start + 1, pos, old_start);
                --pos;
                *pos = value;
            }
            else {
                push_back(back());
                pos = start + index;
                std::copy_backward(pos, finish - 2, finish - 1);
                *pos = value;
            }
            return pos;
        }
*/