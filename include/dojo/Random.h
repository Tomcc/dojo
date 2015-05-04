// Mersenne Twister random number generator -- a C++ class MTRand
// Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
// Richard J. Wagner  v1.1  28 September 2009  wagnerr@umich.edu

// The Mersenne Twister is an algorithm for generating random numbers.  It
// was designed with consideration of the flaws in various other generators.
// The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
// are far greater.  The generator is also fast; it avoids multiplication and
// division, and it benefits from caches and pipelines.  For more information
// see the inventors' web page at
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html

// Reference
// M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
// Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
// Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

// Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
// Copyright (C) 2000 - 2009, Richard J. Wagner
// All rights reserved.

#pragma once

#include "dojo_common_header.h"

#include "Timer.h"

#include <array>

namespace Dojo {
	///A Random implementation using Mersenne Twister	
	class Random {
		// Data
	public:
		enum { N = 624 }; // length of state vector
		enum { SAVE = N + 1 }; // length of array for save()

		typedef std::array<uint32_t, N> BigSeed;

	protected:
		enum { M = 397 }; // period parameter

		uint32_t state[N]; // internal state
		uint32_t* pNext; // next value to get from state
		int left; // number of values left before reload needed

	public:

		///class Random initialized with time()
		Random();

		///Creates a new Random generator using a seed
		Random(RandomSeed oneSeed);

		///Creates a new Random generator using a big seed in an array
		Random(const BigSeed& seed);

		Random(const Random& o);

		// Do NOT use for CRYPTOGRAPHY without securely hashing several returned
		// values together, otherwise the generator state can be learned after
		// reading 624 consecutive values.

		// Access to 32-bit random numbers

		/// integer in [0,2^32-1]
		uint32_t nextInt();
		/// integer in [0,n] for n < 2^32
		uint32_t nextInt(const uint32_t n);

		int nextInt(int min, int max) {
			return (int)(nextDouble() * (max - min)) + min;
		}

		/// real number in [0,1]
		double nextDouble();
		/// real number in [0,n]
		double nextDouble(const double n);

		///real number in [min, max]
		double nextDouble(double min, double max) {
			return min + nextDouble(max - min);
		}

		/// real number in [0,1)
		double nextDoubleExc();
		/// real number in [0,n)
		double nextDoubleExc(const double n);
		/// real number in (0,1)
		double nextDoubleFullExcl();
		/// real number in (0,n)
		double nextDoubleFullExcl(const double n);
		/// same as rand()
		double operator()();

		bool oneEvery(int n) {
			return nextInt(n) == 0;
		}

		/// Access to 53-bit random numbers (capacity of IEEE double precision)
		double rand53();

		/// Access to nonuniform random number distributions
		double randNorm(const double mean = 0.0, const double stddev = 1.0);

		/// Re-seeding functions with same behavior as initializers
		void seed(RandomSeed s);
		void seed(const BigSeed& seed);
		void seed();

		/// Saving and loading generator state
		void save(uint32_t* saveArray) const; // to array of size SAVE
		void load(uint32_t*const loadArray); // from such array
		friend std::ostream& operator<<(std::ostream& os, const Random& Random);
		friend std::istream& operator>>(std::istream& is, Random& Random);
		Random& operator=(const Random& o);

	protected:
		void initialize(RandomSeed oneSeed);
		void reload();

		uint32_t hiBit(const uint32_t u) const {
			return u & 0x80000000UL;
		}

		uint32_t loBit(const uint32_t u) const {
			return u & 0x00000001UL;
		}

		uint32_t loBits(const uint32_t u) const {
			return u & 0x7fffffffUL;
		}

		uint32_t mixBits(const uint32_t u, const uint32_t v) const {
			return hiBit(u) | loBits(v);
		}

		uint32_t magic(const uint32_t u) const {
			return loBit(u) ? 0x9908b0dfUL : 0x0UL;
		}

