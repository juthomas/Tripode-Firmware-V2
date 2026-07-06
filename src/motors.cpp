#include "tripodes.h"

static const int motorFreq = 50;
static const int motorResolution = 8;
static const int motorChannel1 = 0;
static const int motorChannel2 = 1;
static const int motorChannel3 = 2;

typedef void (*t_set_pwm)(int pwm, double tonality);

static void set_pwm0(int pwm, double tonality)
{
	ledcSetup(motorChannel1, (uint32_t)tonality, motorResolution);
	ledcWrite(motorChannel1, pwm);
}

static void set_pwm1(int pwm, double tonality)
{
	ledcSetup(motorChannel2, (uint32_t)tonality, motorResolution);
	ledcWrite(motorChannel2, pwm);
}

static void set_pwm2(int pwm, double tonality)
{
	ledcSetup(motorChannel3, (uint32_t)tonality, motorResolution);
	ledcWrite(motorChannel3, pwm);
}

static const t_set_pwm g_set_pwm[3] = {set_pwm0, set_pwm1, set_pwm2};

void init_motors()
{
	ledcSetup(motorChannel1, motorFreq, motorResolution);
	ledcSetup(motorChannel2, motorFreq, motorResolution);
	ledcSetup(motorChannel3, motorFreq, motorResolution);
	ledcAttachPin(MOTOR_1, motorChannel1);
	ledcAttachPin(MOTOR_2, motorChannel2);
	ledcAttachPin(MOTOR_3, motorChannel3);
}

void update_motor_timers()
{
	for (int i = 0; i < 3; i++)
	{
		if (timers_end[i] != 0 && timers_end[i] < (int)(esp_timer_get_time() / 1000))
		{
			g_set_pwm[i](0, 0);
			timers_end[i] = 0;
			pwmValues[i] = 0;
			toneValues[i] = -1;
		}
	}
}

void motors_handle_udp_command(const String &packet)
{
	if (packet.indexOf('P') > -1 && packet.indexOf('D') > -1 && packet.indexOf('I') > -1)
	{
		int duration = packet.substring(packet.indexOf('D') + 1).toInt();
		int intensity = packet.substring(packet.indexOf('I') + 1).toInt();
		int pin = packet.substring(packet.indexOf('P') + 1).toInt();
		int tonality = packet.indexOf('T') > -1 ? packet.substring(packet.indexOf('T') + 1).toInt() : 440;
		if (pin >= 0 && pin <= 2)
			motors_trigger_note(pin, intensity / 5, tonality, duration);
	}
}

void motors_trigger_note(int pin, int velocity, int tonality, int duration)
{
	if (pin < 0 || pin > 2)
		return;
	timers_end[pin] = (int)(esp_timer_get_time() / 1000) + duration;
	g_set_pwm[pin](velocity, tonality);
	pwmValues[pin] = velocity;
}
