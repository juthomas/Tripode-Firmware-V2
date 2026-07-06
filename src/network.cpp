#include "tripodes.h"
#include <ESPmDNS.h>
#include <esp_wifi.h>

#define MAX_CLIENTS 10
#define WIFI_CHANNEL 6

const IPAddress captiveLocalIP(4, 3, 2, 1);
const IPAddress captiveGatewayIP(4, 3, 2, 1);
const IPAddress captiveSubnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1";

static bool portalDismissed = false;

static const char *CAPTIVE_SUCCESS_HTML = R"html(<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tripode — Connecté</title>
<style>
*{box-sizing:border-box;margin:0}
body{min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px 16px;background:#0a0a0a;color:#f0f0f0;font-family:system-ui,-apple-system,sans-serif}
.card{max-width:320px;width:100%;padding:24px 20px;text-align:center;border:1px solid rgba(255,60,60,.4);background:rgba(20,20,20,.95);border-radius:8px}
h1{font-size:1.5rem;font-weight:700;letter-spacing:.12em;color:#ff4444;margin-bottom:4px}
.icon{width:56px;height:56px;margin:16px auto;line-height:56px;border-radius:50%;border:2px solid #44ff88;color:#44ff88;font-size:1.75rem;font-weight:700}
.lead{font-size:.95rem;color:#fff;font-weight:600;margin-bottom:10px}
p{font-size:.875rem;color:#aaa;line-height:1.5;margin-bottom:8px}
.hint{margin-top:16px;padding:10px 12px;background:#111;border:1px solid #333;border-radius:4px;font-size:.75rem;color:#888;line-height:1.45}
a{color:#ff4444;text-decoration:none;font-weight:600}
a:hover{text-decoration:underline}
</style>
</head>
<body>
<div class="card">
<h1>TRIPODE</h1>
<div class="icon">&#10003;</div>
<p class="lead">Configuration terminée</p>
<p>Le portail captif est fermé. Vous pouvez fermer cette page et retrouver votre connexion internet habituelle.</p>
<p class="hint">Pour reconfigurer le tripode, reconnectez-vous à son WiFi puis ouvrez <a href="http://4.3.2.1/config">4.3.2.1/config</a>.</p>
</div>
</body>
</html>)html";

static void sendCaptiveProbeResponse(AsyncWebServerRequest *request)
{
	if (portalDismissed)
		request->send(200, "text/html", CAPTIVE_SUCCESS_HTML);
	else
		request->redirect(localIPURL);
}

void register_web_routes()
{
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", String(), false);
	});

	server.on("/config/doc", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", "text/html", false);
	});

	server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", "text/html", false);
	});

	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
		if (portalDismissed)
			request->send(200, "text/html", CAPTIVE_SUCCESS_HTML);
		else
			request->redirect("http://logout.net");
	});
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
		request->send(404);
	});
	server.on("/generate_204", [](AsyncWebServerRequest *request) {
		if (portalDismissed)
			request->send(204);
		else
			request->redirect(localIPURL);
	});
	server.on("/redirect", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
	});
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
	});
	server.on("/canonical.html", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
	});
	server.on("/success.txt", [](AsyncWebServerRequest *request) {
		portalDismissed = true;
		request->send(200, "text/html", CAPTIVE_SUCCESS_HTML);
	});
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
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
		{
			if (portalDismissed)
				request->send(200, "text/html", CAPTIVE_SUCCESS_HTML);
			else
				request->redirect(localIPURL);
		}
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
	portalDismissed = false;
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
	MDNS.begin("tripode");
	refresh_target_cache();
	Udp.begin(json_data.udp_input_port);
	register_web_routes();
	server.begin();
}
