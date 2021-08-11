#include <iostream>
#include <algorithm>
#include <random>

#include "netServer.hpp"

#include "game/gameCommon.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	cout << "server-hi" << endl;

	uint16_t port;
	if (argc == 2)
		port = stoi(argv[1]);
	else
		port = 30000;

	Net::NetServer server(port);
	server.start();
	server.run();
	server.stop();

	cout << "server-bye" << endl;
	return 0;
}