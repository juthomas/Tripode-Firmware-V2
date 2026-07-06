#include "tripodes.h"

static char incomingPacket[255];

static int convertCharToBase35(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (c - 'A' + 10);
	if (c >= 'a' && c <= 'z')
		return (c - 'a' + 10);
	if (c >= '0' && c <= '9')
		return (c - '0');
	return 0;
}

static char convertBase35ToChar(int nb)
{
	if (nb < 0)
		return '0';
	if (nb > 35)
		return 'z';
	if (nb < 10)
		return (char)('0' + nb);
	return (char)('a' + nb - 10);
}

void sendUpdMessage(const char *buffer, const IPAddress ipOut, const uint32_t portOut)
{
	Udp.beginPacket(ipOut, portOut);
	Udp.printf("%s", buffer);
	Udp.endPacket();
}

void sendUdpXYZ(t_float3 gyro, const IPAddress ipOut, const uint32_t portOut)
{
	int base35X = (int)fmap(gyro.x, 0, 40000, 0, 9);
	int base35Y = (int)fmap(gyro.y, 0, 40000, 0, 9);
	int base35Z = (int)fmap(gyro.z, 0, 40000, 0, 9);

	Udp.beginPacket(ipOut, portOut);
	Udp.printf("write:1V%c2V%c3V%c;0;0\n",
			   convertBase35ToChar(base35X),
			   convertBase35ToChar(base35Y),
			   convertBase35ToChar(base35Z));
	Udp.endPacket();
}

void sendUdpFractalState(float alpha, const IPAddress ipOut, const uint32_t portOut)
{
	char speed = '9';
	int base35 = (int)fmap(alpha, 0, 1.5, 0, 35);

	Udp.beginPacket(ipOut, portOut);
	Udp.printf("write:2C9\n"
			   " 2%cTE\n"
			   "    \n"
			   "%cR%cJ\n"
			   "  X;%d;%d",
			   speed,
			   convertBase35ToChar(base35),
			   convertBase35ToChar(base35 + 5),
			   json_data.fractal_state_pos_x,
			   json_data.fractal_state_pos_y);
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
