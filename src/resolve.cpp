#include "tripodes.h"
#include <ESPmDNS.h>

#define RESOLVE_TTL_MS 60000

struct TargetCache
{
	std::string host;
	IPAddress ip;
	unsigned long last_ok_ms;
	bool valid;
};

static TargetCache osc_cache;
static TargetCache udp_cache;

static bool ends_with_local(const char *host)
{
	size_t len = strlen(host);
	return len > 6 && strcasecmp(host + len - 6, ".local") == 0;
}

bool resolve_hostname(const char *host, IPAddress &out)
{
	if (!host || !host[0])
		return false;

	if (out.fromString(host))
		return true;

	if (is_ap_mode || WiFi.status() != WL_CONNECTED)
		return false;

	if (ends_with_local(host))
	{
		std::string name(host);
		name.resize(name.size() - 6);
		int addr = MDNS.queryHost(name.c_str(), 2000);
		if (addr > 0)
		{
			out = IPAddress(addr);
			return true;
		}
	}

	if (WiFi.hostByName(host, out))
		return true;

	return false;
}

void invalidate_target_cache()
{
	osc_cache.valid = false;
	udp_cache.valid = false;
}

static bool refresh_one(TargetCache &cache, const std::string &host, const char *label)
{
	if (host.empty())
	{
		cache.valid = false;
		return false;
	}

	unsigned long now = millis();
	if (cache.valid && cache.host == host && (now - cache.last_ok_ms) < RESOLVE_TTL_MS)
		return cache.ip != IPAddress((uint32_t)0);

	IPAddress ip((uint32_t)0);
	if (resolve_hostname(host.c_str(), ip))
	{
		cache.host = host;
		cache.ip = ip;
		cache.last_ok_ms = now;
		cache.valid = true;
		Serial.printf("[%s] %s -> %s\n", label, host.c_str(), ip.toString().c_str());
		return true;
	}

	cache.valid = false;
	Serial.printf("[%s] resolve failed: %s\n", label, host.c_str());
	return false;
}

void refresh_target_cache()
{
	refresh_one(osc_cache, json_data.osc_target_ip, "OSC");
	refresh_one(udp_cache, json_data.udp_target_ip, "UDP");
}

static IPAddress get_cached_target(TargetCache &cache, const std::string &host, const char *label)
{
	unsigned long now = millis();
	if (!cache.valid || cache.host != host || (now - cache.last_ok_ms) >= RESOLVE_TTL_MS)
		refresh_one(cache, host, label);
	return cache.valid ? cache.ip : IPAddress((uint32_t)0);
}

IPAddress get_osc_target_ip()
{
	return get_cached_target(osc_cache, json_data.osc_target_ip, "OSC");
}

IPAddress get_udp_target_ip()
{
	return get_cached_target(udp_cache, json_data.udp_target_ip, "UDP");
}