		uint32_t twist(const uint32_t m, const uint32_t s0, const uint32_t s1) const {
			return m ^ (mixBits(s0, s1) >> 1) ^ magic(s1);
		}

		static uint32_t hash(time_t t, clock_t c);
	};

	// Functions are defined in order of usage to assist inlining

	inline uint32_t Random::hash(time_t t, clock_t c) {
		// Get a uint32_t from t and c
		// Better than uint32_t(x) in case x is floating point in [0,1]
		// Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

		static uint32_t differ = 0; // guarantee time-based seeds will change

		uint32_t h1 = 0;
		unsigned char* p = (unsigned char *) &t;
		for (size_t i = 0; i < sizeof(t); ++i) {
			h1 *= UCHAR_MAX + 2U;
			h1 += p[i];
		}
		uint32_t h2 = 0;
		p = (unsigned char *) &c;
		for (size_t j = 0; j < sizeof(c); ++j) {
			h2 *= UCHAR_MAX + 2U;
			h2 += p[j];
		}
		return (h1 + differ++) ^ h2;
	}

	inline void Random::initialize(RandomSeed seed) {
		// Initialize generator state with seed
		// See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
		// In previous versions, most significant bits (MSBs) of the seed affect
		// only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto.
		register uint32_t* s = state;
		register uint32_t* r = state;
		register int i = 1;
		*s++ = (uint32_t)seed & 0xffffffffUL;
		for (; i < N; ++i) {
			*s++ = (1812433253UL * (*r ^ (*r >> 30)) + i) & 0xffffffffUL;
			r++;
		}
	}

	inline void Random::reload() {
		// Generate N new values in state
		// Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
		static const int MmN = int(M) - int(N); // in case enums are unsigned
		register uint32_t* p = state;
		register int i;
		for (i = N - M; i--; ++p)
			*p = twist(p[M], p[0], p[1]);
		for (i = M; --i; ++p)
			*p = twist(p[MmN], p[0], p[1]);
		*p = twist(p[MmN], p[0], state[0]);

		left = N , pNext = state;
	}

	inline void Random::seed(RandomSeed s) {
		// RandomSeed the generator with a simple uint32_t
		initialize(s);
		reload();
	}

