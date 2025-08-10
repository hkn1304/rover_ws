#include <string>
#include <vector>
#include <iostream>
#include <cstdio>

// OS Specific sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial/serial.h"
#include "nlohmann/json.hpp"
// #include "geometry_msgs::msg::TwistStamped"

using namespace std;

void enumerate_ports()
{
	vector<serial::PortInfo> devices_found = serial::list_ports();

	vector<serial::PortInfo>::iterator iter = devices_found.begin();

	while( iter != devices_found.end() )
	{
		serial::PortInfo device = *iter++;

		printf( "(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
     device.hardware_id.c_str() );
	}
}

int main(int argv, char* argc)
{
// unsigned long baud = 0;
// string port = 0;
// // port, baudrate, timeout in milliseconds
// serial::Serial my_serial(port, baud, serial::Timeout::simpleTimeout(1000));
 enumerate_ports();

}


// bool RobotController::SendCmdVel(geometry_msgs::msg::TwistStamped::SharedPtr msg){
//     nlohmann::json vel_json = {};
//     float l = 0, r = 0;

//     double clamped_value = std::max(-0.5, std::min((double)msg->linear.x, 0.5));
//     float x = static_cast<float>(clamped_value);

//     double clamped_value = std::max(-0.5, std::min((double)msg->angular.z, 0.5));
//     float z = static_cast<float>(clamped_value);

//     vel_json["T"] = 1;
//     vel_json["L"] = (int) l;
//     vel_json["R"] = (int) r;

//     // When running backwards, to rotation must be inverted
//     if(x > 0) {
//         vel_json["L"] = (int)r;
//         vel_json["R"] = (int)l;
//     }

//     size_t bytes_wrote = my_serial.write(vel_json.dump() );
// }

