/**
 * @file    SimplexNoise.h
 * @brief   A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D, 4D).
 *
 * Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#pragma once

#include "dojo_common_header.h"

#include <cstddef>  // size_t

namespace Dojo {

	class Random;

	/**
	 * @brief A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D, 4D).
	 */
	class Noise {
	public:

		/**
		* Constructor of to initialize a fractal noise summation
		*
		* @param[in] random the random used to generate the unique permutation
		* @param[in] frequency    Frequency ("width") of the first octave of noise (default to 1.0)
		* @param[in] amplitude    Amplitude ("height") of the first octave of noise (default to 1.0)
		* @param[in] lacunarity   Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
		* @param[in] persistence  Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
		*/
		explicit Noise(
			Random& random,
			float frequency = 1.0f,
			float amplitude = 1.0f,
			float lacunarity = 2.0f,
			float persistence = 0.5f);

		/**
		* Constructor of to initialize a fractal noise summation
		*
		* @param[in] seed used to generate the unique permutation
		* @param[in] frequency    Frequency ("width") of the first octave of noise (default to 1.0)
		* @param[in] amplitude    Amplitude ("height") of the first octave of noise (default to 1.0)
		* @param[in] lacunarity   Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
		* @param[in] persistence  Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
		*/
		explicit Noise(
			RandomSeed seed,
			float frequency = 1.0f,
			float amplitude = 1.0f,
			float lacunarity = 2.0f,
			float persistence = 0.5f);

		// 1D Perlin simplex noise
		float noise(float x) const;
		// 2D Perlin simplex noise
		float noise(float x, float y) const;
		// 3D Perlin simplex noise
		float noise(float x, float y, float z) const;

		// 1D noise between 0 and 1
		float normalizedNoise(float x) const {
			return (noise(x) + 1.f) * 0.5f;
		}

		// 2D noise between 0 and 1
		float normalizedNoise(float x, float y) const {
			return (noise(x, y) + 1.f) * 0.5f;
		}

		// Fractal/Fractional Brownian Motion (fBm) noise summation
		float fractal(size_t octaves, float x) const;
		float fractal(size_t octaves, float x, float y) const;

	private:
		// Parameters of Fractional Brownian Motion (fBm) : sum of N "octaves" of noise
		float mFrequency;   ///< Frequency ("width") of the first octave of noise (default to 1.0)
		float mAmplitude;   ///< Amplitude ("height") of the first octave of noise (default to 1.0)
		float mLacunarity;  ///< Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
		float mPersistence; ///< Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)

		std::array<uint8_t, 256> perm;

		/**
		* Helper function to hash an integer using the above permutation table
		*
		*  This inline function costs around 1ns, and is called N+1 times for a noise of N dimension.
		*
		*  Using a real hash function would be better to improve the "repeatability of 256" of the above permutation table,
		* but fast integer Hash functions uses more time and have bad random properties.
		*
		* @param[in] i Integer value to hash
		*
		* @return 8-bits hashed value
		*/

		inline uint8_t hash(int32_t i) const {
			return perm[static_cast<uint8_t>(i)];
		}

		void _init(
			Random& random,
			float frequency,
			float amplitude,
			float lacunarity,
			float persistence);
	};
}