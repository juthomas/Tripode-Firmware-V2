#include "tripodes.h"

void IRAM_ATTR button_loop()
{
	left_btn.loop();
	right_btn.loop();
}

void IRAM_ATTR left_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();

	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(0, 0);
	tft.setTextDatum(MC_DATUM);

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.printf("Left btn clicked, clic type : %d", click_type);
	Serial.printf("Left btn clicked, clic type : %d\n", click_type);
}

void IRAM_ATTR right_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();
	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(0, 0);
	tft.setTextDatum(MC_DATUM);

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.printf("Right btn clicked, clic type : %d", click_type);
	Serial.printf("Right btn clicked, clic type : %d\n", click_type);
}
