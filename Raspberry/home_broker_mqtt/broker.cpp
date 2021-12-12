/*
 * broker.cpp
 *
 *  Created on: Dec 11, 2021
 *      Author: nandi
 */
#include <iostream>
#include <pigpio.h>
#include <cstdlib>
#include <cctype>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include "LCD.h"
#include "mqtt/async_client.h"

using namespace std;

const std::string SERVER_ADDRESS { "tcp://192.168.0.14:1883" };
const std::string CLIENT_ID { "homebroker" };
auto TOPICS = mqtt::string_collection::create({"personCount","accessControl" });
const std::vector<int> QOS { 1, 1 };
//const std::string TOPICS {"personCount","accessControl"};
//const int QOS=1;
char mqtt_message[2];
int temp, lum;
constexpr int O_LED = 15;


void MQTT_communication();
void SPI_communication();
void LCD_printing();
std::mutex globalMutex;



int main(int argc, char *argv[]) {

	if (gpioInitialise() < 0) {
		cout << "Error initializing pigpio library..." << endl;
		exit(-1);
	}
	gpioSetPWMrange(O_LED, 1023);

	thread th_arduino(SPI_communication);
	thread th_mqtt(MQTT_communication);
	thread th_affichage(LCD_printing);
	th_arduino.join();
	th_mqtt.join();
	th_affichage.join();
	gpioTerminate();
	return 0;
}

void SPI_communication(){
	uint32_t flags=0x00;
	int hSpi=spiOpen(0,100000,flags);
	if (hSpi<0) {
		cout << "Error SPI initialization..."<<endl;
		gpioTerminate();
		exit(-1);
		}

	// 01: ask for 8 high bits, 02: ask for 8 low bits, 00: end transfer
	const char sBuffer[]={01,02,03,00};
	char rBuffer[5];
	int brightness;
	while (1) {
		spiXfer(hSpi,(char *)sBuffer,(char *)rBuffer,4);
		uint16_t value=(rBuffer[1]<<8) | rBuffer[2];
		lum=value;
		brightness=1023-lum;
		gpioPWM(O_LED,brightness);
		temp=(int)rBuffer[3];
		cout<<"L'intensité de la lumière ambiante reçue : "<<lum<<endl;
		cout<<"La temperature ambiante reçue : "<<temp<<endl;
		//globalMutex.lock();
		//lcd.setPosition(0, 0);
		//lcd.printf("Lum:%d T:%dC\n", (int)value,(int)rBuffer[3]);
		//globalMutex.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	spiClose(hSpi);

}

void MQTT_communication(){
		mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);
	try {
		// Start consumer before connecting to make sure to not miss messages
		cli.start_consuming();
		auto rsp = cli.connect()->get_connect_response();
// If there is no session present, then we need to subscribe, but if
// there is a session, then the server remembers us and our subscriptions.
		if (!rsp.is_session_present())
			cli.subscribe(TOPICS, QOS)->wait();
		while (true) { // Loop to consume read messages
			auto msg = cli.consume_message();
			if (!msg)
				break;
			if(msg->get_topic()=="personCount"){
				cout << msg->get_topic() << ": " << msg->to_string() << endl;
							strcpy(mqtt_message,msg->to_string().c_str());
							//lcd.setPosition(1,0);
							//lcd<<"Personnes:"<<msg->to_string().c_str();
			}
			else if (msg->get_topic()=="accessControl"){
				cout << "Alarme activée"<< endl;
			}

		}
		if (cli.is_connected()) {
			cli.unsubscribe(TOPICS)->wait();
			cli.stop_consuming();
			cli.disconnect()->wait();
		} else
			cout << "\nClient was disconnected" << endl;
	} catch (const mqtt::exception &exc) {
		cerr << "\n " << exc << endl;
		//return 1;
	}
	//return 0;
}

void LCD_printing(){
	LCD lcd(1, 0x27);
	lcd.enableCursor();
	lcd.enableBlinking();
	while(1){
		/*lcd.setPosition(0, 0);
		lcd.putChar(67); // Put char 'C'
		std::this_thread::sleep_for(std::chrono::seconds(1));
		lcd.setPosition(0, 1);
		lcd.putChar(68); // Put char 'D'
		lcd<<"ABC"; // Put string "ABC"
		std::this_thread::sleep_for(std::chrono::seconds(1));*/
		lcd.setPosition(0,0);
		lcd.printf("Lum:%d T:%dC\n",lum,temp);
		lcd.setPosition(0,1);
		lcd.printf("Personnes: %s",mqtt_message);
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

}
