#include "LidarDevice.h"

int LidarDevice::getFacingDistance()
{
	if (rawData.angleBeg <= 9000 || rawData.angleEnd >= 9000)
		return rawData.ρ[angle2Index(90)];
	else
		return -10000;
}

void LidarDevice::onLMDRecive(LIM_HEAD& lim)
{
	if (lim.nCode != LIM_CODE_LMD)
		return;
	LMD_INFO& lmd_info = *LMD_Info(&lim);
	LMD_D_Type* lmd = LMD_D(&lim);

	std::lock_guard<std::mutex> lockGuard(rawData);
	rawData.nNumber = lmd_info.nMDataNum;
	rawData.angleBeg = lmd_info.nBAngle / 1000.;
	rawData.angleEnd = lmd_info.nEAngle / 1000.;
	for (int i = 0; i < lmd_info.nMDataNum; i++)
		rawData.ρ[angle2Index(rawData.angleBeg) + i] = lmd[i];
}

void LidarDevice::onDeviceQuit()
{
	if (status != STATUS_DEFAULT && status != STATUS_CLOSE)
	{
		if (status == STATUS_CONNECTING)
		{
			LIM_HEAD* lim = NULL;
			LIM_Pack(lim, cid, LIM_CODE_IOSET_RELEASE, NULL);
			SendLIM(cid, lim, lim->nLIMLen);
			LIM_Release(lim);

			LIM_Pack(lim, cid, LIM_CODE_STOP_LMD, NULL);
			SendLIM(cid, lim, lim->nLIMLen);
			LIM_Release(lim);

			status = STATUS_LOST;
		}
		CloseEquipmentComm(cid);
		status = STATUS_CLOSE;
	}
}

LidarDevice::~LidarDevice()
{
	onDeviceQuit();
}

void LidarDevice::InitEquipment()
{
	EquipmentCommInit(NULL, onDataCallBack, onStateCallBack); // 初始化设备库
}

int LidarDevice::OpenEquipment(LPCSTR ip, int port)
{
	OpenEquipmentComm(SerialCID, ip, port);
	AutoLock lock(staticDataLock);
	++SerialCID;
	return SerialCID - 1;
}

void LidarDevice::WaitFirstDeviceTryConnected()
{
	using namespace std::chrono;
	while (!initedDeviceNum)
		std::this_thread::sleep_for(10ms);
}

void LidarDevice::StartLMDData()
{
	for (auto& device : DeviceList)
	{
		if (device.second.status == STATUS_CONNECTING)
		{
			LIM_HEAD* lim = NULL;
			LIM_Pack(lim, device.first, LIM_CODE_START_LMD); //Start LMD LIM transferring.
			SendLIM(device.first, lim, lim->nLIMLen);
			LIM_Release(lim);
		}
	}
}

void CALLBACK LidarDevice::onDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr)
{
	LIM_HEAD& lim = *(LIM_HEAD*)_lim;

	if (LIM_CheckSum(&lim) != lim.CheckSum)
		return;
	switch (lim.nCode)
	{
	case LIM_CODE_LMD:
		DeviceList[_cid].onLMDRecive(lim);
		break;
	case LIM_CODE_LMD_RSSI:
		break;
	default:
		break;
	}
}

void CALLBACK LidarDevice::onStateCallBack(int _cid, unsigned int _state_code, const char* _ip, int _port, int _paddr)
{
	switch (_state_code)
	{
	case EQCOMM_STATE_OK:
	{
		LIM_HEAD* lim = NULL;
		LIM_Pack(lim, _cid, LIM_CODE_GET_LDBCONFIG, NULL);
		SendLIM(_cid, lim, lim->nLIMLen);
		LIM_Release(lim);

		staticDataLock.lock();
		++OnlineDeviceNumber;
		staticDataLock.unlock();

		DeviceList[_cid].deviceIP = _ip;
		DeviceList[_cid].cid = _cid;
		DeviceList[_cid].status = STATUS_CONNECTING;
		++initedDeviceNum;
	}
	break;
	case EQCOMM_STATE_ERR:
	case EQCOMM_STATE_LOST:
	{
		AutoLock lock(staticDataLock);
		if (DeviceList[_cid].status == STATUS_CONNECTING)
			DeviceList[_cid].status = STATUS_LOST;
		--OnlineDeviceNumber;
		staticDataLock.unlock();
		++initedDeviceNum;
	}
		break;
	default:
		break;
	}
}