#include <Arduino.h>
#include <Servo.h>

Servo servo;

/*
 *   0°   500uS
 *  45°   833uS
 *  90°  1166uS
 * 135°  1500uS
 * 180°  1833uS
 * 225°  2167uS
 * 270°  2500uS
*/
#define NUM_STEPS 1000

const size_t BUFFER_SIZE = 256;
size_t writePointer = 0;
char serialBuffer[BUFFER_SIZE] = {0};


void setup() {
	pinMode(6, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	Serial.println("Start");

	servo.attach(9, 500, 2500);  // attaches the servo on pin 9 to the servo object
}

void loop() {

	analogWrite(6, 127);

	while(Serial.available())
	{
		int b = Serial.read();
		if (b <= 0)
			continue;

		if(writePointer >= BUFFER_SIZE-1)
			writePointer = 0;

		if(b == '\n')
		{
			serialBuffer[writePointer++] = '\0';
			long out = strtol(serialBuffer, NULL, 10);
			
			if(out >= 450 && out <= 2500)
			{
				servo.writeMicroseconds(out);
				Serial.print(" -> ");
				Serial.print(out);
				Serial.println("uS");
			}

			writePointer = 0;
		}
		else if(isdigit((char) b) || b == '+' || b == '-')
			serialBuffer[writePointer++] = (char) b;
		//else
		//	writePointer = 0;

		Serial.print("[");
		Serial.print(writePointer);
		Serial.print("]");
	}

	//servo.writeMicroseconds(500);
	//delay(2000);
//
	//digitalWrite(LED_BUILTIN, HIGH);
	//for(int i=0; i<NUM_STEPS; i++)
	//{
	//	delay(10);
	//	servo.writeMicroseconds(map(i, 0, NUM_STEPS, 500, 1865)); // calibrate: 180° ^= 1865
	//}
	//digitalWrite(LED_BUILTIN, LOW);
	//delay(5000);


	//digitalWrite(LED_BUILTIN, HIGH);
	//delay(3000);
//
	//analogWrite(6, 0);
	//digitalWrite(LED_BUILTIN, LOW);
	//delay(1000);
//
	//analogWrite(6, 63);
	//digitalWrite(LED_BUILTIN, HIGH);
	//delay(3000);
//
	//analogWrite(6, 0);
	//digitalWrite(LED_BUILTIN, LOW);
	//delay(1000);
//
	//analogWrite(6, 127 + 63);
	//digitalWrite(LED_BUILTIN, HIGH);
	//delay(3000);
//
	//analogWrite(6, 0);
	//digitalWrite(LED_BUILTIN, LOW);
	//delay(1000);
}