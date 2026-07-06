#include "tripodes.h"

static String serial_line;

std::string expand_signal_placeholders(const std::string &value, float dfa_value, t_sensors *sensors)
{
	std::string result = value;
	auto replace_token = [&](const char *token, const std::string &replacement) {
		size_t pos = 0;
		while ((pos = result.find(token, pos)) != std::string::npos)
		{
			result.replace(pos, strlen(token), replacement);
			pos += replacement.length();
		}
	};

	replace_token("{gx}", patch::to_string((int)sensors->gyro.x));
	replace_token("{gy}", patch::to_string((int)sensors->gyro.y));
	replace_token("{gz}", patch::to_string((int)sensors->gyro.z));
	replace_token("{ax}", patch::to_string(sensors->accel.x));
	replace_token("{ay}", patch::to_string(sensors->accel.y));
	replace_token("{az}", patch::to_string(sensors->accel.z));
	replace_token("{dfa}", patch::to_string(dfa_value));
	return result;
}

void execute_signals(float dfa_value, t_sensors *sensors)
{
	IPAddress udpIp = get_udp_target_ip();
	IPAddress oscIp = get_osc_target_ip();

	for (const auto &signal : signal_data)
	{
		std::string payload = expand_signal_placeholders(signal.value, dfa_value, sensors);
		if (signal.type == "udp" && udp_sending)
			sendUpdMessage(payload.c_str(), udpIp, json_data.udp_target_port);
		else if (signal.type == "osc" && osc_sending)
			sendOscFloatMessage(payload.c_str(), dfa_value, oscIp, json_data.osc_target_port);
	}

	if (udp_sending)
	{
		static uint16_t loop_counter = 0;
		if (loop_counter % UPD_MESSAGE_RATE == 0)
		{
			sendUpdMessage("select:0;0", udpIp, json_data.udp_target_port);
			sendUdpXYZ(sensors->gyro, udpIp, json_data.udp_target_port);
		}
		loop_counter = loop_counter > 1000 ? 0 : loop_counter + 1;
	}
}

void poll_serial_config()
{
	while (Serial.available())
	{
		char c = (char)Serial.read();
		if (c == '\n' || c == '\r')
		{
			if (serial_line.length() > 0)
			{
				Serial.printf("[Serial config] %s\n", serial_line.c_str());
				parse_json_from_client((uint8_t *)serial_line.c_str());
				update_spiffs();
				dump_json_to_serial();
				serial_line = "";
			}
		}
		else
		{
			serial_line += c;
			if (serial_line.length() > JSON_SIZE - 1)
				serial_line = "";
		}
	}
}
