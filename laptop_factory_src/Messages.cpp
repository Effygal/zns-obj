#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include "Messages.h"

LaptopOrder::LaptopOrder() {
	customer_id = -1;
	order_number = -1;
	laptop_type = -1;
}

void LaptopOrder::SetOrder(int id, int number, int type) {
	customer_id = id;
	order_number = number;
	laptop_type = type;
}

int LaptopOrder::GetCustomerId() { return customer_id; }
int LaptopOrder::GetOrderNumber() { return order_number; }
int LaptopOrder::GetLaptopType() { return laptop_type; }

int LaptopOrder::Size() {
	return sizeof(customer_id) + sizeof(order_number) + sizeof(laptop_type);
}

void LaptopOrder::Marshal(char *buffer) {
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_laptop_type = htonl(laptop_type);
	int offset = 0;
	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_laptop_type, sizeof(net_laptop_type));
}

void LaptopOrder::Unmarshal(char *buffer) {
	int net_customer_id;
	int net_order_number;
	int net_laptop_type;
	int offset = 0;
	memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(&net_laptop_type, buffer + offset, sizeof(net_laptop_type));

	customer_id = ntohl(net_customer_id);
	order_number = ntohl(net_order_number);
	laptop_type = ntohl(net_laptop_type);
}

bool LaptopOrder::IsValid() {
	return (customer_id != -1);
}

void LaptopOrder::Print() {
	std::cout << "id " << customer_id << " ";
	std::cout << "num " << order_number << " ";
	std::cout << "type " << laptop_type << std::endl;
}

LaptopInfo::LaptopInfo() {
	customer_id = -1;
	order_number = -1;
	laptop_type = -1;
	engineer_id = -1;
	expert_id = -1;
}

void LaptopInfo::SetInfo(int id, int number, int type, int engid, int expid) {
	customer_id = id;
	order_number = number;
	laptop_type = type;
	engineer_id = engid;
	expert_id = expid;
}

void LaptopInfo::CopyOrder(LaptopOrder order) {
	customer_id = order.GetCustomerId();
	order_number = order.GetOrderNumber();
	laptop_type = order.GetLaptopType();
}
void LaptopInfo::SetEngineerId(int id) { engineer_id = id; }
void LaptopInfo::SetExpertId(int id) { expert_id = id; }

int LaptopInfo::GetCustomerId() { return customer_id; }
int LaptopInfo::GetOrderNumber() { return order_number; }
int LaptopInfo::GetLaptopType() { return laptop_type; }
int LaptopInfo::GetEngineerId() { return engineer_id; }
int LaptopInfo::GetExpertId() { return expert_id; }

int LaptopInfo::Size() {
	return sizeof(customer_id) + sizeof(order_number) + sizeof(laptop_type)
		+ sizeof(engineer_id) + sizeof(expert_id);
}

void LaptopInfo::Marshal(char *buffer) {
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_laptop_type = htonl(laptop_type);
	int net_engineer_id = htonl(engineer_id);
	int net_expert_id = htonl(expert_id);
	int offset = 0;

	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_laptop_type, sizeof(net_laptop_type));
	offset += sizeof(net_laptop_type);
	memcpy(buffer + offset, &net_engineer_id, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(buffer + offset, &net_expert_id, sizeof(net_expert_id));

}

void LaptopInfo::Unmarshal(char *buffer) {
	int net_customer_id;
	int net_order_number;
	int net_laptop_type;
	int net_engineer_id;
	int net_expert_id;
	int offset = 0;

	memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(&net_laptop_type, buffer + offset, sizeof(net_laptop_type));
	offset += sizeof(net_laptop_type);
	memcpy(&net_engineer_id, buffer + offset, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(&net_expert_id, buffer + offset, sizeof(net_expert_id));

	customer_id = ntohl(net_customer_id);
	order_number = ntohl(net_order_number);
	laptop_type = ntohl(net_laptop_type);
	engineer_id = ntohl(net_engineer_id);
	expert_id = ntohl(net_expert_id);
}

bool LaptopInfo::IsValid() {
	return (customer_id != -1);
}

void LaptopInfo::Print() {
	std::cout << "id " << customer_id << " ";
	std::cout << "num " << order_number << " ";
	std::cout << "type " << laptop_type << " ";
	std::cout << "engid " << engineer_id << " ";
	std::cout << "expid " << expert_id << std::endl;
}

