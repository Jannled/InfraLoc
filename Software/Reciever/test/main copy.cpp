#include <Arduino.h>

#include "mymath.hpp"

// --- CONFIG ---
//#define DEBUG_VALUES

#define ADC_PIN 28u
#define SAMPLES_PER_SECOND	5000UL // Hz
#define SAMPLE_BUFFER_SIZE	512
#define NUM_IR_RECEIVER 16

constexpr uint8_t MUX_S3 = 6u;
constexpr uint8_t MUX_S2 = 7u;
constexpr uint8_t MUX_S1 = 8u;
constexpr uint8_t MUX_S0 = 9u;

uint8_t mux = 2; // 2 == D3, 0 == D1 ...

/** Ranging From 0 (DC offset) to Nyquist frequency or 0 to SAMPLE_BUFFER_SIZE. 12 */
const unsigned int k = 101;

void plot();
void calculateFFT();
void run_sampling();
void calculateFrequencyBin();
void switchChannel(uint8_t channel);
void sampleReceivers(number_t buffer[][SAMPLE_BUFFER_SIZE], size_t index);

static number_t buffer[SAMPLE_BUFFER_SIZE] = {};
unsigned int signalStrength[NUM_IR_RECEIVER];
uint8_t currentChannel;
static size_t counter = 0;

