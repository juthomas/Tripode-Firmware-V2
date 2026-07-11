#include "tripodes.h"
#include <ESPmDNS.h>
#include <esp_wifi.h>

#define MAX_CLIENTS 10
#define WIFI_CHANNEL 6

const IPAddress captiveLocalIP(4, 3, 2, 1);
const IPAddress captiveGatewayIP(4, 3, 2, 1);
const IPAddress captiveSubnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1/";

static bool portalDismissed = false;

static const char *CAPTIVE_PORTAL_REDIRECT_HTML =
	"<HTML><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=http://4.3.2.1/\"></HEAD>"
	"<BODY></BODY></HTML>";

static const char *FIREFOX_CANONICAL_OK =
	"<HTML><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=http://www.mozilla.org/en-US/firefox/nightly/\"></HEAD></HTML>";

static const char *IOS_CNA_SUCCESS =
	"<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>";

static const char *WINDOWS_NCSI_SUCCESS = "Microsoft NCSI";

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

static void sendIosCnaSuccess(AsyncWebServerRequest *request)
{
	request->send(200, "text/html", IOS_CNA_SUCCESS);
}

static void sendCaptiveProbeResponse(AsyncWebServerRequest *request)
{
	if (portalDismissed)
		sendIosCnaSuccess(request);
	else
		request->send(200, "text/html", CAPTIVE_PORTAL_REDIRECT_HTML);
}

static void sendFirefoxCanonicalResponse(AsyncWebServerRequest *request)
{
	if (portalDismissed)
		request->send(200, "text/html", FIREFOX_CANONICAL_OK);
	else
		request->send(200, "text/html", CAPTIVE_PORTAL_REDIRECT_HTML);
}

static bool is_apple_captive_probe(AsyncWebServerRequest *request)
{
	const String host = request->host();
	return host.indexOf("apple.com") >= 0 || host.indexOf("itools.info") >= 0 ||
		   host.indexOf("ibook.info") >= 0 || host.indexOf("airport.us") >= 0 ||
		   host.indexOf("thinkdifferent.us") >= 0;
}

static void sendWindowsNcsiResponse(AsyncWebServerRequest *request)
{
	if (portalDismissed)
		request->send(200, "text/plain", WINDOWS_NCSI_SUCCESS);
	else
		request->send(200, "text/html", CAPTIVE_PORTAL_REDIRECT_HTML);
}

static const char *wifi_disconnect_reason_text(uint8_t reason)
{
	switch (reason)
	{
	case WIFI_REASON_AUTH_EXPIRE:
		return "AUTH_EXPIRE (mot de passe incorrect ou securite WPA incompatible)";
	case WIFI_REASON_AUTH_FAIL:
		return "AUTH_FAIL";
	case WIFI_REASON_NO_AP_FOUND:
		return "NO_AP_FOUND (SSID introuvable, verifier bande 2.4 GHz)";
	case WIFI_REASON_ASSOC_FAIL:
		return "ASSOC_FAIL";
	case WIFI_REASON_HANDSHAKE_TIMEOUT:
		return "HANDSHAKE_TIMEOUT (WPA2 requis, desactiver WPA3 seul sur la box)";
	case WIFI_REASON_CONNECTION_FAIL:
		return "CONNECTION_FAIL";
	case WIFI_REASON_BEACON_TIMEOUT:
		return "BEACON_TIMEOUT (signal faible ou box trop loin)";
	default:
		return "autre";
	}
}

static void on_wifi_sta_event(WiFiEvent_t event, WiFiEventInfo_t info)
{
	if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
	{
		uint8_t reason = info.wifi_sta_disconnected.reason;
		Serial.printf("[WiFi] STA disconnected, reason %u: %s\n", reason, wifi_disconnect_reason_text(reason));
	}
	else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP)
	{
		Serial.printf("[WiFi] STA connected, IP: %s\n", WiFi.localIP().toString().c_str());
	}
}

