#pragma once

#ifndef _MSC_VER
	#define ___ __attribute__((unused))auto&& __UNUSED__
#else
	#define ___ void __UNUSED__
#endif

template<typename INDEX, INDEX STEP = 1>
class Range {
	const INDEX beginIDX, endIDX;
public:
	class iterator {
		INDEX i;
	public:

		iterator(INDEX i) : i(i) {}

		void operator++() {
			i += STEP;
		}

		INDEX operator*() const {
			return i;
		}

		bool operator != (const iterator& other) const {
			return i != other.i;
		}
	};

	Range(INDEX beginIDX, INDEX endIDX) :
		beginIDX(beginIDX),
		endIDX(endIDX) {
		static_assert(STEP != 0, "Invalid step size");
		DEBUG_ASSERT((STEP > 0 and beginIDX <= endIDX) or (STEP < 0 and beginIDX >= endIDX), "Malformed iterator");
	}

	const iterator begin() const {
		return iterator(beginIDX);
	}

	const iterator end() const {
		return iterator(endIDX);
	}
};

template < typename INDEX, INDEX STEP = 1 >
Range<INDEX, STEP> range(INDEX begin, INDEX end) {
	return Range<INDEX, STEP>(begin, end);
}

template < typename INDEX, INDEX STEP = 1 >
Range<INDEX, STEP> range(INDEX count) {
	return Range<INDEX, STEP>(0, count);
}

template < typename INDEX, INDEX STEP = 1 >
Range<INDEX, STEP> range_incl(INDEX begin, INDEX end) {
	return Range<INDEX, STEP>(begin, end + STEP);
}

template < typename INDEX, INDEX STEP = 1 >
Range<INDEX, STEP> range_incl(INDEX count) {
	return Range<INDEX, STEP>(0, count + STEP);
}

template< int STEP = 1>
Range<int, STEP> rangef(float begin, float end) {
	return Range<int, STEP>((int)begin, (int)end);
}

template < typename INDEX, int STEP = -1 >
Range<int, STEP> reverse_range(INDEX begin, INDEX end) {
	return range<int, STEP>(((int)end) + STEP, ((int)begin) + STEP);
}

template < typename INDEX, int STEP = -1 >
Range<int, STEP> reverse_range(INDEX count) {
	return reverse_range<INDEX, STEP>(0, count);
}


