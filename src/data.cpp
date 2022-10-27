#include "tripodes.h"

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
        {
            last_octet = atoi(&str[i]);
        }
        if (str[i] == '.')
        {
            n_number++;
        }
        i++;
        if (n_number == n)
        {
            return (last_octet);
        }
    }
    return (last_octet);
}

void load_spiffs()
{
    char *filePath = "/data.json";
    fs::File file = SPIFFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }
    Serial.printf("File Size : %d\n", file.size());
    Serial.printf("Free Heap : %d\n", ESP.getFreeHeap());
    uint8_t *buff;
    size_t read_index = 1;
    size_t buff_size = file.size();
    buff = (uint8_t *)malloc(sizeof(char) * (buff_size + 1));
    if ((read_index = file.read(buff, (buff_size))) > 0)
    {
        buff[read_index] = '\0';
        Serial.printf("[SPIFFS]\n%s", buff);
    }
    else
    {
        error_msg("Error reading SPIFFS");
    }

    StaticJsonDocument<JSON_SIZE> doc;

    DeserializationError error = deserializeJson(doc, buff);
    if (error)
    {
        error_msg(error.c_str());
        return;
    }
    for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
    {
        switch (json_data_parser[i].type)
        {
        case TYPE_IP:
        {

            IPAddress *tmp_addr = (IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset);
            *tmp_addr = IPAddress(get_octet((const char *)doc["settings"][json_data_parser_patch[i]], 1),
                                  get_octet((const char *)doc["settings"][json_data_parser_patch[i]], 2),
                                  get_octet((const char *)doc["settings"][json_data_parser_patch[i]], 3),
                                  get_octet((const char *)doc["settings"][json_data_parser_patch[i]], 4));
        }
        break;
        case TYPE_INTEGER:
        {
            uint32_t *tmp_addr = (uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
            *tmp_addr = (const uint32_t)doc["settings"][json_data_parser_patch[i]];
        }
        break;
        case TYPE_STRING:
        default:
        {
            std::string *tmp_addr = (std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
            *tmp_addr = (std::string)patch::to_string((const char *)doc["settings"][json_data_parser_patch[i].c_str()]);
        }
        break;
        }
    }

    signal_data.clear();
    for (uint16_t i = 0; i < doc["signals"].size(); i++)
    {
        signal_data.push_back((t_signal_data){.value = patch::to_string(doc["signals"][i]["value"].as<const char*>()),
                                              .type =  patch::to_string(doc["signals"][i]["type"].as<const char*>())});
    }

    free(buff);
    file.close();

    for (std::vector<t_signal_data>::size_type i = 0; i != signal_data.size(); i++)
    {
        Serial.printf("Value : %s, type:%s\n", signal_data[i].value.c_str(), signal_data[i].type.c_str());
    }
}

std::string serialize_json_data()
{
    StaticJsonDocument<JSON_SIZE> doc;

    for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
    {
        switch (json_data_parser[i].type)
        {
        case TYPE_IP:
        {
            doc["settings"][json_data_parser_patch[i]] = ((IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset))->toString().c_str();
        }
        break;
        case TYPE_INTEGER:
        {
            doc["settings"][json_data_parser_patch[i]] = *(uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
        }
        break;
        case TYPE_STRING:
        default:
        {
            doc["settings"][json_data_parser_patch[i]] = *(std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
        }
        break;
        }
    }

    for (std::vector<t_signal_data>::size_type i = 0; i != signal_data.size(); i++)
    {
        doc["signals"][i]["value"] = signal_data[i].value.c_str();
        doc["signals"][i]["type"] = signal_data[i].type.c_str();
        // Serial.printf("Value : %s, type:%s\n", signal_data[i].value, signal_data[i].type);
    }
    std::string buff;
    serializeJsonPretty(doc, buff);
    return (buff);
}

void update_spiffs()
{
    char *filePath = "/data.json";
    fs::File file = SPIFFS.open(filePath, "w");
    file.print(serialize_json_data().c_str());
    file.close();
}
