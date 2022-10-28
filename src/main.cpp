#define SET_GLOBAL_VAR
#include "tripodes.h"

void setup_ap()
{
	WiFi.mode(WIFI_AP);
	WiFi.softAP(json_data.ap_ssid.c_str(), json_data.ap_pswd.c_str(), 1, 0, 10);
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	Udp.begin(json_data.udp_input_port);
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/index.html", String(), false); });
	ws.onEvent(onEvent);
	
	server.serveStatic("/tripode.ico", SPIFFS, "/tripode.ico");
	server.serveStatic("/main.css", SPIFFS, "/main.css");
	server.serveStatic("/shaderScript.js", SPIFFS, "/shaderScript.js");

	server.addHandler(&ws);
	server.begin();
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

void setup()
{
	init_basic_functions();
	display_home_page();

	load_spiffs();
	setup_ap();
	print_json_data();

	// update_spiffs();
}

void loop()
{
	// put your main code here, to run repeatedly:
}