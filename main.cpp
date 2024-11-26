#include <iostream> // Подключаем библиотеку для ввода-вывода
#include <map> // Подключаем библиотеку для работы с ассоциативными массивами (словари)
#include <vector> // Подключаем библиотеку для работы с векторами

// Шаблон класса пользовательского аллокатора
template <typename T>
class CustomAllocator {
public:
    using value_type = T; // Определяем тип значения
    using pointer = T*; // псевдоним pointer для указателя на тип T
    using const_pointer = const T*; // const_pointer как указатель на константный тип T
    using reference = T&; // псевдоним для ссылки на тип T
    using const_reference = const T&; // Определяем константную ссылку на тип T
    using size_type = std::size_t; // Определяем тип для размера
    using difference_type = std::ptrdiff_t; // Определяем тип для разности указателей

    // Конструктор без аргументов
    CustomAllocator() noexcept {}

    // Конструктор копирования
    CustomAllocator(const CustomAllocator& other) noexcept {}

    // Оператор присваивания
    CustomAllocator& operator=(const CustomAllocator& other) noexcept {
        return *this; // Возвращаем текущий объект
    }

    // Выделение памяти
    pointer allocate(size_type n) {
        if (n == 0) { // Если запрашивается 0 байт, возвращаем nullptr
            return nullptr;
        }
        // Используем `new` для выделения памяти.
        return static_cast<pointer>(operator new(n * sizeof(T))); // Возвращаем указатель на выделенную память
    }

    // Освобождение памяти
    void deallocate(pointer p, size_type n) {
        operator delete(p); // Освобождаем память по указанному указателю
    }

    // Макрос `std::allocator_traits` для проверки соответствия требованиям стандартного аллокатора.
    template <typename U>
    struct rebind {
        using other = CustomAllocator<U>; // Позволяет использовать аллокатор для другого типа U
    };

    void construct(pointer p, const T& value) {
        new (p) T(value); // Вызов placement new для конструирования объекта в выделенной памяти
    }

    void destroy(pointer p) {
        p->~T(); // Вызов деструктора объекта по указанному указателю
    }
};

// Шаблонный класс контейнера
template <typename T, typename Allocator = std::allocator<T>>
class MyContainer {
public:
    using value_type = T; // Определяем тип значения контейнера
    using allocator_type = Allocator; // Определяем тип аллокатора
    using size_type = std::size_t; // Определяем тип для размера
    using iterator = T*; // Определяем итератор как указатель на тип T
    using const_iterator = const T*; // Определяем константный итератор как константный указатель на тип T

private:
    Allocator alloc; // Экземпляр аллокатора
    T* data; // Указатель на данные контейнера
    size_type size_; // Текущий размер контейнера

public:
    // Конструктор без аргументов
    MyContainer(const Allocator& alloc = Allocator()) : alloc(alloc), data(nullptr), size_(0) {}

    // Конструктор копирования
    MyContainer(const MyContainer& other) : alloc(other.alloc), size_(other.size_) {
        data = alloc.allocate(size_); // Выделяем память для данных контейнера
        for (size_type i = 0; i < size_; ++i) { 
            alloc.construct(data + i, other.data[i]); // Копируем элементы из другого контейнера
        }
    }

    // Оператор присваивания
    MyContainer& operator=(const MyContainer& other) {
        if (this != &other) { // Проверка на самоприсваивание
            for (size_type i = 0; i < size_; ++i) { 
                alloc.destroy(data + i); // Уничтожаем старые элементы
            }
            alloc.deallocate(data, size_); // Освобождаем старую память

            alloc = other.alloc; // Копируем аллокатор из другого контейнера
            size_ = other.size_; // Копируем размер из другого контейнера
            data = alloc.allocate(size_); // Выделяем новую память для данных контейнера
            
            for (size_type i = 0; i < size_; ++i) { 
                alloc.construct(data + i, other.data[i]); // Копируем элементы из другого контейнера в новый массив данных
            }
        }
        return *this; // Возвращаем текущий объект контейнера
    }

