///////////////////////////////////////////////////////////////////////////
////                                                                   ////
////                             DS3231.c                              ////
////                                                                   ////
////                      Driver for CCS C compiler                    ////
////                                                                   ////
////     Driver for Maxim DS3231 serial I2C real-time clock (RTC).     ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////
////                                                                   ////
////                     https://simple-circuit.com/                   ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////


#if defined DS3231_I2C_NO_STREAM
  #define RTC_I2C_START()   i2c_start()
  #define RTC_I2C_STOP()    i2c_stop()
  #define RTC_I2C_WRITE(x)  i2c_write(x)
  #define RTC_I2C_READ(x)   i2c_read(x)
#elif defined DS3231_I2C_STREAM
  #define RTC_I2C_START()   i2c_start(DS3231_I2C_STREAM)
  #define RTC_I2C_STOP()    i2c_stop(DS3231_I2C_STREAM)
  #define RTC_I2C_WRITE(x)  i2c_write(DS3231_I2C_STREAM, x)
  #define RTC_I2C_READ(x)   i2c_read(DS3231_I2C_STREAM, x)
#else
  #define RTC_I2C_START()   i2c_start(DS3231_STREAM)
  #define RTC_I2C_STOP()    i2c_stop(DS3231_STREAM)
  #define RTC_I2C_WRITE(x)  i2c_write(DS3231_STREAM, x)
  #define RTC_I2C_READ(x)   i2c_read(DS3231_STREAM, x)
#endif

#include <stdint.h>

#define DS3231_ADDRESS       0xD0
#define DS3231_REG_SECONDS   0x00
#define DS3231_REG_AL1_SEC   0x07
#define DS3231_REG_AL2_MIN   0x0B
#define DS3231_REG_CONTROL   0x0E
#define DS3231_REG_STATUS    0x0F
#define DS3231_REG_TEMP_MSB  0x11

typedef enum
{
  SUNDAY = 1,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
} RTC_DOW;

typedef enum
{
  JANUARY = 1,
  FEBRUARY,
  MARCH,
  APRIL,
  MAY,
  JUNE,
  JULY,
  AUGUST,
  SEPTEMBER,
  OCTOBER,
  NOVEMBER,
  DECEMBER
} RTC_Month;

typedef struct rtc_tm
{
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  RTC_DOW dow;
  uint8_t day;
  RTC_Month month;
  uint8_t year;
} RTC_Time;

typedef enum
{
  ONCE_PER_SECOND = 0x0F,
  SECONDS_MATCH = 0x0E,
  MINUTES_SECONDS_MATCH = 0x0C,
  HOURS_MINUTES_SECONDS_MATCH = 0x08,
  DATE_HOURS_MINUTES_SECONDS_MATCH = 0x0,
  DAY_HOURS_MINUTES_SECONDS_MATCH = 0x10
} al1;

typedef enum
{
  ONCE_PER_MINUTE = 0x0E,
  MINUTES_MATCH = 0x0C,
  HOURS_MINUTES_MATCH = 0x08,
  DATE_HOURS_MINUTES_MATCH = 0x0,
  DAY_HOURS_MINUTES_MATCH = 0x10
} al2;

typedef enum
{
  OUT_OFF = 0x00,
  OUT_INT = 0x04,
  OUT_1Hz = 0x40,
  OUT_1024Hz = 0x48,
  OUT_4096Hz = 0x50,
  OUT_8192Hz = 0x58
} INT_SQW;

RTC_Time c_time, c_alarm1, c_alarm2;

///////////////////////// All Functions /////////////////////////
                                                               //
uint8_t bcd_to_decimal(uint8_t number);                        //
uint8_t decimal_to_bcd(uint8_t number);                        //
void RTC_Set(RTC_Time *time_t);                                //
RTC_Time *RTC_Get();                                           //
void Alarm1_Set(RTC_Time *time_t, al1 _config);                //
RTC_Time *Alarm1_Get();                                        //
void Alarm1_Enable();                                          //
void Alarm1_Disable();                                         //
int1 Alarm1_IF_Check();                                        //
void Alarm1_IF_Reset();                                        //
int1 Alarm1_Status();                                          //
void Alarm2_Set(RTC_Time *time_t, al2 _config);                //
RTC_Time *Alarm2_Get();                                        //
void Alarm2_Enable();                                          //
void Alarm2_Disable();                                         //
int1 Alarm2_IF_Check();                                        //
void Alarm2_IF_Reset();                                        //
int1 Alarm2_Status();                                          //
void IntSqw_Set(INT_SQW _config);                              //
void Enable_32kHZ();                                           //
void Disable_32kHZ();                                          //
void OSC_Start();                                              //
void OSC_Stop();                                               //
int16_t Get_Temperature();                                     //
uint8_t RTC_Read_Reg(uint8_t reg_address);                     //
void RTC_Write_Reg(uint8_t reg_address, uint8_t reg_value);    //
                                                               //
/////////////////////////////////////////////////////////////////

// converts BCD to decimal
uint8_t bcd_to_decimal(uint8_t number)
{
  return ( (number >> 4) * 10 + (number & 0x0F) );
}

