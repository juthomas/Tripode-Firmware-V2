#include "tripodes.h"

void sendOscFloatMessage(const char *oscPrefix, float oscMessage, const IPAddress ipOut, const uint32_t portOut)
{
	char tmpOscPrefix[64];
	snprintf(tmpOscPrefix, sizeof(tmpOscPrefix), "%s_%s", oscPrefix, json_data.tripode_id.c_str());
	OSCMessage msg(tmpOscPrefix);
	msg.add(oscMessage);
	Udp.beginPacket(ipOut, portOut);
	msg.send(Udp);
	Udp.endPacket();
	msg.empty();
}
