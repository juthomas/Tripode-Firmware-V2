
#ifndef TRIPODES_H
#define TRIPODES_H
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <L3G.h>
#include <Button2.h>
#include <WiFi.h>
#include "jsonParser.h"
#include <ESPAsyncWebServer.h>

#include <sstream>

/* SPIFFS Parsing*/

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

/* WiFi & Server */
void setup_ap();
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

/* Buttons */
#define LEFT_BTN 0
#define RIGHT_BTN 35
void IRAM_ATTR button_loop();
void IRAM_ATTR left_btn_handler(Button2 &btn);
void IRAM_ATTR right_btn_handler(Button2 &btn);

/* Pages */
void display_home_page();
void error_msg(std::string message);

/* Data */
void print_json_data();
uint8_t get_octet(const char *str, uint8_t n);
void load_spiffs();
std::string serialize_json_data();
void update_spiffs();

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

/* Definition of global variables */
#ifdef SET_GLOBAL_VAR
t_json_data json_data;
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
L3G gyro;
/* Buttons */
Button2 left_btn(LEFT_BTN);
Button2 right_btn(RIGHT_BTN);
hw_timer_t *buttons_timer;
/* Server */
WiFiUDP Udp;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
/* Simple use of global variables */
#else
extern t_json_data json_data;
extern Button2 left_btn;
extern Button2 right_btn;
extern hw_timer_t *buttons_timer;
extern TFT_eSPI tft;
extern L3G gyro;
extern WiFiUDP Udp;
extern AsyncWebServer server;
extern AsyncWebSocket ws;
#endif
#endif