// converts decimal to BCD
uint8_t decimal_to_bcd(uint8_t number)
{
  return ( ((number / 10) << 4) + (number % 10) );
}

// sets time and date
void RTC_Set(RTC_Time *time_t)
{
  // convert decimal to BCD
  time_t->day     = decimal_to_bcd(time_t->day);
  time_t->month   = decimal_to_bcd(time_t->month);
  time_t->year    = decimal_to_bcd(time_t->year);
  time_t->hours   = decimal_to_bcd(time_t->hours);
  time_t->minutes = decimal_to_bcd(time_t->minutes);
  time_t->seconds = decimal_to_bcd(time_t->seconds);
  // end conversion

  // write data to the RTC chip
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_SECONDS);
  RTC_I2C_WRITE(time_t->seconds);
  RTC_I2C_WRITE(time_t->minutes);
  RTC_I2C_WRITE(time_t->hours);
  RTC_I2C_WRITE(time_t->dow);
  RTC_I2C_WRITE(time_t->day);
  RTC_I2C_WRITE(time_t->month);
  RTC_I2C_WRITE(time_t->year);
  RTC_I2C_STOP();
}

// reads time and date
RTC_Time *RTC_Get()
{
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_SECONDS);
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS | 0x01);
  c_time.seconds = RTC_I2C_READ(1);
  c_time.minutes = RTC_I2C_READ(1);
  c_time.hours   = RTC_I2C_READ(1);
  c_time.dow   = RTC_I2C_READ(1);
  c_time.day   = RTC_I2C_READ(1);
  c_time.month = RTC_I2C_READ(1);
  c_time.year  = RTC_I2C_READ(0);
  RTC_I2C_STOP();

  // convert BCD to decimal
  c_time.seconds = bcd_to_decimal(c_time.seconds);
  c_time.minutes = bcd_to_decimal(c_time.minutes);
  c_time.hours   = bcd_to_decimal(c_time.hours);
  c_time.day     = bcd_to_decimal(c_time.day);
  c_time.month   = bcd_to_decimal(c_time.month);
  c_time.year    = bcd_to_decimal(c_time.year);
  // end conversion

  return &c_time;
}

// sets alarm1 details
void Alarm1_Set(RTC_Time *time_t, al1 _config)
{
  // convert decimal to BCD
  time_t->day     = decimal_to_bcd(time_t->day);
  time_t->hours   = decimal_to_bcd(time_t->hours);
  time_t->minutes = decimal_to_bcd(time_t->minutes);
  time_t->seconds = decimal_to_bcd(time_t->seconds);
  // end conversion

  // write data to the RTC chip
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_AL1_SEC);
  RTC_I2C_WRITE( (time_t->seconds) | (bit_test(_config, 0) << 7) );
  RTC_I2C_WRITE( (time_t->minutes) | (bit_test(_config, 1) << 7) );
  RTC_I2C_WRITE( (time_t->hours) | (bit_test(_config, 2) << 7) );
  if ( bit_test(_config, 4) )
    RTC_I2C_WRITE( (time_t->dow) | 0x40 | (bit_test(_config, 3) << 7) );
  else
    RTC_I2C_WRITE( (time_t->day) | (bit_test(_config, 3) << 7) );
  RTC_I2C_STOP();
}

// reads alarm1 details
RTC_Time *Alarm1_Get()
{
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_AL1_SEC);
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS | 0x01);
  c_alarm1.seconds = RTC_I2C_READ(1) & 0x7F;
  c_alarm1.minutes = RTC_I2C_READ(1) & 0x7F;
  c_alarm1.hours   = RTC_I2C_READ(1) & 0x3F;
  c_alarm1.dow = c_alarm1.day = RTC_I2C_READ(0) & 0x3F;
  RTC_I2C_STOP();

  // convert BCD to decimal
  c_alarm1.seconds = bcd_to_decimal(c_alarm1.seconds);
  c_alarm1.minutes = bcd_to_decimal(c_alarm1.minutes);
  c_alarm1.hours   = bcd_to_decimal(c_alarm1.hours);
  c_alarm1.day     = bcd_to_decimal(c_alarm1.day);
  // end conversion

  return &c_alarm1;
}

// enables alarm1
void Alarm1_Enable()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg |= 0x01;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

// disables alarm1
void Alarm1_Disable()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg &= 0xFE;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

// checks if alarm1 occurred, returns 1 if yes and 0 if no
int1 Alarm1_IF_Check()
{
  uint8_t stat_reg = RTC_Read_Reg(DS3231_REG_STATUS);
  return bit_test(stat_reg, 0);
}

// resets alarm1 flag bit
void Alarm1_IF_Reset()
{
  uint8_t stat_reg = RTC_Read_Reg(DS3231_REG_STATUS);
  stat_reg &= 0xFE;
  RTC_Write_Reg(DS3231_REG_STATUS, stat_reg);
}

// returns TRUE (1) if alarm1 is enabled and FALSE (0) if disabled
int1 Alarm1_Status()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  if(ctrl_reg & 0x01)
    return 1;
  else
    return 0;
}

