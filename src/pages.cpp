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
