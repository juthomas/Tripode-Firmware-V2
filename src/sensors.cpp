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

float updateDFA(t_sensors sensors)
{
	static float *x = 0;
	static float *x_tmp = 0;
	static float *alpha_mean = 0;

	const size_t size_x = 400;
	const size_t size_tmp_x = 20;
	const size_t size_alpha_mean = 50;
	static size_t current_index = 0;
	static size_t current_alpha_index = 0;

	if (x == 0)
	{
		x = (float *)malloc(sizeof(float) * size_x);
		x_tmp = (float *)malloc(sizeof(float) * size_x);
		alpha_mean = (float *)malloc(sizeof(float) * size_alpha_mean);
		for (size_t i = 0; i < size_x; i++)
			x[i] = 0.0;
		for (size_t i = 0; i < size_alpha_mean; i++)
			alpha_mean[i] = 0.0;
	}

	x[current_index] = map((long)sqrtf(sensors.gyro.x * sensors.gyro.x + sensors.gyro.y * sensors.gyro.y + sensors.gyro.z * sensors.gyro.z), 0L, 37000L, 0L, 100L);
	for (size_t i = 0; i < size_x; i++)
		x_tmp[size_x - i - 1] = x[(current_index - i) % size_x];

	for (size_t i = 0; i < size_tmp_x; i++)
	{
		size_t beg_i = map(i, 0, size_tmp_x, 0, size_x);
		float maxVal = 0;
		for (size_t j = 0; j < size_x / size_tmp_x; j++)
		{
			if (maxVal < x_tmp[beg_i + j])
				maxVal = x_tmp[beg_i + j];
		}
		x_tmp[i] = maxVal;
	}

	current_index = current_index >= size_x - 1 ? 0 : current_index + 1;

	float dfa_value = dfa(x_tmp, size_tmp_x, 1, 4, 0.2f);
	dfa_value = abs(dfa_value);
	alpha_mean[current_alpha_index] = dfa_value;
	dfa_value = mean(alpha_mean, size_alpha_mean);
	current_alpha_index = current_alpha_index >= size_alpha_mean - 1 ? 0 : current_alpha_index + 1;
	dfa_value = (float)fmap(dfa_value, 0, 18, 0, 1.5);

	if (SCALE_DFA == 1 && mean(x_tmp, size_tmp_x) < SCALE_DFA_TRESHOLD)
		dfa_value /= (float)fmap(mean(x_tmp, size_tmp_x), 0, SCALE_DFA_TRESHOLD, 6, 1);

	return dfa_value;
}
