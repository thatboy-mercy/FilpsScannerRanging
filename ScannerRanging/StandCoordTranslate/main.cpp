
//#include "RadarMapping.h"

#include <chrono>
#include <httplib.h>
#include "Config.h"
#include "../LidarDataPackage/EquipmentComm/EquipmentComm.h"
#include "../LidarDataPackage/LIM/lim.h"
#include "../../../Logger/Logger/Logger.hpp"
#include <json.hpp>

enum { CID_DATA = 0XAA, CID_DISTANCE };
bool dataDeviceOnline = false;
bool distanceDeviceOnline = false;
std::string dataDeviceIP;
std::string distanceDeviceIP;
clock_t dataSendTimeSpan;
clock_t deviceConnectTimeOut;
int mapXAccuracy;
constexpr auto PropertiesFilePath = "Application.properties";
thatboy::logger::FileLogger logger("lidar.log");

std::string serverHost;
std::string serverPath;
int serverPort;
// 数据
std::vector<int> lidarPolar;
int lidarBegin;
int lidarEnd;
int lidarX;
std::mutex lidarPolarLock;
std::mutex lidarXLock;

#define LOG_INFO(...) logger.log(thatboy::logger::LogLevel::Info, ##__VA_ARGS__);
#define LOG_WARNING(...) logger.log(thatboy::logger::LogLevel::Warning, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__);
#define LOG_ERROR(...) logger.log(thatboy::logger::LogLevel::Error, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__);
#define LOG_FATAL(...) logger.log(thatboy::logger::LogLevel::Fatal, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__);

void CALLBACK onDataCallBack(INT _cid, UINT _lim_code, LPVOID _lim, INT _lim_len, INT _paddr);
void CALLBACK onStateCallBack(INT _cid, UINT _state_code, LPCSTR _ip, INT _port, INT _paddr);
bool waitDevicesOnline(int timeOutMs);
void startDevices();
inline bool devicesOnline();

int main()
{
	using thatboy::logger::LogLevel;
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
		std::string serverUrl;
		Config cfg(PropertiesFilePath);
		cfg.ReadInto(dataDeviceIP, "DataDeviceIp");
		cfg.ReadInto(distanceDeviceIP, "DistanceDeviceIp");
		cfg.ReadInto(dataSendTimeSpan, "DataSendTimeSpan");
		cfg.ReadInto(deviceConnectTimeOut, "DeviceConnectTimeOut");
		cfg.ReadInto(mapXAccuracy, "MapXAccuracy");
		cfg.ReadInto(serverUrl, "ServerUrl");

		auto postIpMark = serverUrl.find_first_of(':');
		auto postPortMark = serverUrl.find_first_of('/');
		serverHost = serverUrl.substr(0, postIpMark);
		serverPath = serverUrl.substr(postPortMark);
		serverPort = stoi(serverUrl.substr(postIpMark + 1, postPortMark));
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
		, " DataDevice<", dataDeviceIP
		, "> DistanceDevice<", distanceDeviceIP
		, "> DataSendTimeSpan<", dataSendTimeSpan
		, "> DeviceConnectTimeOut<", deviceConnectTimeOut
		, "> MapXAccuracy<", mapXAccuracy, ">.");

	// 初始化并连接设备
	if (!EquipmentCommInit(NULL, onDataCallBack, onStateCallBack))
		return INIT_FAILED;
	LOG_INFO("Device library initialization complete.");
	OpenEquipmentComm(CID_DATA, dataDeviceIP.c_str(), 2112);
	OpenEquipmentComm(CID_DISTANCE, distanceDeviceIP.c_str(), 2112);
	if (!waitDevicesOnline(deviceConnectTimeOut))
	{
		LOG_FATAL("Failed to connect devices. Timeout!");
		LOG_INFO("Devices Status: "
			, "DataDevice "	, dataDeviceOnline ? "Online" : "Offline"
			, "; DistanceDevice ", distanceDeviceOnline ? "Online" : "Offline", ".");
		return CONNECTION_TIMEOUT;
	}
	LOG_INFO("Devices online.");


	// 启动
	startDevices();
	LOG_INFO("Devices started.");

	httplib::Client client(serverHost, serverPort);

	while (devicesOnline())
	{
		nlohmann::json data;
		lidarXLock.lock();
		lidarPolarLock.lock();
		// 保存数据
		data["x"] = lidarX;
		data["begin_angle"] = lidarBegin;
		data["end_angle"] = lidarEnd;
		for (const auto& polar : lidarPolar)
			data["polar"].push_back(polar);
		lidarPolarLock.unlock();
		lidarXLock.unlock();

		auto ret = client.Post(serverPath.c_str(), data.dump(), "application/json");
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

		std::this_thread::sleep_for(std::chrono::milliseconds(dataSendTimeSpan));
	}
	LOG_INFO("Devices offline.");
	LOG_INFO("Devices Status: "
		, "DataDevice ", dataDeviceOnline ? "Online" : "Offline"
		, "; DistanceDevice ", distanceDeviceOnline ? "Online" : "Offline", ".");

	LOG_INFO("Application closed normally.");
	return 0;
}

