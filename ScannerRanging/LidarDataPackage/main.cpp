#include <httplib.h>

#include "../LidarDataPackage/LimDevice.h"
#include "../LidarDataPackage/fps.h"
#include "../LidarDataPackage/EasyPX.h"

#include <conio.h>


int main()
{
	using namespace thatboy;
	LimDevice::InitEquipment();
	LimDevice::OpenEquipment("192.168.1.210");
	LimDevice::WaitFirstDeviceConnected();
	LimDevice::StartLMDData();

	EasyPX::initgraph(900, 900, EasyPX::EW_EXALLPROP);
	BeginBatchDraw();
	setbkcolor(WHITE);
	setlinestyle(PS_SOLID, 1);
	settextcolor(BLACK);
	setbkmode(TRANSPARENT);

	bool bIfLine = true;
	bool bIfPoint = true;
	bool bIfBg = true;
	bool bIfInfo = true;
	bool bIfMeasure = true;
	bool bIfGrid = true;
	bool bIfArea = true;
	bool bIfFixed = true;
	bool bIfFromInfoBox = false;

	MOUSEMSG Msg = { NULL };
	POINT mousePos = { NULL };
	POINT originPos = { NULL };
	size_t shutDownGridnfoFrameCount = 0;

	EasyPX::UI::InfoBox infoBox;

	originPos.x = getwidth() / 2;
	originPos.y = getheight() / 2;
	float scale = 1;
	setaspectratio(scale, scale);
	setorigin(originPos.x, originPos.y);

	infoBox.addInfo("I", "I - Info");
	infoBox.addInfo("P", "P - Point");
	infoBox.addInfo("L", "L - Line");
	infoBox.addInfo("A", "A - Area");
	infoBox.addInfo("B", "B - Background");
	infoBox.addInfo("G", "G - Grid");
	infoBox.addInfo("M", "M - Measure");
	infoBox.addInfo("F", "F - Fix the Info");
	infoBox.addInfo("R", "R - Reset the view");
	infoBox.addInfo(" ", " ");
	infoBox.addInfo("Scale", "Scale:");
	infoBox.addInfo("Angle", "Angle:");
	infoBox.addInfo("FPS", "FPS:");
	infoBox.addInfo("Borders", "Borders:");
	infoBox.addInfo("Len", "BorderContinusLen:");
	infoBox.addInfo("grid", "The fps is lower than 5, grid drawing has been turned off automatically:");
	infoBox.addInfo("fpsWarn", "The fps is lower than 20, recommend to turn off grid drawing by \'G\'.");

	infoBox.setArg("Scale", "1");
	infoBox.setArg("FPS", "0.00");

	infoBox.showInfo("grid", false);
	infoBox.showInfo("fpsWarn", false);

	infoBox.setInfoColor("grid", RED);
	infoBox.setInfoColor("fpsWarn", RED);

	infoBox.setArg("F", "Fixed");
	infoBox.setArg("I", "Show");
	infoBox.setArg("P", "Show");
	infoBox.setArg("L", "Show");
	infoBox.setArg("A", "Show");
	infoBox.setArg("B", "Show");
	infoBox.setArg("G", "Show");
	infoBox.setArg("M", "Show");

	infoBox.setArgColor("F", GREEN);
	infoBox.setArgColor("I", GREEN);
	infoBox.setArgColor("P", GREEN);
	infoBox.setArgColor("L", GREEN);
	infoBox.setArgColor("A", GREEN);
	infoBox.setArgColor("B", GREEN);
	infoBox.setArgColor("G", GREEN);
	infoBox.setArgColor("M", GREEN);

	auto& device = LimDevice::DeviceList.begin()->second;
	infoBox.setArg("Angle", to_string((int)device.angleBeg) + '~' + to_string((int)device.angleEnd));
	infoBox.setArg("Len", device.borderContinusLen);

	infoBox.x = 10;
	infoBox.y = 10;

	FlushMouseMsgBuffer();
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
					if (Msg.mkCtrl)
					{
						device.borderContinusLen += Msg.wheel / 120;
						if (device.borderContinusLen < 0)
							device.borderContinusLen = 0;
						else if (device.borderContinusLen > 1000)
							device.borderContinusLen = 1000;
						infoBox.setArg("Len", device.borderContinusLen);
					}
					else
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
				}
				if (Msg.uMsg == WM_LBUTTONDOWN || Msg.uMsg == WM_RBUTTONDOWN)
				{
					bIfFromInfoBox = !bIfFixed && (Msg.x > infoBox.x + originPos.x && Msg.y > infoBox.y + originPos.y && Msg.x < infoBox.x + originPos.x + infoBox.getInfoBoxWidth() && Msg.y < infoBox.y + originPos.y + infoBox.getInfoBoxHeight());
					mousePos.x = Msg.x;
					mousePos.y = Msg.y;
				}
				if (Msg.mkLButton)
				{
					if (bIfFromInfoBox)
					{
						infoBox.x += Msg.x - mousePos.x;
						infoBox.y += Msg.y - mousePos.y;
						mousePos.x = Msg.x;
						mousePos.y = Msg.y;
					}
					else
					{
						originPos.x += Msg.x - mousePos.x;
						originPos.y += Msg.y - mousePos.y;
						mousePos.x = Msg.x;
						mousePos.y = Msg.y;
						bIfOriChanged = true;
					}
				}
			}
			if (bIfScaleChanged)
			{
				setaspectratio(scale, scale);
				infoBox.setArg("Scale", scale, true, 0, 4);
			}
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
				infoBox.setArg("A", bIfArea ? "Show" : "Hide");
				infoBox.setArgColor("A", bIfArea ? GREEN : RED);
				break;
			case 'i':
			case 'I':
				bIfInfo = !bIfInfo;
				infoBox.setArg("I", bIfInfo ? "Show" : "Hide");
				infoBox.setArgColor("I", bIfInfo ? GREEN : RED);
				break;
			case 'b':
			case 'B':
				bIfBg = !bIfBg;
				infoBox.setArg("B", bIfBg ? "Show" : "Hide");
				infoBox.setArgColor("B", bIfBg ? GREEN : RED);
				break;
			case 'l':
			case 'L':
				bIfLine = !bIfLine;
				infoBox.setArg("L", bIfLine ? "Show" : "Hide");
				infoBox.setArgColor("L", bIfLine ? GREEN : RED);
				break;
			case 'p':
			case 'P':
				bIfPoint = !bIfPoint;
				infoBox.setArg("P", bIfPoint ? "Show" : "Hide");
				infoBox.setArgColor("P", bIfPoint ? GREEN : RED);
				break;
			case 'f':
			case 'F':
				bIfFixed = !bIfFixed;
				infoBox.setArg("F", bIfFixed ? "Fixed" : "Unfixed", true);
				infoBox.setArgColor("F", bIfFixed ? GREEN : RED);
				if (bIfFixed)
				{
					infoBox.x += originPos.x;
					infoBox.y += originPos.y;
				}
				else
				{
					infoBox.x -= originPos.x;
					infoBox.y -= originPos.y;
				}
				break;
			case 'g':
			case 'G':
				bIfGrid = !bIfGrid;
				infoBox.setArg("G", bIfGrid ? "Show" : "Hide");
				infoBox.setArgColor("G", bIfGrid ? GREEN : RED);
				break;
			case 'm':
			case 'M':
				bIfMeasure = !bIfMeasure;
				infoBox.setArg("M", bIfMeasure ? "Show" : "Hide");
				infoBox.setArgColor("M", bIfMeasure ? GREEN : RED);
				break;
			case 'r':
			case 'R':
				scale = 1;
				originPos.x = getwidth() / 2;
				originPos.y = getheight() / 2;
				setaspectratio(scale, scale);
				setorigin(originPos.x, originPos.y);
				infoBox.setArg("Scale", scale, true, 0, 4);
				break;
			case 'q':
			case 'Q':
				goto closeJmp;
			default:
				break;
			}
		}

		cleardevice();

		if (bIfBg)
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
			if (bIfArea)
				solidpolygon(device.drawPolyCoord.data(), device.drawPolyCoord.size());
			if (bIfLine)
			{
				for (const auto& border : device.borderStore)
				{
					moveto(border.front().x, -border.front().y);
					for (const auto& pt : border)
						lineto(pt.x, -pt.y);
				}
			}
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
			settextcolor(0X282828);
			setlinecolor(0XCC7A00);
