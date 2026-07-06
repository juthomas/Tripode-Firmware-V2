#include "tripodes.h"

double fmap(double x, double in_min, double in_max, double out_min, double out_max)
{
	const double dividend = out_max - out_min;
	const double divisor = in_max - in_min;
	const double delta = x - in_min;
	if (divisor == 0)
		return -1;
	return (delta * dividend + (divisor / 2.0)) / divisor + out_min;
}

void init_sensors()
{
	pinMode(ADC_EN, OUTPUT);
	digitalWrite(ADC_EN, HIGH);
	Wire.begin();
	if (!gyro.init())
	{
		error_msg("Autodetect gyro type");
		while (1)
			;
	}
	gyro.enableDefault();
	accel.begin();
	mag.begin();
}

void update_sensors(t_sensors *sensors)
{
	sensors_event_t accel_event;
	sensors_event_t mag_event;
	accel.getEvent(&accel_event);
	mag.getEvent(&mag_event);
	gyro.read();

	sensors->accel.x = accel_event.acceleration.x;
	sensors->accel.y = accel_event.acceleration.y;
	sensors->accel.z = accel_event.acceleration.z;
	sensors->gyro.x = gyro.g.x;
	sensors->gyro.y = gyro.g.y;
	sensors->gyro.z = gyro.g.z;
	sensors->mag.x = mag_event.magnetic.x;
	sensors->mag.y = mag_event.magnetic.y;
	sensors->mag.z = mag_event.magnetic.z;
}
