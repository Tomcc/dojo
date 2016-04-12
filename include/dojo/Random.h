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
#include "Vector.h"

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
		uint32_t* pget; // get value to get from state
		int left; // number of values left before reload needed

	public:
		static Random instance;

		//create a "small" 64 bit seed from the OS random number source
		static RandomSeed makeRandomSeed();

		//creates a mersenne twister seed from the OS random number source
		static BigSeed makeBigRandomSeed();

		///class Random initialized with std::random_device
		Random();

		///Creates a new Random generator using a seed
		explicit Random(RandomSeed oneSeed);

		///Creates a new Random generator using a big seed in an array
		explicit Random(const BigSeed& seed);

		Random(const Random& o);

		// Do NOT use for CRYPTOGRAPHY without securely hashing several returned
		// values together, otherwise the generator state can be learned after
		// reading 624 consecutive values.

		// Access to 32-bit random numbers

		/// integer in [0,2^32-1]
		uint32_t getInt();
		/// integer in [0,n] for n < 2^32
		uint32_t getInt(const uint32_t n);

		int getInt(int min, int max);

		int64_t getInt64() {
			auto low = getInt();
			return ((int64_t)getInt() << 32) | low;
		}

		/// real number in [0,1]
		float getFloat();

		/// real number in [0,1]
		double getDouble();

		/// either 1 or -1 with equal probability
		float getSign();

		int getIntSign() {
			return (getInt() % 2) ? 1 : -1;
		}

		/// real number in [0,n]
		float getFloat(const float n);

		///real number in [min, max]
		float getFloat(float min, float max) {
			return min + getFloat(max - min);
		}

		/// real number in [0,1)
		float getFloatExc();
		/// real number in [0,n)
		float getFloatExc(const float n);
		/// real number in (0,1)
		float getFloatFullExcl();
		/// real number in (0,n)
		float getFloatFullExcl(const float n);

		///return a random point in the cube between min and max
		Vector getPoint(const Vector& min, const Vector& max);

		///return a random point in the square between min and max, with a given z
		Vector get2DPoint(const Vector& min, const Vector& max, float z = 0);

		///return a random 2D unit vector on a circle
		Vector get2DUnitVector();

		bool oneEvery(int n) {
			return getInt(n) == 0;
		}

		///pick one element index from the vector
		template<class CTR>
		typename CTR::const_iterator pickIteratorFrom(const CTR& c) {
			return c.begin() + getInt(c.size() - 1);
		}

		///pick one element from the vector
		template<class CTR>
		const auto& pickFrom(const CTR& c) {
			return *pickIteratorFrom(c);
		}

		/// Access to 53-bit random numbers (capacity of IEEE float precision)
		double rand53();

		/// Access to nonuniform random number distributions
		double randNorm(const double mean = 0.0, const double stddev = 1.0);

		/// Re-seeding functions with same behavior as initializers
		void seed(RandomSeed s);
		void seed(const BigSeed& seed);
		void seed();

		/// Saving and loading generator state
		void save(uint32_t* saveArray) const; // to array of size SAVE
		void load(uint32_t* const loadArray); // from such array

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
}