#define TEXT_ARC(_len) \
			arc(-_len##00,-_len##00,_len##00,_len##00, device.angleBeg * LimDevice::pi / 180, device.angleEnd * LimDevice::pi / 180); \
			outtextxy(_len##00*cos(-device.angleBeg * LimDevice::pi / 180)-10, _len##00*sin(-device.angleBeg * LimDevice::pi / 180), TEXT(#_len##"m"));

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
			double fps = ::fps();

			infoBox.setArg("FPS", fps, false, 0, 4);
			infoBox.setArg("Borders", device.borderStore.size());

			if (fps < 5)
			{
				bIfGrid = false;
				infoBox.setArg("G", "Hide");
				infoBox.setArgColor("G", RED);
				infoBox.showInfo("grid", true);
				shutDownGridnfoFrameCount = 500;
			}
			if (shutDownGridnfoFrameCount > 0)
			{
				--shutDownGridnfoFrameCount;
				if (shutDownGridnfoFrameCount == 1)
					infoBox.showInfo("grid", false);
			}
			else
			{
				if (fps < 20)
				{
					infoBox.showInfo("fpsWarn", true);
				}
				else
				{
					infoBox.showInfo("fpsWarn", false);
				}
			}
			if (bIfFixed)
				infoBox.drawInfo(infoBox.x - originPos.x, infoBox.y - originPos.y);
			else
				infoBox.drawInfo();
			if (Msg.mkRButton)
			{
				setorigin(0, 0);
				setaspectratio(1, 1);
				settextcolor(BLACK);
				setlinestyle(PS_SOLID, 2);
				setlinecolor(MAGENTA);
				line(originPos.x, originPos.y, Msg.x, Msg.y);
				setlinestyle(PS_SOLID, 3);
				setlinecolor(YELLOW);
				line(mousePos.x, mousePos.y, Msg.x, Msg.y);
				outtextxy((Msg.x + mousePos.x) / 2, (Msg.y + mousePos.y) / 2, (to_string(sqrt((Msg.x - mousePos.x) * (Msg.x - mousePos.x) + (Msg.y - mousePos.y) * (Msg.y - mousePos.y)) / scale / 100) + LimDevice::tstring(TEXT("m"))).c_str());
				outtextxy((Msg.x + originPos.x) / 2, (Msg.y + originPos.y) / 2, (to_string(sqrt((Msg.x - originPos.x) * (Msg.x - originPos.x) + (Msg.y - originPos.y) * (Msg.y - originPos.y)) / scale / 100) + LimDevice::tstring(TEXT("m"))).c_str());
				outtextxy(Msg.x, Msg.y + 30, (to_string(atan2(Msg.y - originPos.y, -Msg.x + originPos.x) * 180 / LimDevice::pi + 180) + LimDevice::tstring(TEXT("°"))).c_str());
				setaspectratio(scale, scale);
				setorigin(originPos.x, originPos.y);
			}

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

