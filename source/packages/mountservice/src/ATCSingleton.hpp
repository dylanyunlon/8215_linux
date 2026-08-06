#ifndef ATC_SINGLETON_HPP
#define ATC_SINGLETON_HPP

#include <memory>

template<class T>
class Singleton {
public:
	static T *getInstance() {
		static T s_instance;

		return &s_instance;
	}
};

template<class T>
class SingletonPTR {
public:
	static std::shared_ptr<T> getInstance() {
		static std::shared_ptr<T> s_instancePtr(new T());

		return s_instancePtr;
	}
};

#endif /* end of ATCSingleton.hpp */