#include "tripodes.h"

static const char *WEBCFG_PREFIX = "WEBCFG:";
static const char *WEBCFG_READY = "WEBCFG:READY";

static String serial_line;
static bool webcfg_ready_sent = false;

void emit_webcfg_ready_once()
{
	if (!webcfg_ready_sent)
	{
		Serial.println(WEBCFG_READY);
		webcfg_ready_sent = true;
	}
}

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
				const char *payload = serial_line.c_str();
				if (strncmp(payload, WEBCFG_PREFIX, strlen(WEBCFG_PREFIX)) == 0)
					payload += strlen(WEBCFG_PREFIX);
				Serial.printf("[Serial config] %s\n", payload);
				parse_json_from_client((uint8_t *)payload);
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
