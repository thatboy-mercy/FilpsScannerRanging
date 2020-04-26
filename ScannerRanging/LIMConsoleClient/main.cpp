#include <httplib.h>

#include "../LidarDataPackage/LimDevice.h"
#include <sstream>
#include <iomanip>
#include <conio.h>

constexpr int limServerPort = 5056;

httplib::Server server;

template<typename T>
std::string fmtString(T in, int width = 0, int prec = 0) {
	std::ostringstream s;
	s << std::setw(width) << std::setprecision(prec) << in;
	return s.str();
}

void sendPolar(const httplib::Request&, httplib::Response&res)
{
	std::string body;
	auto& device = LimDevice::DeviceList.begin()->second;
	std::string points;

	body += "PolarCoord\r\n";
	body += device.deviceIP + "\r\n";
	body += fmtString(device.angleBeg)+"-"+ fmtString(device.angleEnd) + "\r\n";
	body += fmtString(device.polarCoord.size()) + "\r\n{\r\n";

	device.LockCoord();
	for (auto& polar : device.polarCoord)
		body += "\t{" + fmtString(polar.angle) + "," + fmtString(polar.length) + "}\r\n";
	device.UnlockCoord();
	body += "}\r\n";

	res.set_content(body, "text/plain");
}

void sendRecta(const httplib::Request&, httplib::Response&res)
{
	std::string body;
	auto& device = LimDevice::DeviceList.begin()->second;
	std::string points;

	body += "RectaCoord\r\n";
	body += device.deviceIP + "\r\n";
	body += fmtString(device.angleBeg) + "-" + fmtString(device.angleEnd) + "\r\n";
	body += fmtString(device.rectaCoord.size()) + "\r\n{\r\n";

	device.LockCoord();
	for (auto& polar : device.rectaCoord)
		body += "\t{" + fmtString(polar.x) + "," + fmtString(polar.y) + "}\r\n";
	device.UnlockCoord();
	body += "}\r\n";

	res.set_content(body, "text/plain");
}

void quit(const httplib::Request&, httplib::Response&)
{
	server.stop();
}

int main()
{
	LimDevice::InitEquipment();
	LimDevice::OpenEquipment("192.168.1.210");
	LimDevice::WaitFirstDeviceConnected();
	LimDevice::StartLMDData();

	server.bind_to_port("localhost", limServerPort);
	server.set_base_dir(".");
	server.Get("/lidar/polarcoord", sendPolar);
	server.Get("/lidar/rectacoord", sendRecta);
	server.Get("/quit", quit);
	server.listen_after_bind();
	server.listen_after_bind();
	return 0;
}
