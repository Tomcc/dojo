#include "stdafx.h"

#include "Random.h"
#include "dojomath.h"

using namespace Dojo;

Random Random::instance;

uint32_t Random::hash(time_t t, clock_t c) {
	// Get a uint32_t from t and c
	// Better than uint32_t(x) in case x is floating point in [0,1]
	// Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

	static uint32_t differ = 0; // guarantee time-based seeds will change

	uint32_t h1 = 0;
	unsigned char* p = (unsigned char*)&t;

	for (size_t i = 0; i < sizeof(t); ++i) {
		h1 *= UCHAR_MAX + 2U;
		h1 += p[i];
	}

	uint32_t h2 = 0;
	p = (unsigned char*)&c;

	for (size_t j = 0; j < sizeof(c); ++j) {
		h2 *= UCHAR_MAX + 2U;
		h2 += p[j];
	}

	return (h1 + differ++) ^ h2;
}

void Random::initialize(RandomSeed seed) {
	// Initialize generator state with seed
	// See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
	// In previous versions, most significant bits (MSBs) of the seed affect
	// only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto.
	uint32_t* s = state;
	uint32_t* r = state;
	int i = 1;
	*s++ = (uint32_t)seed & 0xffffffffUL;

	for (; i < N; ++i) {
		*s++ = (1812433253UL * (*r ^ (*r >> 30)) + i) & 0xffffffffUL;
		r++;
	}
}

void Random::reload() {
	// Generate N new values in state
	// Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
	static const int MmN = int(M) - int(N); // in case enums are unsigned
	uint32_t* p = state;
	int i;

	for (i = N - M; i--; ++p) {
		*p = twist(p[M], p[0], p[1]);
	}

	for (i = M; --i; ++p) {
		*p = twist(p[MmN], p[0], p[1]);
	}

	*p = twist(p[MmN], p[0], state[0]);

	left = N, pget = state;
}

void Random::seed(RandomSeed s) {
	// RandomSeed the generator with a simple uint32_t
	initialize(s);
	reload();
}

void Random::seed(const BigSeed& seed) {
	DEBUG_ASSERT(seed.size() > 0, "Invalid seed");

	// RandomSeed the generator with an array of uint32_t's
	// There are 2^19937-1 possible initial states.  This function allows
	// all of those to be accessed by providing at least 19937 bits (with a
	// default seed length of N = 624 uint32_t's).  Any bits above the lower 32
	// in each element are discarded.
	// Just call seed() if you want to get array from /dev/urandom
	initialize(19650218UL);
	int i = 1;
	uint32_t j = 0;
	int k = (N > seed.size() ? N : seed.size());

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

		if (j >= seed.size()) {
			j = 0;
		}
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

Random::Random() {
	//use an high-precision timer to grab microseconds
	seed((RandomSeed)(Timer().currentTime() * 1000000));
}

Random::Random(RandomSeed oneSeed) {
	seed(oneSeed);
}

Random::Random(const BigSeed& bigSeed) {
	seed(bigSeed);
}

Random::Random(const Random& o) {
	const uint32_t* t = o.state;
	uint32_t* s = state;
	int i = N;

	for (; i--; *s++ = *t++) {
	}

	left = o.left;
	pget = &state[N - left];
}

uint32_t Random::getInt() {
	// Pull a 32-bit integer from the generator state
	// Every other access function simply transforms the numbers extracted here

	if (left == 0) {
		reload();
	}

	--left;

	uint32_t s1;
	s1 = *pget++;
	s1 ^= (s1 >> 11);
	s1 ^= (s1 << 7) & 0x9d2c5680UL;
	s1 ^= (s1 << 15) & 0xefc60000UL;
	return (s1 ^ (s1 >> 18));
}

uint32_t Random::getInt(const uint32_t n) {
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
	return (uint32_t)getFloat((float)n);
}

int Random::getInt(int min, int max) {
	DEBUG_ASSERT(max > min, "Invalid random");

	return getInt(max - min) + min;
}

#define INV_RANGE (1.f / 4294967295.f)

float Random::getFloat() {
	return float(getInt()) * INV_RANGE;
}

float Random::getSign() {
	return (getInt() % 2 == 0) ? 1.f : -1.f;
}

double Random::getDouble() {
	return double(getInt()) * INV_RANGE;
}

float Random::getFloat(const float n) {
	return getFloat() * n;
}

float Random::getFloatExc() {
	return float(getInt()) * INV_RANGE;
}

float Random::getFloatExc(const float n) {
	return getFloatExc() * n;
}

float Random::getFloatFullExcl() {
	return (float(getInt()) + 0.5f) * INV_RANGE;
}

float Random::getFloatFullExcl(const float n) {
	return getFloatFullExcl() * n;
}

Vector Random::getPoint(const Vector& min, const Vector& max) {
	return{
		getFloat(min.x, max.x),
		getFloat(min.y, max.y),
		getFloat(min.z, max.z)
	};
}

Vector Random::get2DPoint(const Vector& min, const Vector& max, float z) {
	return{
		getFloat(min.x, max.x),
		getFloat(min.y, max.y),
		z
	};
}

Vector Random::get2DUnitVector() {
	auto a = getFloat(0, Math::TAU);

	return Vector(cosf(a), sinf(a));
}


double Random::rand53() {
	uint32_t a = getInt() >> 5, b = getInt() >> 6;
	return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0); // by Isaku Wada
}

double Random::randNorm(const double mean /*= 0.0*/, const double stddev /*= 1.0*/) {
	// Return a real number from a normal (Gaussian) distribution with given
	// mean and standard deviation by polar form of Box-Muller transformation
	double x, y, r;

	do {
		x = 2.0 * getDouble() - 1.0;
		y = 2.0 * getDouble() - 1.0;
		r = x * x + y * y;
	}
	while (r >= 1.0 || r == 0.0);

	double s = sqrt(-2.0 * log(r) / r);
	return mean + x * s * stddev;
}

void Random::save(uint32_t* saveArray) const {
	const uint32_t* s = state;
	uint32_t* sa = saveArray;
	int i = N;

	for (; i--; *sa++ = *s++) {
	}

	*sa = left;
}

void Random::load(uint32_t* const loadArray) {
	uint32_t* s = state;
	uint32_t* la = loadArray;
	int i = N;

	for (; i--; *s++ = *la++) {
	}

	left = *la;
	pget = &state[N - left];
}

Random& Random::operator=(const Random& o) {
	if (this == &o) {
		return (*this);
	}

	const uint32_t* t = o.state;
	uint32_t* s = state;
	int i = N;

	for (; i--; *s++ = *t++) {
	}

	left = o.left;
	pget = &state[N - left];
	return (*this);
}
