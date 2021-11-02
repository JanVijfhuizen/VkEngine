#pragma once

template <typename T>
class Singleton final
{
public:
	[[nodiscard]] static T& Get();
	static void Set(T* instance);

private:
	static T* _instance;
};

template <typename T>
T& Singleton<T>::Get()
{
	return *_instance;
}

template <typename T>
void Singleton<T>::Set(T* instance)
{
	_instance = instance;
}

template <typename T>
T* Singleton<T>::_instance = nullptr;
