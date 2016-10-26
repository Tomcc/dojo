#pragma once

#include <cassert>
#include <vector>

template<typename T>
class vec_view {
public:
	class const_iterator {
	public:
		const_iterator(const T* where)
			: mWhere(where) {
		}

		const_iterator& operator++() {
			++mWhere;
			return *this;
		}

		bool operator !=(const const_iterator& c) const {
			return c.mWhere != mWhere;
		}

		bool operator==(const const_iterator& c) const {
			return c.mWhere == mWhere;
		}

		const T& operator*() const {
			return *mWhere;
		}
	private:
		const T* mWhere;
	};


	size_t size() const {
		return mEnd - mStart;
	}

	size_t byte_size() const {
		return size() * sizeof(T);
	}

	bool empty() const {
		return mStart == mEnd;
	}

	const T* data() const {
		return mStart;
	}

	vec_view(const T* start, const T* end) :
		mStart(start),
		mEnd(end) {

		assert(mStart <= mEnd);
	}

	vec_view() : vec_view(nullptr, nullptr) {}

	vec_view(const std::vector<T>& vec) : vec_view(vec.data(), vec.data() + vec.size()) {}
	vec_view(const std::initializer_list<T>& list) : vec_view(&(*list.begin()), &(*list.end())) {}

	template<size_t SIZE>
	vec_view(const T(&array)[SIZE]) : vec_view(array, array + SIZE) {}


	const_iterator begin() const {
		return{ mStart };
	}

	const_iterator end() const {
		return{ mEnd };
	}

	const T& operator[](size_t where) const {
		assert(where < size());

		return mStart[where];
	}

	const T& back() const {
		assert(!empty());
		return *(mEnd - 1);
	}

	vec_view<T> slice(size_t start) const {
		assert(start < size());
		return{ mStart + start, mEnd };
	}

	vec_view<T> slice(size_t start, size_t end) const {
		assert(start < size());
		assert(end <= size());
		assert(start <= end);
		return{ mStart + start, mStart + end };
	}

	std::vector<T> to_owned() const {
		std::vector<T> owned(size());
		for (auto&& element : *this) {
			owned.emplace_back(element);
		}
		return owned;
	}

private:
	const T* mStart, *mEnd;
};



