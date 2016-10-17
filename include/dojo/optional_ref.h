#pragma once

namespace Dojo {
	template <class T>
	class reference_wrapper {
	public:
		// types
		typedef T type;

		// construct/copy/destroy
		reference_wrapper(T& ref) : ptr(std::addressof(ref)) {}
		reference_wrapper() : ptr(nullptr) {}
		reference_wrapper(T&&) = delete;
		reference_wrapper(const reference_wrapper&) = default;

		// access
		operator T& () const { return *ptr; }
		T& get() const { return *ptr; } //TODO remove and use C++17's operator dot here

		operator bool() const {
			return ptr != nullptr;
		}

		size_t hash() const {
			return reinterpret_cast<size_t>(ptr);
		}

	private:
		T* ptr;
	};

	template<typename T>
	class optional_ref {
	public:
		optional_ref() : ptr(nullptr) {} //invalid constructor
		optional_ref(T& ref) : ptr(&ref) {} //valid constructor

		T& unwrap() const {
			DEBUG_ASSERT(ptr, "Invalid dereference");
			return *ptr;
		}

		T& unwrap_or(T& default_val) const  {
			return ptr ? *ptr : default_val;
		}

		reference_wrapper<T> to_ref() const {
			if (ptr) {
				return *ptr;
			}
			else {
				return{};
			}
		}

		T* to_raw_ptr() const {
			return ptr;
		}

		bool is_some() const {
			return ptr != nullptr;
		}

		bool is_none() const {
			return ptr == nullptr;
		}

		bool operator==(const T& ref) const {
			return ptr == &ref;
		}

		bool operator != (const optional_ref<T>& ref) const {
			return ptr != ref.ptr;
		}
	private:
		T* ptr;
	};
}

namespace std {
	///hash specialization for unordered_maps
	template <class T>
	struct hash<Dojo::reference_wrapper<T>> {
		// hash functor for vector
		size_t operator()(const Dojo::reference_wrapper<T>& _Keyval) const {
			return _Keyval.hash();
		}
	};
}
