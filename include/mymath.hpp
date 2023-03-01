#ifndef MY_MATH_H
#define MY_MATH_H

#include <cmath>

#include <Arduino.h>

#ifdef MATH_DOUBLE_PRECISION
typedef double number_t;
#else
typedef float number_t;
#endif

constexpr auto MY_PI = acos(-1);

typedef struct
{
	number_t re;
	number_t im;
} cmplx_t;

// https://stackoverflow.com/a/34465458
template<int N>
struct SinCache {
	explicit constexpr SinCache(int k): lut() {
		for(int i=0; i<N; i++)
			lut[i] = sin((-2*MY_PI*k*i) / N);
	}
	number_t lut[N];
};

template<int N>
struct CosCache {
	explicit constexpr CosCache(int k): lut() {
		for(int i=0; i<N; i++)
			lut[i] = cos((-2*MY_PI*k*i) / N);
	}
	number_t lut[N];
};

number_t sum(number_t values[], size_t n);

cmplx_t sum(cmplx_t values[], size_t n);

number_t manhattanDistance(cmplx_t value);

number_t euclideanDistance(cmplx_t value);

/**
 * @brief Calculate the frequency bin k of the discrete Fourier transformation
 * 
 * @param values 
 * @param n N The number of samples
 * @param k The desired frequency (Calculated as P(Full)/P(Event))
 * @return X
 */
cmplx_t fourierComponent(number_t values[], size_t n, number_t k);

/**
 * @brief 
 * 
 * @tparam n 
 * @tparam k 
 * @param values 
 * @return cmplx_t 
 */
template<int n, int k>
cmplx_t cachedFourierComponent(const number_t values[])
{
	constexpr auto fourierCache_cos = CosCache<n>(k);
	constexpr auto fourierCache_sin = SinCache<n>(k);

	static_assert(abs(MY_PI - 3.141) < 0.001);

	// Sanity check the correct generation of the LUT
	static_assert(abs(cos(-2*MY_PI*k*16 / n) - fourierCache_cos.lut[16]) < 0.0000000000000001);
	static_assert(abs(sin(-2*MY_PI*k*16 / n) - fourierCache_sin.lut[16]) < 0.0000000000000001);

	cmplx_t big_x = {0};

	for(size_t i=0; i<n; i++)
	{
		big_x.re += values[i] * fourierCache_cos.lut[i];
		big_x.im += values[i] * fourierCache_sin.lut[i];
	}

	return big_x;
}

/**
 * @brief Calculate the DC offset for the given values (basically k=0). 
 * 
 * The method will generate the factors for the fourier transform at compile.
 * 
 * @tparam n 
 * @param values 
 * @return number_t 
 */
template<int n>
number_t cachedFourierDC(const number_t values[])
{
	static constexpr auto fourierCache_cos = CosCache<n>(0);

	number_t dcOffset = 0.0;

	for(size_t i=0; i<n; i++)
		dcOffset += values[i] * fourierCache_cos.lut[i];

	return dcOffset;
}

/**
 * @brief Hamming Window
 * 
 * Note: constexpr functions with loops or generally anything with more than one line need C++14
 * 
 * @param values 
 * @param N 
 * @return constexpr number_t* 
 */
number_t* hammingWindow(number_t* values, size_t N);

/**
 * @brief Blackman Window
 * 
 * Note: constexpr functions with loops or generally anything with more than one line need C++14
 * 
 * @param values Pointer to an array that will hold the calculated factors 
 * @param N length of the values
 * @return constexpr number_t* 
 */
number_t* blackmanWindow(number_t* values, size_t N);

#endif // MY_MATH_H
