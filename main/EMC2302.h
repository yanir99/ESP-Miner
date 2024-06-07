#ifndef EMC2302_H_
#define EMC2302_H_


#define EMC2302_I2CADDR_DEFAULT 0x2F ///< EMC2302 default i2c address
#define EMC2302_WHOAMI 0xFD          ///< Chip ID register
#define EMC2302_MANUFACTURER_ID 0xFE ///< Manufacturer ID
#define EMC2302_REVISION 0xFF        ///< Chip revision

#define EMC2302_CONFIG 0x20          ///< configuration register
#define EMC2302_STATUS 0x24          ///< status register
#define EMC2302_STALL_STATUS 0x25    ///< fan stall status
#define EMC2302_SPIN_STATUS 0x26     ///< fan spin status
#define EMC2302_DRIVE_FAIL_STATUS 0x27 ///< drive fail status
#define EMC2302_INT_ENABLE 0x29      ///< interrupt enable register
#define EMC2302_PWM_POLARITY 0x2A    ///< PWM polarity config
#define EMC2302_PWM_OUTPUT 0x2B      ///< PWM output config
#define EMC2302_PWM_BASEF45 0x2C     ///<
#define EMC2302_PWM_BASEF123         ///<

#define EMC2302_FAN1_SETTING 0x30       ///< Fan 1 setting
#define EMC2302_PWM1_DIVIDE 0x31        ///< PWM 1 divider
#define EMC2302_FAN1_CONFIG1 0x32       ///< Fan 1 configuration 1 register
#define EMC2302_FAN1_CONFIG2 0x33       ///< Fan 1 configuration 2 register
#define EMC2302_GAIN1 0x35              ///< Gain 1 register
#define EMC2302_FAN1_SPINUP_CONFIG 0x36 ///< Fan 1 spinup configuration register
#define EMC2302_FAN1_MAX_STEP 0x37      ///< Fan 1 max step register
#define EMC2302_FAN1_MIN_DRIVE 0x38     ///< Fan 1 minimum drive register
#define EMC2302_FAN1_TACH_COUNT 0x39    ///< Fan 1 valid TACH count
#define EMC2302_FAN1_DRV_FAIL_LOW 0x3A  ///< Fan 1 drive fail band low byte
#define EMC2302_FAN1_DRV_FAIL_HIGH 0x3B ///< Fan 1 drive fail band high byte
#define EMC2302_TACH1_TARGET_LSB 0x3C   ///< Tach 1 target low byte
#define EMC2302_TACH1_TARGET_MSB 0x3D   ///< Tach 1 target high byte
#define EMC2302_TACH1_LSB 0x3E          ///< Tach 1 reading low byte
#define EMC2302_TACH1_MSB 0x3F          ///< Tach 1 reading high byte

#define EMC2302_FAN2_SETTING 0x40       ///< Fan 2 setting
#define EMC2302_PWM2_DIVIDE 0x41        ///< PWM 2 divider
#define EMC2302_FAN2_CONFIG1 0x42       ///< Fan 2 configuration 1 register
#define EMC2302_FAN2_CONFIG2 0x43       ///< Fan 2 configuration 2 register
#define EMC2302_GAIN2 0x45              ///< Gain 2 register
#define EMC2302_FAN2_SPINUP_CONFIG 0x46 ///< Fan 2 spinup configuration register
#define EMC2302_FAN2_MAX_STEP 0x47      ///< Fan 2 max step register
#define EMC2302_FAN2_MIN_DRIVE 0x48     ///< Fan 2 minimum drive register
#define EMC2302_FAN2_TACH_COUNT 0x49    ///< Fan 2 valid TACH count
#define EMC2302_FAN2_DRV_FAIL_LOW 0x4A  ///< Fan 2 drive fail band low byte
#define EMC2302_FAN2_DRV_FAIL_HIGH 0x4B ///< Fan 2 drive fail band high byte
#define EMC2302_TACH2_TARGET_LSB 0x4C   ///< Tach 2 target low byte
#define EMC2302_TACH2_TARGET_MSB 0x4D   ///< Tach 2 target high byte
#define EMC2302_TACH2_LSB 0x4E          ///< Tach 2 reading low byte
#define EMC2302_TACH2_MSB 0x4F          ///< Tach 2 reading high byte

#define EMC2302_FAN_RPM_NUMERATOR 5400000 ///< Conversion unit to convert LSBs to fan RPM
#define _TEMP_LSB 0.125                   ///< single bit value for internal temperature readings

/**
 * @brief
 *
 * Allowed values for `setDataRate`.
 */
typedef enum
{
    EMC2302_RATE_1_16_HZ, ///< 1_16_HZ
    EMC2302_RATE_1_8_HZ,  ///< 1_8_HZ
    EMC2302_RATE_1_4_HZ,  ///< 1_4_HZ
    EMC2302_RATE_1_2_HZ,  ///< 1_2_HZ
    EMC2302_RATE_1_HZ,    ///< 1_HZ
    EMC2302_RATE_2_HZ,    ///< 2_HZ
    EMC2302_RATE_4_HZ,    ///< 4_HZ
    EMC2302_RATE_8_HZ,    ///< 8_HZ
    EMC2302_RATE_16_HZ,   ///< 16_HZ
    EMC2302_RATE_32_HZ,   ///< 32_HZ
} emc2302_rate_t;

void EMC2302_init(bool);
void EMC2302_set_fan_speed(uint8_t, float);
uint16_t EMC2302_get_fan_speed(uint8_t);

float EMC2302_get_external_temp(void);
uint8_t EMC2302_get_internal_temp(void);

#endif /* EMC2302_H_ */
