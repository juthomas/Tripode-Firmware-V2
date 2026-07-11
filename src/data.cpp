#include "tripodes.h"

void write_default_config()
{
	json_data.udp_target_ip = "192.168.4.2";
	json_data.udp_target_port = 49160;
	json_data.udp_input_port = 49141;
	json_data.osc_target_ip = "192.168.4.2";
	json_data.osc_target_port = 2002;
	json_data.sta_ssid = "tripodesAP";
	json_data.sta_pswd = "44448888";
	json_data.ap_ssid = "tripodesAP";
	json_data.ap_pswd = "44448888";
	json_data.tripode_id = "1_1";
	json_data.fractal_state_pos_x = 0;
	json_data.fractal_state_pos_y = 10;
	json_data.glyph_pos_x = 48;
	json_data.glyph_pos_y = 6;
	json_data.gyro_norm_min = 0;
	json_data.gyro_norm_max = 35;
	json_data.accel_norm_min = 0;
	json_data.accel_norm_max = 35;
	json_data.mag_norm_min = 0;
	json_data.mag_norm_max = 35;
	json_data.signal_poll_ms = 25;

	signal_data.clear();
	signal_data.push_back({.value = "write:;#0D300I255P1;12;12", .type = "udp", .enabled = true});
	signal_data.push_back({.value = "select:0;0", .type = "udp", .enabled = true});
	signal_data.push_back({.value = "write:1V{G:X}2V{G:Y}3V{G:Z};0;0", .type = "udp", .enabled = true});
	signal_data.push_back({.value = "/gyroscope_x:{G:X}", .type = "osc", .enabled = true});
	signal_data.push_back({.value = "/gyroscope_y:{G:Y}", .type = "osc", .enabled = true});
	signal_data.push_back({.value = "/gyroscope_z:{G:Z}", .type = "osc", .enabled = true});
	update_spiffs();
	Serial.println("[SPIFFS] Created default /data.json");
}

void dump_json_to_serial()
{
	Serial.println("[SPIFFS] Current configuration:");
	Serial.println(serialize_json_data().c_str());
}

void print_json_data()
{
	Serial.printf("upd_target_ip : %s\n", json_data.udp_target_ip.c_str());
	Serial.printf("upd_target_port : %d\n", json_data.udp_target_port);
	Serial.printf("upd_input_port : %d\n", json_data.udp_input_port);
	Serial.printf("osc_target_ip : %s\n", json_data.osc_target_ip.c_str());
	Serial.printf("osc_target_port : %d\n", json_data.osc_target_port);
	Serial.printf("sta_ssid : %s\n", json_data.sta_ssid.c_str());
	Serial.printf("sta_pswd : %s\n", json_data.sta_pswd.c_str());
	Serial.printf("ap_ssid : %s\n", json_data.ap_ssid.c_str());
	Serial.printf("ap_pswd : %s\n", json_data.ap_pswd.c_str());
	Serial.printf("tripode_id : %s\n", json_data.tripode_id.c_str());
}

uint8_t get_octet(const char *str, uint8_t n)
{
	size_t i = 0;
	uint8_t n_number = 0;
	uint8_t last_octet = 0;
	while (str[i])
	{
		if (i == 0 || str[i - 1] == '.')
			last_octet = atoi(&str[i]);
		if (str[i] == '.')
			n_number++;
		i++;
		if (n_number == n)
			return last_octet;
	}
	return last_octet;
}

static void trim_string(std::string &value)
{
	while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r' || value.back() == '\n'))
		value.pop_back();

	size_t start = 0;
	while (start < value.size() && (value[start] == ' ' || value[start] == '\t'))
		start += 1;

	if (start > 0)
		value.erase(0, start);
}

void trim_wifi_credentials()
{
	trim_string(json_data.sta_ssid);
	trim_string(json_data.sta_pswd);
	trim_string(json_data.ap_ssid);
	trim_string(json_data.ap_pswd);
}

