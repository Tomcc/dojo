#pragma once

#include <string>

//#define STL_DROP_IN

namespace utf {
	typedef uint32_t character;

	class string {
	public:
		static const size_t npos;

		class const_iterator {
		public:
			character& operator*();
			const_iterator& operator++();
			const_iterator operator+(size_t i)const;
			bool operator!=(const const_iterator&) const;
			bool operator==(const const_iterator&) const;
		};

		class iterator {
		public:
			iterator(const_iterator);
			character& operator*();
			iterator& operator++();
			bool operator!=(const iterator&) const;
			iterator operator+(size_t i)const;
		};

		class const_reverse_iterator {
		public:
			character& operator*();
			const_reverse_iterator& operator++();
			bool operator!=(const const_reverse_iterator&) const;
		};

		class reverse_iterator {
		public:
			reverse_iterator(const_reverse_iterator);
			character& operator*();
			reverse_iterator& operator++();
			bool operator!=(const reverse_iterator&) const;

		};


		string() {}
		string(const string& str) {
			assign(str);
		}
		string(const char* s) {
			assign(s);
		}
		string(const char* s, size_t n) {
			assign(s, n);
		}
		string(size_t n, character c) {
			assign(n, c);
		}
		template <class InputIterator>
		string(InputIterator first, InputIterator last) {
			assign(first, last);
		}

		string(std::initializer_list<character> il) {
			assign(std::move(il));
		}

		string(string&& str) noexcept {
			assign(std::move(str));
		}

		string(std::string&& str) noexcept : raw(std::move(str)) {}
		
		string& operator= (const string& str) {
			return assign(str);
		}
		string& operator= (const char* s) {
			return assign(s);
		}
		string& operator= (character c) {
			return assign(c);
		}
		string& operator= (std::initializer_list<character> il) {
			return assign(std::move(il));
		}
		string& operator= (string&& str) noexcept {
			return assign(std::move(str));
		}

		size_t size() const;
		size_t length() const;

		const std::string& bytes() const {
			return raw;
		}
		std::string& bytes() {
			return raw;
		}

		iterator begin() noexcept;
		iterator end() noexcept;


		const_iterator begin() const noexcept;
		const_iterator end() const noexcept;
		const_iterator cbegin() const noexcept {
			return begin();
		}
		const_iterator cend() const noexcept {
			return end();
		}

		size_t max_size() const noexcept {
			return raw.max_size();
		}

		void resize(size_t n);
		void resize(size_t n, character c);

		size_t capacity() const noexcept {
			return raw.capacity();
		}
		
		void clear() noexcept {
			raw.clear();
		}

		bool empty() const noexcept {
			return raw.empty(); 
		}

		void shrink_to_fit() {
			raw.shrink_to_fit();
		}

		character& front() {
			return *begin();
		}
		const character& front() const {
			return *begin();
		}
		
		string& operator+= (const string& str) {
			return append(str);
		}
		string& operator+= (const char* s) {
			return append(s);
		}
		string& operator+= (character c) {
			return append(c);
		}
		string& operator+= (std::initializer_list<character> il) {
			return append(std::move(il));
		}

		string& append(const string& str);
		string& append(const string& str, size_t subpos, size_t sublen = npos);
		string& append(const char* s) {
			raw.append(s);
			return *this;
		}
		string& append(const char* s, size_t n) {
			raw.append(s, n);
			return *this;
		}
		string& append(size_t n, character c);
		template <class InputIterator>
		string& append(InputIterator first, InputIterator last);
		string& append(std::initializer_list<character> il);
		string& append(character c);

		void push_back(character c);

		string& assign(const string& str);
		string& assign(const string& str, size_t subpos, size_t sublen);
		string& assign(const char* s) {
			raw.assign(s);
			return *this;
		}
		string& assign(const char* s, size_t n) {
			raw.assign(s, n);
			return *this;
		}
		string& assign(size_t n, character c);
		template <class InputIterator>
		string& assign(InputIterator first, InputIterator last);
		string& assign(std::initializer_list<character> il);
		string& assign(string&& str) noexcept;
		string& assign(character);

		iterator insert(const_iterator p, size_t n, character c);
		iterator insert(const_iterator p, character c);
		template <class InputIterator>
		iterator insert(iterator p, InputIterator first, InputIterator last);
		string& insert(const_iterator p, std::initializer_list<character> il);

		iterator erase(const_iterator p);
		iterator erase(const_iterator first, const_iterator last);

		string& replace(const_iterator i1, const_iterator i2, const string& str);
		string& replace(const_iterator i1, const_iterator i2, const char* s);
		string& replace(const_iterator i1, const_iterator i2, const char* s, size_t n);
		string& replace(const_iterator i1, const_iterator i2, size_t n, character c);
		template <class InputIterator>
		string& replace(const_iterator i1, const_iterator i2, InputIterator first, InputIterator last);
		string& replace(const_iterator i1, const_iterator i2, std::initializer_list<character> il);
		
		void swap(string& str) {
			std::swap(*this, str);
		}

		int compare(const string& str) const noexcept;
		
		const_iterator find(const string& str, size_t pos = 0) const noexcept;
		const_iterator find(const char* s, size_t pos = 0) const;
		const_iterator find(const char* s, size_t pos, size_t n) const;
		const_iterator find(character c, size_t pos = 0) const noexcept;

		const_iterator find_last_not_of(const string& str, size_t pos = npos) const noexcept;
		const_iterator find_last_not_of(const char* s, size_t pos = npos) const;
		const_iterator find_last_not_of(const char* s, size_t pos, size_t n) const;
		const_iterator find_last_not_of(character c, size_t pos = npos) const noexcept;
		
