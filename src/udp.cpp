#include "tripodes.h"

static char incomingPacket[255];

void sendUpdMessage(const char *buffer, const IPAddress ipOut, const uint32_t portOut)
{
	Udp.beginPacket(ipOut, portOut);
	Udp.printf("%s", buffer);
	Udp.endPacket();
}

void sendOrcaLine(const char *line, uint16_t x, uint16_t y, const IPAddress ipOut, const uint32_t portOut)
{
	Udp.beginPacket(ipOut, portOut);
	Udp.printf("write:%s;%d;%d", line, x, y);
	Udp.endPacket();
}

void look_for_udp_message()
{
	int packetSize = Udp.parsePacket();
	if (!packetSize)
		return;

	int len = Udp.read(incomingPacket, 255);
	if (len > 0)
		incomingPacket[len] = 0;

	Serial.printf("UDP packet contents: %s\n", incomingPacket);
	motors_handle_udp_command(String(incomingPacket));
}
