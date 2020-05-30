#include <httplib.h>
#include <json.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <conio.h>
#include <cmath>
#include "CoordTrans.h"


using json = nlohmann::json;
using Client = httplib::Client;
using Request = httplib::Request;
using Response = httplib::Response;
using namespace std::chrono;

constexpr int limServerPort = 5056;

void getDistance(const Request& req, Response& res);
void getHeight(const Request& req, Response& res);
void getCoord(const Request& req, Response& res);
void getInfo(const Request& req, Response& res);
void getConnect(const Request& req, Response& res);
void getDisconnect(const Request& req, Response& res);

int main()
{
	// 配置
	/*
	data_device_ip
	distance_device_ip

	07
	
	
	*/



	LidarDevice::InitEquipment();
	LidarDevice::OpenEquipment("192.168.31.210");
	LidarDevice::OpenEquipment("192.168.31.202");
	LidarDevice::WaitFirstDeviceTryConnected();

	while (LidarDevice::OnlineDeviceNumber < 2)
		std::this_thread::sleep_for(10ms);
	LidarDevice::StartLMDData();
	Client client("192.168.241.128", 9090);

	auto& deviceMap = LidarDevice::DeviceList[128];
	auto& deviceDst = LidarDevice::DeviceList[129];
	CoordTrans coord;

	while (true)
	{
		//constexpr int Y_STEP = 50;
		//constexpr int X_STEP = 50;
		//constexpr int X_NUM = 60;
		//constexpr int Y_NUM = 180;
		constexpr int DEBUG_SCALE = 10;
		constexpr int Y_STEP = 5;
		constexpr int X_STEP = 5;
		constexpr int H_MAX = 400;
		
		int distance = 0;
		deviceMap.LockRawData();
		coord.ProcessCoord(deviceMap.rawData);
		deviceMap.UnlockRawData();

		json body;
		if (coord.hist.empty() || coord.hist.empty())
		{
			std::this_thread::sleep_for(50ms);
			continue;
		}

		if (int dst = deviceDst.getFacingDistance(); dst > 0)
			distance = dst;


		body["cid"] = LidarDevice::DeviceList.begin()->first;
		// todo 放大过
		body["x"] = (distance + X_STEP / 2) / X_STEP * X_STEP * DEBUG_SCALE;
		body["intime"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		
		// 清理一下起始点
		//int begin = ceil(coord.beginY / (double)Y_STEP) * Y_STEP;
		//int end = floor(coord.endY / (double)Y_STEP) * Y_STEP;
		int begin = coord.beginY;
		int end = coord.endY;

		// 发送完整连续数据
		for (int yPtr = begin; yPtr < end; yPtr++)
		{
			for (size_t i = 0; i < DEBUG_SCALE; i++)
			{
				body["h"].push_back(coord.hist[yPtr - coord.beginY] * DEBUG_SCALE);	// TODO
			}
		}
		body["h"].push_back(coord.hist[end - 1 - coord.beginY] * DEBUG_SCALE);	// TODO
		
		//body["begin"] = begin;
		//body["end"] = end;
		// 将负坐标系转换过来
		body["begin"] = (begin + 250)* DEBUG_SCALE;
		body["end"] = (end + 250)* DEBUG_SCALE;

		for (const auto& ρ : coord.rawData.ρ)
			body["raw"].push_back(ρ);
		body["angle_begin"] = coord.rawData.angleBeg;
		body["angle_end"] = coord.rawData.angleEnd;

		std::cout << body.dump() << std::endl;
		client.Post("/map/col", body.dump(), "application/json");

		std::this_thread::sleep_for(500ms);
	}
	return 0;
}