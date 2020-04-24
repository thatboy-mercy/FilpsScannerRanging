#include "LimDevice.h"
#include <easyx.h>
#include <conio.h>
#include "fps.h"

#include <easyx.h>

namespace thatboy
{
	namespace EasyX
	{
		// EasyX Window Extent Properties 
		enum :int
		{
			EW_HASMAXIMIZI = 0X10    // Enable the maximize button
			, EW_HASSIZEBOX = 0X20   // Enable the sizebox
		};

		// 
		namespace _DataPackage {
			typedef LRESULT CALLBACK CallBackType(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			CallBackType* OLD_EasyXProc;
			LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
			{
				switch (msg)
				{
				case WM_SIZE:
					Resize(nullptr, LOWORD(lParam), HIWORD(lParam));
					break;
				default:
					return CallWindowProc(_DataPackage::OLD_EasyXProc, hwnd, msg, wParam, lParam);
				}
				return 0;
			}
		}


		HWND initgraph(int width, int height, int flag = NULL)	// Initialize the graphics environment.
		{
			HWND hEasyX = ::initgraph(width, height, flag & ~(EW_HASMAXIMIZI | EW_HASSIZEBOX));
			_DataPackage::OLD_EasyXProc = reinterpret_cast<_DataPackage::CallBackType*>(GetWindowLong(hEasyX, GWL_WNDPROC));
			SetWindowLong(hEasyX, GWL_WNDPROC, reinterpret_cast<LONG>(_DataPackage::WndProc));
			SetWindowLong(hEasyX, GWL_STYLE, GetWindowLong(hEasyX, GWL_STYLE) | (flag & EW_HASMAXIMIZI ? WS_MAXIMIZEBOX : 0) | (flag & EW_HASSIZEBOX ? WS_SIZEBOX : 0));
			return hEasyX;
		}
	}
}


int main()
{
	using namespace thatboy;
	LimDevice::InitEquipment();
	LimDevice::OpenEquipment("192.168.1.210");
	LimDevice::WaitFirstDeviceConnected();
	LimDevice::StartLMDData();

	EasyX::initgraph(900, 900, EasyX::EW_HASMAXIMIZI | EasyX::EW_HASSIZEBOX);
	BeginBatchDraw();
	setbkcolor(WHITE);
	setlinestyle(PS_SOLID, 1);
	settextcolor(BLACK);
	setbkmode(TRANSPARENT);

	bool bIfLine = true;
	bool bIfPoint = true;
	bool bIfUI = true;
	bool bIfInfo = true;
	bool bIfMeasure = true;
	bool bIfGrid = true;
	bool bIfArea = true;

	MOUSEMSG Msg = { NULL };
	POINT mousePos = { NULL };
	POINT originPos = { NULL };
	size_t shutDownGridnfoFrameCount = 0;

	originPos.x = getwidth() / 2;
	originPos.y = getheight() / 2;
	float scale = 1;
	setaspectratio(scale, scale);
	setorigin(originPos.x, originPos.y);

	FlushMouseMsgBuffer();

	auto& device = LimDevice::DeviceList.begin()->second;
	while (LimDevice::OnlineDeviceNumber > 0)
	{
		if (MouseHit())
		{
			bool bIfOriChanged = false;
			bool bIfScaleChanged = false;
			while (MouseHit())
			{
				Msg = GetMouseMsg();
				if (Msg.wheel != 0)
				{
					while (Msg.wheel > 0)
					{
						scale *= 1.1;
						Msg.wheel -= 120;
						if (scale > 400)
						{
							Msg.wheel = 0;
							scale = 400;
						}
					}
					while (Msg.wheel < 0)
					{
						scale *= 0.9;
						Msg.wheel += 120;
						if (scale < 0.000125)
						{
							Msg.wheel = 0;
							scale = 0.000125;
						}
					}
					bIfScaleChanged = true;
					bIfOriChanged = true;
				}
				if (Msg.uMsg == WM_LBUTTONDOWN)
				{
					mousePos.x = Msg.x;
					mousePos.y = Msg.y;
				}
				if (Msg.mkLButton)
				{
					originPos.x += Msg.x - mousePos.x;
					originPos.y += Msg.y - mousePos.y;
					mousePos.x = Msg.x;
					mousePos.y = Msg.y;
					bIfOriChanged = true;
				}
			}
			if (bIfScaleChanged)
				setaspectratio(scale, scale);
			if (bIfOriChanged)
				setorigin(originPos.x, originPos.y);
		}
		if (_kbhit())
		{
			switch (_getch())
			{
			case 'a':
			case 'A':
				bIfArea = !bIfArea;
				break;
			case 'i':
			case 'I':
				bIfInfo = !bIfInfo;
				break;
			case 'u':
			case 'U':
				bIfUI = !bIfUI;
				break;
			case 'l':
			case 'L':
				bIfLine = !bIfLine;
				break;
			case 'p':
			case 'P':
				bIfPoint = !bIfPoint;
				break;
			case 'g':
			case 'G':
				bIfGrid = !bIfGrid;
				break;
			case 'm':
			case 'M':
				bIfMeasure = !bIfMeasure;
				break;
			case 'r':
			case 'R':
				scale = 1;
				originPos.x = getwidth() / 2;
				originPos.y = getheight() / 2;
				setaspectratio(scale, scale);
				setorigin(originPos.x, originPos.y);
				break;
			case 'q':
			case 'Q':
				goto closeJmp;
			default:
				break;
			}
		}

		cleardevice();

		if (bIfUI)
		{
			// 扇形
			setlinecolor(0XD5C9C9);
			setfillcolor(0XD5C9C9);
			solidpie(-10000, -10000, 10000, 10000, device.angleBeg * LimDevice::pi / 180, device.angleEnd * LimDevice::pi / 180);

			// 网格
			if (bIfGrid)
			{
				setlinecolor(0XB4A8A8);
				for (int i = 0; i < 100; i++)
				{
					for (int j = 0; j < 100; j++)
					{
						line(-i * 100, -j * 100, i * 100, -j * 100);
						line(-i * 100, j * 100, i * 100, j * 100);
						line(-i * 100, -j * 100, -i * 100, j * 100);
						line(i * 100, -j * 100, i * 100, j * 100);
					}
				}
			}

			setlinecolor(0X423E3E);
			setlinestyle(PS_SOLID, 3);
			line(-10000, 0, 10000, 0);
			line(0, -10000, 0, 0);

		}
		device.LockCoord();
		if (bIfArea || bIfLine)
		{
			setlinecolor(0X00F000);
			setfillcolor(0XF37E31);
			if (bIfArea && !bIfLine)
				solidpolygon(device.drawPolyCoord.data(), device.drawPolyCoord.size());
			else if (!bIfArea && bIfLine)
				polygon(device.drawPolyCoord.data(), device.drawPolyCoord.size());
			else if (bIfArea && bIfLine)
				fillpolygon(device.drawPolyCoord.data(), device.drawPolyCoord.size());
		}
		// 中心点
		setfillcolor(RGB(0XF0, 0, 0));
		solidcircle(0, 0, 3);
		if (bIfPoint)
		{
			setfillcolor(RGB(0, 0, 0X80));
			for (const auto& pt : device.rectaCoord)
				solidcircle(pt.x, -pt.y, 1);
		}
		device.UnlockCoord();

		if (bIfMeasure)
		{
			// 测量
			setlinecolor(0XCC7A00);
#define TEXT_ARC(_len) \
			arc(-_len##00,-_len##00,_len##00,_len##00, device.angleBeg * LimDevice::pi / 180, device.angleEnd * LimDevice::pi / 180); \
			outtextxy(_len##00*cos(-device.angleBeg * LimDevice::pi / 180)-10, _len##00*sin(-device.angleBeg * LimDevice::pi / 180), L## #_len##"m");

			TEXT_ARC(2);
			TEXT_ARC(5);
			TEXT_ARC(10);
			TEXT_ARC(15);
			TEXT_ARC(20);
			TEXT_ARC(30);
			TEXT_ARC(50);
			TEXT_ARC(80);
			TEXT_ARC(100);
#undef TEXT_ARC
		}

		if (bIfInfo)
		{
			int fps = (int)::fps();
			setorigin(0, 0);
			setaspectratio(1, 1);
			setlinestyle(PS_SOLID, 4);
			setlinecolor(0X252525);
			setfillcolor(0XE0E0E0);

			fillrectangle(5, 5, textwidth(L"M - Show/Hide Measure") + 30, (textheight(' ') + 2)* (fps < 20 || shutDownGridnfoFrameCount > 0 ? 14 : 11) + 25);

			outtextxy(17, (textheight(' ') + 2) * 0 + 15, L"I - Show/Hide Info");
			outtextxy(17, (textheight(' ') + 2) * 1 + 15, L"P - Show/Hide Point");
			outtextxy(17, (textheight(' ') + 2) * 2 + 15, L"L - Show/Hide Line");
			outtextxy(17, (textheight(' ') + 2) * 3 + 15, L"A - Show/Hide Area");
			outtextxy(17, (textheight(' ') + 2) * 4 + 15, L"U - Show/Hide UI");
			outtextxy(17, (textheight(' ') + 2) * 5 + 15, L"G - Show/Hide Grid");
			outtextxy(17, (textheight(' ') + 2) * 6 + 15, L"M - Show/Hide Measure");
			outtextxy(17, (textheight(' ') + 2) * 7 + 15, L"R - Reset the view");
			outtextxy(17, (textheight(' ') + 2) * 8 + 15, (LimDevice::tstring(L"Scale: ") + to_tstring(scale)).c_str());
			outtextxy(17, (textheight(' ') + 2) * 9 + 15, (LimDevice::tstring(L"Angle: ") + to_tstring((int)device.angleBeg) + L'~' + to_tstring((int)device.angleEnd)).c_str());

			outtextxy(17, (textheight(' ') + 2) * 10 + 15, (LimDevice::tstring(L"FPS: ") + to_tstring(fps)).c_str());
			
			if (fps < 5)
			{
				bIfGrid = false;
				shutDownGridnfoFrameCount = 500;
			}
			if (shutDownGridnfoFrameCount > 0)
			{
				--shutDownGridnfoFrameCount;
				RECT rt{ 17,(textheight(' ') + 2) * 11 + 15 ,textwidth(L"M - Show/Hide Measure") + 22 ,(textheight(' ') + 2) * 14 + 25 };
				settextcolor(RED);
				drawtext(L"The fps is lower than 5, grid drawing has been turned off automatically", &rt, DT_LEFT | DT_WORDBREAK);
				settextcolor(BLACK);
			}
			else if (fps < 20)
			{
				RECT rt{ 17,(textheight(' ') + 2) * 11 + 15 ,textwidth(L"M - Show/Hide Measure") + 22 ,(textheight(' ') + 2) * 14 + 25 };
				settextcolor(RED);
				drawtext(L"The fps is lower than 20, recommend to turn off grid drawing by 'G'.", &rt, DT_LEFT | DT_WORDBREAK);
				settextcolor(BLACK);
			}


			if (Msg.mkRButton)
			{
				setlinestyle(PS_SOLID, 1);
				setlinecolor(YELLOW);
				line(originPos.x, originPos.y, Msg.x, Msg.y);
				outtextxy(Msg.x, Msg.y + 30, (to_tstring(sqrt((Msg.x - originPos.x)* (Msg.x - originPos.x) + (Msg.y - originPos.y) * (Msg.y - originPos.y)) / scale / 100) + LimDevice::tstring(L"m")).c_str());
				outtextxy(Msg.x, Msg.y + 60, (to_tstring(atan2(Msg.y - originPos.y, -Msg.x + originPos.x) * 180 / LimDevice::pi + 180) + LimDevice::tstring(L"°")).c_str());
			}

			setlinestyle(PS_SOLID, 1);
			setaspectratio(scale, scale);
			setorigin(originPos.x, originPos.y);
		}


		FlushBatchDraw();

		using namespace std::chrono;
		std::this_thread::sleep_for(5ms);
	}

closeJmp:
	EndBatchDraw();
	closegraph();
	return 0;
}

