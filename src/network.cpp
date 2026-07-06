#include "tripodes.h"
#include <esp_wifi.h>

#define MAX_CLIENTS 10
#define WIFI_CHANNEL 6

const IPAddress captiveLocalIP(4, 3, 2, 1);
const IPAddress captiveGatewayIP(4, 3, 2, 1);
const IPAddress captiveSubnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1";

void register_web_routes()
{
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", String(), false);
	});

	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
		request->redirect("http://logout.net");
	});
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
		request->send(404);
	});
	server.on("/generate_204", [](AsyncWebServerRequest *request) {
		request->redirect(localIPURL);
	});
	server.on("/redirect", [](AsyncWebServerRequest *request) {
		request->redirect(localIPURL);
	});
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
		request->redirect(localIPURL);
	});
	server.on("/canonical.html", [](AsyncWebServerRequest *request) {
		request->redirect(localIPURL);
	});
	server.on("/success.txt", [](AsyncWebServerRequest *request) {
		request->send(200);
	});
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
		request->redirect(localIPURL);
	});

	server.serveStatic("/index.css", SPIFFS, "/index.css");
	server.serveStatic("/index.js", SPIFFS, "/index.js");
	server.serveStatic("/background.png", SPIFFS, "/background.png");
	server.serveStatic("/react.svg", SPIFFS, "/react.svg");
	server.serveStatic("/vite.svg", SPIFFS, "/vite.svg");
	server.serveStatic("/tripode.ico", SPIFFS, "/tripode.ico");
	server.serveStatic("/main.css", SPIFFS, "/main.css");
	server.serveStatic("/shaderScript.js", SPIFFS, "/shaderScript.js");

	ws.onEvent(onEvent);
	server.addHandler(&ws);
	server.onNotFound([](AsyncWebServerRequest *request) {
		if (is_ap_mode)
			request->redirect(localIPURL);
		else
			request->send(404);
	});
}

void setup_ap()
{
	is_ap_mode = true;
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(captiveLocalIP, captiveGatewayIP, captiveSubnetMask);
	WiFi.softAP(json_data.ap_ssid.c_str(), json_data.ap_pswd.c_str(), WIFI_CHANNEL, 0, MAX_CLIENTS);

	dnsServer.setTTL(300);
	dnsServer.start(53, "*", captiveLocalIP);

	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();
	delay(100);

	Serial.print("AP IP address: ");
	Serial.println(WiFi.softAPIP());
	Udp.begin(json_data.udp_input_port);
	init_motors();
	register_web_routes();
	server.begin();
}

void setup_sta()
{
	is_ap_mode = false;
	WiFi.mode(WIFI_STA);
	WiFi.begin(json_data.sta_ssid.c_str(), json_data.sta_pswd.c_str());
	tft.drawString("Connecting", tft.width() / 2, tft.height() / 2);
	uint64_t timeStamp = millis();

	init_motors();

	Serial.println("Connecting STA...");
	while (WiFi.status() != WL_CONNECTED)
	{
		if (millis() - timeStamp > 60000)
			ESP.restart();
		delay(500);
		tft.fillScreen(TFT_BLACK);
		tft.drawString("Connecting...", tft.width() / 2, tft.height() / 2);
	}

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());
	Udp.begin(json_data.udp_input_port);
	register_web_routes();
	server.begin();
}
