#pragma once

#ifndef _LIM_DEVICE_H_
#define _LIM_DEVICE_H_

#include <string>
#include <mutex>
#include <map>
#include <atomic>
#include <vector>
#include <algorithm>
#include <cmath>

#include "../LidarDataPackage/EquipmentComm/EquipmentComm.h"
#include "../LidarDataPackage/LIM/lim.h"

constexpr size_t RAW_DATA_SIZE = 541;
constexpr double PI = 3.141592654;

struct RawData
	: public std::mutex
{
	int nNumber;
	double angleBeg;
	double angleEnd;
	int ¦Ñ[RAW_DATA_SIZE];

	RawData& operator=(const RawData&raw)
	{
		std::lock_guard<std::mutex> lock(*this);
		nNumber = raw.nNumber;
		angleBeg = raw.angleBeg;
		angleEnd = raw.angleEnd;
		for (size_t i = 0; i < RAW_DATA_SIZE; i++)
			¦Ñ[i] = raw.¦Ñ[i];
		return *this;
	}
};


struct LidarDevice
{
	static constexpr int angle2Index(double angle)
	{
		return (angle - (-45)) * 2;
	}
	static constexpr double index2Angle(int index)
	{
		return index / 2. - 45;
	}

public:
	using AutoLock = std::lock_guard<std::mutex>;

	enum : DWORD
	{
		STATUS_DEFAULT = 0,
		STATUS_CONNECTING,
		STATUS_LOST,
		STATUS_CLOSE,
	};
public:
	/// Êý¾Ý
	RawData rawData;
	DWORD status = 0;

	std::string deviceIP;

	inline static std::mutex staticDataLock;
	inline static int OnlineDeviceNumber = 0;
	inline static int SerialCID = 0x80;
	inline static int initedDeviceNum = 0;
	inline static std::map<int, LidarDevice> DeviceList;

	void LockRawData() { rawData.lock(); }
	void UnlockRawData() { rawData.unlock(); }


	~LidarDevice();

	static void InitEquipment();
	static int OpenEquipment(LPCSTR ip, int port = LIM_USER_PORT);
	static void WaitFirstDeviceTryConnected();
	static void StartLMDData();

	int getFacingDistance();
protected:
	int cid = 0XFFFFFFF;
	void onLMDRecive(LIM_HEAD& lim);
	void onDeviceQuit();
	static void CALLBACK onDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr);
	static void CALLBACK onStateCallBack(int _cid, unsigned int _state_code, const char* _ip, int _port, int _paddr);
};

#endif // !_LIM_DEVICE_H_