// sets alarm2 details
void Alarm2_Set(RTC_Time *time_t, al2 _config)
{
  // convert decimal to BCD
  time_t->day     = decimal_to_bcd(time_t->day);
  time_t->hours   = decimal_to_bcd(time_t->hours);
  time_t->minutes = decimal_to_bcd(time_t->minutes);
  // end conversion

  // write data to the RTC chip
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_AL2_MIN);
  RTC_I2C_WRITE( (time_t->minutes) | (bit_test(_config, 1) << 7) );
  RTC_I2C_WRITE( (time_t->hours) | (bit_test(_config, 2) << 7) );
  if ( bit_test(_config, 4) )
    RTC_I2C_WRITE( (time_t->dow) | 0x40 | (bit_test(_config, 3) << 7) );
  else
    RTC_I2C_WRITE( (time_t->day) | (bit_test(_config, 3) << 7) );
  RTC_I2C_STOP();
}

// reads alarm2 details
RTC_Time *Alarm2_Get()
{
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_AL2_MIN);
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS | 0x01);
  c_alarm2.minutes = RTC_I2C_READ(1) & 0x7F;
  c_alarm2.hours   = RTC_I2C_READ(1) & 0x3F;
  c_alarm2.dow = c_alarm2.day = RTC_I2C_READ(0) & 0x3F;
  RTC_I2C_STOP();

  // convert BCD to decimal
  c_alarm2.minutes = bcd_to_decimal(c_alarm2.minutes);
  c_alarm2.hours   = bcd_to_decimal(c_alarm2.hours);
  c_alarm2.day     = bcd_to_decimal(c_alarm2.day);
  // end conversion

  return &c_alarm2;
}

// enables alarm2
void Alarm2_Enable()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg |= 0x02;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

//disables alarm2
void Alarm2_Disable()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg &= 0xFD;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

// checks if alarm2 occurred, returns 1 if yes and 0 if no
int1 Alarm2_IF_Check()
{
  uint8_t stat_reg = RTC_Read_Reg(DS3231_REG_STATUS);
  return bit_test(stat_reg, 1);
}

// resets alarm2 flag bit
void Alarm2_IF_Reset()
{
  uint8_t stat_reg = RTC_Read_Reg(DS3231_REG_STATUS);
  stat_reg &= 0xFD;
  RTC_Write_Reg(DS3231_REG_STATUS, stat_reg);
}

// returns TRUE (1) if alarm2 is enabled and FALSE (0) if disabled
int1 Alarm2_Status()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  if(ctrl_reg & 0x02)
    return 1;
  else
    return 0;
}

// writes 'reg_value' to register of address 'reg_address'
void RTC_Write_Reg(uint8_t reg_address, uint8_t reg_value)
{
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(reg_address);
  RTC_I2C_WRITE(reg_value);
  RTC_I2C_STOP();
}

// returns the value stored in register of address 'reg_address'
uint8_t RTC_Read_Reg(uint8_t reg_address)
{
  uint8_t reg_data;

  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(reg_address);
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS | 0x01);
  reg_data = RTC_I2C_READ(0);
  RTC_I2C_STOP();

  return reg_data;
}

// sets INT/SQW pin configuration
void IntSqw_Set(INT_SQW _config)
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg &= 0xA3;
  ctrl_reg |= _config;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

// enables 32kHz (pin 32kHz)
void Enable_32kHZ()
{
  uint8_t stat_reg = RTC_Read_Reg(DS3231_REG_STATUS);
  stat_reg |= 0x08;
  RTC_Write_Reg(DS3231_REG_STATUS, stat_reg);
}

// disables 32kHz (pin 32kHz)
void Disable_32kHZ()
{
  uint8_t stat_reg = RTC_Read_Reg(DS3231_REG_STATUS);
  stat_reg &= 0xF7;
  RTC_Write_Reg(DS3231_REG_STATUS, stat_reg);
}

// starts RTC oscillator
void OSC_Start()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg &= 0x7F;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

// stops RTC oscillator
void OSC_Stop()
{
  uint8_t ctrl_reg = RTC_Read_Reg(DS3231_REG_CONTROL);
  ctrl_reg |= 0x80;
  RTC_Write_Reg(DS3231_REG_CONTROL, ctrl_reg);
}

// returns chip temperature
// Temperature is stored in hundredths C (output value of "3125" equals 31.25 °C).
int16_t Get_Temperature()
{
  uint8_t t_msb, t_lsb;
  uint16_t c_temp;
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS);
  RTC_I2C_WRITE(DS3231_REG_TEMP_MSB);
  RTC_I2C_START();
  RTC_I2C_WRITE(DS3231_ADDRESS | 0x01);
  t_msb = RTC_I2C_READ(1);
  t_lsb = RTC_I2C_READ(0);
  RTC_I2C_STOP();

  c_temp = (uint16_t)t_msb << 2 | t_lsb >> 6;

  if(t_msb & 0x80)
    c_temp |= 0xFC00;

  return c_temp * 25;
}
