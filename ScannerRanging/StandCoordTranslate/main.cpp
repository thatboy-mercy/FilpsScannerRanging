
//#include "RadarMapping.h"

#include <chrono>
#include <httplib>
#include "../LidarDataPackage/EquipmentComm/EquipmentComm.h"
#include "../LidarDataPackage/LIM/lim.h"

enum { CID_DATA = 0XAA, CID_DISTANCE };
bool dataDeviceOnline = false;
bool distanceDeviceOnline = false;
std::string dataDeviceIP;
std::string distanceDeviceIP;

void CALLBACK onDataCallBack(INT _cid, UINT _lim_code, LPVOID _lim, INT _lim_len, INT _paddr);
void CALLBACK onStateCallBack(INT _cid, UINT _state_code, LPCSTR _ip, INT _port, INT _paddr);
bool waitDeviceOnline(int timeOutMs);



int main()
{
	// 初始化并连接设备
	EquipmentCommInit(NULL, onDataCallBack, onStateCallBack);
	OpenEquipmentComm(CID_DATA, dataDeviceIP.c_str(), 2112);
	OpenEquipmentComm(CID_DISTANCE, distanceDeviceIP.c_str(), 2112);
	waitDeviceOnline(5000);




	return 0;
}

void CALLBACK onDataCallBack(INT _cid, UINT _lim_code, LPVOID _lim, INT _lim_len, INT _paddr)
{
	LIM_HEAD& lim = *(LIM_HEAD*)_lim;

	if (LIM_CheckSum(&lim) != lim.CheckSum)
		return;
	switch (lim.nCode)
	{
	case LIM_CODE_LMD:
		//
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

bool waitDeviceOnline(int timeOutMs)

{
	using namespace std::chrono;
	auto in{ system_clock::now() };
	auto duration = std::chrono::milliseconds(timeOutMs);

	while (!dataDeviceOnline && !distanceDeviceOnline)
	{
		std::this_thread::sleep_for(10ms);
		if (system_clock::now() - in > duration)
			return false;
	}
	return true;
}
