#include "mymath.hpp"

number_t sum(number_t values[], size_t n)
{
	int val = 0;
	
	for(size_t i=0; i<n; i++)
		val += values[i];

	return val;
}

cmplx_t sum(cmplx_t values[], size_t n)
{
	cmplx_t val = {0};
	
	for(size_t i=0; i<n; i++)
	{
		val.re += values[i].re;
		val.im += values[i].im;
	}

	return val;
}

number_t manhattanDistance(cmplx_t value)
{
	return value.re + value.im;
}

number_t euclideanDistance(cmplx_t value)
{
	return sqrt(value.re*value.re + value.im*value.im);
}

cmplx_t fourierComponent(number_t values[], size_t n, number_t k)
{
	cmplx_t big_x = {0};

	for(size_t i=0; i<n; i++)
	{
		big_x.re += values[i] * cos((-2*MY_PI*k*i) / n);
		big_x.im += values[i] * sin((-2*MY_PI*k*i) / n);
	}

	return big_x;
}

number_t* hammingWindow(number_t* values, size_t N)
{
	for(size_t i=0; i<N; i++)
		values[i] *= 0.54 - 0.46 * cos((2*MY_PI*i)/N);
	return values;
}

number_t* blackmanWindow(number_t* values, size_t N)
{
	for(size_t i=0; i<N; i++)
		values[i] *= 0.42 - 0.5 * cos((2*MY_PI*i)/N) + 0.08 * cos((4*MY_PI*i)/N);
	return values;
}