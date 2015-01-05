#pragma once

#include <vector>
#include <memory>

namespace Dojo {
	template< class T >
	class SmallSet
	{
	public:

		typedef std::vector<T> Container;
		typedef typename Container::iterator iterator;
		typedef typename Container::const_iterator const_iterator;

		template< class E >
		static typename std::vector<E>::iterator find(SmallSet<E>& c, const E& elem) {
			auto itr = c.begin();
			for (; itr != c.end(); ++itr) {
				if (*itr == elem)
					return itr;
			}
			return c.end();
		}

		template< class E >
		static typename std::vector< std::unique_ptr<E> >::iterator find(SmallSet< std::unique_ptr<E> >& c, const E& elem) {
			auto itr = c.begin();
			for (; itr != c.end() && itr->get() != &elem; ++itr);
			return itr;
		}

		SmallSet(int initialSize = 0) :
			c(initialSize) {

		}

		iterator find(const T& elem) {
			return find<T>(*this, elem);
		}

		bool contains(const T& elem) const {
			return find(elem) == c.end();
		}

		template <class... Args>
		void emplace(Args&&... args) {
			c.emplace_back(std::forward<Args>(args)...);
		}

		iterator erase(const iterator& where) {
			*where = std::move(c.back());
			c.pop_back();

			return where; //continue exactly where we were
		}

		iterator erase(const T& elem) {
			auto itr = find(elem);
			if (itr != end())
				return erase(itr);
			else
				return end();
		}

		T& operator[](int idx) {
			return c[idx];
		}

		iterator begin() { return c.begin(); }
		const_iterator begin() const { return c.begin(); }

		iterator end() { return c.end(); }
		const_iterator end() const { return c.end(); }

		size_t size() const {
			return c.size();
		}

		bool empty() const {
			return c.empty();
		}

		void clear() {
			c.clear();
		}

	protected:

		std::vector<T> c;
	private:
	};
}