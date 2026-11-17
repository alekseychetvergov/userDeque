# myDeque

`myDeque` — реализация двусторонней очереди на C++20. Основной контейнер — шаблон `Deque::MyDeque<T, Allocator, CHUNK_SIZE>`; заголовки проекта находятся в `include/`.

## Преимущества перед `std::deque`

Контейнер `Deque::MyDeque<T, Allocator, CHUNK_SIZE>` спроектирован на базе C++20 с упором на предсказуемость работы с памятью, строгую безопасность исключений и удобство пакетных операций:

* **Пакетная вставка в оба конца по диапазону итераторов (`push_back` и `push_front`).** В стандартном `std::deque` вставка диапазона в начало или конец требует вызова `insert(begin()/end(), first, last)` или использования адаптеров `std::back_inserter` / `std::front_inserter` (которые вставляют элементы по одному). В `MyDeque` реализованы прямые перегрузки с ограничением концептом `std::input_iterator`:
  * `push_back(first, last)` — заранее вычисляет дистанцию `std::distance(first, last)`, оценивает свободное место в текущем чанке и правой части карты, при необходимости одним этапом расширяет карту указателей и конструирует элементы напрямую по индексам чанков без лишних проверок границ на каждой итерации.
  * `push_front(first, last)` — предоставляет аналогичный удобный интерфейс для пакетного добавления диапазона в начало очереди.
* **Управление размером чанка (`CHUNK_SIZE`) через параметр шаблона.** В `std::deque` размер блока памяти жёстко зашит в реализацию стандартной библиотеки (например, 16 байт в MSVC или 512 байт в libstdc++). В `MyDeque` параметр `CHUNK_SIZE` (по умолчанию `8`) задаётся пользователем на этапе компиляции, что позволяет подбирать размер блока под размер кэш-линии процессора и размер хранимых объектов `T`.
* **Явный контроль ёмкости (`reserve` и `capasity`).** В отличие от `std::deque`, который полностью скрывает текущую ёмкость буфера и не имеет метода `reserve()`, `MyDeque` позволяет заранее зарезервировать память под нужное количество элементов и отследить выделенный объём через `capasity()`.
* **Строгая гарантия безопасности исключений (Strong Exception Guarantee) с транзакционным откатом.** При реаллокации карты указателей и выделении новых чанков операции выполняются во временном буфере `MyDequeBuff` («на стороне»). Если аллокатор выбросит `std::bad_alloc` или конструктор элемента `T` завершится исключением, все уже сконструированные в рамках операции объекты корректно уничтожаются, а исходный контейнер остаётся в неизменном валидном состоянии.
* **Собственная легковесная инфраструктура PMR-аллокаторов (`my_memory/my_allocator.hpp`).** Проект включает готовые реализации `SimpleAlloc::MySimpleAllocator` и полиморфного аллокатора `PmrAllocator::MyPmrAllocator` на базе `MyMemoryResource` с поддержкой выравнивания (`std::align_val_t`), `rebind` и корректной обработкой `propagate_on_container_copy_assignment` / `propagate_on_container_move_assignment`.
* **Единая C++20-реализация и прозрачность (Header-only).** Поведение контейнера, стратегия роста карты и инвалидация итераторов идентичны на всех компиляторах (MSVC, GCC, Clang), а итераторы `DequeIterator` поддерживают трёхстороннее сравнение (`operator<=>`) и безопасное неявное преобразование из `iterator` в `const_iterator` без `const_cast`.

### Сравнение `Deque::MyDeque` и `std::deque`

| Критерий | `Deque::MyDeque` | `std::deque` |
| :--- | :--- | :--- |
| **Пакетная вставка в конец/начало** | Прямые методы `push_back(first, last)` и `push_front(first, last)` | Через `insert(...)` или поэлементно через `std::back_inserter` / `std::front_inserter` |
| **Размер чанка** | Настраивается пользователем (`CHUNK_SIZE`) | Фиксирован реализацией STL |
| **Предварительное резервирование** | Поддерживается (`reserve()`, `capasity()`) | Отсутствует |
| **Полиморфная память (PMR)** | Встроенные `MyPmrAllocator` и `MyMemoryResource` + совместимость с STL-аллокаторами | Через `std::pmr::deque` (`<memory_resource>`) |
| **Кроссплатформенная предсказуемость** | Одинаковая модель памяти на всех платформах (C++20) | Зависит от стандартной библиотеки компилятора |

## Требования

