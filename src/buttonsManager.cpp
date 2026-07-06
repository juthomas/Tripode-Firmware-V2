#include "tripodes.h"

static void cycle_display_mode()
{
	if (current_mode & STA_MASK)
	{
		if ((current_mode & MODE_MASK) == STD_MODE)
			current_mode = SENSORS_MODE | STA_MASK;
		else if ((current_mode & MODE_MASK) == SENSORS_MODE)
			current_mode = DFA_MODE | STA_MASK;
		else if ((current_mode & MODE_MASK) == DFA_MODE)
			current_mode = RUNE_MODE | STA_MASK;
		else if ((current_mode & MODE_MASK) == RUNE_MODE)
			current_mode = MIDI_MODE | STA_MASK;
		else if ((current_mode & MODE_MASK) == MIDI_MODE)
			current_mode = STD_MODE | STA_MASK;
	}
	else if (current_mode & AP_MASK)
	{
		if ((current_mode & MODE_MASK) == STD_MODE)
			current_mode = SENSORS_MODE | AP_MASK;
		else if ((current_mode & MODE_MASK) == SENSORS_MODE)
			current_mode = DFA_MODE | AP_MASK;
		else if ((current_mode & MODE_MASK) == DFA_MODE)
			current_mode = RUNE_MODE | AP_MASK;
		else if ((current_mode & MODE_MASK) == RUNE_MODE)
			current_mode = MIDI_MODE | AP_MASK;
		else if ((current_mode & MODE_MASK) == MIDI_MODE)
			current_mode = AP_MODE | AP_MASK;
		else if ((current_mode & MODE_MASK) == AP_MODE)
			current_mode = STD_MODE | AP_MASK;
	}
}

void IRAM_ATTR button_loop()
{
	left_btn.loop();
	right_btn.loop();
}

void IRAM_ATTR left_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();

	if (current_mode == NONE_MODE)
	{
		current_mode = AP_MODE | AP_MASK;
		return;
	}

	if (click_type == SINGLE_CLICK || click_type == LONG_CLICK)
		udp_sending = !udp_sending;
	if (click_type == DOUBLE_CLICK)
		osc_sending = !osc_sending;
}

void IRAM_ATTR right_btn_handler(Button2 &btn)
{
	uint32_t click_type = btn.getClickType();

	if (current_mode == NONE_MODE)
	{
		current_mode = STD_MODE | STA_MASK;
		return;
	}

	if (click_type == SINGLE_CLICK || click_type == LONG_CLICK)
		cycle_display_mode();
}
