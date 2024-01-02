/*
* Example PIO project for TinyWATCH hardware that show how to use touch, and how to draw to a sprite (canvas)
* and then how to push (blit) that to the TFT screen. 
*
* Also includes peripherals classes for IMU, RTC, Fuel Gauge and Buzzer.
*
* Author: Seon Rozenblum - Unexpected Maker
* Date: 15-Dec-2023
* License: GPL 3.0
*/

#include <SPI.h>
#include <Wire.h>

#include <TFT_eSPI.h>
#include "cst816t.h" // capacitive touch

#include "peripherals/buzzer.h"
#include "peripherals/battery.h"
#include "peripherals/imu.h"
#include "peripherals/rtc.h"

#include "fonts/RobotoMono_Regular_All.h"

// display
#define TFT_X 240
#define TFT_Y 280

#define TFT_CS 16
#define TFT_RST 17
#define TFT_DC 15
#define TFT_LED 13

// touch
#define TP_SDA 5
#define TP_SCL 10
#define TP_RST 12
#define TP_IRQ 11

// other stuff
#define VBUS_SENSE 34
#define BUZZER 18
#define PWR_SHUTDOWN 21

// Helper function to get a 565 color from an RGB color
constexpr uint16_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}

// Create peripheral objects
Battery battery;
IMU imu;
RTC rtc;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite canvas = TFT_eSprite(&tft);

// Touch is initialised on I2C bus 2, using IO5 for SDA, and IO10 for SCL
cst816t touchpad(TP_RST, TP_IRQ);

uint32_t color = 0;
int16_t start_x, start_y;
unsigned long touch_time = 0;
unsigned long last_touch = 0;
bool is_touched = false;

bool vbus_present()
{
	bool vbus = digitalRead(VBUS_SENSE);
	return (vbus);
}

void init_screen()
{
	if (touchpad.begin(mode_change))
	{
		if (vbus_present())
		{
			Serial.print("Touch started\nVersion: ");
			Serial.println(touchpad.version());
		}
	}
	tft.init();
	tft.setRotation(0);

	// We draw into a sprite (like a canvas) rather than directly to the TFT
	// Then we push the sprite to the TFT wen we are done with the frame.
	// We do this so the screen update is instant, rather than seeing it build.
	canvas.setFreeFont(RobotoMono_Regular[9]);
	canvas.setSwapBytes(true);
	canvas.createSprite(240, 280);
	canvas.setRotation(0);
	canvas.setTextDatum(4);

	// turn om backlight to 200 of 255.
	analogWrite(TFT_LED, 200);

	canvas.fillSprite(TFT_BLACK);
	canvas.setTextColor(TFT_YELLOW);
	canvas.drawString("Touch and Drag", 120, 130);
	canvas.setTextColor(TFT_GREEN);
	canvas.drawString("The Screen!", 120, 150);
	canvas.pushSprite(0, 0);

	canvas.setFreeFont(RobotoMono_Regular[7]);
}

void process_touch()
{
	if (touchpad.available())
	{
		if (!is_touched && touchpad.finger_num == 1)
		{
			color = RGB(random(255), random(255), random(255));

			start_x = touchpad.x;
			start_y = touchpad.y;
			is_touched = true;
			touch_time = last_touch = millis();

			BuzzerUI({ {2000, 10} });
			
			canvas.setTextColor(TFT_WHITE);
			canvas.fillSprite(TFT_BLACK);
			canvas.drawString("("+String(start_x)+","+String(start_y)+")", start_x, start_y);
		}
		else if (is_touched && touchpad.finger_num == 1)
		{
			canvas.fillSprite(TFT_BLACK);
			canvas.drawWideLine(start_x, start_y, touchpad.x, touchpad.y, 3, color);
			canvas.drawString("("+String(start_x)+","+String(start_y)+")", start_x, start_y);
			canvas.drawString("("+String(touchpad.x)+","+String(touchpad.y)+")", touchpad.x, touchpad.y);

			last_touch = millis();

		}
		else if (is_touched && touchpad.finger_num == 0)
		{
			canvas.drawString("Touch Time: "+String((last_touch-touch_time)/1000)+"s", 120, 260);

			Buzzer({
				{2000, 40},
				{0, 15},
				{2000, 40},
			});
		}
	}
	else
	{
		is_touched = false;
	}

	// This pushed (blits) the sprite to the screen
	canvas.pushSprite(0, 0);
}


void setup()
{
	pinMode(0, INPUT_PULLUP);
	pinMode(PWR_SHUTDOWN, OUTPUT);
	digitalWrite(PWR_SHUTDOWN, LOW);

	analogWriteResolution(8);

	// Initialise the Buzzer
	init_buzzer(BUZZER, 4);
	esp_register_shutdown_handler([]()
	{
		deinit_buzzer(BUZZER);
	});

	if (vbus_present())
	{
		Serial.begin(115200);
		delay(1000);
		Serial.println("hello");
	}

	init_screen();

	Wire.begin(8, 9);

	// Start the peripherals
	rtc.init();
	battery.init();
	imu.init();
}

void loop()
{
	process_touch();
}

