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

void display_ram_usage()
{
	TFT_eSprite drawing_sprite = TFT_eSprite(&tft);
	drawing_sprite.setColorDepth(8);
	drawing_sprite.createSprite(tft.width(), tft.height());

	uint32_t x = 0;
	uint32_t y = 0;

	uint32_t color1 = TFT_GREEN;
	uint32_t color2 = TFT_WHITE;
	uint32_t color3 = TFT_BLUE;
	uint32_t color4 = TFT_RED;

	ESP.getFreeHeap();
	

	drawing_sprite.fillSprite(TFT_BLACK);
	drawing_sprite.setTextSize(2);
	drawing_sprite.setTextFont(1);
	drawing_sprite.setTextColor(TFT_GREEN);
	drawing_sprite.setTextDatum(MC_DATUM);

	drawing_sprite.setCursor(x, y);

	drawing_sprite.printf("Ram Usage");

	drawing_sprite.fillRect(x, y + 30, map((long)(532480 - ESP.getFreeHeap()), 0, 532480, 0, 100), 30, color1);
	drawing_sprite.setCursor(x + 35, y + 37);
	drawing_sprite.setTextColor(TFT_DARKGREY);
	drawing_sprite.printf("%02ld%%", map((long)(532480 - ESP.getFreeHeap()), 0, 532480, 0, 100));
	drawing_sprite.setTextSize(1);
	drawing_sprite.setCursor(x + 1, y + 65);
	drawing_sprite.setTextColor(TFT_WHITE);
	drawing_sprite.printf("%d/532480 Bytes", 532480 - ESP.getFreeHeap());

	drawing_sprite.drawRect(x, y + 30, 100, 30, color2);
	drawing_sprite.pushSprite(0, 0);
	drawing_sprite.deleteSprite();
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
