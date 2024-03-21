#include <iostream>
#include <memory>

#include "ServerThread.h"
#include "ServerStub.h"

LaptopInfo LaptopFactory::
CreateRegularLaptop(LaptopOrder order, int engineer_id) {
	LaptopInfo laptop;
	laptop.CopyOrder(order);
	laptop.SetEngineerId(engineer_id);
	laptop.SetExpertId(-1);
	return laptop;
}

LaptopInfo LaptopFactory::
CreateCustomLaptop(LaptopOrder order, int engineer_id) {
	LaptopInfo laptop;
	laptop.CopyOrder(order);
	laptop.SetEngineerId(engineer_id);

	std::promise<LaptopInfo> prom;
	std::future<LaptopInfo> fut = prom.get_future();

	std::unique_ptr<ExpertRequest> req = 
		std::unique_ptr<ExpertRequest>(new ExpertRequest);
	req->laptop = laptop;
	req->prom = std::move(prom);

	erq_lock.lock();
	erq.push(std::move(req));
	erq_cv.notify_one();
	erq_lock.unlock();

	laptop = fut.get();
	return laptop;
}

void LaptopFactory::
EngineerThread(std::unique_ptr<ServerSocket> socket, int id) {
	int engineer_id = id;
	int laptop_type;
	LaptopOrder order;
	LaptopInfo laptop;

	ServerStub stub;

	stub.Init(std::move(socket));

	while (true) {
		order = stub.ReceiveOrder();
		if (!order.IsValid()) {
			break;	
		}
		laptop_type = order.GetLaptopType();
		switch (laptop_type) {
			case 0:
				laptop = CreateRegularLaptop(order, engineer_id);
				break;
			case 1:
				laptop = CreateCustomLaptop(order, engineer_id);
				break;
			default:
				std::cout << "Undefined laptop type: "
					<< laptop_type << std::endl;

		}
		stub.SendLaptop(laptop);
	}
}

void LaptopFactory::ExpertThread(int id) {
	std::unique_lock<std::mutex> ul(erq_lock, std::defer_lock);
	while (true) {
		ul.lock();

		if (erq.empty()) {
			erq_cv.wait(ul, [this]{ return !erq.empty(); });
		}

		auto req = std::move(erq.front());
		erq.pop();

		ul.unlock();

		std::this_thread::sleep_for(std::chrono::microseconds(100));
		req->laptop.SetExpertId(id);
		req->prom.set_value(req->laptop);	
	}
}



