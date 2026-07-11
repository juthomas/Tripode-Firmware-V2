#define SET_GLOBAL_VAR
#include "tripodes.h"

void init_basic_functions()
{
	Serial.begin(115200);
	EEPROM.begin(EEPROM_SIZE);

	tft.init();
	tft.setRotation(0);
	tft.fillScreen(TFT_BLACK);
	if (TFT_BL > 0)
	{
		pinMode(TFT_BL, OUTPUT);
		digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
	}
	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(0, 0);
	tft.setTextDatum(MC_DATUM);

	if (!SPIFFS.begin())
		error_msg("Mounting SPIFFS");

	init_sensors();

	buttons_timer = timerBegin(3, 80, true);
	timerAttachInterrupt(buttons_timer, &button_loop, false);
	timerAlarmWrite(buttons_timer, 50 * 1000, true);
	timerAlarmEnable(buttons_timer);
	right_btn.setClickHandler(right_btn_handler);
	right_btn.setLongClickHandler(right_btn_handler);
	right_btn.setDoubleClickHandler(right_btn_handler);
	right_btn.setTripleClickHandler(right_btn_handler);
	left_btn.setClickHandler(left_btn_handler);
	left_btn.setLongClickHandler(left_btn_handler);
	left_btn.setDoubleClickHandler(left_btn_handler);
	left_btn.setTripleClickHandler(left_btn_handler);
}

void setup()
{
	current_mode = NONE_MODE;
	init_basic_functions();
	display_home_page();
	load_spiffs();
	print_json_data();
	emit_webcfg_ready_once();

	Serial.println("Select mode: left=AP, right=STA");

	for (;;)
	{
		poll_serial_config();

		if (current_mode & AP_MASK)
		{
			setup_ap();
			break;
		}
		else if (current_mode & STA_MASK)
		{
			setup_sta();
			break;
		}
		delay(10);
	}
}

void loop()
{
	t_sensors sensors = {};
	static uint32_t last_signal_poll_ms = 0;

	if (is_ap_mode)
		dnsServer.processNextRequest();

	ws.cleanupClients(2);

	poll_serial_config();

	if ((current_mode & MODE_MASK) == MIDI_MODE)
		play_midi_notes();
	else
		look_for_udp_message();

	update_motor_timers();

	if (!i2s_on)
		update_sensors(&sensors);

	uint32_t now = millis();
	if (now - last_signal_poll_ms >= get_signal_poll_interval_ms())
	{
		execute_signals(&sensors);
		last_signal_poll_ms = now;
	}

	draw_current_mode_screen(&sensors);

	delay(25);
}
