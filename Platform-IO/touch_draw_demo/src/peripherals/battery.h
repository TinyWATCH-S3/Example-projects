#pragma once

#include <Adafruit_MAX1704X.h>

class Battery
{
	public:
		bool init();
		float get_voltage(bool forced);
		float get_percent(bool forced);
		void set_hibernate(bool state);
		uint8_t get_alert_status();
		void clear_alert_status(uint8_t status);


	private:
		Adafruit_MAX17048 maxlipo;
		unsigned long next_battery_read = 0;
		float cached_voltage = 0;
		float cached_percent = 0;

};