static void apply_json_settings(JsonObject settings)
{
	for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
	{
		if (!settings.containsKey(json_data_parser_patch[i].c_str()))
			continue;

		switch (json_data_parser[i].type)
		{
		case TYPE_IP:
		{
			IPAddress *tmp_addr = (IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset);
			const char *ip = settings[json_data_parser_patch[i].c_str()];
			*tmp_addr = IPAddress(get_octet(ip, 1), get_octet(ip, 2), get_octet(ip, 3), get_octet(ip, 4));
		}
		break;
		case TYPE_INTEGER:
		{
			uint32_t *tmp_addr = (uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			*tmp_addr = settings[json_data_parser_patch[i].c_str()];
		}
		break;
		case TYPE_SIGNED:
		{
			int32_t *tmp_addr = (int32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			*tmp_addr = settings[json_data_parser_patch[i].c_str()].as<int32_t>();
		}
		break;
		case TYPE_STRING:
		default:
		{
			std::string *tmp_addr = (std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
			*tmp_addr = patch::to_string((const char *)settings[json_data_parser_patch[i].c_str()]);
		}
		break;
		}
	}
	trim_wifi_credentials();
}

uint32_t get_signal_poll_interval_ms()
{
	uint32_t ms = json_data.signal_poll_ms;
	if (ms < 5 || ms > 1000)
		return 25;
	return ms;
}

static void normalize_signal_poll_ms()
{
	json_data.signal_poll_ms = get_signal_poll_interval_ms();
}

void load_spiffs()
{
	const char *filePath = "/data.json";
	fs::File file = SPIFFS.open(filePath, "r");
	if (!file)
	{
		Serial.println("Failed to open /data.json - writing defaults");
		write_default_config();
		file = SPIFFS.open(filePath, "r");
		if (!file)
			return;
	}

	size_t buff_size = file.size();
	uint8_t *buff = (uint8_t *)malloc(sizeof(char) * (buff_size + 1));
	size_t read_index = file.read(buff, buff_size);
	buff[read_index] = '\0';
	file.close();

	Serial.printf("[SPIFFS]\n%s\n", buff);

	{
		StaticJsonDocument<JSON_SIZE> doc;
		DeserializationError error = deserializeJson(doc, buff);
		if (error)
		{
			error_msg(error.c_str());
			free(buff);
			return;
		}

		if (doc.containsKey("settings"))
			apply_json_settings(doc["settings"].as<JsonObject>());
		else
			apply_json_settings(doc.as<JsonObject>());

		signal_data.clear();
		if (doc.containsKey("signals"))
		{
			for (uint16_t i = 0; i < doc["signals"].size(); i++)
			{
				JsonObject sig = doc["signals"][i];
				bool enabled = !sig.containsKey("enabled") || sig["enabled"].as<bool>();
				signal_data.push_back({
					.value = patch::to_string(sig["value"].as<const char *>()),
					.type = patch::to_string(sig["type"].as<const char *>()),
					.enabled = enabled});
			}
		}
	}

	if (json_data.gyro_norm_min == 0 && json_data.gyro_norm_max == 0 &&
		json_data.accel_norm_min == 0 && json_data.accel_norm_max == 0 &&
		json_data.mag_norm_min == 0 && json_data.mag_norm_max == 0)
	{
		json_data.gyro_norm_max = 35;
		json_data.accel_norm_max = 35;
		json_data.mag_norm_max = 35;
	}

	normalize_signal_poll_ms();

	free(buff);
	dump_json_to_serial();
	invalidate_target_cache();
	refresh_target_cache();
}

std::string serialize_json_data(bool include_meta, bool meta_saved, bool compact)
{
	static StaticJsonDocument<JSON_SIZE> doc;
	doc.clear();

	for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
	{
		switch (json_data_parser[i].type)
		{
		case TYPE_IP:
			doc["settings"][json_data_parser_patch[i]] = ((IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset))->toString().c_str();
			break;
		case TYPE_INTEGER:
			doc["settings"][json_data_parser_patch[i]] = *(uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			break;
		case TYPE_SIGNED:
			doc["settings"][json_data_parser_patch[i]] = *(int32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			break;
		case TYPE_STRING:
		default:
			doc["settings"][json_data_parser_patch[i]] = *(std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
			break;
		}
	}

	for (std::vector<t_signal_data>::size_type i = 0; i != signal_data.size(); i++)
	{
		doc["signals"][i]["value"] = signal_data[i].value.c_str();
		doc["signals"][i]["type"] = signal_data[i].type.c_str();
		doc["signals"][i]["enabled"] = signal_data[i].enabled;
	}

	if (include_meta)
		doc["meta"]["saved"] = meta_saved;

	std::string buff;
	if (compact)
		serializeJson(doc, buff);
	else
		serializeJsonPretty(doc, buff);
	return buff;
}

bool update_spiffs()
{
	fs::File file = SPIFFS.open("/data.json", "w");
	if (!file)
	{
		Serial.println("[SPIFFS] Failed to open /data.json for writing");
		return false;
	}

	std::string content = serialize_json_data(false, false);
	size_t written = file.print(content.c_str());
	file.close();

	if (written != content.length())
	{
		Serial.println("[SPIFFS] Write to /data.json incomplete");
		return false;
	}

	return true;
}
