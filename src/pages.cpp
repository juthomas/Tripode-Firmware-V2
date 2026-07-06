#include "tripodes.h"

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
	tft.setTextColor(TFT_WHITE);
}

void display_ram_usage()
{
	static uint32_t last_draw = 0;
	if (millis() - last_draw < 2000)
		return;
	last_draw = millis();
	tft.setCursor(0, 220);
	tft.setTextColor(TFT_DARKGREY);
	tft.printf("Heap:%d", ESP.getFreeHeap());
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

void draw_current_mode_screen(t_sensors *sensors, float dfa_value)
{
	const char *ssid_label = is_ap_mode ? json_data.ap_ssid.c_str() : json_data.sta_ssid.c_str();

	if ((current_mode & MODE_MASK) == STD_MODE)
		drawMotorsActivity(tft, pwmValues, json_data.udp_input_port, ssid_label, udp_sending, osc_sending);
	else if ((current_mode & MODE_MASK) == MIDI_MODE)
		drawMidiActivity(tft, pwmValues, toneValues, json_data.udp_input_port, ssid_label, udp_sending, osc_sending);
	else if ((current_mode & MODE_MASK) == SENSORS_MODE)
		drawSensorsActivity(tft, *sensors, oscAddress, udp_sending, osc_sending);
	else if ((current_mode & MODE_MASK) == DFA_MODE)
		drawAlpha(tft, dfa_value, udp_sending, osc_sending);
	else if ((current_mode & MODE_MASK) == RUNE_MODE)
		drawRunes(tft, dfa_value, udp_sending, osc_sending);
	else if ((current_mode & MODE_MASK) == AP_MODE)
		drawNetworkActivity(udp_sending, osc_sending);
}
