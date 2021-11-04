#pragma once
#include <cstdint>

template <typename T>
class Pool
{
public:
	virtual void Construct(uint32_t size);
	virtual void Cleanup();

	[[nodiscard]] virtual T Get();
	virtual void Add(T& instance);

	[[nodiscard]] uint32_t GetSize() const;

private:
	T* _values = nullptr;
	uint32_t _count = 0;
	uint32_t _size = 0;
};

template <typename T>
void Pool<T>::Construct(uint32_t size)
{
	_size = size;
	_values = new T[size];
}

template <typename T>
void Pool<T>::Cleanup()
{
	delete[] _values;
}

template <typename T>
T Pool<T>::Get()
{
	assert(_count > 0);
	return _values[_count--];
}

template <typename T>
void Pool<T>::Add(T& instance)
{
	assert(_count < _size);
	_values[_count++] = instance;
}

template <typename T>
uint32_t Pool<T>::GetSize() const
{
	return _size;
}
