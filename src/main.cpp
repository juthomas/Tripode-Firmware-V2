#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <L3G.h>
#include <Button2.h>
#include <WiFi.h>
#include "jsonParser.h"
#include <sstream>

// #include <EEPROM.h>
// #define EEPROM_SIZE 512
#define LEFT_BTN 0
#define RIGHT_BTN 35

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
L3G gyro;
hw_timer_t *buttons_timer;
Button2 left_btn(LEFT_BTN);
Button2 right_btn(RIGHT_BTN);

typedef struct s_json_data
{
	IPAddress udp_target_ip;
	uint32_t udp_target_port;

	uint32_t udp_input_port;
	IPAddress osc_target_ip;

	uint32_t osc_target_port;

	std::string sta_ssid;
	std::string sta_pswd;
	std::string ap_ssid;
	std::string ap_pswd;

	std::string tripode_id;
} t_json_data;

enum e_json_data_types
{
	TYPE_IP,
	TYPE_STRING,
	TYPE_INTEGER,
};

typedef struct s_json_data_parser
{
	size_t offset;
	std::string name;
	e_json_data_types type;

} t_json_data_parser;

static const std::string json_data_parser_patch[] = {"upd_target_ip",
													 "upd_target_port",
													 "upd_input_port",
													 "osc_target_ip",
													 "osc_target_port",
													 "sta_ssid",
													 "sta_pswd",
													 "ap_ssid",
													 "ap_pswd",
													 "tripode_id"};

static const t_json_data_parser json_data_parser[] = {
	(t_json_data_parser){.offset = offsetof(t_json_data, udp_target_ip), .name = "udp_target_ip", .type = TYPE_IP},
	(t_json_data_parser){.offset = offsetof(t_json_data, udp_target_port), .name = "udp_target_port", .type = TYPE_INTEGER},
	(t_json_data_parser){.offset = offsetof(t_json_data, udp_input_port), .name = "udp_input_port", .type = TYPE_INTEGER},
	(t_json_data_parser){.offset = offsetof(t_json_data, osc_target_ip), .name = "osc_target_ip", .type = TYPE_IP},
	(t_json_data_parser){.offset = offsetof(t_json_data, osc_target_port), .name = "osc_target_port", .type = TYPE_INTEGER},
	(t_json_data_parser){.offset = offsetof(t_json_data, sta_ssid), .name = "sta_ssid", .type = TYPE_STRING},
	(t_json_data_parser){.offset = offsetof(t_json_data, sta_pswd), .name = "sta_pswd", .type = TYPE_STRING},
	(t_json_data_parser){.offset = offsetof(t_json_data, ap_ssid), .name = "ap_ssid", .type = TYPE_STRING},
	(t_json_data_parser){.offset = offsetof(t_json_data, ap_pswd), .name = "ap_pswd", .type = TYPE_STRING},
	(t_json_data_parser){.offset = offsetof(t_json_data, tripode_id), .name = "tripode_id", .type = TYPE_STRING}};

t_json_data json_data;

void IRAM_ATTR button_loop()
{
	left_btn.loop();
	right_btn.loop();
}

void IRAM_ATTR left_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.printf("Left btn clicked, clic type : %d", click_type);
}

void IRAM_ATTR right_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.printf("Right btn clicked, clic type : %d", click_type);
}

void display_home_page()
{
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.printf("Please selected your \nmode \n(with bottom buttons)");
	tft.setCursor(0, 230);
	tft.setTextColor(TFT_RED);
	tft.printf("AP mode");
	tft.setCursor(85, 230);
	tft.setTextColor(TFT_BLUE);
	tft.printf("STA mode");
	tft.setCursor(0, 0);
	tft.setTextColor(TFT_BLACK);
}

void error_msg(std::string message)
{
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.setTextSize(2);
	tft.setRotation(3);
	tft.setTextColor(TFT_RED);
	tft.printf("[ERROR] ");
	tft.setTextColor(TFT_WHITE);
	tft.printf("%s", message.c_str());
	Serial.printf("[ERROR] %s\n", message.c_str());
	tft.setTextSize(1);
	tft.setRotation(0);
}

void setup_ap()
{
	WiFi.mode(WIFI_AP);
}

