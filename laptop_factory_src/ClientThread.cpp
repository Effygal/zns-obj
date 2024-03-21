#include "ClientThread.h"
#include "Messages.h"

#include <iostream>

ClientThreadClass::ClientThreadClass() {}

void ClientThreadClass::
ThreadBody(std::string ip, int port, int id, int orders, int type) {
	customer_id = id;
	num_orders = orders;
	laptop_type = type;
	if (!stub.Init(ip, port)) {
		std::cout << "Thread " << customer_id << " failed to connect" << std::endl;
		return;
	}
	for (int i = 0; i < num_orders; i++) {
		LaptopOrder order;
		LaptopInfo laptop;
		order.SetOrder(customer_id, i, laptop_type);

		timer.Start();
		laptop = stub.OrderLaptop(order);
		timer.EndAndMerge();

		if (!laptop.IsValid()) {
			std::cout << "Invalid laptop " << customer_id << std::endl;
			break;	
		} 
	}
}

ClientTimer ClientThreadClass::GetTimer() {
	return timer;	
}

