#include "tripodes.h"

void onMessage(AsyncWebSocketClient *client, uint8_t *data)
{
    client->text("I got your text message");
}

void send_json_to_client(AsyncWebSocketClient *client, uint8_t *data)
{

    Serial.printf("Json for client : %s\n", serialize_json_data().c_str());
    client->text(serialize_json_data().c_str());
    update_spiffs();
}

void parse_json_from_client(uint8_t *data)
{
    StaticJsonDocument<JSON_SIZE> doc;

    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        error_msg(error.c_str());
        return;
    }

    JsonObject documentRoot = doc.as<JsonObject>();

    if (doc.containsKey("signals"))
    {
        signal_data.clear();
        for (uint16_t i = 0; i < doc["signals"].size(); i++)
        {
            signal_data.push_back((t_signal_data){.value = patch::to_string(doc["signals"][i]["value"].as<const char *>()),
                                                  .type = patch::to_string(doc["signals"][i]["type"].as<const char *>())

            });
        }
        for (std::vector<t_signal_data>::size_type i = 0; i != signal_data.size(); i++)
        {
            Serial.printf("Value : %s, type:%s\n", signal_data[i].value.c_str(), signal_data[i].type.c_str());
        }
    }

    for (JsonPair keyValue : documentRoot)
    {
        // Serial.printf("%s:%s", keyValue.key().c_str(), keyValue.value().as<const char *>());
        for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
        {
            Serial.printf("--Key to store : '%s'\n", keyValue.key().c_str());
            Serial.printf("--Cmp to store : '%s'\n", json_data_parser[i].name.c_str());

            if (strcmp(json_data_parser[i].name.c_str(), keyValue.key().c_str()) == 0)
            {
                Serial.printf("--Value to store : %s\n", (std::string)patch::to_string((const char *)keyValue.value()).c_str());
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
                case TYPE_STRING:
                default:
                {
                    std::string *tmp_addr = (std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
                    *tmp_addr = (std::string)patch::to_string((const char *)keyValue.value());
                }
                break;
                }
            }
        }
    }
    print_json_data();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

    if (type == WS_EVT_CONNECT)
    {
        // client connected
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        // client->printf("Hello Client %u :)", client->id());
        send_json_to_client(client, data);
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        // client disconnected
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        // error was received from the other end
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        // pong message was received (in response to a ping request maybe)
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        // data packet
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len)
        {
            // the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
            if (info->opcode == WS_TEXT)
            {
                data[len] = 0;
                Serial.printf("%s\n", (char *)data);
                parse_json_from_client(data);
                send_json_to_client(client, data);
            }
            else
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    Serial.printf("%02x ", data[i]);
                }
                Serial.printf("\n");
            }
            // if (info->opcode == WS_TEXT)
            //     client->text("I got your text message");
            // else
            //     client->binary("I got your binary message");
        }
        else
        {
            // message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0)
            {
                if (info->num == 0)
                    Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);
            if (info->message_opcode == WS_TEXT)
            {
                data[len] = 0;
                Serial.printf("%s\n", (char *)data);
                parse_json_from_client(data);

                send_json_to_client(client, data);
            }
            else
            {
                for (size_t i = 0; i < len; i++)
                {
                    Serial.printf("%02x ", data[i]);
                }
                Serial.printf("\n");
            }

            if ((info->index + len) == info->len)
            {
                Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final)
                {
                    Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    // if (info->message_opcode == WS_TEXT)
                    //     client->text("I got your text message");
                    // else
                    //     client->binary("I got your binary message");
                }
            }
        }
    }
}
