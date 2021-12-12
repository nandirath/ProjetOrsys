/*
 * broker.cpp
 *
 *  Created on: Dec 11, 2021
 *      Author: nandi
 */

#include "LCD.h"
#include <iostream>
#include <pigpio.h>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std;
int main(int argc, char *argv[]) {
	if (gpioInitialise() < 0) {
		cout << "Error initializing pigpio library..." << endl;
		exit(-1);
	}
	LCD lcd(1, 0x27);
	lcd.enableCursor();
	lcd.enableBlinking();
	uint32_t flags=0x00;
	int hSpi=spiOpen(0,100000,flags);
	if (hSpi<0) {
			cout << "Error SPI initialization..."<<endl;
			gpioTerminate();
			exit(-1);
	}
	// 01: ask for 8 high bits, 02: ask for 8 low bits, 00: end transfer
	const char sBuffer[]={01,02,03,00};
	char rBuffer[6];

	while (1) {
		spiXfer(hSpi,(char *)sBuffer,(char *)rBuffer,4);
		for(int i=0; i<6; i++)cout<<"rBuffer["<<i<<"]="<<(int)rBuffer[i]<<endl;
		uint16_t value=(rBuffer[1]<<8) | rBuffer[2];
		cout<<"L'intensité de la lumière ambiante reçue : "<<value<<endl;
		cout<<"La temperature ambiante reçue : "<<(int)rBuffer[3]<<endl;
		lcd.setPosition(0, 0);
		lcd.printf("HELLO WORLD");
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	spiClose(hSpi);
	gpioTerminate();
	return 0;
}