		const_iterator find_first_not_of(const string& str, size_t pos = 0) const noexcept;
		const_iterator find_first_not_of(const char* s, size_t pos = 0) const;
		const_iterator find_first_not_of(const char* s, size_t pos, size_t n) const;
		const_iterator find_first_not_of(character c, size_t pos = 0) const noexcept;

		const_iterator find_last_of(const string& str, size_t pos = npos) const noexcept;
		const_iterator find_last_of(const char* s, size_t pos = npos) const;
		const_iterator find_last_of(const char* s, size_t pos, size_t n) const;
		const_iterator find_last_of(character c, size_t pos = npos) const noexcept;

		const_iterator find_first_of(const string& str, size_t pos = 0) const noexcept;
		const_iterator find_first_of(const char* s, size_t pos = 0) const;
		const_iterator find_first_of(const char* s, size_t pos, size_t n) const;
		const_iterator find_first_of(character c, size_t pos = 0) const noexcept;
		
		const_iterator rfind(const string& str, size_t pos = npos) const noexcept;
		const_iterator rfind(const char* s, size_t pos = npos) const;
		const_iterator rfind(const char* s, size_t pos, size_t n) const;
		const_iterator rfind(char c, size_t pos = npos) const noexcept;

		std::string::allocator_type get_allocator() const noexcept {
			return raw.get_allocator();
		}
		
		size_t hash_code() const {
			return std::hash<std::string>()(raw);
		}

		string substr(const_iterator start, const_iterator end) const {
			return string(start, end);
		}

#ifdef STL_DROP_IN
		string(const string& str, size_t pos, size_t len = npos);

		//all these go from O(1) to O(n), don't use if possible
		reverse_iterator rbegin() noexcept;
		reverse_iterator rend() noexcept;

		const_reverse_iterator rbegin() const noexcept;
		const_reverse_iterator rend() const noexcept;
		const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}
		const_reverse_iterator crend() const noexcept {
			return rend();
		}

		character& operator[] (size_t pos);
		const character& operator[] (size_t pos) const;

		character& at(size_t pos);
		const character& at(size_t pos) const;

		character& back() {
			return *rbegin();
		}
		const character& back() const {
			return *rbegin();
		}
		
		string& insert(size_t pos, const string& str);
		string& insert(size_t pos, const string& str, size_t subpos, size_t sublen = npos);
		string& insert(size_t pos, const char* s);
		string& insert(size_t pos, const char* s, size_t n);
		string& insert(size_t pos, size_t n, character c);

		string& erase(size_t pos = 0, size_t len = npos);

		string& replace(size_t pos, size_t len, const string& str);
		string& replace(size_t pos, size_t len, const string& str, size_t subpos, size_t sublen = npos);
		string& replace(size_t pos, size_t len, const char* s);
		string& replace(size_t pos, size_t len, const char* s, size_t n);
		string& replace(size_t pos, size_t len, size_t n, character c);

		void pop_back();

		int compare(size_t pos, size_t len, const string& str) const;
		int compare(size_t pos, size_t len, const string& str,
			size_t subpos, size_t sublen = npos) const;
		int compare(const char* s) const;
		int compare(size_t pos, size_t len, const char* s) const;
		int compare(size_t pos, size_t len, const char* s, size_t n) const;

		string substr(size_t pos = 0, size_t len = npos) const;
#endif

	private:
		std::string raw;
	};

	bool operator== (const string& lhs, const string& rhs) noexcept {
		return lhs.compare(rhs) == 0;
	}
	
	bool operator!= (const string& lhs, const string& rhs) noexcept {
		return !(lhs == rhs);
	}
	
	bool operator<  (const string& lhs, const string& rhs) noexcept {
		return lhs.compare(rhs) < 0;
	}

	bool operator>  (const string& lhs, const string& rhs) noexcept {
		return lhs.compare(rhs) > 0;
	}

	bool operator<= (const string& lhs, const string& rhs) noexcept {
		return !(lhs > rhs);
	}

	bool operator>= (const string& lhs, const string& rhs) noexcept {
		return !(lhs < rhs);
	}

	string operator+ (const string& lhs, const string& rhs);
	string operator+ (string&&      lhs, string&&      rhs);
	string operator+ (string&&      lhs, const string& rhs);
	string operator+ (const string& lhs, string&&      rhs);
	string operator+ (const string& lhs, const char*   rhs);
	string operator+ (string&&      lhs, const char*   rhs);
	string operator+ (const char*   lhs, const string& rhs);
	string operator+ (const char*   lhs, string&&      rhs);
	string operator+ (const string& lhs, char          rhs);
	string operator+ (string&&      lhs, char          rhs);
	string operator+ (char          lhs, const string& rhs);
	string operator+ (char          lhs, string&&      rhs);

#ifdef STL_DROP_IN
	bool operator== (const char*   lhs, const string& rhs);
	bool operator== (const string& lhs, const char*   rhs);
	bool operator!= (const char*   lhs, const string& rhs);
	bool operator!= (const string& lhs, const char*   rhs);
	bool operator<  (const char*   lhs, const string& rhs);
	bool operator<  (const string& lhs, const char*   rhs);
	bool operator<= (const char*   lhs, const string& rhs);
	bool operator<= (const string& lhs, const char*   rhs);
	bool operator>  (const char*   lhs, const string& rhs);
	bool operator>  (const string& lhs, const char*   rhs);
	bool operator>= (const char*   lhs, const string& rhs);
	bool operator>= (const string& lhs, const char*   rhs);
#endif

	template<typename T>
	string to_string(T t) {
		return std::to_string(t);
	}
}

namespace std {
	///hash specialization for unordered_maps
	template <>
	struct hash<utf::string> {
		// hash functor for vector
		size_t operator()(const utf::string& _Keyval) const {
			return _Keyval.hash_code();
		}
	};
}

