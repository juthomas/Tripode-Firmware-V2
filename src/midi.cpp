#include "tripodes.h"

static char incomingPacket[255];

static int convertOrcaMidiToHalfTones(char c)
{
	switch (c)
	{
	case 'C': return 0;
	case 'c': return 1;
	case 'D': return 2;
	case 'd': return 3;
	case 'E':
	case 'e': return 4;
	case 'F': return 5;
	case 'f': return 6;
	case 'G': return 7;
	case 'g': return 8;
	case 'A': return 9;
	case 'a': return 10;
	case 'B':
	case 'b': return 11;
	default: return 0;
	}
}

static double notefrequencies[] = {
	16.35, 17.32, 18.35, 19.45, 20.60, 21.83, 23.12, 24.50, 25.96, 27.50, 29.14, 30.87,
	32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00, 51.91, 55.00, 58.27, 61.74,
	65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.83, 110.00, 116.54, 123.47,
	130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 220.00, 233.08, 246.94,
	261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88,
	523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 880.00, 932.33, 987.77,
	1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760.00, 1864.66, 1975.53,
	2093.00, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.00, 3729.31, 3951.07,
	4186.01, 4434.91, 4698.63, 4978.03, 5274.04, 5587.65, 5919.91, 6271.93, 6644.88, 7040.00, 7458.62, 7902.13
};

void play_midi_notes()
{
	int packetSize = Udp.parsePacket();
	if (!packetSize)
		return;

	int len = Udp.read(incomingPacket, 255);
	if (len > 0)
		incomingPacket[len] = 0;
	if (len <= 5)
		return;

	int16_t octave = convertCharToBase35(incomingPacket[2]);
	int16_t note = convertOrcaMidiToHalfTones(incomingPacket[3]);
	int16_t velocity = convertCharToBase35(incomingPacket[4]) * 7;
	int16_t length = convertCharToBase35(incomingPacket[5]);
	int noteIndex = octave * 12 + note;
	if (noteIndex < 0 || noteIndex >= 108)
		return;

	int freq = (int)notefrequencies[noteIndex];
	if (timers_end[0] == 0)
	{
		motors_trigger_note(0, velocity / 5, freq, length * 100);
		toneValues[0] = noteIndex;
	}
	else if (timers_end[1] == 0)
	{
		motors_trigger_note(1, velocity / 5, freq, length * 100);
		toneValues[1] = noteIndex;
	}
	else if (timers_end[2] == 0)
	{
		motors_trigger_note(2, velocity / 5, freq, length * 100);
		toneValues[2] = noteIndex;
	}
}
