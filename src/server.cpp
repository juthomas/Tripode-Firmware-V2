#include "tripodes.h"
#include <vector>

void parse_json_from_client(uint8_t *data)
{
	StaticJsonDocument<JSON_SIZE> doc;
	DeserializationError error = deserializeJson(doc, data, JSON_SIZE);
	if (error)
	{
		Serial.printf("[ERROR] %s\n", error.c_str());
		return;
	}

	if (doc.containsKey("signals"))
	{
		signal_data.clear();
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

	if (doc.containsKey("settings"))
	{
		JsonObject settings = doc["settings"].as<JsonObject>();
		for (JsonPair keyValue : settings)
		{
			for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
			{
				if (strcmp(json_data_parser[i].name.c_str(), keyValue.key().c_str()) != 0)
					continue;
				switch (json_data_parser[i].type)
				{
				case TYPE_IP:
				{
					IPAddress *tmp_addr = (IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = IPAddress(get_octet((const char *)keyValue.value(), 1),
										  get_octet((const char *)keyValue.value(), 2),
										  get_octet((const char *)keyValue.value(), 3),
										  get_octet((const char *)keyValue.value(), 4));
				}
				break;
				case TYPE_INTEGER:
				{
					uint32_t *tmp_addr = (uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = (const uint32_t)keyValue.value();
				}
				break;
				case TYPE_SIGNED:
				{
					int32_t *tmp_addr = (int32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = keyValue.value().as<int32_t>();
				}
				break;
				case TYPE_STRING:
				default:
				{
					std::string *tmp_addr = (std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = patch::to_string((const char *)keyValue.value());
				}
				break;
				}
			}
		}
	}
	else
	{
		for (JsonPair keyValue : doc.as<JsonObject>())
		{
			if (strcmp(keyValue.key().c_str(), "signals") == 0)
				continue;
			for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
			{
				if (strcmp(json_data_parser[i].name.c_str(), keyValue.key().c_str()) != 0)
					continue;
				switch (json_data_parser[i].type)
				{
				case TYPE_IP:
				{
					IPAddress *tmp_addr = (IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = IPAddress(get_octet((const char *)keyValue.value(), 1),
										  get_octet((const char *)keyValue.value(), 2),
										  get_octet((const char *)keyValue.value(), 3),
										  get_octet((const char *)keyValue.value(), 4));
				}
				break;
				case TYPE_INTEGER:
				{
					uint32_t *tmp_addr = (uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = (const uint32_t)keyValue.value();
				}
				break;
				case TYPE_SIGNED:
				{
					int32_t *tmp_addr = (int32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = keyValue.value().as<int32_t>();
				}
				break;
				case TYPE_STRING:
				default:
				{
					std::string *tmp_addr = (std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
					*tmp_addr = patch::to_string((const char *)keyValue.value());
				}
				break;
				}
			}
		}
	}
	print_json_data();
	json_data.signal_poll_ms = get_signal_poll_interval_ms();
	invalidate_target_cache();
	refresh_target_cache();
	invalidate_wifi_qr_screen();
}

static void send_json_to_client(AsyncWebSocketClient *client)
{
	bool saved = update_spiffs();
	std::string payload = serialize_json_data(true, saved, true);
	client->text(payload.c_str());
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	if (type == WS_EVT_CONNECT)
	{
		Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		send_json_to_client(client);
	}
	else if (type == WS_EVT_DISCONNECT)
	{
		Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
	}
	else if (type == WS_EVT_ERROR)
	{
		Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
	}
	else if (type == WS_EVT_DATA)
	{
		AwsFrameInfo *info = (AwsFrameInfo *)arg;
		if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
		{
			std::vector<uint8_t> msg(len + 1);
			memcpy(msg.data(), data, len);
			msg[len] = 0;
			parse_json_from_client(msg.data());
			send_json_to_client(client);
		}
	}
}