static bool connect_wifi_sta(uint32_t timeout_ms)
{
	WiFi.onEvent(on_wifi_sta_event);

	WiFi.mode(WIFI_OFF);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.persistent(false);
	WiFi.setAutoReconnect(false);
	WiFi.setSleep(false);
	WiFi.disconnect(true, true);
	delay(200);

	trim_wifi_credentials();

	Serial.printf("[WiFi] Connecting to \"%s\" (password length: %u)\n",
				  json_data.sta_ssid.c_str(),
				  (unsigned)json_data.sta_pswd.length());
	Serial.println("[WiFi] Astuce: TP-Link en WPA2-PSK sur 2.4 GHz (pas WPA3 seul)");

	WiFi.begin(json_data.sta_ssid.c_str(), json_data.sta_pswd.c_str());

	const uint32_t start = millis();
	uint32_t last_retry = start;

	while (WiFi.status() != WL_CONNECTED)
	{
		if (millis() - start > timeout_ms)
		{
			Serial.println("[WiFi] Connection timeout");
			return false;
		}

		if (millis() - last_retry > 10000)
		{
			Serial.println("[WiFi] Retry connection...");
			WiFi.disconnect(true);
			delay(200);
			WiFi.begin(json_data.sta_ssid.c_str(), json_data.sta_pswd.c_str());
			last_retry = millis();
		}

		delay(250);
	}

	return true;
}

void register_web_routes()
{
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
		if (!is_ap_mode)
		{
			request->redirect("/config");
			return;
		}
		if (is_apple_captive_probe(request))
			sendCaptiveProbeResponse(request);
		else
			request->send(SPIFFS, "/index.html", "text/html", false);
	});

	server.on("/config/doc", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", "text/html", false);
	});

	server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", "text/html", false);
	});

	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
		if (portalDismissed)
			request->send(200, "text/plain", "Microsoft Connect Test");
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
			request->send(200, "text/html", CAPTIVE_PORTAL_REDIRECT_HTML);
	});
	server.on("/redirect", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
	});
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
	});
	server.on("/library/test/success.html", [](AsyncWebServerRequest *request) {
		sendCaptiveProbeResponse(request);
	});
	server.on("/canonical.html", [](AsyncWebServerRequest *request) {
		sendFirefoxCanonicalResponse(request);
	});
	server.on("/success.txt", [](AsyncWebServerRequest *request) {
		if (portalDismissed)
			request->send(200, "text/plain", "success");
		else
			request->send(200, "text/html", CAPTIVE_PORTAL_REDIRECT_HTML);
	});
	server.on("/portal/done", [](AsyncWebServerRequest *request) {
		portalDismissed = true;
		request->send(200, "text/html", CAPTIVE_SUCCESS_HTML);
	});
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
		sendWindowsNcsiResponse(request);
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
				sendIosCnaSuccess(request);
			else
				request->send(200, "text/html", CAPTIVE_PORTAL_REDIRECT_HTML);
		}
		else
			request->send(SPIFFS, "/index.html", "text/html", false);
	});
}

void setup_ap()
{
	is_ap_mode = true;
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(captiveLocalIP, captiveGatewayIP, captiveSubnetMask);
	WiFi.softAP(json_data.ap_ssid.c_str(), json_data.ap_pswd.c_str(), WIFI_CHANNEL, 0, MAX_CLIENTS);

	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();
	delay(100);

	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(captiveLocalIP, captiveGatewayIP, captiveSubnetMask);
	WiFi.softAP(json_data.ap_ssid.c_str(), json_data.ap_pswd.c_str(), WIFI_CHANNEL, 0, MAX_CLIENTS);

	dnsServer.setTTL(300);
	dnsServer.start(53, "*", captiveLocalIP);

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
	tft.drawString("Connecting", tft.width() / 2, tft.height() / 2);

	init_motors();

	Serial.println("Connecting STA...");
	if (!connect_wifi_sta(60000))
	{
		Serial.println("[WiFi] Failed to join network, restarting...");
		delay(1000);
		ESP.restart();
	}

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());
	MDNS.begin("tripode");
	refresh_target_cache();
	Udp.begin(json_data.udp_input_port);
	register_web_routes();
	server.begin();
}
