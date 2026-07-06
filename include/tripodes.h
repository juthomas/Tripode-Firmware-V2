
#ifndef TRIPODES_H
#define TRIPODES_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <L3G.h>
#include <Button2.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_LSM303DLH_Mag.h>
#include <OSCMessage.h>
#include <EEPROM.h>
#include "jsonParser.h"
#include <ESPAsyncWebServer.h>
#include <vector>
#include <sstream>
#include <math.h>

#define JSON_SIZE 4096

/* Hardware pins */
#define TFT_BL 4
#define ADC_EN 14
#define ADC_PIN 34
#define LEFT_BTN 0
#define RIGHT_BTN 35
#define MOTOR_1 25
#define MOTOR_2 26
#define MOTOR_3 27
#define VREF 1100
#define EEPROM_SIZE 512

#define UDP_DRAWING 1
#define UPD_MESSAGE_RATE 1
#define UPD_DRAW_RATE 300

enum e_wifi_modes
{
	NONE_MODE = 0,
	AP_MASK = 0b00001,
	STA_MASK = 0b00010,
	STD_MODE = 0b00100,
	SENSORS_MODE = 0b01000,
	WIFI_QR_MODE = 0b01100,
	AP_MODE = 0b10000,
	RUNE_MODE = 0b10100,
	MIDI_MODE = 0b11000,
	MODE_MASK = 0b11100,
};

typedef struct s_signal_data
{
	std::string value;
	std::string type;
} t_signal_data;

typedef struct s_json_data
{
	std::string udp_target_ip;
	uint32_t udp_target_port;
	uint32_t udp_input_port;
	std::string osc_target_ip;
	uint32_t osc_target_port;
	std::string sta_ssid;
	std::string sta_pswd;
	std::string ap_ssid;
	std::string ap_pswd;
	std::string tripode_id;
	int32_t fractal_state_pos_x;
	int32_t fractal_state_pos_y;
	int32_t glyph_pos_x;
	int32_t glyph_pos_y;
	int32_t gyro_norm_min;
	int32_t gyro_norm_max;
	int32_t accel_norm_min;
	int32_t accel_norm_max;
	int32_t mag_norm_min;
	int32_t mag_norm_max;
} t_json_data;

enum e_json_data_types
{
	TYPE_IP,
	TYPE_STRING,
	TYPE_INTEGER,
	TYPE_SIGNED,
};

typedef struct s_json_data_parser
{
	size_t offset;
	std::string name;
	e_json_data_types type;
} t_json_data_parser;

typedef struct s_float3
{
	float x;
	float y;
	float z;
} t_float3;

typedef struct s_sensors
{
	t_float3 accel;
	t_float3 gyro;
	t_float3 mag;
} t_sensors;

static const std::string json_data_parser_patch[] = {
	"upd_target_ip", "upd_target_port", "upd_input_port",
	"osc_target_ip", "osc_target_port",
	"sta_ssid", "sta_pswd", "ap_ssid", "ap_pswd", "tripode_id",
	"fractal_state_pos_x", "fractal_state_pos_y", "glyph_pos_x", "glyph_pos_y",
	"gyro_norm_min", "gyro_norm_max", "accel_norm_min", "accel_norm_max", "mag_norm_min", "mag_norm_max"};

static const t_json_data_parser json_data_parser[] = {
	{.offset = offsetof(t_json_data, udp_target_ip), .name = "upd_target_ip", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, udp_target_port), .name = "upd_target_port", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, udp_input_port), .name = "upd_input_port", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, osc_target_ip), .name = "osc_target_ip", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, osc_target_port), .name = "osc_target_port", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, sta_ssid), .name = "sta_ssid", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, sta_pswd), .name = "sta_pswd", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, ap_ssid), .name = "ap_ssid", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, ap_pswd), .name = "ap_pswd", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, tripode_id), .name = "tripode_id", .type = TYPE_STRING},
	{.offset = offsetof(t_json_data, fractal_state_pos_x), .name = "fractal_state_pos_x", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, fractal_state_pos_y), .name = "fractal_state_pos_y", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, glyph_pos_x), .name = "glyph_pos_x", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, glyph_pos_y), .name = "glyph_pos_y", .type = TYPE_INTEGER},
	{.offset = offsetof(t_json_data, gyro_norm_min), .name = "gyro_norm_min", .type = TYPE_SIGNED},
	{.offset = offsetof(t_json_data, gyro_norm_max), .name = "gyro_norm_max", .type = TYPE_SIGNED},
	{.offset = offsetof(t_json_data, accel_norm_min), .name = "accel_norm_min", .type = TYPE_SIGNED},
	{.offset = offsetof(t_json_data, accel_norm_max), .name = "accel_norm_max", .type = TYPE_SIGNED},
	{.offset = offsetof(t_json_data, mag_norm_min), .name = "mag_norm_min", .type = TYPE_SIGNED},
	{.offset = offsetof(t_json_data, mag_norm_max), .name = "mag_norm_max", .type = TYPE_SIGNED}};

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