void CALLBACK onDataCallBack(INT _cid, UINT _lim_code, LPVOID _lim, INT _lim_len, INT _paddr)
{
	auto& lim = *static_cast<LIM_HEAD*>(_lim);

	if (LIM_CheckSum(&lim) != lim.CheckSum)
		return;
	switch (lim.nCode)
	{
	case LIM_CODE_LMD:
		switch (lim.nCID)
		{
		case CID_DATA:
		{
			// 保存极坐标数组
			LMD_INFO& lmd_info = *LMD_Info(&lim);
			LMD_D_Type* lmd = LMD_D(&lim);
			std::lock_guard<std::mutex> lockGuard(lidarPolarLock);
			lidarBegin = lmd_info.nBAngle;
			lidarEnd = lmd_info.nEAngle;
			if (lidarPolar.size() != lmd_info.nMDataNum)
				lidarPolar.resize(lmd_info.nMDataNum);
			for (int i = 0; i < lmd_info.nMDataNum; i++)
				lidarPolar[i] = lmd[i];
		}
			break;
		case CID_DISTANCE:
		{
			// 保存90°距离
			LMD_INFO& lmd_info = *LMD_Info(&lim);
			LMD_D_Type* lmd = LMD_D(&lim);
			std::lock_guard<std::mutex> lockGuard(lidarXLock);
			if (lmd_info.nBAngle <= 9000 || lmd_info.nEAngle >= 9000)
				lidarX = lmd[(90 - lmd_info.nBAngle) * 2] / 10 * 10;
			else
				lidarX = -10000;
		}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CALLBACK onStateCallBack(INT _cid, UINT _state_code, LPCSTR _ip, INT _port, INT _paddr)
{
	switch (_state_code)
	{
	case EQCOMM_STATE_OK:
	{
		LIM_HEAD* lim = NULL;
		LIM_Pack(lim, _cid, LIM_CODE_GET_LDBCONFIG, NULL);
		SendLIM(_cid, lim, lim->nLIMLen);
		LIM_Release(lim);

		if (_cid == CID_DATA)
			dataDeviceOnline = true;
		else if (_cid == CID_DISTANCE)
			distanceDeviceOnline = true;
	}
	break;
	case EQCOMM_STATE_ERR:
	case EQCOMM_STATE_LOST:
	{
		if (_cid == CID_DATA)
			dataDeviceOnline = false;
		else if (_cid == CID_DISTANCE)
			distanceDeviceOnline = false;
	}
	break;
	default:
		break;
	}
}


bool waitDevicesOnline(int timeOutMs)
{
	using namespace std::chrono;
	auto in{ steady_clock::now() };
	auto duration = std::chrono::milliseconds(timeOutMs);

	while (!dataDeviceOnline && !distanceDeviceOnline)
	{
		std::this_thread::sleep_for(10ms);
		if (steady_clock::now() - in > duration)
			return false;
	}
	return true;
}

void startDevices()
{
	LIM_HEAD* lim = nullptr;
	LIM_Pack(lim, CID_DATA, LIM_CODE_START_LMD);
	SendLIM(CID_DATA, lim, lim->nLIMLen);
	LIM_Release(lim);
	lim = nullptr;
	LIM_Pack(lim, CID_DISTANCE, LIM_CODE_START_LMD);
	SendLIM(CID_DISTANCE, lim, lim->nLIMLen);
	LIM_Release(lim);
}

inline bool devicesOnline()
{
	return dataDeviceOnline && distanceDeviceOnline;
}
