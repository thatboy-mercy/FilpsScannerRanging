#pragma once

#ifndef _LIM_DEVICE_H_
#define _LIM_DEVICE_H_

#include <string>
#include <mutex>
#include <map>
#include <atomic>
#include <vector>
#include <algorithm>

#include "EquipmentComm/EquipmentComm.h"
#include "LIM/lim.h"

struct LimDevice
{
#ifdef UNICODE
	using tstring = std::wstring;
#	ifndef to_tstring
#	define to_tstring std::to_wstring
#	endif
#else
	using tstring = std::string;
#	ifndef to_string
#	define to_string std::to_string
#	endif
#endif // UNICODE

public:
	constexpr static double pi = 3.141592654;
	struct PolarCoord
	{
		double length;
		double angle;
	};
	struct RectaCoord
	{
		double x;
		double y;
	};

	/// 数据
	bool isTryConnected = false;
	std::atomic_bool isConnected = false;
	std::string deviceIP;
	double angleBeg = -45;
	double angleEnd = 225;
	double borderContinusLen = 100;
	size_t BorderLeastNumber = 3;

	std::vector<POINT> drawPolyCoord;
	std::vector<PolarCoord> polarCoord;
	std::vector<RectaCoord> rectaCoord;
	std::vector<std::vector<RectaCoord>> borderStore;

	inline static std::mutex staticDataLock;
	inline static int OnlineDeviceNumber = 0;
	inline static int SerialCID = 0x80;
	inline static bool hasDeviceTryConnected = false;
	inline static std::map<int, LimDevice> DeviceList;

	void LockCoord() { coordLock.lock(); }
	void UnlockCoord() { coordLock.unlock(); }
	void SetBorderContinusLen(double len) { borderContinusLen = len; }
	void SetBorderLeastNumber(size_t num) { BorderLeastNumber = num; }

	~LimDevice()
	{
		onDeviceQuit();
	}

	static void InitEquipment()
	{
		EquipmentCommInit(NULL, onDataCallBack, onStateCallBack); // 初始化设备库
	}
	static void OpenEquipment(LPCSTR ip, int port = LIM_USER_PORT)
	{
		OpenEquipmentComm(SerialCID, ip, port);
		staticDataLock.lock();
		++SerialCID;
		staticDataLock.unlock();
	}
	static void WaitFirstDeviceConnected()
	{
		using namespace std::chrono;
		while (!hasDeviceTryConnected)
			std::this_thread::sleep_for(10ms);
	}
	static void StartLMDData()
	{
		for (auto& device : DeviceList)
		{
			if (device.second.isConnected)
			{
				LIM_HEAD* lim = NULL;
				LIM_Pack(lim, device.first, LIM_CODE_START_LMD); //Start LMD LIM transferring.
				SendLIM(device.first, lim, lim->nLIMLen);
				LIM_Release(lim);
			}
		}
	}

protected:
	std::mutex coordLock;
	int cid = 0XFFFFFFF;

	void calcRectaCoord()
	{
		size_t indexWrite = 0;
		for (size_t indexRead = 0; indexRead < polarCoord.size(); indexRead++)
		{
			if (polarCoord[indexRead].length > 900)
				continue;
			if (indexWrite >= rectaCoord.size())
			{
				rectaCoord.resize(indexWrite + 1);
				drawPolyCoord.resize(indexWrite + 2);
			}
			rectaCoord[indexWrite].x = polarCoord[indexRead].length * cos(polarCoord[indexRead].angle / 180 * pi);
			rectaCoord[indexWrite].y = polarCoord[indexRead].length * sin(polarCoord[indexRead].angle / 180 * pi);
			drawPolyCoord[indexWrite + 1].x = rectaCoord[indexWrite].x;
			drawPolyCoord[indexWrite + 1].y = -rectaCoord[indexWrite].y;
			indexWrite++;
		}
		if (rectaCoord.size() != indexWrite || drawPolyCoord.size() != indexWrite + 1)
		{
			rectaCoord.resize(indexWrite);
			drawPolyCoord.resize(indexWrite + 1);
		}
		// Border
		double borderContinusLen2 = borderContinusLen * borderContinusLen;
		borderStore.clear();
		std::vector<RectaCoord> border;
		for (const auto& pt : rectaCoord)
		{
			if (border.empty())
				border.push_back(pt);
			else
			{
				auto& last = border.back();
				if ((pt.x - last.x) * (pt.x - last.x) + (pt.y - last.y) * (pt.y - last.y) > borderContinusLen2)
				{
					if (border.size() >= BorderLeastNumber)
						borderStore.push_back(border);
					border.clear();
				}
				border.push_back(pt);
			}
		}
		if (border.size() >= BorderLeastNumber)
			borderStore.push_back(border);
	}

	void onLMDRecive(LIM_HEAD& lim)
	{
		if (lim.nCode != LIM_CODE_LMD)
			return;
		LMD_INFO& lmd_info = *LMD_Info(&lim);
		LMD_D_Type* lmd = LMD_D(&lim);

		angleBeg = lmd_info.nBAngle / 1000.;
		angleEnd = lmd_info.nEAngle / 1000.;

		std::lock_guard<std::mutex> lockGuard(coordLock);
		if (polarCoord.size() != lmd_info.nMDataNum)
			polarCoord.resize(lmd_info.nMDataNum);
		for (int i = 0; i < lmd_info.nMDataNum; i++)
		{
			polarCoord[i].angle = static_cast<double>((lmd_info.nBAngle + i * (float)(lmd_info.nEAngle - lmd_info.nBAngle) / (lmd_info.nMDataNum - 1)) / 1000.0);
			polarCoord[i].length = static_cast<double>(lmd[i]);
		}
		calcRectaCoord();
	}

	void onDeviceQuit()
	{
		if (isTryConnected)
		{
			if (isConnected)
			{
				LIM_HEAD* lim = NULL;
				LIM_Pack(lim, cid, LIM_CODE_IOSET_RELEASE, NULL);
				SendLIM(cid, lim, lim->nLIMLen);
				LIM_Release(lim);

				LIM_Pack(lim, cid, LIM_CODE_STOP_LMD, NULL);
				SendLIM(cid, lim, lim->nLIMLen);
				LIM_Release(lim);

				isConnected = false;
			}
			CloseEquipmentComm(cid);
		}
	}
public:

	static void CALLBACK onDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr)
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

	static void CALLBACK onStateCallBack(int _cid, unsigned int _state_code, const char* _ip, int _port, int _paddr)
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

			DeviceList[_cid].polarCoord.reserve(541);
			DeviceList[_cid].rectaCoord.reserve(541);
			DeviceList[_cid].drawPolyCoord.reserve(542);
			DeviceList[_cid].deviceIP = _ip;
			DeviceList[_cid].isConnected = true;
			DeviceList[_cid].cid = _cid;
		}
		break;
		case EQCOMM_STATE_ERR:
		case EQCOMM_STATE_LOST:
			staticDataLock.lock();
			if (DeviceList[_cid].isConnected)
				DeviceList[_cid].isConnected = false;
			--OnlineDeviceNumber;
			staticDataLock.unlock();
			break;
		default:
			break;
		}
		DeviceList[_cid].isTryConnected = true;
		hasDeviceTryConnected = true;
	}
};

#endif // !_LIM_DEVICE_H_

