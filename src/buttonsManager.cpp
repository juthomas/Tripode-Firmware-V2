#include "tripodes.h"
// SINGLE_CLICK      1
// DOUBLE_CLICK      2
// TRIPLE_CLICK      3
// LONG_CLICK        4

void IRAM_ATTR button_loop()
{
	left_btn.loop();
	right_btn.loop();
}

void IRAM_ATTR left_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();

	if (!current_mode)
	{
		current_mode = MODE_AP;
	}

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
	if (!current_mode)
	{
		current_mode = MODE_STA;
	}

	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(0, 0);
	tft.setTextDatum(MC_DATUM);

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.printf("Right btn clicked, clic type : %d", click_type);
	Serial.printf("Right btn clicked, clic type : %d\n", click_type);
}
