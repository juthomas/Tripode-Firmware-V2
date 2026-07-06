#include "tripodes.h"

static String serial_line;

void execute_signals(t_sensors *sensors)
{
	IPAddress udpIp = get_udp_target_ip();
	IPAddress oscIp = get_osc_target_ip();

	for (const auto &signal : signal_data)
	{
		if (signal.type == "udp" && udp_sending)
		{
			std::string payload = expand_signal_placeholders(signal.value, sensors);
			sendUpdMessage(payload.c_str(), udpIp, json_data.udp_target_port);
		}
		else if (signal.type == "osc" && osc_sending)
			execute_osc_signal(signal.value, sensors, oscIp, json_data.osc_target_port);
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
