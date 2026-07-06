#include "tripodes.h"

void sendOscFloatMessage(const char *oscPrefix, float oscMessage, const IPAddress ipOut, const uint32_t portOut)
{
	char tmpOscPrefix[64];
	snprintf(tmpOscPrefix, sizeof(tmpOscPrefix), "%s_%s", oscPrefix, json_data.tripode_id.c_str());
	OSCMessage msg(tmpOscPrefix);
	msg.add(oscMessage);
	Udp.beginPacket(ipOut, portOut);
	msg.send(Udp);
	Udp.endPacket();
	msg.empty();
}

void send_sensor_osc(float dfa_value, t_sensors *sensors)
{
	IPAddress ipOut = get_osc_target_ip();
	uint32_t portOut = json_data.osc_target_port;

	sendOscFloatMessage("/dfa", dfa_value, ipOut, portOut);
	sendOscFloatMessage("/accelerometer_x", sensors->accel.x, ipOut, portOut);
	sendOscFloatMessage("/accelerometer_y", sensors->accel.y, ipOut, portOut);
	sendOscFloatMessage("/accelerometer_z", sensors->accel.z, ipOut, portOut);
	sendOscFloatMessage("/accelerometer_normal", (float)sqrt(sensors->accel.x * sensors->accel.x + sensors->accel.y * sensors->accel.y + sensors->accel.z * sensors->accel.z), ipOut, portOut);
	sendOscFloatMessage("/gyroscope_x", sensors->gyro.x, ipOut, portOut);
	sendOscFloatMessage("/gyroscope_y", sensors->gyro.y, ipOut, portOut);
	sendOscFloatMessage("/gyroscope_z", sensors->gyro.z, ipOut, portOut);
	sendOscFloatMessage("/gyroscope_normal", (float)sqrt(sensors->gyro.x * sensors->gyro.x + sensors->gyro.y * sensors->gyro.y + sensors->gyro.z * sensors->gyro.z), ipOut, portOut);
	sendOscFloatMessage("/magnetometer_x", sensors->mag.x, ipOut, portOut);
	sendOscFloatMessage("/magnetometer_y", sensors->mag.y, ipOut, portOut);
	sendOscFloatMessage("/magnetometer_z", sensors->mag.z, ipOut, portOut);
}