    // Деструктор
    ~MyContainer() {
        for (size_type i = 0; i < size_; ++i) { 
            alloc.destroy(data + i); // Уничтожаем все элементы в контейнере
        }
        alloc.deallocate(data, size_); // Освобождаем память, занимаемую данными контейнера
    }

    // Добавление элемента в конец контейнера
    void push_back(const T& value) {
        T* newData = alloc.allocate(size_ + 1); // Выделяем память для нового элемента
        
        for (size_type i = 0; i < size_; ++i) { 
            alloc.construct(newData + i, data[i]); // Копируем старые элементы в новый массив данных 
        }
        
        alloc.construct(newData + size_, value); // Конструируем новый элемент в конце нового массива данных
        
        alloc.deallocate(data, size_); // Освобождаем старую память
        
        data = newData; // Обновляем указатель на данные контейнера 
        size_++; // Увеличиваем размер контейнера на 1 
    }

    // Метод для вывода элементов контейнера на экран 
    void display() const {
        for (size_type i = 0; i < size_; ++i) { 
            std::cout << data[i] << " ";  // Выводим каждый элемент массива данных 
        }
        std::cout << std::endl;  // Переход на новую строку после вывода всех элементов 
    }

    // Доступ к элементам по индексу 
    T& operator[](size_type index) {
        return data[index];  // Возвращаем элемент по указанному индексу 
    }

  	// const-доступ к элементам по индексу 
  	const T& operator[](size_type index) const {
      	return data[index]; 	// Возвращаем константный элемент по указанному индексу 
  	}

  	// Получение размера контейнера 
  	size_type size() const {
      	return size_; 	// Возвращаем текущий размер контейнера 
  	}
};

// Функция для вычисления факториала числа n 
int factorial(int n) {
  	if (n == 0) { 
      	return 1; 	// Факториал нуля равен единице 
  	}
  	return n * factorial(n - 1); 	// Рекурсивно вычисляем факториал 
}

int main() {
  	// Используем стандартный аллокатор для создания стандартной карты (словаря)
  	std::map<int, int> standardMap;
  
  	// Используем собственный аллокатор для создания карты (словаря)
  	using CustomMap = std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>>>;
  	CustomMap customMap;

  	// Заполняем стандартный map факториалами от 0 до 9 
  	for (int i = 0; i < 10; ++i) { 
      	standardMap[i] = factorial(i); 
  	}

  	std::cout << "Standard map values:" << std::endl;
  
  	// Выводим содержимое стандартного словаря на экран 
  	for (const auto& pair : standardMap) { 
      	std::cout << pair.first << " " << pair.second << std::endl;
  	}

  	// Создаем стандартный контейнер с использованием стандартного аллокатора 
  	MyContainer<int> myContainer;

  	// Заполняем стандартный контейнер числами от 0 до 9 
  	for (int i = 0; i < 10; ++i) { 
      	myContainer.push_back(i); 
  	}

  	// Создаем собственный контейнер с собственным аллокатором 
  	MyContainer<int, CustomAllocator<int>> myCustomContainer;

  	// Заполняем собственный контейнер числами от 0 до 9  
  	for (int i = 0; i < 10; ++i) { 
      	myCustomContainer.push_back(i); 
  	}

  	// Выводим содержимое стандартного контейнера на экран  
  	std::cout << "My container values:" << std::endl;
  	myContainer.display(); 

  	// Выводим содержимое собственного контейнера с элементами  
  	std::cout << "My custom container with 10 elements:" << std::endl;
  	myCustomContainer.display(); 

  	// Создаем собственный словарь с собственным аллокатором  
  	std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>>> myCustomDictionary;

  	// Заполняем словарь факториалами от 0 до 13  
  	for (int i = 0; i < 14; ++i) { 
      	myCustomDictionary[i] = factorial(i); 
  	}

  	// Выводим содержимое словаря на экран  
  	std::cout << "My custom dictionary with 14 elements (factorials):" << std::endl;
  
  	for (const auto& pair : myCustomDictionary) { 
      	std::cout << pair.first << " " << pair.second << std::endl;
  	}

	return 0; // Завершаем программу с кодом возврата 0 (успешно)
}