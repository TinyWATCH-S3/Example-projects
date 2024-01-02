#include "imu.h"
#include "rtc.h"

extern RTC rtc;

void IMU::init()
{
	imu_ready = true;
	mag_ready = true;

	if (imu.beginI2C(i2cAddress) != BMI2_OK)
	{
		// Not connected, inform user
		Serial.println("Error: BMI270 not connected, check wiring and I2C address!");
		imu_ready = false;
	}
	else
	{
		Serial.println(F("Found BMI270"));

		imu.setAccelPowerMode(BMI2_POWER_OPT_MODE);
		imu.setGyroPowerMode(BMI2_POWER_OPT_MODE, BMI2_POWER_OPT_MODE);

		// Reorient the IMU because it's worn upside down from BOSCH's intended orientation
		// The sensor assumes the axes have a particular orientation with respect to
		// the watch face, where +Y is at 12 o'clock, and +X is at 3 o'clock. You
		// can remap the axes if needed by uncommenting the code below
		bmi2_remap axes;
		axes.x = BMI2_AXIS_NEG_X;
		axes.y = BMI2_AXIS_NEG_Y;
		axes.z = BMI2_AXIS_NEG_Z;
		imu.remapAxes(axes);
	}

	mag = Adafruit_MMC5603(12345);
	if (!mag.begin(MMC56X3_DEFAULT_ADDRESS))
	{
		// I2C mode
		/* There was a problem detecting the MMC5603 ... check your connections */
		Serial.println("Ooops, no MMC5603 detected ... Check your wiring!");
		mag_ready = false;
	}
	else
	{
		Serial.println(F("Found MMC56X3"));
		mag.magnetSetReset();
		// mag.setContinuousMode(true);

	}

	next_imu_read = millis();
}

static bool interrupt_happened = false;
static void step_interrupt()
{
	interrupt_happened = true;
}

void IMU::set_hibernate(bool state)
{
	if (!imu_ready)
		return;

	if (state)
	{
		imu.disableFeature(BMI2_ACCEL);
		imu.disableFeature(BMI2_GYRO);
	}
	else
	{
		imu.enableFeature(BMI2_ACCEL);
		imu.enableFeature(BMI2_GYRO);
	}
}

void IMU::update()
{
	if (imu_ready)
		imu.getSensorData();
}

float IMU::get_accel_x()
{
	return (imu.data.accelX);
}

float IMU::get_accel_y()
{
	if (!imu_ready)
		return 0;

	return (imu.data.accelY);
}

float IMU::get_accel_z()
{
	return (imu.data.accelZ);
}

float IMU::get_gyro_x()
{
	return (imu.data.gyroX);
}

float IMU::get_gyro_y()
{
	if (!imu_ready)
		return 0;

	return (imu.data.gyroY);
}

float IMU::get_gyro_z()
{
	if (!imu_ready)
		return 0;

	return (imu.data.gyroZ);
}

float IMU::get_pitch()
{
	if (!mag_ready)
		return 0;

	float p = (180-(atan2(imu.data.accelY , imu.data.accelZ) * 57.3)) + 0.85;
  	return (p-180) * -1;
}

float IMU::get_roll()
{
	if (!mag_ready)
		return 0;

	return (atan2((imu.data.accelX) , sqrt(imu.data.accelY * imu.data.accelY + imu.data.accelZ * imu.data.accelZ)) * 57.3);
}

float IMU::get_yaw()
{
	if (!mag_ready)
		return 0;

	float hi_cal[3];
	float heading = 0;

  	/* Get a new sensor event */
	sensors_event_t event;
	mag.getEvent(&event);

  	float Pi = 3.14159;

	// Put raw magnetometer readings into an array
	float mag_data[] = {event.magnetic.x,
						event.magnetic.y,
						event.magnetic.z
						};

	// Apply hard-iron offsets
	for (uint8_t i = 0; i < 3; i++  )
	{
		hi_cal[i] = mag_data[i] - hard_iron[i];
	}

	// Apply soft-iron scaling
	for (uint8_t i = 0; i < 3; i++  )
	{
		mag_data[i] = (soft_iron[i][0] * hi_cal[0]) +
					(soft_iron[i][1] * hi_cal[1]) +
					(soft_iron[i][2] * hi_cal[2]);
	}

	// Non tilt compensated compass heading
	heading = (atan2(mag_data[0], mag_data[1]) * 180) / Pi;

	// Apply magnetic declination to convert magnetic heading
	// to geographic heading
	heading += mag_decl;

	// Normalize to 0-360
	if (heading < 0)
		heading = 360 + heading;

	return heading;
}


bool IMU::is_looking_at_face()
{
	if (!imu_ready)
		return false;

	float p = get_pitch();
	float r = get_roll();
	// Watch on left hand!
	// Silly calculation to detect if the wrist is casing the wearers face.
	return (p >= 30 && p <= 50 && r > -10 && r < 10);
}