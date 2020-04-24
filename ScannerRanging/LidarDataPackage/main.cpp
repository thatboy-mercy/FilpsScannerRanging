#include "LimDevice.h"
#include <easyx.h>
#include <conio.h>

int main()
{
	LimDevice::InitEquipment();
	LimDevice::OpenEquipment("192.168.1.210");
	LimDevice::WaitFirstDeviceConnected();
	LimDevice::StartLMDData();

	initgraph(900, 900);
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
					}
					while (Msg.wheel < 0)
					{
						scale *= 0.9;
						Msg.wheel += 120;
					}
					bIfScaleChanged = true;
					bIfOriChanged = true;
				}
				if (Msg.uMsg == WM_LBUTTONDOWN)
				{
					mousePos.x = Msg.x;
					mousePos.y = Msg.y;
				}
				else if (Msg.uMsg == WM_MBUTTONDOWN)
				{
					scale = 1;
					originPos.x = getwidth() / 2;
					originPos.y = getheight() / 2;
					setaspectratio(scale, scale);
					setorigin(originPos.x, originPos.y);
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
			setorigin(0, 0);
			setaspectratio(1, 1);
			setlinestyle(PS_SOLID, 4);
			setlinecolor(0X252525);
			setfillcolor(0XE0E0E0);
			fillrectangle(5, 5, textwidth(L"M - Show Measure") + 25, textheight(' ') * 8 + 25);
			outtextxy(15, textheight(' ') * 0 + 15, L"I - Show Info");
			outtextxy(15, textheight(' ') * 1 + 15, L"P - Show Point");
			outtextxy(15, textheight(' ') * 2 + 15, L"L - Show Line");
			outtextxy(15, textheight(' ') * 3 + 15, L"A - Show Area");
			outtextxy(15, textheight(' ') * 4 + 15, L"U - Show UI");
			outtextxy(15, textheight(' ') * 5 + 15, L"M - Show Measure");

			outtextxy(15, textheight(' ') * 6 + 15, (LimDevice::tstring(L"Scale: ") + to_string(scale)).c_str());
			outtextxy(15, textheight(' ') * 7 + 15, (LimDevice::tstring(L"Angle: ") + to_string((int)device.angleBeg) + L'~' + to_string((int)device.angleEnd)).c_str());

			setlinestyle(PS_SOLID, 1);
			setaspectratio(scale, scale);
			setorigin(originPos.x, originPos.y);
		}


		FlushBatchDraw();

		using namespace std::chrono;
		std::this_thread::sleep_for(100ms);
	}

closeJmp:
	EndBatchDraw();
	closegraph();
	return 0;
}

