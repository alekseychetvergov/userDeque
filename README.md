# myDeque

`myDeque` — реализация двусторонней очереди на C++20. Основной контейнер — шаблон `Deque::MyDeque<T, Allocator, CHUNK_SIZE>`; заголовки проекта находятся в `include/`.

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

Сборка создаётся в `out/build/x64-debug` или `out/build/x64-release`. Команда сборки по умолчанию собирает тесты и пример.

## Запуск тестов

Запустить все тесты через CTest:

```powershell
ctest --test-dir out/build/x64-debug --output-on-failure
```

Чтобы запустить один тест GoogleTest, можно передать фильтр исполняемому файлу напрямую. Например:

```powershell
.\out\build\x64-debug\tests\deque_tests.exe --gtest_filter="MyDequeConstructorTest.ClearMovedFromDeque"
```

Для запуска всей тестовой программы напрямую используйте:

```powershell
.\out\build\x64-debug\tests\deque_tests.exe
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

Для собственного CMake-проекта добавьте каталог `include` в пути заголовков и используйте стандарт C++20:

```cmake
target_include_directories(my_app PRIVATE path/to/myDeque/include)
target_compile_features(my_app PRIVATE cxx_std_20)
```

Контейнер header-only, поэтому отдельную библиотеку для линковки собирать не требуется.
