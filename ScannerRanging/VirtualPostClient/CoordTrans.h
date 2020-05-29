#pragma once
#include "LidarDevice.h"

struct VerticalCoord
{
	double y;
	double h;
};

template<typename T>
struct LockVector
	: public std::vector<T>
	, public std::mutex
{};


/// <summary>
/// 坐标转换
/// </summary>
struct CoordTrans
{
	using AutoLock = std::lock_guard<std::mutex>;

	static constexpr double MaxLength = 5000.;
	RawData rawData;
	LockVector<VerticalCoord> legalCoord;
	int legalρ[RAW_DATA_SIZE];
	LockVector<int> hist;
	int beginY;
	int endY;

	/// <summary>
	/// 角度到下标转化
	/// </summary>
	/// <param name="angle">角度</param>
	/// <returns>下标</returns>
	static constexpr int angle2Index(double angle)
	{
		return (angle - (-45)) * 2;
	}
	static constexpr double index2Angle(int index)
	{
		return index / 2. - 45;
	}


	/// <summary>
	/// 处理坐标点
	/// </summary>
	/// <param name="_rawData"> 原始数据 </param>
	void ProcessCoord(const RawData& _rawData)
	{
		rawData = _rawData;

		double angleBeg = rawData.angleBeg;
		double angleEnd = rawData.angleEnd;
		// 
		if (angleBeg > 90 || angleEnd < 90)
			return;

		// 移除异常坐标
		for (size_t i = 0; i < RAW_DATA_SIZE; i++)
		{
			if (rawData.ρ[i] <= 0 || rawData.ρ[i] >= MaxLength)
				legalρ[i] = -1;
			else
				legalρ[i] = rawData.ρ[i];
		}

		AutoLock lock1(legalCoord);
		legalCoord.clear();
		// 转换直角坐标，排除异常的位置
		{

			double lastY = 0;
			for (int i = angle2Index(90); i < angle2Index(180); i++)
			{
				auto& length = legalρ[i];
				if (length < 0|| length > MaxLength)
					continue;
				auto angle = index2Angle(i);

				VerticalCoord thisCoord{ length * cos(angle / 180 * PI), length * sin(angle / 180 * PI) };
				if (thisCoord.y < lastY)
				{
					lastY = thisCoord.y;
					legalCoord.push_back(thisCoord);
				}
			}
			std::reverse(legalCoord.begin(), legalCoord.end());
			lastY = 0;
			for (int i = angle2Index(90) - 1; i >= angle2Index(0); i--)
			{
				auto& length = legalρ[i];
				if (length < 0 || length > MaxLength)
					continue;
				auto angle = index2Angle(i);

				VerticalCoord thisCoord{ length * cos(angle / 180 * PI), length * sin(angle / 180 * PI) };
				if (thisCoord.y > lastY)
				{
					lastY = thisCoord.y;
					legalCoord.push_back(thisCoord);
				}
			}
		}

		// 插值
		AutoLock lock2(hist);
		hist.clear();
		beginY = std::lround(legalCoord.front().y);
		endY = std::lround(legalCoord.back().y);
		int lastY = beginY;
		int lastH = std::lround(legalCoord.front().h);
		for (const auto& [y, h] : legalCoord)
		{
			int nextY = std::lround(y);
			int nextH = std::lround(h);
			for (int i = lastY; i < nextY; i++)
			{
				hist.push_back(i > (lastY + nextY) / 2 ? nextH : lastH);
			}
			lastY = nextY;
			lastH = nextH;
		}
	}
};

