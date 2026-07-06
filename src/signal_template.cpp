#include "tripodes.h"
#include <cctype>
#include <vector>

enum e_output_format
{
	FMT_B35,
	FMT_HEX,
	FMT_DEC,
};

enum e_axis
{
	AXIS_AVG,
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
};

enum e_sensor
{
	SENSOR_G,
	SENSOR_A,
	SENSOR_M,
};

struct token_spec
{
	e_sensor sensor;
	e_axis axis;
	e_output_format format;
};

static void trim_inplace(std::string &s)
{
	size_t start = 0;
	while (start < s.size() && isspace((unsigned char)s[start]))
		start++;
	size_t end = s.size();
	while (end > start && isspace((unsigned char)s[end - 1]))
		end--;
	s = s.substr(start, end - start);
}

static void trim_osc_prefix(std::string &s)
{
	trim_inplace(s);
	while (!s.empty() && s.back() == ':')
		s.pop_back();
	trim_inplace(s);
}

static void uppercase_inplace(std::string &s)
{
	for (char &c : s)
		c = (char)toupper((unsigned char)c);
}

static float clampf(float v, float lo, float hi)
{
	if (v < lo)
		return lo;
	if (v > hi)
		return hi;
	return v;
}

static float get_raw_value(e_sensor sensor, e_axis axis, t_sensors *sensors)
{
	t_float3 *vec = nullptr;
	switch (sensor)
	{
	case SENSOR_G:
		vec = &sensors->gyro;
		break;
	case SENSOR_A:
		vec = &sensors->accel;
		break;
	case SENSOR_M:
		vec = &sensors->mag;
		break;
	}
	if (axis == AXIS_AVG)
		return (vec->x + vec->y + vec->z) / 3.0f;
	if (axis == AXIS_X)
		return vec->x;
	if (axis == AXIS_Y)
		return vec->y;
	return vec->z;
}

static void get_raw_range(e_sensor sensor, double &raw_min, double &raw_max)
{
	switch (sensor)
	{
	case SENSOR_G:
		raw_min = -37000;
		raw_max = 37000;
		break;
	case SENSOR_A:
		raw_min = -40;
		raw_max = 40;
		break;
	case SENSOR_M:
		raw_min = -100;
		raw_max = 100;
		break;
	}
}

static void get_norm_range(e_sensor sensor, int32_t &norm_min, int32_t &norm_max)
{
	switch (sensor)
	{
	case SENSOR_G:
		norm_min = json_data.gyro_norm_min;
		norm_max = json_data.gyro_norm_max;
		break;
	case SENSOR_A:
		norm_min = json_data.accel_norm_min;
		norm_max = json_data.accel_norm_max;
		break;
	case SENSOR_M:
		norm_min = json_data.mag_norm_min;
		norm_max = json_data.mag_norm_max;
		break;
	}
}

static bool parse_token_spec(const std::string &inner, token_spec &spec)
{
	if (inner.empty())
		return false;

	std::vector<std::string> tokens;
	size_t start = 0;
	for (size_t i = 0; i <= inner.size(); i++)
	{
		if (i == inner.size() || inner[i] == ':')
		{
			std::string tok = inner.substr(start, i - start);
			trim_inplace(tok);
			uppercase_inplace(tok);
			if (!tok.empty())
				tokens.push_back(tok);
			start = i + 1;
		}
	}

	if (tokens.empty())
		return false;

	char c = tokens[0][0];
	if (c == 'G')
		spec.sensor = SENSOR_G;
	else if (c == 'A')
		spec.sensor = SENSOR_A;
	else if (c == 'M')
		spec.sensor = SENSOR_M;
	else
		return false;

	spec.axis = AXIS_AVG;
	spec.format = FMT_B35;

	for (size_t i = 1; i < tokens.size(); i++)
	{
		const std::string &tok = tokens[i];
		if (tok == "X")
			spec.axis = AXIS_X;
		else if (tok == "Y")
			spec.axis = AXIS_Y;
		else if (tok == "Z")
			spec.axis = AXIS_Z;
		else if (tok == "B35")
			spec.format = FMT_B35;
		else if (tok == "HEX")
			spec.format = FMT_HEX;
		else if (tok == "DEC")
			spec.format = FMT_DEC;
		else
			return false;
	}
	return true;
}

static bool normalized_sensor_value(const std::string &inner, t_sensors *sensors, float &out)
{
	token_spec spec;
	if (!parse_token_spec(inner, spec))
	{
		Serial.printf("[signal_template] invalid token: {%s}\n", inner.c_str());
		return false;
	}

	float raw = get_raw_value(spec.sensor, spec.axis, sensors);
	double raw_min, raw_max;
	int32_t norm_min, norm_max;
	get_raw_range(spec.sensor, raw_min, raw_max);
	get_norm_range(spec.sensor, norm_min, norm_max);

	float normalized = (float)fmap(raw, raw_min, raw_max, norm_min, norm_max);
	out = clampf(normalized, (float)norm_min, (float)norm_max);
	return true;
}

static std::string format_value(float normalized, int32_t norm_min, int32_t norm_max, e_output_format fmt)
{
	float clamped = clampf(normalized, (float)norm_min, (float)norm_max);

	switch (fmt)
	{
	case FMT_B35:
	{
		int b35 = (int)round(fmap(clamped, norm_min, norm_max, 0, 35));
		return std::string(1, convertBase35ToChar(b35));
	}
	case FMT_DEC:
		return patch::to_string((int)round(clamped));
	case FMT_HEX:
	{
		char buf[16];
		snprintf(buf, sizeof(buf), "%X", (int)round(clamped));
		return std::string(buf);
	}
	}
	return "";
}

static std::string evaluate_token(const std::string &inner, t_sensors *sensors)
{
	token_spec spec;
	if (!parse_token_spec(inner, spec))
	{
		Serial.printf("[signal_template] invalid token: {%s}\n", inner.c_str());
		return "";
	}

	float normalized;
	if (!normalized_sensor_value(inner, sensors, normalized))
		return "";

	int32_t norm_min, norm_max;
	get_norm_range(spec.sensor, norm_min, norm_max);
	return format_value(normalized, norm_min, norm_max, spec.format);
}

std::string expand_signal_placeholders(const std::string &value, t_sensors *sensors)
{
	std::string result = value;
	size_t pos = 0;
	while ((pos = result.find('{', pos)) != std::string::npos)
	{
		size_t end = result.find('}', pos);
		if (end == std::string::npos)
			break;

		std::string inner = result.substr(pos + 1, end - pos - 1);
		std::string replacement = evaluate_token(inner, sensors);
		result.replace(pos, end - pos + 1, replacement);
		pos += replacement.length();
	}
	return result;
}

void execute_osc_signal(const std::string &value, t_sensors *sensors, const IPAddress ipOut, const uint32_t portOut)
{
	size_t search = 0;
	while ((search = value.find('{', search)) != std::string::npos)
	{
		size_t end = value.find('}', search);
		if (end == std::string::npos)
			break;

		std::string inner = value.substr(search + 1, end - search - 1);
		float osc_value;
		if (!normalized_sensor_value(inner, sensors, osc_value))
		{
			search = end + 1;
			continue;
		}

		std::string prefix = value;
		prefix.erase(search, end - search + 1);
		trim_osc_prefix(prefix);

		if (!prefix.empty())
			sendOscFloatMessage(prefix.c_str(), osc_value, ipOut, portOut);

		search = end + 1;
	}
}