Для сборки на Windows нужны:

- MSVC из Visual Studio или Visual Studio Build Tools с компонентами C++ (полная IDE не требуется);
- CMake версии 3.20 или новее;
- Ninja;
- vcpkg. Переменная окружения `VCPKG_ROOT` должна указывать на каталог vcpkg.

Зависимость GoogleTest устанавливается через vcpkg manifest (`vcpkg.json`) при конфигурации CMake.

## Сборка из консоли без IDE

Установите Visual Studio Build Tools с компонентами C++, а также CMake, Ninja и vcpkg. Откройте **Developer PowerShell for VS Build Tools**, чтобы переменные среды MSVC были настроены, затем перейдите в корень репозитория и выполните:

```powershell
$env:VCPKG_ROOT = "C:\tools\vcpkg"
cmake --preset x64-debug
cmake --build --preset x64-debug
ctest --test-dir out/build/x64-debug --output-on-failure
.\out\build\x64-debug\examples\deque_example.exe
```

Замените `C:\tools\vcpkg` на фактический путь к вашей установке vcpkg. Те же команды можно выполнять из Developer PowerShell, запущенного из терминала или меню «Пуск»; открывать Visual Studio IDE не нужно.

## Сборка

В PowerShell из корня репозитория выполните:

```powershell
cmake --preset x64-debug
cmake --build --preset x64-debug
```

В проекте также определён preset `x64-release`:

```powershell
cmake --preset x64-release
cmake --build --preset x64-release
```

Сборка и запуск на Linux в режиме Debug `linux-debug`

``` Linux CMD
cmake --preset linux-debug
cmake --build --preset linux-debug
```

Сборка и запуск на Linux в режиме Release `linux-release`

``` Linux CMD
cmake --preset linux-release
cmake --build --preset linux-release
```

Сборка создаётся в `out/build/x64-debug` или `out/build/x64-release`. Команда сборки по умолчанию собирает тесты и пример.

## Запуск тестов

Запустить все тесты через CTest:

```powershell
ctest --test-dir out/build/x64-debug --output-on-failure
```
или
``` Linux CMD
ctest --test-dir out/build/linux-debug -V
```

Чтобы запустить один тест GoogleTest, можно передать фильтр исполняемому файлу напрямую. Например:

```powershell
.\out\build\x64-debug\tests\deque_tests.exe --gtest_filter="MyDequeConstructorTest.ClearMovedFromDeque"
```

Для запуска всей тестовой программы напрямую используйте:

```powershell
.\out\build\x64-debug\tests\deque_tests.exe
```
или
``` Linux CMD
./out/build/linux-debug/examples/deque_example
```

## Пример из `examples`

Сборка создаёт цель `deque_example`. Запустите её так:

```powershell
cmake --build --preset x64-debug --target deque_example
.\out\build\x64-debug\examples\deque_example.exe
```

Собираемая цель запускает [`examples/quick_start.cpp`](examples/quick_start.cpp) — минимальный пример операций с деком через публичный API. В файле [`examples/example_usage.cpp`](examples/example_usage.cpp) также сохранены более старые демонстрации буфера, итераторов, алгоритмов и копирования/перемещения. Они исследуют внутреннее состояние контейнера и сейчас не входят в собираемую цель.

## Минимальный пример использования

```cpp
#include <iostream>
#include <myDeque/my_deque.hpp>

int main() {
	Deque::MyDeque<int> values;

	values.push_back(20);
	values.push_front(10);
	values.push_back(30);

	for (const int value : values) {
		std::cout << value << ' ';
	}
}
```

### Пример пакетной вставки по итераторам

```cpp
#include <vector>
#include <list>
#include <myDeque/my_deque.hpp>

int main() {
    // Настраиваем размер чанка = 16 элементов
    Deque::MyDeque<int, std::allocator<int>, 16> dq;

    std::vector<int> tail = {10, 20, 30, 40};
    std::list<int> head = {1, 2, 3};

    // Пакетная вставка диапазонов в конец и в начало за один вызов
    dq.push_back(tail.begin(), tail.end());
    dq.push_front(head.begin(), head.end());
}
```

Для собственного CMake-проекта добавьте каталог `include` в пути заголовков и используйте стандарт C++20:

```cmake
target_include_directories(my_app PRIVATE path/to/myDeque/include)
target_compile_features(my_app PRIVATE cxx_std_20)
```

Контейнер header-only, поэтому отдельную библиотеку для линковки собирать не требуется.
