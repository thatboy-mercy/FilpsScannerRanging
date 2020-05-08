

#include <httplib.h>
#include <json.hpp>
#include <sstream>
#include <iomanip>
#include <conio.h>
#include "../LidarDataPackage/LimDevice.h"

using json = nlohmann::json;
using Server = httplib::Server;
using Request = httplib::Request;
using Response = httplib::Response;

constexpr int limServerPort = 5056;
Server server;

template<typename T>
std::string fmtString(T in, int width = 0, int prec = 0) {
	std::ostringstream s;
	s << std::setw(width) << std::setprecision(prec) << in;
	return s.str();
}

void getPosition(const Request& req, Response& res)
{
	json param;

	if (!req.has_param("cid"))
	{
		param["error"] = "Invalid parameter: no cid given.";
		res.set_content(param.dump(), "application/json");
		return;
	}
	auto cid = std::stoi(req.get_param_value("cid"));
	param["cid"] = cid;

	auto& device = LimDevice::DeviceList[cid];
	param["try_connected"] = device.isTryConnected;
	param["online"] = device.isConnected.operator bool();
	param["ip"] = device.deviceIP;
	if (!device.isConnected)
	{
		res.set_content(param.dump(), "application/json");
		return;
	}

	if (req.has_param("type"))
	{
		param["begin_angle"] = device.angleBeg;
		param["end_angle"] = device.angleEnd;

		for (int i = req.get_param_value_count("type") - 1; i >= 0; i--)
		{
			auto type = req.get_param_value("type", i);


			// 反向高度坐标(插值）
			if (!type.compare(0, 13, "interpolation_coord", 13))
			{
				param["types"].push_back("interpolation_coord");

				if (!device.yPriorCoord.empty())
				{
					device.LockCoord();

					for (auto& [x, y] : device.yInterpolationCoord)
					{
						param["interpolation_coord"]["x"].push_back(x);
						param["interpolation_coord"]["y"].push_back(y);
					}
					device.UnlockCoord();
				}
			}
			// 线段
			else if (!type.compare(0, 7, "borders", 7))
			{
				param["types"].push_back("borders");
				device.LockCoord();

				for (auto& line : device.borderStore)
				{
					param["borders"].push_back(json());
					auto& thisBorder = param["borders"].back();
					for (auto& [x, y] : line)
					{
						thisBorder["x"].push_back(x);
						thisBorder["y"].push_back(y);
					}
				}

				device.UnlockCoord();
			}
			// 过滤极坐标
			else if (!type.compare(0, 5, "polar_coord", 5))
			{
				param["types"].push_back("polar_coord");
				device.LockCoord();
				for (auto& [length, angle] : device.polarCoord)
				{
					if (length > LimDevice::MaxLength)
						continue;
					param["polar_coord"]["length"].push_back(length);
					param["polar_coord"]["angle"].push_back(angle);
				}
				device.UnlockCoord();
			}
			// 原始极坐标
			else if (!type.compare(0, 5, "orign_coord", 5))
			{
				param["types"].push_back("orign_coord");
				device.LockCoord();
				for (auto& [length, angle] : device.polarCoord)
				{
					param["orign_coord"]["length"].push_back(length);
					param["orign_coord"]["angle"].push_back(angle);
				}
				device.UnlockCoord();
			}
			// 直角坐标
			else if (!type.compare(0, 5, "recta_coord", 5))
			{
				param["types"].push_back("recta_coord");
				device.LockCoord();
				for (auto& [x, y] : device.rectaCoord)
				{
					param["recta_coord"]["x"].push_back(x);
					param["recta_coord"]["y"].push_back(y);
				}
				device.UnlockCoord();
			}
			else
			{
				param["unknow_types"].push_back(type);
			}
		}
	}
	res.set_content(param.dump(), "application/json");
}

void connectDevice(const Request& req, Response& res)
{
	if (!req.has_param("ip"))
		return res.set_content(R"({"error": "Invalid parameter IP"})"_json.dump(), "application/json");
	auto ip = req.get_param_value("ip");

	for (auto& [cid, device]: LimDevice::DeviceList)
		if (device.deviceIP == ip && device.isConnected)
			return res.set_content(R"({"error": "Device is online"})"_json.dump(), "application/json");

	auto cid = LimDevice::OpenEquipment(ip.c_str()/*"192.168.1.210"*/);
	LimDevice::WaitFirstDeviceTryConnected();
	LimDevice::StartLMDData();

	json body;
	body["info"] = "Tried to connect device " + req.get_param_value("ip");
	res.set_content(body.dump(), "application/json");
}

void getDeviceInfo(const Request& req, Response& res)
{
	json param;
	if (req.has_param("online_number"))
		param["online_number"] = LimDevice::OnlineDeviceNumber;
	if (req.has_param("device_list"))
	{
		json deviceListParam;
		for (auto& [cid, device] : LimDevice::DeviceList)
		{
			deviceListParam["cid"] = cid;
			deviceListParam["ip"] = device.deviceIP;
			deviceListParam["online"] = device.isConnected.operator bool();
			param["device_list"].push_back(deviceListParam);
		}
	}
	res.set_content(param.dump(), "application/json");
}

int main()
{
	LimDevice::InitEquipment();

	server.bind_to_port("localhost", limServerPort);
	server.set_mount_point("/", ".");

	/*
	/distance
	/height
	/set
	/coord
	/info
	/connect
	/disconnect
	/shutdown
	/quit
	*/

	server.Get("/lidar/position", getPosition);
	server.Get("/lidar/connect", connectDevice);
	server.Get("/lidar/info", getDeviceInfo);
	server.Get("/quit", [](const Request&, Response&) {server.stop(); });


	server.listen_after_bind();
	return 0;
}
