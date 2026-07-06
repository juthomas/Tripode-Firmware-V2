#include "tripodes.h"

void drawNetworkActivity(bool is_udp_sending, bool is_osc_sending)
{
	TFT_eSprite drawing_sprite = TFT_eSprite(&tft);
	drawing_sprite.setColorDepth(8);
	drawing_sprite.createSprite(tft.width(), tft.height());
	drawing_sprite.fillSprite(TFT_BLACK);
	drawing_sprite.setTextSize(1);
	drawing_sprite.setTextFont(1);
	drawing_sprite.setTextDatum(MC_DATUM);
	drawing_sprite.setCursor(0, 0);

	uint16_t v = analogRead(ADC_PIN);
	float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (VREF / 1000.0);
	drawing_sprite.setTextColor(TFT_RED);
	drawing_sprite.printf("Voltage : ");
	drawing_sprite.setTextColor(TFT_WHITE);
	drawing_sprite.printf("%.2fv\n", battery_voltage);

	drawing_sprite.setTextColor(TFT_RED);
	drawing_sprite.printf("AP SSID: ");
	drawing_sprite.setTextColor(TFT_WHITE);
	drawing_sprite.printf("%s\n", json_data.ap_ssid.c_str());

	drawing_sprite.setTextColor(TFT_RED);
	drawing_sprite.printf("Clients : ");
	drawing_sprite.setTextColor(TFT_WHITE);
	drawing_sprite.printf("%d\n", WiFi.softAPgetStationNum());

	drawing_sprite.setTextColor(TFT_RED);
	drawing_sprite.printf("Portal : ");
	drawing_sprite.setTextColor(TFT_WHITE);
	drawing_sprite.printf("http://4.3.2.1/config\n");

	drawBatteryLevel(&drawing_sprite, 100, 0, battery_voltage);
	drawUpdSendingActivity(&drawing_sprite, is_udp_sending, is_osc_sending);
	drawing_sprite.pushSprite(0, 0);
	drawing_sprite.deleteSprite();
}
