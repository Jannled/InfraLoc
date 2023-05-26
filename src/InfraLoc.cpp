#include "InfraLoc.h"

#include <Arduino.h>
#include <bitset>         // std::bitset

InfraLoc::InfraLoc(pin_index_t data, pin_index_t s0, pin_index_t s1, pin_index_t s2, pin_index_t s3) :
    data{data}, s0{s0}, s1{s1}, s2{s2}, s3{s3}
{

}


void gatherSample()
{

}

void InfraLoc::switchChannel(uint8_t channel)
{
	std::bitset<sizeof(uint8_t)*8> channelBits(channel);
	digitalWrite(s0, channelBits.test(0));
	digitalWrite(s1, channelBits.test(1));
	digitalWrite(s2, channelBits.test(2));
	digitalWrite(s3, channelBits.test(3));
	delayMicroseconds(2);
}