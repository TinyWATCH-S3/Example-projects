#pragma once

#include "RV3028C7.h"

typedef struct
{
	uint8_t secs = 0;
	uint8_t mins = 0;
	uint8_t hours = 0;

} TT_Time;

class RTC
{
	public:
		bool init();
		void setup_interrupt();
		bool check_interrupt();
		bool set_time_from_NTP();
		void set_hourly_alarm(uint minuets);

		String get_hours_string(bool padded, bool is24hour);
		String get_mins_string(bool padded);
		String get_secs_string(bool padded);
		String get_day_date();
		String get_day_of_week();
		String get_month_date();
		String get_time_string(bool padded, bool is24hour);
		int get_hours();
		int get_mins();
		int get_seconds();

		uint16_t get_day();
		uint16_t get_month();
		uint16_t get_year();

		TT_Time time_components;
		RV3028C7 rtc;

	private:
		bool enabled = false;

		uint8_t interruptPin = 33;
		unsigned long next_rtc_read = 0;
		uint16_t cached_day = 0;
		uint cached_month = 0;
		uint16_t cached_year = 0;
};
