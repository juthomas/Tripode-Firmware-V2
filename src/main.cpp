#define SET_GLOBAL_VAR
#include "tripodes.h"
// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <list>

// Captive Portal
#include <DNSServer.h>
#include <esp_wifi.h>		   //Used for mpdu_rx_disable android workaround
#include <AsyncTCP.h>		   //https://github.com/me-no-dev/AsyncTCP using the latest dev version from @me-no-dev
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev

#define MAX_CLIENTS 10 // ESP32 supports up to 10 but I have not tested it yet
#define WIFI_CHANNEL 6 // 2.4ghz channel 6 https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)

const IPAddress localIP(4, 3, 2, 1);		  // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress gatewayIP(4, 3, 2, 1);		  // IP address of the network should be the same as the local IP for captive portals
const IPAddress subnetMask(255, 255, 255, 0); // no need to change: https://avinetworks.com/glossary/subnet-mask/

const String localIPURL = "http://4.3.2.1"; // a string version of the local IP with http, used for redirecting clients to your webpage

DNSServer dnsServer;

void setup_ap()
{
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
	// WiFi.softAP(json_data.ap_ssid.c_str(), json_data.ap_pswd.c_str(), 6, 0, 10);
	WiFi.softAP(json_data.ap_ssid.c_str(), NULL, 6, 0, 10);

	dnsServer.setTTL(300);			   // set 5min client side cache for DNS
	dnsServer.start(53, "*", localIP); // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request

	esp_wifi_stop();
	esp_wifi_deinit();

	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT(); // We use the default config ...
	my_config.ampdu_rx_enable = false;						   //... and modify only what we want.

	esp_wifi_init(&my_config); // set the new config
	esp_wifi_start();		   // Restart WiFi
	delay(100);				   // this is necessary don't ask me why

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	Udp.begin(json_data.udp_input_port);
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/index.html", String(), false); });

	// Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request)
			  { request->redirect("http://logout.net"); }); // windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request)
			  { request->send(404); }); // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request)
			  { request->redirect(localIPURL); }); // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request)
			  { request->redirect(localIPURL); }); // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request)
			  { request->redirect(localIPURL); }); // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request)
			  { request->redirect(localIPURL); }); // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request)
			  { request->send(200); }); // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request)
			  { request->redirect(localIPURL); }); // windows call home

	server.onNotFound([](AsyncWebServerRequest *request)
					  { request->send(404); });
	ws.onEvent(onEvent);


	// Files used for web server
	server.serveStatic("/index.css", SPIFFS, "/index.css");
	server.serveStatic("/index.js", SPIFFS, "/index.js");
	server.serveStatic("/background.png", SPIFFS, "/background.png");
	server.serveStatic("/react.svg", SPIFFS, "/react.svg");
	server.serveStatic("/vite.svg", SPIFFS, "/vite.svg");

	// the catch all
	server.onNotFound([](AsyncWebServerRequest *request)
					  { request->redirect(localIPURL); });

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

void printMacAddress(const uint8_t *mac)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void loop()
{
	dnsServer.processNextRequest(); // I call this atleast every 10ms in my other projects (can be higher but I haven't tested it for stability)
	delay(1);						// seems to help with stability, if you are doing other things in the loop this may not be needed
}