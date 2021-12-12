//============================================================================
// Name        : connectionMQTT.cpp
// Author      : CA ABID
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"

using namespace std;

const string SERVER_ADDRESS { "tcp://192.168.1.17:1883" };
const string CLIENT_ID { "user1" };
const string TOPIC {"ORSYS"};
const int QOS=1;

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
	mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);
	try {
		// Start consumer before connecting to make sure to not miss messages
		cli.start_consuming();
		auto rsp = cli.connect()->get_connect_response();
// If there is no session present, then we need to subscribe, but if
// there is a session, then the server remembers us and our subscriptions.
		if (!rsp.is_session_present())
			cli.subscribe(TOPIC, QOS)->wait();
		while (true) { // Loop to consume read messages
			auto msg = cli.consume_message();
			if (!msg)
				break;
			cout << msg->get_topic() << ": " << msg->to_string() << endl;
		}
		if (cli.is_connected()) {
			cli.unsubscribe(TOPIC)->wait();
			cli.stop_consuming();
			cli.disconnect()->wait();
		} else
			cout << "\nClient was disconnected" << endl;
	} catch (const mqtt::exception &exc) {
		cerr << "\n " << exc << endl;
		return 1;
	}
	return 0;
}