	inline void Random::seed(const BigSeed& seed) {
		DEBUG_ASSERT(seed.size() > 0, "Invalid seed");

		// RandomSeed the generator with an array of uint32_t's
		// There are 2^19937-1 possible initial states.  This function allows
		// all of those to be accessed by providing at least 19937 bits (with a
		// default seed length of N = 624 uint32_t's).  Any bits above the lower 32
		// in each element are discarded.
		// Just call seed() if you want to get array from /dev/urandom
		initialize(19650218UL);
		register int i = 1;
		register uint32_t j = 0;
		register int k = (N > seed.size() ? N : seed.size());
		for (; k; --k) {
			state[i] =
				state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1664525UL);
			state[i] += (seed[j] & 0xffffffffUL) + j;
			state[i] &= 0xffffffffUL;
			++i;
			++j;
			if (i >= N) {
				state[0] = state[N - 1];
				i = 1;
			}
			if (j >= seed.size())
				j = 0;
		}
		for (k = N - 1; k; --k) {
			state[i] =
				state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1566083941UL);
			state[i] -= i;
			state[i] &= 0xffffffffUL;
			++i;
			if (i >= N) {
				state[0] = state[N - 1];
				i = 1;
			}
		}
		state[0] = 0x80000000UL; // MSB is 1, assuring non-zero initial array
		reload();
	}

	inline Random::Random() {
		//use an high-precision timer to grab microseconds
		Timer t;
		seed((RandomSeed)(t.currentTime() * 1000000));
	}

	inline Random::Random(RandomSeed oneSeed) {
		seed(oneSeed);
	}

	inline Random::Random(const BigSeed& bigSeed) {
		seed(bigSeed);
	}

	inline Random::Random(const Random& o) {
		register const uint32_t* t = o.state;
		register uint32_t* s = state;
		register int i = N;
		for (; i--; *s++ = *t++) {
		}
		left = o.left;
		pNext = &state[N - left];
	}

	inline uint32_t Random::nextInt() {
		// Pull a 32-bit integer from the generator state
		// Every other access function simply transforms the numbers extracted here

		if (left == 0)
			reload();
		--left;

		register uint32_t s1;
		s1 = *pNext++;
		s1 ^= (s1 >> 11);
		s1 ^= (s1 << 7) & 0x9d2c5680UL;
		s1 ^= (s1 << 15) & 0xefc60000UL;
		return (s1 ^ (s1 >> 18));
	}

	inline uint32_t Random::nextInt(const uint32_t n) {
		/*
		// Find which bits are used in n
		// Optimized by Magnus Jonsson (magnus@smartelectronix.com)
		uint32_t used = n;
		used |= used >> 1;
		used |= used >> 2;
		used |= used >> 4;
		used |= used >> 8;
		used |= used >> 16;

		// Draw numbers until one is found in [0,n]
		uint32_t i;
		do
			i = randInt() & used;  // toss unused bits to shorten search
		while( i > n );
		return i;*/

		//HACK-  much faster while property-changing
		return (uint32_t)nextDouble((double)n);
	}

	inline double Random::nextDouble() {
		return double(nextInt()) * (1.0 / 4294967295.0);
	}

	inline double Random::nextDouble(const double n) {
		return nextDouble() * n;
	}

	inline double Random::nextDoubleExc() {
		return double(nextInt()) * (1.0 / 4294967296.0);
	}

	inline double Random::nextDoubleExc(const double n) {
		return nextDoubleExc() * n;
	}

	inline double Random::nextDoubleFullExcl() {
		return (double(nextInt()) + 0.5) * (1.0 / 4294967296.0);
	}

	inline double Random::nextDoubleFullExcl(const double n) {
		return nextDoubleFullExcl() * n;
	}

	inline double Random::rand53() {
		uint32_t a = nextInt() >> 5, b = nextInt() >> 6;
		return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0); // by Isaku Wada
	}

	inline double Random::randNorm(const double mean, const double stddev) {
		// Return a real number from a normal (Gaussian) distribution with given
		// mean and standard deviation by polar form of Box-Muller transformation
		double x, y, r;
		do {
			x = 2.0 * nextDouble() - 1.0;
			y = 2.0 * nextDouble() - 1.0;
			r = x * x + y * y;
		}
		while (r >= 1.0 || r == 0.0);
		double s = sqrt(-2.0 * log(r) / r);
		return mean + x * s * stddev;
	}

	inline double Random::operator()() {
		return nextDouble();
	}

	inline void Random::save(uint32_t* saveArray) const {
		register const uint32_t* s = state;
		register uint32_t* sa = saveArray;
		register int i = N;
		for (; i--; *sa++ = *s++) {
		}
		*sa = left;
	}

	inline void Random::load(uint32_t*const loadArray) {
		register uint32_t* s = state;
		register uint32_t* la = loadArray;
		register int i = N;
		for (; i--; *s++ = *la++) {
		}
		left = *la;
		pNext = &state[N - left];
	}

	inline std::ostream& operator<<(std::ostream& os, const Random& r) {
		register const uint32_t* s = r.state;
		register int i = r.N;
		for (; i--; os << *s++ << "\t") {
		}
		return os << r.left;
	}

	inline std::istream& operator>>(std::istream& is, Random& r) {
		register uint32_t* s = r.state;
		register int i = r.N;
		for (; i--; is >> *s++) {
		}
		is >> r.left;
		r.pNext = &r.state[r.N - r.left];
		return is;
	}

	inline Random& Random::operator=(const Random& o) {
		if (this == &o)
			return (*this);
		register const uint32_t* t = o.state;
		register uint32_t* s = state;
		register int i = N;
		for (; i--; *s++ = *t++) {
		}
		left = o.left;
		pNext = &state[N - left];
		return (*this);
	}
}
