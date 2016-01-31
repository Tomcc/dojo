#pragma once
template <class T>
class reference_wrapper {
public:
	// types
	typedef T type;

	// construct/copy/destroy
	reference_wrapper(T& ref) noexcept : ptr(std::addressof(ref)) {}
	reference_wrapper() noexcept : ptr(nullptr) {}
	reference_wrapper(T&&) = delete;
	reference_wrapper(const reference_wrapper&) noexcept = default;

	// access
	operator T& () const noexcept { return *ptr; }
	T& get() const noexcept { return *ptr; } //TODO remove and use C++17's operator dot here

	operator bool() const {
		return ptr != nullptr;
	}

private:
	T* ptr;
};

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

	reference_wrapper<const T> cast() const {
		if (ptr) {
			return *ptr;
		}
		else {
			return{};
		}
	}

	bool is_some() const {
		return ptr != nullptr;
	}

	bool is_none() const {
		return ptr == nullptr;
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

	reference_wrapper<T> cast() const {
		if (ptr) {
			return *ptr;
		}
		else {
			return{};
		}
	}

	bool is_some() const {
		return ptr != nullptr;
	}

	bool is_none() const {
		return ptr == nullptr;
	}

protected:
	T* ptr;
};
