#include "tripodes.h"
#include <qrcode.h>

#define QR_QUIET_ZONE_MODULES 4
#define QR_MIN_MODULE_PX 3

static void trim_string(std::string &s)
{
	while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n'))
		s.pop_back();
	size_t start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
		start++;
	if (start > 0)
		s.erase(0, start);
}

static void append_wifi_escaped(std::string &out, const char *value)
{
	if (!value)
		return;
	for (size_t i = 0; value[i]; i++)
	{
		char c = value[i];
		if (c == '\\' || c == ';' || c == ',' || c == '"')
			out += '\\';
		out += c;
	}
}

static std::string build_wifi_qr_payload(const char *ssid, const char *password)
{
	std::string payload = "WIFI:S:";
	append_wifi_escaped(payload, ssid);
	payload += ";T:";
	if (password && password[0])
		payload += "WPA";
	else
		payload += "nopass";
	payload += ";";
	if (password && password[0])
	{
		payload += "P:";
		append_wifi_escaped(payload, password);
		payload += ";";
	}
	payload += ";";
	return payload;
}

struct QrCache
{
	std::string payload_key;
	QRCode qrcode;
	uint8_t data[256];
	bool valid;
};

static QrCache qr_cache = {};
static std::string screen_cache_key;

// Max byte payload per QR version, ECC-L (ISO/IEC 18004). ricmoo accepts
// overflow silently on too-small versions, producing unscannable codes.
static const uint16_t QR_BYTE_CAPACITY_ECC_LOW[] = {
	0, 17, 32, 53, 78, 106, 134, 154,
};

static bool payload_fits_qr_version(uint8_t version, size_t payload_len)
{
	if (version == 0 || version >= sizeof(QR_BYTE_CAPACITY_ECC_LOW) / sizeof(QR_BYTE_CAPACITY_ECC_LOW[0]))
		return false;
	return payload_len <= QR_BYTE_CAPACITY_ECC_LOW[version];
}

void invalidate_wifi_qr_screen()
{
	screen_cache_key.clear();
}

static bool ensure_qr_cached(const std::string &cache_key, const std::string &wifi_payload)
{
	if (qr_cache.valid && qr_cache.payload_key == cache_key &&
		payload_fits_qr_version(qr_cache.qrcode.version, wifi_payload.length()))
		return true;

	qr_cache.valid = false;
	for (uint8_t version = 2; version <= 6; version++)
	{
		if (!payload_fits_qr_version(version, wifi_payload.length()))
			continue;

		if (qrcode_initText(&qr_cache.qrcode, qr_cache.data, version, ECC_LOW, wifi_payload.c_str()) == 0)
		{
			qr_cache.payload_key = cache_key;
			qr_cache.valid = true;
			return true;
		}
	}

	qr_cache.valid = false;
	Serial.printf("[QR] encode failed for payload=%s\n", wifi_payload.c_str());
	return false;
}

static int qr_scale_for_modules(int modules, int max_px)
{
	int scale = max_px / (modules + 2 * QR_QUIET_ZONE_MODULES);
	if (scale < QR_MIN_MODULE_PX)
		scale = QR_MIN_MODULE_PX;
	int total = modules * scale + 2 * QR_QUIET_ZONE_MODULES * scale;
	while (scale > QR_MIN_MODULE_PX && total > max_px)
	{
		scale--;
		total = modules * scale + 2 * QR_QUIET_ZONE_MODULES * scale;
	}
	return scale;
}

static void draw_qr_error_on_tft(const char *message)
{
	tft.fillScreen(TFT_WHITE);
	tft.setTextSize(1);
	tft.setTextFont(1);
	tft.setTextDatum(TL_DATUM);
	tft.setTextColor(TFT_RED);
	tft.setCursor(0, 0);
	tft.printf("QR error\n%s\n", message);
}

static void draw_qr_on_tft(int max_w, int max_h, int &out_scale)
{
	if (!qr_cache.valid)
	{
		out_scale = 0;
		return;
	}

	int modules = qr_cache.qrcode.size;
	int scale = qr_scale_for_modules(modules, max_w < max_h ? max_w : max_h);
	int qr_px = modules * scale;
	int quiet_px = QR_QUIET_ZONE_MODULES * scale;
	int total = qr_px + 2 * quiet_px;
	int x = (max_w - total) / 2;
	int y = (max_h - total) / 2;

	tft.fillRect(x, y, total, total, TFT_WHITE);

	for (int row = 0; row < modules; row++)
	{
		for (int col = 0; col < modules; col++)
		{
			if (!qrcode_getModule(&qr_cache.qrcode, col, row))
				continue;
			tft.fillRect(x + quiet_px + col * scale,
						 y + quiet_px + row * scale,
						 scale, scale, TFT_BLACK);
		}
	}

	out_scale = scale;
}

void drawWifiQrScreen(bool is_udp_sending, bool is_osc_sending)
{
	(void)is_udp_sending;
	(void)is_osc_sending;

	std::string ssid = is_ap_mode ? json_data.ap_ssid : json_data.sta_ssid;
	std::string pswd = is_ap_mode ? json_data.ap_pswd : json_data.sta_pswd;
	trim_string(ssid);
	trim_string(pswd);

	std::string payload = build_wifi_qr_payload(ssid.c_str(), pswd.c_str());
	std::string cache_key = payload;

	if (screen_cache_key == cache_key)
		return;

	if (ssid.empty())
	{
		draw_qr_error_on_tft("SSID vide");
		screen_cache_key = cache_key;
		return;
	}

	if (!ensure_qr_cached(cache_key, payload))
	{
		draw_qr_error_on_tft(payload.c_str());
		screen_cache_key = cache_key;
		return;
	}

	tft.fillScreen(TFT_WHITE);

	int scale = 0;
	draw_qr_on_tft(tft.width(), tft.height(), scale);

	Serial.printf("[QR] payload=%s len=%u version=%u modules=%u scale=%d\n",
				  payload.c_str(), (unsigned)payload.length(),
				  qr_cache.qrcode.version, qr_cache.qrcode.size, scale);

	screen_cache_key = cache_key;
}