void setup1()
{
	Serial.begin(115200);
	delay(100);
	Serial.println("Starting IR receiver");
	delay(2000);
	Serial.println("IR Receiver");

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(15, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	pinMode(ADC_PIN, INPUT);

	Serial.println(MY_PI, 16);
	Serial.flush();
}

void loop1()
{
	//switchChannel(channel);
	delay(1000);

	Serial.print("[");
	for(int i=0; i<NUM_IR_RECEIVER; i++)
	{
		if(i > 0)
			Serial.print(",");

		switchChannel(i);
		delay(10);
		Serial.print(analogRead(ADC_PIN));
	}
	Serial.println("]");
}

void loop2()
{
	// Gather samples
	run_sampling();

	// If buffer is full, perform Fourier transform
	if(counter >= SAMPLE_BUFFER_SIZE)
	{	
		if(currentChannel == 0)
		{
			Serial.print("[");
			Serial.print(signalStrength[0]);
			for(int i=1; i<NUM_IR_RECEIVER; i++)
			{
				Serial.print(", ");
				Serial.print(signalStrength[i]);
			}
			Serial.println("]");
		}

		digitalWrite(LED_BUILTIN, HIGH);
		#ifdef DEBUG_VALUES
		calculateFFT();
		#else
		calculateFrequencyBin();
		#endif
		digitalWrite(LED_BUILTIN, LOW);
		counter = 0; // Reset the sample counter
		currentChannel = (currentChannel + 1) % NUM_IR_RECEIVER; // Switch through channels
		switchChannel(currentChannel);
	}
}

void run_sampling()
{
	//static number_t buffer[NUM_IR_RECEIVER][SAMPLE_BUFFER_SIZE] = {};
	static unsigned long lastTime = micros();
	static bool state = false;

	// Perform conversion at regular intervals
	const unsigned long now = micros();
	//const long waitTime = lastTime - now + 1000UL; //(long) (1000000.0/SAMPLES_PER_SECOND);

	// Wait until taking a sample
	if(now - lastTime <= 1000000UL/SAMPLES_PER_SECOND)
		return;

	gpio_put(15, state = !state); // Measured frequency is 2 * 100kHz

	// Perform ADC
	//sampleReceivers(buffer, counter++);
	buffer[counter++] = analogRead(ADC_PIN);
	lastTime = now;
}

void calculateFrequencyBin()
{
	number_t result = euclideanDistance(cachedFourierComponent<SAMPLE_BUFFER_SIZE, k>(buffer));
	number_t dcOffset = cachedFourierDC<SAMPLE_BUFFER_SIZE>(buffer);

	signalStrength[currentChannel] = result;
}

void calculateFFT()
{
	number_t output[SAMPLE_BUFFER_SIZE] = {};
	
	// Not Fast But Discrete O(n^2)
	for(int k=0; k<SAMPLE_BUFFER_SIZE; k++)
		output[k] = euclideanDistance(fourierComponent(buffer, SAMPLE_BUFFER_SIZE, k));

	#ifdef DEBUG_VALUES
	Serial.print('[');
	Serial.print(output[0]);
	for(int k=1; k<SAMPLE_BUFFER_SIZE; k++)
	{
		Serial.print(',');
		Serial.print(output[k]);
	}
	Serial.println(']');
	#endif
}

/*void calculateDFT2()
{
	// Calculate the magnitude of the Fourier Component at the desired frequency band
	number_t result[NUM_IR_RECEIVER];
	number_t dcOffset[NUM_IR_RECEIVER];
	for(int i=0; i<NUM_IR_RECEIVER; i++)
	{
		result[i] = euclideanDistance(cachedFourierComponent<SAMPLE_BUFFER_SIZE, k>(buffer[i]));
		dcOffset[i] = cachedFourierDC<SAMPLE_BUFFER_SIZE>(buffer[i]);
	}

	#ifdef DEBUG_VALUES
	//hammingWindow(buffer[mux], SAMPLE_BUFFER_SIZE);
	Serial.print("[");
	Serial.print(buffer[mux][0]);
	for(int i=1; i<SAMPLE_BUFFER_SIZE; i++)
	{
		Serial.print(",");
		Serial.print(buffer[mux][i]);
	}
	Serial.println("]");

	#else

	size_t peakChannel = 0;
	for(int i=0; i<NUM_IR_RECEIVER; i++)
	{
		if(result[i] > result[peakChannel])
			peakChannel = i;
	}

	Serial.print("Peak at D");
	Serial.print(peakChannel+1);
	Serial.print(": |");
	Serial.print(result[peakChannel]);
	Serial.println("|");

	Serial.print("[");
	Serial.print(result[0]);
	for(int i=1; i<NUM_IR_RECEIVER; i++)
	{
		Serial.print(", ");
		Serial.print(result[i]);
	}
	Serial.println("]");

	//Serial.print("Re:");
	//Serial.print(result[mux].re);
	//Serial.print(",");
	//Serial.print("Im:");
	//Serial.print(result[mux].im);
	//Serial.print(",");
	//Serial.print("Abs:");
	//Serial.println(ec);
	#endif
}*/

//void loop2()
//{
//	#ifdef DEBUG_VALUES
//	plot();
//	#else 
//	
//	static number_t buffer[NUM_IR_RECEIVER][SAMPLE_BUFFER_SIZE] = {};
//	sampleReceivers(buffer);
//
//	number_t k = 12;
//	hammingWindow(buffer, SAMPLE_BUFFER_SIZE);
//	cmplx_t fc = fourierComponent(buffer, SAMPLE_BUFFER_SIZE, k);
//	number_t y = euclideanDistance(fc);
//
//	Serial.print("Re:");
//	Serial.print(fc.re);
//	Serial.print(",");
//	Serial.print("Im:");
//	Serial.print(fc.im);
//	Serial.print(",");
//	Serial.print("Abs:");
//	Serial.println(y);
//
//	#endif // DEBUG_VALUES
//}

//void plot()
//{
//	static float buffer[SAMPLE_BUFFER_SIZE];
//
//	digitalWrite(MUX_S0, mux & (1 << 0)); // LOW
//	digitalWrite(MUX_S1, mux & (1 << 1)); // LOW
//	digitalWrite(MUX_S2, mux & (1 << 2)); // HIGH
//	digitalWrite(MUX_S3, mux & (1 << 3)); // HIGH
//
//	for(int i=0; i<SAMPLE_BUFFER_SIZE; i++)
//	{
//		buffer[i] = analogRead(ADC_PIN);
//		delay(1000/SAMPLES_PER_SECOND);
//	}
//
//	Serial.print("[");
//	Serial.print(buffer[0]);
//	for(int i=1; i<SAMPLE_BUFFER_SIZE; i++)
//	{
//		Serial.print(",");
//		Serial.print(buffer[i]);
//	}
//	Serial.println("]");
//}

void switchChannel(uint8_t channel)
{
	digitalWrite(MUX_S0, channel & (1 << 0));
	digitalWrite(MUX_S1, channel & (1 << 1));
	digitalWrite(MUX_S2, channel & (1 << 2));
	digitalWrite(MUX_S3, channel & (1 << 3));
	delayMicroseconds(2);
}

void sampleReceivers(number_t buffer[][SAMPLE_BUFFER_SIZE], const size_t index)
{
	#ifdef DEBUG_VALUES
	digitalWrite(MUX_S0, mux & (1 << 0));
	digitalWrite(MUX_S1, mux & (1 << 1));
	digitalWrite(MUX_S2, mux & (1 << 2));
	digitalWrite(MUX_S3, mux & (1 << 3));
	delayMicroseconds(2);
	buffer[mux][index] = analogRead(ADC_PIN);
	
	#else
	for(int channel=0; channel<NUM_IR_RECEIVER; channel++)
	{
		switchChannel(channel);
		buffer[channel][index] = analogRead(ADC_PIN);
	}
	#endif
}