void init_basic_functions()
{
	/* SERIAL INIT */
	Serial.begin(115200);

	/* SCREEN INIT */
	tft.init();
	tft.setRotation(0);
	tft.fillScreen(TFT_BLACK);
	if (TFT_BL > 0)
	{											// TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
		pinMode(TFT_BL, OUTPUT);				// Set backlight pin to output mode
		digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
	}
	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(0, 0);
	tft.setTextDatum(MC_DATUM);

	/* FILESYSTEM INIT */
	if (!SPIFFS.begin())
	{
		error_msg("Mounting SPIFFS");
	}

	/* SENSORS INIT */
	Wire.begin();
	if (!gyro.init())
	{
		error_msg("Autodetect gyro type");
		while (1)
			;
	}
	gyro.enableDefault();

	/* BUTTONS INIT */
	buttons_timer = timerBegin(3, 80, true);
	timerAttachInterrupt(buttons_timer, &button_loop, false);
	timerAlarmWrite(buttons_timer, 50 * 1000, true);
	timerAlarmEnable(buttons_timer);
	right_btn.setClickHandler(right_btn_handler);
	right_btn.setLongClickHandler(right_btn_handler);
	right_btn.setDoubleClickHandler(right_btn_handler);
	right_btn.setTripleClickHandler(right_btn_handler);
	left_btn.setClickHandler(left_btn_handler);
	left_btn.setLongClickHandler(left_btn_handler);
	left_btn.setDoubleClickHandler(left_btn_handler);
	left_btn.setTripleClickHandler(left_btn_handler);
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

namespace patch
{
	template <typename T>
	std::string to_string(const T &n)
	{
		std::ostringstream stm;
		stm << n;
		return stm.str();
	}
}

void print_json_data()
{
	Serial.printf("upd_target_ip : %s\n", json_data.udp_target_ip.toString().c_str());
	Serial.printf("upd_target_port : %d\n", json_data.udp_target_port);
	Serial.printf("upd_input_port : %d\n", json_data.udp_input_port);
	Serial.printf("osc_target_ip : %s\n", json_data.osc_target_ip.toString().c_str());
	Serial.printf("osc_target_port : %d\n", json_data.osc_target_port);
	Serial.printf("sta_ssid : %s\n", json_data.sta_ssid.c_str());
	Serial.printf("sta_pswd : %s\n", json_data.sta_pswd.c_str());
	Serial.printf("ap_ssid : %s\n", json_data.ap_ssid.c_str());
	Serial.printf("ap_pswd : %s\n", json_data.ap_pswd.c_str());
	Serial.printf("tripode_id : %s\n", json_data.tripode_id.c_str());
}

void load_spiffs(char *filePath = "/data.json")
{
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

	StaticJsonDocument<400> doc;

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
			*tmp_addr = IPAddress(get_octet((const char *)doc[json_data_parser_patch[i]], 1),
								  get_octet((const char *)doc[json_data_parser_patch[i]], 2),
								  get_octet((const char *)doc[json_data_parser_patch[i]], 3),
								  get_octet((const char *)doc[json_data_parser_patch[i]], 4));
		}
		break;
		case TYPE_INTEGER:
		{
			uint32_t *tmp_addr = (uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			*tmp_addr = (const uint32_t)doc[json_data_parser_patch[i]];
		}
		break;
		case TYPE_STRING:
		default:
		{
			std::string *tmp_addr = (std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
			*tmp_addr = (std::string)patch::to_string((const char *)doc[json_data_parser_patch[i].c_str()]);
		}
		break;
		}
	}
	free(buff);
	file.close();
}

void update_spiffs(char *filePath = "/data.json")
{
	StaticJsonDocument<400> doc;

	for (uint16_t i = 0; i < sizeof(json_data_parser) / sizeof(t_json_data_parser); i++)
	{
		switch (json_data_parser[i].type)
		{
		case TYPE_IP:
		{

			doc[json_data_parser_patch[i]] = ((IPAddress *)((uint32_t)&json_data + json_data_parser[i].offset))->toString().c_str();
		}
		break;
		case TYPE_INTEGER:
		{
			doc[json_data_parser_patch[i]] = *(uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			// uint32_t *tmp_addr = (uint32_t *)((uint32_t)&json_data + json_data_parser[i].offset);
			// *tmp_addr = (const uint32_t)doc[json_data_parser_patch[i]];
		}
		break;
		case TYPE_STRING:
		default:
		{
			doc[json_data_parser_patch[i]] = *(std::string *)((uint32_t)&json_data + json_data_parser[i].offset);
		}
		break;
		}
	}

	std::string buff;
	serializeJsonPretty(doc, buff);
	fs::File file = SPIFFS.open(filePath, "w");
	file.print(buff.c_str());
	file.close();
}

void setup()
{
	init_basic_functions();
	display_home_page();

	StaticJsonDocument<200> doc;
	char json[] =
		"{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[{\"id\":\"42\",\"value\":\"33\"} ,{\"id\":\"43\",\"value\":\"34\"}]}";
	// Deserialize the JSON document
	DeserializationError error = deserializeJson(doc, json);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
		return;
	}

	const char *sensor = doc["sensor"];
	long time = doc["time"];
	double latitude = doc["data"][0]["id"];
	double longitude = doc["data"][0]["value"];

	// Print values.
	Serial.println(doc["data"].size());
	Serial.println(latitude, 6);
	Serial.println(longitude, 6);

	load_spiffs();
	update_spiffs();
}

void loop()
{
	// put your main code here, to run repeatedly:
}