/* Data */
void print_json_data();
void dump_json_to_serial();
uint8_t get_octet(const char *str, uint8_t n);
void write_default_config();
void load_spiffs();
std::string serialize_json_data(bool include_meta = false, bool meta_saved = true);
bool update_spiffs();
void parse_json_from_client(uint8_t *data);
void poll_serial_config();
void emit_webcfg_ready_once();

/* Network */
void setup_ap();
void setup_sta();
void register_web_routes();
bool resolve_hostname(const char *host, IPAddress &out);
void invalidate_target_cache();
void refresh_target_cache();
IPAddress get_udp_target_ip();
IPAddress get_osc_target_ip();
extern DNSServer dnsServer;

/* Server */
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

/* Buttons */
void IRAM_ATTR button_loop();
void IRAM_ATTR left_btn_handler(Button2 &btn);
void IRAM_ATTR right_btn_handler(Button2 &btn);

/* Pages */
void display_home_page();
void display_ram_usage();
void error_msg(std::string message);
void draw_current_mode_screen(t_sensors *sensors);

/* Sensors */
void init_sensors();
void update_sensors(t_sensors *sensors);
double fmap(double x, double in_min, double in_max, double out_min, double out_max);

/* OSC / UDP */
void sendOscFloatMessage(const char *oscPrefix, float oscMessage, const IPAddress ipOut, const uint32_t portOut);
void sendUpdMessage(const char *buffer, const IPAddress ipOut, const uint32_t portOut);
void sendOrcaLine(const char *line, uint16_t x, uint16_t y, const IPAddress ipOut, const uint32_t portOut);
void look_for_udp_message();
void execute_signals(t_sensors *sensors);
std::string expand_signal_placeholders(const std::string &value, t_sensors *sensors);
void execute_osc_signal(const std::string &value, t_sensors *sensors, const IPAddress ipOut, const uint32_t portOut);

/* Motors / MIDI */
void init_motors();
void update_motor_timers();
void motors_handle_udp_command(const String &packet);
void motors_trigger_note(int pin, int velocity, int tonality, int duration);
void play_midi_notes();

/* Encoding */
int convertCharToBase35(char c);
char convertBase35ToChar(int nb);

/* UI */
void drawUpdSendingActivity(TFT_eSprite *sprite, bool udp_activity, bool osc_activity);
void drawBatteryLevel(TFT_eSprite *sprite, int x, int y, float voltage);
void drawCursors(TFT_eSprite *sprite, int x, int y, int w, int h, int min, int max, int value, uint32_t color);
void drawMidiActivity(TFT_eSPI tft, int32_t pwmValues[3], int32_t toneValues[3], int32_t localUdpPort, const char *ssid, bool is_upd_sending, bool is_osc_sending);
void drawMotorsActivity(TFT_eSPI tft, int32_t pwmValues[3], int32_t localUdpPort, const char *ssid, bool is_upd_sending, bool is_osc_sending);
void drawSensorsActivity(TFT_eSPI tft, t_sensors sensors, int32_t oscAddress, bool is_upd_sending, bool is_osc_sending);
void drawRunes(TFT_eSPI tft, float alpha, bool is_upd_sending, bool is_osc_sending);
void drawNetworkActivity(bool is_udp_sending, bool is_osc_sending);
void drawWifiQrScreen(bool is_udp_sending, bool is_osc_sending);
void invalidate_wifi_qr_screen();

#ifdef SET_GLOBAL_VAR
uint8_t current_mode = NONE_MODE;
bool udp_sending = false;
bool osc_sending = false;
bool i2s_on = false;
bool is_ap_mode = false;
unsigned int oscAddress = 1;

int timers_end[3] = {0, 0, 0};
int pwmValues[3] = {0, 0, 0};
int toneValues[3] = {-1, -1, -1};

std::vector<t_signal_data> signal_data;
t_json_data json_data;

TFT_eSPI tft = TFT_eSPI();
L3G gyro;
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

Button2 left_btn(LEFT_BTN);
Button2 right_btn(RIGHT_BTN);
hw_timer_t *buttons_timer;

WiFiUDP Udp;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
#else
extern uint8_t current_mode;
extern bool udp_sending;
extern bool osc_sending;
extern bool i2s_on;
extern bool is_ap_mode;
extern unsigned int oscAddress;
extern int timers_end[3];
extern int pwmValues[3];
extern int toneValues[3];
extern std::vector<t_signal_data> signal_data;
extern t_json_data json_data;
extern Button2 left_btn;
extern Button2 right_btn;
extern hw_timer_t *buttons_timer;
extern TFT_eSPI tft;
extern L3G gyro;
extern Adafruit_LSM303_Accel_Unified accel;
extern Adafruit_LSM303DLH_Mag_Unified mag;
extern WiFiUDP Udp;
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern DNSServer dnsServer;
#endif

#endif
