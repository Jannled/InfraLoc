#include <Arduino.h>

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

// Defines
void taskGatherSample();
void taskCalculateFFT();
void switchChannel(uint8_t channel);

void setup()
{
	Serial.begin(115200);
	delay(100);
	Serial.println("Infraloc v3");

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(15, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	pinMode(ADC_PIN, INPUT);

	Serial.println(MY_PI, 16);
	Serial.flush();

	switchChannel(0);
}

void loop()
{
	// Task 1
	static auto lastTime = micros();
	auto now = micros();
	if(!(now - lastTime <= 1000000UL/SAMPLES_PER_SECOND))
	{
		lastTime = now;
		taskGatherSample();
	}
	
	// Task 2
	if(sampleIndex >= SAMPLE_BUFFER_SIZE)
		taskCalculateFFT();
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

	// Select the next channel
	muxChannel = (muxChannel + 1) % NUM_IR_RECEIVER;
	//switchChannel(muxChannel);
	
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
	number_t dcOffset = cachedFourierDC<SAMPLE_BUFFER_SIZE>(sampleBuffer);
	number_t result = euclideanDistance(cachedFourierComponent<SAMPLE_BUFFER_SIZE, k>(sampleBuffer));

	Serial.print("Channel D" + muxChannel+1);
	Serial.print(", DC Offset: ");
	Serial.print(dcOffset);
	Serial.print("Signal strength: ");
	Serial.println(result);
	#endif

	digitalWrite(LED_BUILTIN, LOW);
}

void switchChannel(uint8_t channel)
{
	digitalWrite(MUX_S0, channel & (1 << 0));
	digitalWrite(MUX_S1, channel & (1 << 1));
	digitalWrite(MUX_S2, channel & (1 << 2));
	digitalWrite(MUX_S3, channel & (1 << 3));
	delayMicroseconds(2);
}
