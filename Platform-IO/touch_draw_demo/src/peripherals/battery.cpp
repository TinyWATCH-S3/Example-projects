#include "peripherals/battery.h"

bool vbus_present();

bool Battery::init()
{
	if (!maxlipo.begin())
	{
		Serial.println(F("Couldn't find MAX17048?\nMake sure a battery is plugged in!"));
		return false;
	}
	Serial.print(F("Found MAX17048"));
	Serial.print(F(" with Chip ID: 0x"));
	Serial.println(maxlipo.getChipID(), HEX);

	next_battery_read = millis();

	return true;
}

void Battery::set_hibernate(bool state)
{
	if (state)
		maxlipo.hibernate();
	else
  		maxlipo.wake();
}

float Battery::get_voltage(bool forced)
{
	if (forced)
	{
		cached_voltage = maxlipo.cellVoltage();
		cached_percent = maxlipo.cellPercent();
		next_battery_read = millis();
	}
	else if (millis() - next_battery_read > 500) {   // we only read the IC every 500ms
		next_battery_read = millis();
		cached_voltage = maxlipo.cellVoltage();
		cached_percent = maxlipo.cellPercent();
	}
	return cached_voltage;
}

float Battery::get_percent(bool forced)
{
	if (forced)
	{
		cached_percent = maxlipo.cellPercent();
		cached_voltage = maxlipo.cellVoltage();
		next_battery_read = millis();
	}
	else if (millis() - next_battery_read > 500) {   // we only read the IC every 500ms
		next_battery_read = millis();
		cached_percent = maxlipo.cellPercent();
		cached_voltage = maxlipo.cellVoltage();
	}

	return cached_percent;
}

uint8_t Battery::get_alert_status()
{
	uint8_t status = maxlipo.getAlertStatus();
	Serial.print("BAT Alert Status: ");
	Serial.println(status);
	clear_alert_status(status);
	return status;
}

void Battery::clear_alert_status(uint8_t status)
{
	Serial.print("BAT Clear Alert Status: ");
	Serial.println(status);
	maxlipo.clearAlertFlag(status);
}

