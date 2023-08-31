#include <Arduino.h>

#include <bitset>         // std::bitset

#include "mymath.hpp"

// --- CONFIG ---
//#define DEBUG_VALUES

#define ADC_PIN 28u
#define SAMPLES_PER_SECOND	50000UL // Hz
#define SAMPLE_BUFFER_SIZE	512
#define NUM_IR_RECEIVER 16

constexpr uint8_t MUX_S3 = 6u;
constexpr uint8_t MUX_S2 = 7u;
constexpr uint8_t MUX_S1 = 8u;
constexpr uint8_t MUX_S0 = 9u;

uint8_t muxChannel = 2; // 2 == D3, 0 == D1 ...

/** Ranging From 0 (DC offset) to Nyquist frequency or 0 to SAMPLE_BUFFER_SIZE. 12 */
const unsigned int k = 11;

number_t sampleBuffer[SAMPLE_BUFFER_SIZE] = {0};
size_t sampleIndex = 0;

number_t transformedSignals[2][NUM_IR_RECEIVER] = {0};

// Defines
void taskGatherSample();
void taskCalculateFFT();
void switchChannel(uint8_t channel);
void taskPrintResults();
void printNumberArray(number_t values[], size_t n);

// Fourier Cache
number_t fourierCacheSin[SAMPLE_BUFFER_SIZE];
number_t fourierCacheCos[SAMPLE_BUFFER_SIZE];
number_t fourierCacheCosZero[SAMPLE_BUFFER_SIZE];

void setup()
{
	Serial.begin(115200);
	delay(100);
	Serial.println("InfraLoc v3");

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(15, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	pinMode(ADC_PIN, INPUT);

	Serial.println(MY_PI, 16);
	Serial.flush();

	switchChannel(0);

	// Cache the factors for the FFT
	Serial.println("Generating Fourier Cache...");
	generateFourierCacheSin(fourierCacheSin, SAMPLE_BUFFER_SIZE, k);
	generateFourierCacheCos(fourierCacheCos, SAMPLE_BUFFER_SIZE, k);
	generateFourierCacheCos(fourierCacheCosZero, SAMPLE_BUFFER_SIZE, 0);
	Serial.println("Done!");
}

void loop()
{
	// Task 1
	static auto lastTime = micros();
	auto now = micros();
	if(!(now - lastTime <= 1000000UL/SAMPLES_PER_SECOND))
	{
		taskGatherSample();
		lastTime = now;
	}
	
	// Task 2
	if(sampleIndex >= SAMPLE_BUFFER_SIZE)
	{
		taskCalculateFFT();

		// Task 3
		if(muxChannel == 0)
			taskPrintResults();
	}
}

void taskGatherSample()
{
	sampleBuffer[sampleIndex++] = analogRead(ADC_PIN);
}

void taskCalculateFFT()
{
	digitalWrite(LED_BUILTIN, HIGH);

	// Reset the samples
	sampleIndex = 0;
	
	#ifdef DEBUG_VALUES
	Serial.print("[");
	Serial.print(sampleBuffer[0]);
	for(int i=1; i<SAMPLE_BUFFER_SIZE; i++)
	{
		Serial.print(",");
		Serial.print(sampleBuffer[i]);
	}
	Serial.println("]");
	#else
	// Calculate FFT
	number_t dcOffset = cachedFourierDC(sampleBuffer, SAMPLE_BUFFER_SIZE, k, fourierCacheCosZero);
	number_t result = euclideanDistance(cachedFourierComponent(sampleBuffer, SAMPLE_BUFFER_SIZE, k, fourierCacheSin, fourierCacheCos));

	transformedSignals[0][muxChannel] = dcOffset;
	transformedSignals[1][muxChannel] = result;

	// Select the next channel
	muxChannel = (muxChannel + 1) % NUM_IR_RECEIVER;
	switchChannel(muxChannel);

	//char output[512] = "";
	//snprintf(output, 512, "Channel D%02d, DC Offset %9.1f, Signal strength: %9.1f", muxChannel+1, dcOffset, result);
	//Serial.println(output);
	//delay(100);
	#endif

	digitalWrite(LED_BUILTIN, LOW);
}

void taskPrintResults()
{
	printNumberArray(transformedSignals[1], NUM_IR_RECEIVER);

	uint8_t highestIndex = 0;
	for(size_t i=0; i<NUM_IR_RECEIVER; i++)
	{
		if(transformedSignals[1][i] > transformedSignals[1][highestIndex])
			highestIndex = i;
	}

	number_t angle = highestIndex * (360/NUM_IR_RECEIVER);

	const uint8_t leftChannel = (highestIndex - 1) % NUM_IR_RECEIVER;
	const uint8_t rightChannel = (highestIndex + 1) % NUM_IR_RECEIVER;

	const number_t leftValue = transformedSignals[1][leftChannel];
	const number_t currentValue = transformedSignals[1][highestIndex];
	const number_t rightValue = transformedSignals[1][rightChannel];

	if(leftValue > rightValue)
		angle -= (leftValue/currentValue) * (360/NUM_IR_RECEIVER);
	else
		angle += (rightValue/currentValue) * (360/NUM_IR_RECEIVER);

	//Serial.print("[");
	//Serial.print(angle);
	//Serial.println(", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]");
	//Serial.print("°  ");
}

void printNumberArray(number_t values[], size_t n)
{
	Serial.print("[");
	Serial.print(values[0]);
	for(size_t i=1; i<n; i++)
	{
		Serial.print(",");
		Serial.print(values[i]);
	}
	Serial.println("]");
}

void switchChannel(uint8_t channel)
{
	std::bitset<sizeof(uint8_t)*8> channelBits(channel);
	digitalWrite(MUX_S0, channelBits.test(0));
	digitalWrite(MUX_S1, channelBits.test(1));
	digitalWrite(MUX_S2, channelBits.test(2));
	digitalWrite(MUX_S3, channelBits.test(3));
	delayMicroseconds(2);
}
