#pragma once

template<typename T>
class const_optional_ref {
public:
	const_optional_ref() : ptr(nullptr) {} //invalid constructor
	const_optional_ref(const T& ref) : ptr(&obj) {} //valid constructor
	const_optional_ref(const T* ptr) : ptr(ptr) {} //conversion constructor

	const T& unwrap() const {
		DEBUG_ASSERT(ptr, "Invalid dereference");
		return *ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

protected:
	const T* ptr;
};

template<typename T>
class optional_ref {
public:
	friend class const_optional_ref<T>;

	optional_ref() : ptr(nullptr) {} //invalid constructor
	optional_ref(T& ref) : ptr(&ref) {} //valid constructor
	optional_ref(T* ptr) : ptr(ptr) {} //conversion constructor

	T& unwrap() {
		DEBUG_ASSERT(ptr, "Invalid dereference");
		return *ptr;
	}
	const T& unwrap() const {
		DEBUG_ASSERT(ptr, "Invalid dereference");
		return *ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

protected:
	T* ptr;
};
