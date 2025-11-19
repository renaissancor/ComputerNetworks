#pragma once

template<typename T>
class Singleton {
protected:
	Singleton() = default;
	~Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
public:
	inline static T& GetInstance() noexcept {
		static T instance;
		return instance;
	}
};

