
//#include "RadarMapping.h"

#include <chrono>
#include <httplib.h>
#include <json.hpp>
#include "RadarMapping.hpp"


#define LOG_INFO(...) Radar::logger.log(thatboy::logger::LogLevel::Info, ##__VA_ARGS__);
#define LOG_WARNING(...) Radar::logger.log(thatboy::logger::LogLevel::Warning, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__);
#define LOG_ERROR(...) Radar::logger.log(thatboy::logger::LogLevel::Error, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__);
#define LOG_FATAL(...) Radar::logger.log(thatboy::logger::LogLevel::Fatal, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__);


int main()
{
	// 退出码
	enum: int {
		SUCCESS = 0
		, FILE_NOTFOUND
		, KEY_NOTFOUND
		, CONNECTION_TIMEOUT
		, INIT_FAILED
	};

	// 程序启动
	LOG_INFO("Application started.");

	// 加载配置文件
	try
	{
		Radar::loadProperites();
	}
	catch (Config::File_not_found e)
	{
		LOG_FATAL("Load Properties Failed! ", e.filename, " NOT FOUND!");
		return FILE_NOTFOUND;
	}
	catch (Config::Key_not_found e)
	{
		LOG_ERROR("Load Properties Failed! ", "KEY ", e.key, " NOT FOUND!");
		return KEY_NOTFOUND;
	}
	LOG_INFO("Properties: "
		, " DataDevice<", Radar::dataDeviceIP
		, "> DistanceDevice<", Radar::distanceDeviceIP
		, "> DataSendTimeSpan<", Radar::dataSendTimeSpan
		, "> DeviceConnectTimeOut<", Radar::deviceConnectTimeOut
		, "> MapXAccuracy<", Radar::mapXAccuracy, ">.");

	// 初始化并连接设备
	if (!EquipmentCommInit(NULL, Radar::onDataCallBack, Radar::onStateCallBack))
		return INIT_FAILED;
	LOG_INFO("Device library initialization complete.");
	OpenEquipmentComm(Radar::CID_DATA, Radar::dataDeviceIP.c_str(), 2112);
	OpenEquipmentComm(Radar::CID_DISTANCE, Radar::distanceDeviceIP.c_str(), 2112);
	if (!Radar::waitDevicesOnline(Radar::deviceConnectTimeOut))
	{
		LOG_FATAL("Failed to connect devices. Timeout!");
		LOG_INFO("Devices Status: "
			, "DataDevice "	, Radar::dataDeviceOnline ? "Online" : "Offline"
			, "; DistanceDevice ", Radar::distanceDeviceOnline ? "Online" : "Offline", ".");
		return CONNECTION_TIMEOUT;
	}
	LOG_INFO("Devices online.");


	// 启动
	Radar::startDevices();
	LOG_INFO("Devices started.");

	httplib::Client client(Radar::serverHost, Radar::serverPort);

	while (Radar::devicesOnline())
	{
		nlohmann::json data;
		Radar::lidarXLock.lock();
		Radar::lidarPolarLock.lock();
		// 保存数据
		data["x"] = Radar::lidarX;
		data["begin_angle"] = Radar::lidarBegin;
		data["end_angle"] = Radar::lidarEnd;
		for (const auto& polar : Radar::lidarPolar)
			data["polar"].push_back(polar);
		Radar::lidarPolarLock.unlock();
		Radar::lidarXLock.unlock();

		auto ret = client.Post(Radar::serverPath.c_str(), data.dump(), "application/json");
		if (!ret)
		{
			LOG_FATAL("Failed to connect server. Timeout!");
		}
		else if (ret->status != 200)
		{
			LOG_ERROR("Server Code ", ret->status, " body: {", ret->body, "}");
		}
		else
		{
			LOG_INFO("Send data to server successed. body: {", ret->body, "}");
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(Radar::dataSendTimeSpan));
	}
	LOG_INFO("Devices offline.");
	LOG_INFO("Devices Status: "
		, "DataDevice ", Radar::dataDeviceOnline ? "Online" : "Offline"
		, "; DistanceDevice ", Radar::distanceDeviceOnline ? "Online" : "Offline", ".");

	LOG_INFO("Application closed normally.");
	return 0;
}
