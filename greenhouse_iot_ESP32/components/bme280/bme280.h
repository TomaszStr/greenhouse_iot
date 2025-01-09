/** \mainpage
*
****************************************************************************
* Copyright (C) 2015 - 2016 Bosch Sensortec GmbH
*
* File : bme280.h
*
* Date : 2016/07/04
*
* Revision : 2.0.5(Pressure and Temperature compensation code revision is 1.1
*               and Humidity compensation code revision is 1.0)
*
* Usage: Sensor Driver for BME280 sensor
*
****************************************************************************
*
* \section License
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
*   Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the distribution.
*
*   Neither the name of the copyright holder nor the names of the
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER
* OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*
* The information provided is believed to be accurate and reliable.
* The copyright holder assumes no responsibility
* for the consequences of use
* of such information nor for any infringement of patents or
* other rights of third parties which may result from its use.
* No license is granted by implication or otherwise under any patent or
* patent rights of the copyright holder.
**************************************************************************/
/*! \file bme280.h
    \brief BME280 Sensor Driver Support Header File */


#ifndef BME280_H
#define BME280_H

/*signed integer types*/
typedef	signed char  s8;/**< used for signed 8bit */
typedef	signed short int s16;/**< used for signed 16bit */
typedef	signed int s32;/**< used for signed 32bit */
typedef	signed long long int s64;/**< used for signed 64bit */

/*unsigned integer types*/
typedef	unsigned char u8;/**< used for unsigned 8bit */
typedef	unsigned short int u16;/**< used for unsigned 16bit */
typedef	unsigned int u32;/**< used for unsigned 32bit */
typedef	unsigned long long int u64;/**< used for unsigned 64bit */

/*! @brief
 *	If your machine support 64 bit
 *	define the MACHINE_64_BIT
 */
#define BME280_64BITSUPPORT_PRESENT

// Constant identifying the BME280 chip
#define BME280_CHIP_ID (0x60)

// I2C addresses for the BME280 sensor
#define BME280_I2C_ADDRESS1 (0x76)
#define BME280_I2C_ADDRESS2 (0x77)

// Operating modes of the sensor
#define BME280_SLEEP_MODE      (0x00) // Sleep mode
#define BME280_FORCED_MODE     (0x01) // Forced mode, single measurement
#define BME280_NORMAL_MODE     (0x03) // Normal mode, continuous measurements
#define BME280_SOFT_RESET_CODE (0xB6) // Code used for soft resetting the measurement control registers

// Standby time definitions (idle time between measurements in normal mode)
#define BME280_STANDBY_TIME_1_MS     (0x00)
#define BME280_STANDBY_TIME_63_MS    (0x01)
#define BME280_STANDBY_TIME_125_MS   (0x02)
#define BME280_STANDBY_TIME_250_MS   (0x03)
#define BME280_STANDBY_TIME_500_MS   (0x04)
#define BME280_STANDBY_TIME_1000_MS  (0x05)
#define BME280_STANDBY_TIME_10_MS    (0x06)
#define BME280_STANDBY_TIME_20_MS    (0x07)

// Oversampling definitions
#define BME280_OVERSAMP_SKIPPED (0x00) // No oversampling
#define BME280_OVERSAMP_1X      (0x01) // Oversampling x1
#define BME280_OVERSAMP_2X      (0x02) // Oversampling x2
#define BME280_OVERSAMP_4X      (0x03) // Oversampling x4
#define BME280_OVERSAMP_8X      (0x04) // Oversampling x8
#define BME280_OVERSAMP_16X     (0x05) // Oversampling x16

// Filter settings definitions (IIR filter)
#define BME280_FILTER_COEFF_OFF (0x00) // Filter off
#define BME280_FILTER_COEFF_2   (0x01) // Filter x2
#define BME280_FILTER_COEFF_4   (0x02) // Filter x4
#define BME280_FILTER_COEFF_8   (0x03) // Filter x8
#define BME280_FILTER_COEFF_16  (0x04) // Filter x16

//	DELAY DEFINITIONS
#define T_INIT_MAX              (20) // 20/16 = 1.2500 ms
#define T_MEASURE_PER_OSRS_MAX  (37) // 37/16 = 2.3125 ms
#define T_SETUP_PRESSURE_MAX    (10) // 10/16 = 0.6250 ms
#define T_SETUP_HUMIDITY_MAX    (10) // 10/16 = 0.6250 ms


// /****************************************************/
// /**\name	ERROR CODE DEFINITIONS  */
// /***************************************************/
// #define	SUCCESS						((u8)0)
// #define E_BME280_NULL_PTR       	((s8)-127)
// #define E_BME280_COMM_RES       	((s8)-1)
// #define E_BME280_OUT_OF_RANGE   	((s8)-2)
// #define ERROR						((s8)-1)
// #define BME280_CHIP_ID_READ_FAIL	((s8)-1)
// #define BME280_CHIP_ID_READ_SUCCESS	((u8)0)

// Error codes
#define SUCCESS                      (0)    // Operation successful
#define E_BME280_NULL_PTR            (-127) // Error: NULL pointer
#define E_BME280_COMM_RES            (-1)   // Communication error
#define E_BME280_OUT_OF_RANGE        (-2)   // Value out of range
#define ERROR                        (-1)   // General error
#define BME280_CHIP_ID_READ_FAIL     (-1)   // Failed to read chip ID
#define BME280_CHIP_ID_READ_SUCCESS  (0)    // Successfully read chip ID

/**
 * @brief Sensor ID register. Contains the chip ID.
 * The expected value is 0x60, which identifies the BME280 sensor.
 */
#define BME280_REG_ID             0xD0  

/**
 * @brief Reset register. Contains the reset word to reset the device.
 * To reset the sensor, write 0xB6 to this register. 
 * Writing any other value has no effect. 
 * The read value is always 0x00.
 */
#define BME280_REG_RESET          0xE0  

/**
 * @brief Humidity control register. Configures the humidity measurement options.
 * The settings for humidity oversampling are controlled here.
 * Changes to this register take effect after writing to the CTRL_MEAS register.
 */
#define BME280_CTRL_HUMIDITY_REG       0xF2  

/**
 * @brief Status register. Contains bits indicating the status of the sensor.
 * - Bit 3 (measuring[0]): Set to 1 when a conversion is ongoing, returns to 0 when the result is transferred.
 * - Bit 0 (im_update[0]): Set to 1 when NVM data is being copied to image registers.
 */
#define BME280_REG_STATUS         0xF3  

/**
 * @brief Measurement control register. Configures temperature and pressure measurement options.
 * - Oversampling and sensor mode are controlled through this register.
 * Changes to this register take effect after writing to the CTRL_HUM register.
 */
#define BME280_CTRL_MEAS_REG      0xF4  

/**
 * @brief Configuration register. Configures the IIR filter and standby time in normal mode.
 * In normal mode, writes to this register may be ignored, but in sleep mode, they are not.
 */
#define BME280_CONFIG_REG         0xF5 

/**
 * @brief MSB register for raw pressure data.
 * Contains the most significant part of the pressure data (up[19:12]).
 */
#define BME280_REG_PRESS_MSB      0xF7  

/**
 * @brief LSB register for raw pressure data.
 * Contains the least significant part of the pressure data (up[11:4]).
 */
#define BME280_REG_PRESS_LSB      0xF8  

/**
 * @brief XLSB register for raw pressure data.
 * Contains the least significant bit part of the pressure data (up[3:0]).
 */
#define BME280_REG_PRESS_XLSB     0xF9  

/**
 * @brief MSB register for raw temperature data.
 * Contains the most significant part of the temperature data (ut[19:12]).
 */
#define BME280_REG_TEMP_MSB       0xFA  

/**
 * @brief LSB register for raw temperature data.
 * Contains the least significant part of the temperature data (ut[11:4]).
 */
#define BME280_REG_TEMP_LSB       0xFB  

/**
 * @brief XLSB register for raw temperature data.
 * Contains the least significant bit part of the temperature data (ut[3:0]).
 */
#define BME280_REG_TEMP_XLSB      0xFC  

/**
 * @brief MSB register for raw humidity data.
 * Contains the most significant part of the humidity data (uh[15:8]).
 */
#define BME280_REG_HUM_MSB        0xFD  

/**
 * @brief LSB register for raw humidity data.
 * Contains the least significant part of the humidity data (uh[7:0]).
 */
#define BME280_REG_HUM_LSB        0xFE  


#define BME280_RETURN_FUNCTION_TYPE          s8

/********************************************/
/**\name	ENABLE FLOATING OUTPUT      */
/**************************************/

/*!
* @brief If the user wants to support floating point calculations, please set
	the following define. If floating point
	calculation is not wanted or allowed
	(e.g. in Linux kernel), please do not set the define. */
#define BME280_ENABLE_FLOAT

/*!
* @brief If the user wants to support 64 bit integer calculation
	(needed for optimal pressure accuracy) please set
	the following define. If int64 calculation is not wanted
	(e.g. because it would include
	large libraries), please do not set the define. */
#define BME280_ENABLE_INT64


/***************************************************************/
/**\name	BUS READ AND WRITE FUNCTION POINTERS        */
/***************************************************************/
/*!
	@brief Define the calling convention of YOUR bus communication routine.
	@note This includes types of parameters. This example shows the
	configuration for an SPI bus link.

    If your communication function looks like this:

    write_my_bus_xy(u8 device_addr, u8 register_addr,
    u8 * data, u8 length);

    The BME280_WR_FUNC_PTR would equal:

	BME280_WR_FUNC_PTR s8 (* bus_write)(u8,
    u8, u8 *, u8)

    Parameters can be mixed as needed refer to the
    refer BME280_BUS_WRITE_FUNC  macro.


*/
/** defines the return parameter type of the BME280_WR_FUNCTION */
#define BME280_BUS_WR_RETURN_TYPE s8


// links the order of parameters defined in BME280_BUS_WR_PARAM_TYPE to function calls used inside the API
#define BME280_BUS_WR_PARAM_TYPES u8, u8, u8 *, u8

/* links the order of parameters defined in
BME280_BUS_WR_PARAM_TYPE to function calls used inside the API*/
#define BME280_BUS_WR_PARAM_ORDER(device_addr, register_addr, register_data, wr_len)

/* never change this line */
#define BME280_BUS_WRITE_FUNC(device_addr, register_addr, register_data, wr_len) bus_write(device_addr, register_addr, register_data, wr_len)



//defines the return parameter type of the BME280_RD_FUNCTION
#define BME280_BUS_RD_RETURN_TYPE s8

//brief defines the calling parameter types of the BME280_RD_FUNCTION
#define BME280_BUS_RD_PARAM_TYPES (u8, u8, u8 *, u8)

// links the order of parameters defined in BME280_BUS_RD_PARAM_TYPE to function calls used inside the API
#define BME280_BUS_RD_PARAM_ORDER (device_addr, register_addr, register_data)

/* never change this line */
#define BME280_BUS_READ_FUNC(device_addr, register_addr, register_data, rd_len) bus_read(device_addr, register_addr, register_data, rd_len)



/**************************************************************/
/**\name	STRUCTURE DEFINITIONS                         */
/**************************************************************/
/*!
 * @brief This structure holds all device specific calibration parameters
 */
struct bme280_calibration_param_t {
	u16 dig_T1; // Calibration T1 data: Unsigned calibration value for temperature compensation.
    s16 dig_T2; // Calibration T2 data: Signed calibration value for temperature compensation.
    s16 dig_T3; // Calibration T3 data: Signed calibration value for temperature compensation.
    
	u16 dig_P1; // Calibration P1 data: Unsigned calibration value for pressure compensation.
    s16 dig_P2; // Calibration P2 data: Signed calibration value for pressure compensation.
    s16 dig_P3; // Calibration P3 data: Signed calibration value for pressure compensation.
    s16 dig_P4; // Calibration P4 data: Signed calibration value for pressure compensation.
    s16 dig_P5; // Calibration P5 data: Signed calibration value for pressure compensation.
    s16 dig_P6; // Calibration P6 data: Signed calibration value for pressure compensation.
    s16 dig_P7; // Calibration P7 data: Signed calibration value for pressure compensation.
    s16 dig_P8; // Calibration P8 data: Signed calibration value for pressure compensation.
    s16 dig_P9; // Calibration P9 data: Signed calibration value for pressure compensation.
    
	u8  dig_H1; // Calibration H1 data: Unsigned calibration value for humidity compensation.
    s16 dig_H2; // Calibration H2 data: Signed calibration value for humidity compensation.
    u8  dig_H3; // Calibration H3 data: Unsigned calibration value for humidity compensation.
    s16 dig_H4; // Calibration H4 data: Signed calibration value for humidity compensation, combining split msb and lsb data.
    s16 dig_H5; // Calibration H5 data: Signed calibration value for humidity compensation, combining split msb and lsb data.
    s8  dig_H6; // Calibration H6 data: Signed calibration value for humidity compensation.
    
	s32 t_fine; // Calibration T_FINE data: Intermediate temperature data used in calculations for other sensor compensations.
};


/****************************************************/
/**\name	BUS READ AND WRITE FUNCTION POINTERS */
/***************************************************/
#define BME280_WR_FUNC_PTR s8 (*bus_write)(u8, u8, u8 *, u8)

#define BME280_RD_FUNC_PTR s8 (*bus_read)(u8, u8, u8 *, u8)

#define BME280_MDELAY_DATA_TYPE u32


// This structure holds BME280 initialization parameters
struct bme280_t {
	/**< calibration parameters*/
	struct bme280_calibration_param_t cal_param;

	u8 chip_id;/**< chip id of the sensor*/
	u8 dev_addr;/**< device address of the sensor*/

	u8 oversamp_temperature;/**< temperature over sampling*/
	u8 oversamp_pressure;/**< pressure over sampling*/
	u8 oversamp_humidity;/**< humidity over sampling*/
	u8 ctrl_hum_reg;/**< status of control humidity register*/
	u8 ctrl_meas_reg;/**< status of control measurement register*/
	u8 config_reg;/**< status of configuration register*/

	u16 altitude;/**< altitude in meters used to adjust pressure measurements*/

	BME280_WR_FUNC_PTR;/**< bus write function pointer*/
	BME280_RD_FUNC_PTR;/**< bus read function pointer*/
	void (*delay_msec)(BME280_MDELAY_DATA_TYPE);/**< delay function pointer*/
};

/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION                       */
/**************************************************************/
/*!
 *	@brief This function is used for initialize
 *	the bus read and bus write functions
 *  and assign the chip id and I2C address of the BME280 sensor
 *	chip id is read in the register 0xD0 bit from 0 to 7
 *
 *	 @param bme280 structure pointer.
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_init(struct bme280_t *bme280);

/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION UNCOMPENSATED TEMPERATURE */
/**************************************************************/
/*!
 *	@brief This API is used to read uncompensated temperature
 *	in the registers 0xFA, 0xFB and 0xFC
 *	@note 0xFA -> MSB -> bit from 0 to 7
 *	@note 0xFB -> LSB -> bit from 0 to 7
 *	@note 0xFC -> LSB -> bit from 4 to 7
 *
 * @param bme280 structure pointer.
 * @param v_uncomp_temperature_s32 : The value of uncompensated temperature
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_temperature(struct bme280_t *bme280, s32 *v_uncomp_temperature_s32);

/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION TRUE TEMPERATURE */
/**************************************************************/
/*!
 * @brief Reads actual temperature from uncompensated temperature
 * @note Returns the value in 0.01 degree Centigrade
 * Output value of "5123" equals 51.23 DegC.
 *
 *
 *	@param bme280 structure pointer.
 *  @param  v_uncomp_temperature_s32 : value of uncompensated temperature
 *
 *
 *  @return Returns the actual temperature
 *
*/
s32 bme280_compensate_temperature_int32(struct bme280_t *bme280, s32 v_uncomp_temperature_s32);

/*!
 * @brief Reads actual temperature from uncompensated temperature
 * @note Returns the value with 500LSB/DegC centred around 24 DegC
 * output value of "5123" equals(5123/500)+24 = 34.246DegC
 *
 *	@param bme280 structure pointer.
 *  @param v_uncomp_temperature_s32: value of uncompensated temperature
 *
 *
 *
 *  @return Return the actual temperature as s16 output
 *
*/
s16 bme280_compensate_temperature_int32_sixteen_bit_output(struct bme280_t *bme280, s32 v_uncomp_temperature_s32);





/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION UNCOMPENSATED PRESSURE */
/**************************************************************/
/*!
 *	@brief This API is used to read uncompensated pressure.
 *	in the registers 0xF7, 0xF8 and 0xF9
 *	@note 0xF7 -> MSB -> bit from 0 to 7
 *	@note 0xF8 -> LSB -> bit from 0 to 7
 *	@note 0xF9 -> LSB -> bit from 4 to 7
 *
 *
 *	@param bme280 structure pointer.
 *	@param v_uncomp_pressure_s32 : The value of uncompensated pressure
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_pressure(struct bme280_t *bme280, s32 *v_uncomp_pressure_s32);

/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION TRUE PRESSURE */
/**************************************************************/
/*!
 * @brief Reads actual pressure from uncompensated pressure
 * @note Returns the value in Pascal(Pa)
 * Output value of "96386" equals 96386 Pa =
 * 963.86 hPa = 963.86 millibar
 *
 *
 *  @param bme280 structure pointer.
 *  @param v_uncomp_pressure_s32 : value of uncompensated pressure
 *
 *
 *
 *  @return Return the actual pressure output as u32
 *
*/
u32 bme280_compensate_pressure_int32(struct bme280_t *bme280, s32 v_uncomp_pressure_s32);





/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION UNCOMPENSATED HUMIDITY */
/**************************************************************/
/*!
 *	@brief This API is used to read uncompensated humidity.
 *	in the registers 0xF7, 0xF8 and 0xF9
 *	@note 0xFD -> MSB -> bit from 0 to 7
 *	@note 0xFE -> LSB -> bit from 0 to 7
 *
 *
 *  @param bme280 structure pointer.
 *	@param v_uncomp_humidity_s32 : The value of uncompensated humidity
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_humidity(struct bme280_t *bme280, s32 *v_uncomp_humidity_s32);

/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION RELATIVE HUMIDITY */
/**************************************************************/
/*!
 * @brief Reads actual humidity from uncompensated humidity
 * @note Returns the value in %rH as unsigned 32bit integer
 * in Q22.10 format(22 integer 10 fractional bits).
 * @note An output value of 42313
 * represents 42313 / 1024 = 41.321 %rH
 *
 *
 *  @param bme280 structure pointer.
 *  @param  v_uncomp_humidity_s32: value of uncompensated humidity
 *
 *  @return Return the actual relative humidity output as u32
 *
*/
u32 bme280_compensate_humidity_int32(struct bme280_t *bme280, s32 v_uncomp_humidity_s32);

/*!
 * @brief Reads actual humidity from uncompensated humidity
 * @note Returns the value in %rH as unsigned 16bit integer
 * @note An output value of 42313
 * represents 42313/512 = 82.643 %rH
 *
 *
 *  @param bme280 structure pointer.
 *  @param v_uncomp_humidity_s32: value of uncompensated humidity
 *
 *
 *  @return Return the actual relative humidity output as u16
 *
*/
u16 bme280_compensate_humidity_int32_sixteen_bit_output(struct bme280_t *bme280, s32 v_uncomp_humidity_s32);





/**************************************************************/
/**\name	FUNCTION FOR  INTIALIZATION UNCOMPENSATED PRESSURE,
 TEMPERATURE AND HUMIDITY */
/**************************************************************/
/*!
 * @brief This API used to read uncompensated
 * pressure,temperature and humidity
 *
 *
 *
 *  @param bme280 structure pointer.
 *  @param  v_uncomp_pressure_s32: The value of uncompensated pressure.
 *  @param  v_uncomp_temperature_s32: The value of uncompensated temperature
 *  @param  v_uncomp_humidity_s32: The value of uncompensated humidity.
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_pressure_temperature_humidity(
	struct bme280_t *bme280,
	s32 *v_uncomp_pressure_s32,
	s32 *v_uncomp_temperature_s32,
	s32 *v_uncomp_humidity_s32
);

/**************************************************************/
/**\name	FUNCTION FOR TRUE UNCOMPENSATED PRESSURE,
 TEMPERATURE AND HUMIDITY */
/**************************************************************/
/*!
 * @brief This API used to read true pressure, temperature and humidity
 *
 *
 *
 *  @param bme280 structure pointer.
 *	@param  v_pressure_u32 : The value of compensated pressure.
 *	@param  v_temperature_s32 : The value of compensated temperature.
 *	@param  v_humidity_u32 : The value of compensated humidity.
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_read_pressure_temperature_humidity(
	struct bme280_t *bme280,
	u32 *v_pressure_u32,
	s32 *v_temperature_s32,
	u32 *v_humidity_u32);





/**************************************************************/
/**\name	FUNCTION FOR CALIBRATION */
/**************************************************************/
/*!
 *	@brief This API is used to
 *	calibration parameters used for calculation in the registers
 *
 *  parameter | Register address |   bit
 *------------|------------------|----------------
 *	dig_T1    |  0x88 and 0x89   | from 0 : 7 to 8: 15
 *	dig_T2    |  0x8A and 0x8B   | from 0 : 7 to 8: 15
 *	dig_T3    |  0x8C and 0x8D   | from 0 : 7 to 8: 15
 *	dig_P1    |  0x8E and 0x8F   | from 0 : 7 to 8: 15
 *	dig_P2    |  0x90 and 0x91   | from 0 : 7 to 8: 15
 *	dig_P3    |  0x92 and 0x93   | from 0 : 7 to 8: 15
 *	dig_P4    |  0x94 and 0x95   | from 0 : 7 to 8: 15
 *	dig_P5    |  0x96 and 0x97   | from 0 : 7 to 8: 15
 *	dig_P6    |  0x98 and 0x99   | from 0 : 7 to 8: 15
 *	dig_P7    |  0x9A and 0x9B   | from 0 : 7 to 8: 15
 *	dig_P8    |  0x9C and 0x9D   | from 0 : 7 to 8: 15
 *	dig_P9    |  0x9E and 0x9F   | from 0 : 7 to 8: 15
 *	dig_H1    |         0xA1     | from 0 to 7
 *	dig_H2    |  0xE1 and 0xE2   | from 0 : 7 to 8: 15
 *	dig_H3    |         0xE3     | from 0 to 7
 *
 * 
 * 	@param bme280 structure pointer.
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_calib_param(struct bme280_t *bme280);





/**************************************************************/
/**\name	FUNCTION FOR TEMPERATURE OVER SAMPLING */
/**************************************************************/
/*!
 *	@brief This API is used to get
 *	the temperature oversampling setting in the register 0xF4
 *	bits from 5 to 7
 *
 *	value               |   Temperature oversampling
 * ---------------------|---------------------------------
 *	0x00                | Skipped
 *	0x01                | BME280_OVERSAMP_1X
 *	0x02                | BME280_OVERSAMP_2X
 *	0x03                | BME280_OVERSAMP_4X
 *	0x04                | BME280_OVERSAMP_8X
 *	0x05,0x06 and 0x07  | BME280_OVERSAMP_16X
 *
 *
 *  @param bme280 structure pointer.
 *  @param v_value_u8 : The value of temperature over sampling
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_oversamp_temperature(struct bme280_t *bme280, u8 *v_value_u8);

/*!
 *	@brief This API is used to set
 *	the temperature oversampling setting in the register 0xF4
 *	bits from 5 to 7
 *
 *	value               |   Temperature oversampling
 * ---------------------|---------------------------------
 *	0x00                | Skipped
 *	0x01                | BME280_OVERSAMP_1X
 *	0x02                | BME280_OVERSAMP_2X
 *	0x03                | BME280_OVERSAMP_4X
 *	0x04                | BME280_OVERSAMP_8X
 *	0x05,0x06 and 0x07  | BME280_OVERSAMP_16X
 *
 *
 * 	@param bme280 structure pointer.
 *  @param v_value_u8 : The value of temperature over sampling
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_oversamp_temperature(struct bme280_t *bme280, u8 v_value_u8);




/**************************************************************/
/**\name	FUNCTION FOR PRESSURE OVER SAMPLING */
/**************************************************************/
/*!
 *	@brief This API is used to get
 *	the pressure oversampling setting in the register 0xF4
 *	bits from 2 to 4
 *
 *	value              | Pressure oversampling
 * --------------------|--------------------------
 *	0x00               | Skipped
 *	0x01               | BME280_OVERSAMP_1X
 *	0x02               | BME280_OVERSAMP_2X
 *	0x03               | BME280_OVERSAMP_4X
 *	0x04               | BME280_OVERSAMP_8X
 *	0x05,0x06 and 0x07 | BME280_OVERSAMP_16X
 *
 * 
 *	@param bme280 structure pointer.
 *  @param v_value_u8 : The value of pressure oversampling
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_oversamp_pressure(struct bme280_t *bme280, u8 *v_value_u8);

/*!
 *	@brief This API is used to set
 *	the pressure oversampling setting in the register 0xF4
 *	bits from 2 to 4
 *
 *	value              | Pressure oversampling
 * --------------------|--------------------------
 *	0x00               | Skipped
 *	0x01               | BME280_OVERSAMP_1X
 *	0x02               | BME280_OVERSAMP_2X
 *	0x03               | BME280_OVERSAMP_4X
 *	0x04               | BME280_OVERSAMP_8X
 *	0x05,0x06 and 0x07 | BME280_OVERSAMP_16X
 *
 *
 *  @param bme280 structure pointer.
 *  @param v_value_u8 : The value of pressure oversampling
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_oversamp_pressure(struct bme280_t *bme280, u8 v_value_u8);



/**************************************************************/
/**\name	FUNCTION FOR HUMIDITY OVER SAMPLING */
/**************************************************************/
/*!
 *	@brief This API is used to get
 *	the humidity oversampling setting in the register 0xF2
 *	bits from 0 to 2
 *
 *	value               | Humidity oversampling
 * ---------------------|-------------------------
 *	0x00                | Skipped
 *	0x01                | BME280_OVERSAMP_1X
 *	0x02                | BME280_OVERSAMP_2X
 *	0x03                | BME280_OVERSAMP_4X
 *	0x04                | BME280_OVERSAMP_8X
 *	0x05,0x06 and 0x07  | BME280_OVERSAMP_16X
 *
 *
 *  @param bme280 structure pointer.
 *  @param  v_value_u8 : The value of humidity over sampling
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_oversamp_humidity(struct bme280_t *bme280, u8 *v_value_u8);

/*!
 *	@brief This API is used to set
 *	the humidity oversampling setting in the register 0xF2
 *	bits from 0 to 2
 *
 *	value               | Humidity oversampling
 * ---------------------|-------------------------
 *	0x00                | Skipped
 *	0x01                | BME280_OVERSAMP_1X
 *	0x02                | BME280_OVERSAMP_2X
 *	0x03                | BME280_OVERSAMP_4X
 *	0x04                | BME280_OVERSAMP_8X
 *	0x05,0x06 and 0x07  | BME280_OVERSAMP_16X
 *
 *
 *  @param bme280 structure pointer.
 *  @param  v_value_u8 : The value of humidity over sampling
 *
 *
 * @note The "BME280_CTRL_HUMIDITY_REG_OVERSAMP_HUMIDITY"
 * register sets the humidity
 * data acquisition options of the device.
 * @note changes to this registers only become
 * effective after a write operation to
 * "BME280_CTRL_MEAS_REG" register.
 * @note In the code automated reading and writing of
 *	"BME280_CTRL_HUMIDITY_REG_OVERSAMP_HUMIDITY"
 * @note register first set the
 * "BME280_CTRL_HUMIDITY_REG_OVERSAMP_HUMIDITY"
 *  and then read and write
 *  the "BME280_CTRL_MEAS_REG" register in the function.
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_oversamp_humidity(struct bme280_t *bme280, u8 v_value_u8);




/**************************************************************/
/**\name	FUNCTION FOR POWER MODE*/
/**************************************************************/
/*!
 *	@brief This API used to get the
 *	Operational Mode from the sensor in the register 0xF4 bit 0 and 1
 *
 *
 *	@param bme280 structure pointer.
 *	@param v_power_mode_u8 : The value of power mode
 *  value           |    mode
 * -----------------|------------------
 *	0x00            | BME280_SLEEP_MODE
 *	0x01 and 0x02   | BME280_FORCED_MODE
 *	0x03            | BME280_NORMAL_MODE
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_power_mode(struct bme280_t *bme280, u8 *v_power_mode_u8);

/*!
 *	@brief This API used to set the
 *	Operational Mode from the sensor in the register 0xF4 bit 0 and 1
 *
 *
 *	@param bme280 structure pointer.
 *	@param v_power_mode_u8 : The value of power mode
 *  value           |    mode
 * -----------------|------------------
 *	0x00            | BME280_SLEEP_MODE
 *	0x01 and 0x02   | BME280_FORCED_MODE
 *	0x03            | BME280_NORMAL_MODE
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_power_mode(struct bme280_t *bme280, u8 v_power_mode_u8);



/**************************************************************/
/**\name	FUNCTION FOR SOFT RESET*/
/**************************************************************/
/*!
 * @brief Used to reset the sensor
 * The value 0xB6 is written to the 0xE0
 * register the device is reset using the
 * complete power-on-reset procedure.
 * @note Soft reset can be easily set using bme280_set_softreset().
 * @note Usage Hint : bme280_set_softreset()
 *
 * 
 *	@param bme280 structure pointer.
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_soft_rst(struct bme280_t *bme280);



/**************************************************************/
/**\name	FUNCTION FOR IIR FILTER*/
/**************************************************************/
/*!
 *	@brief This API is used to reads filter setting
 *	in the register 0xF5 bit 3 and 4
 *
 *
 *	@param bme280 structure pointer.
 *	@param v_value_u8 : The value of IIR filter coefficient
 *
 *	value	    |	Filter coefficient
 * -------------|-------------------------
 *	0x00        | BME280_FILTER_COEFF_OFF
 *	0x01        | BME280_FILTER_COEFF_2
 *	0x02        | BME280_FILTER_COEFF_4
 *	0x03        | BME280_FILTER_COEFF_8
 *	0x04        | BME280_FILTER_COEFF_16
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_filter(struct bme280_t *bme280, u8 *v_value_u8);

/*!
 *	@brief This API is used to write filter setting
 *	in the register 0xF5 bit 3 and 4
 *
 *
 *	@param bme280 structure pointer.
 *	@param v_value_u8 : The value of IIR filter coefficient
 *
 *	value	    |	Filter coefficient
 * -------------|-------------------------
 *	0x00        | BME280_FILTER_COEFF_OFF
 *	0x01        | BME280_FILTER_COEFF_2
 *	0x02        | BME280_FILTER_COEFF_4
 *	0x03        | BME280_FILTER_COEFF_8
 *	0x04        | BME280_FILTER_COEFF_16
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_filter(struct bme280_t *bme280, u8 v_value_u8);



/**************************************************************/
/**\name	FUNCTION FOR STANDBY DURATION*/
/**************************************************************/
/*!
 *	@brief This API used to Read the
 *	standby duration time from the sensor in the register 0xF5 bit 5 to 7
 *
 *  @param bme280 structure pointer.
 *	@param v_standby_durn_u8 : The value of standby duration time value.
 *  value       | standby duration
 * -------------|-----------------------
 *    0x00      | BME280_STANDBY_TIME_1_MS
 *    0x01      | BME280_STANDBY_TIME_63_MS
 *    0x02      | BME280_STANDBY_TIME_125_MS
 *    0x03      | BME280_STANDBY_TIME_250_MS
 *    0x04      | BME280_STANDBY_TIME_500_MS
 *    0x05      | BME280_STANDBY_TIME_1000_MS
 *    0x06      | BME280_STANDBY_TIME_2000_MS
 *    0x07      | BME280_STANDBY_TIME_4000_MS
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_standby_durn(struct bme280_t *bme280, u8 *v_standby_durn_u8);
/*!
 *	@brief This API used to write the
 *	standby duration time from the sensor in the register 0xF5 bit 5 to 7
 *
 * 	@param bme280 structure pointer.
 *	@param v_standby_durn_u8 : The value of standby duration time value.
 *
 *  value       | standby duration
 * -------------|-----------------------
 *    0x00      | BME280_STANDBY_TIME_1_MS
 *    0x01      | BME280_STANDBY_TIME_63_MS
 *    0x02      | BME280_STANDBY_TIME_125_MS
 *    0x03      | BME280_STANDBY_TIME_250_MS
 *    0x04      | BME280_STANDBY_TIME_500_MS
 *    0x05      | BME280_STANDBY_TIME_1000_MS
 *    0x06      | BME280_STANDBY_TIME_2000_MS
 *    0x07      | BME280_STANDBY_TIME_4000_MS
 *
 *	@note Normal mode comprises an automated perpetual
 *	cycling between an (active)
 *	Measurement period and an (inactive) standby period.
 *	@note The standby time is determined by
 *	the contents of the register t_sb.
 *	Standby time can be set using BME280_STANDBY_TIME_125_MS.
 *	@note Usage Hint : bme280_set_standby_durn(BME280_STANDBY_TIME_125_MS)
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_standby_durn(struct bme280_t *bme280, u8 v_standby_durn_u8);



/**************************************************************/
/**\name	FUNCTION FOR FORCE MODE DATA READ*/
/**************************************************************/
/*!
 * @brief This API used to read uncompensated
 * temperature,pressure and humidity in forced mode
 *
 *
 *  @param bme280 structure pointer.
 *	@param v_uncom_pressure_s32: The value of uncompensated pressure
 *	@param v_uncom_temperature_s32: The value of uncompensated temperature
 *	@param v_uncom_humidity_s32: The value of uncompensated humidity
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_get_forced_uncomp_pressure_temperature_humidity(
	struct bme280_t *bme280,
	s32 *v_uncom_pressure_s32,
	s32 *v_uncom_temperature_s32,
	s32 *v_uncom_humidity_s32);

/**************************************************************/
/**\name	FUNCTION FOR COMMON READ AND WRITE */
/**************************************************************/
/*!
 * @brief
 *	This API write the data to
 *	the given register
 *
 *
 *  @param bme280 structure pointer.
 *	@param v_addr_u8 -> Address of the register
 *	@param v_data_u8 -> The data from the register
 *	@param v_len_u8 -> no of bytes to read
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
 */
BME280_RETURN_FUNCTION_TYPE bme280_write_register(
	struct bme280_t *bme280,
	u8 v_addr_u8,
	u8 *v_data_u8,
	u8 v_len_u8);

/*!
 * @brief
 *	This API reads the data from
 *	the given register
 *
 *
 *  @param bme280 structure pointer.
 *	@param v_addr_u8 -> Address of the register
 *	@param v_data_u8 -> The data from the register
 *	@param v_len_u8 -> no of bytes to read
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
 */
BME280_RETURN_FUNCTION_TYPE bme280_read_register(
	struct bme280_t *bme280,
	u8 v_addr_u8,
	u8 *v_data_u8,
	u8 v_len_u8);





/**************************************************************/
/**\name	FUNCTION FOR FLOAT OUTPUT TEMPERATURE*/
/**************************************************************/
#ifdef BME280_ENABLE_FLOAT
/*!
 * @brief Reads actual temperature from uncompensated temperature
 * @note returns the value in Degree centigrade
 * @note Output value of "51.23" equals 51.23 DegC.
 *
 *
 *  @param bme280 structure pointer.
 *  @param v_uncom_temperature_s32 : value of uncompensated temperature
 *
 *
 *  @return  Return the actual temperature in floating point
 *
*/
double bme280_compensate_temperature_double(struct bme280_t *bme280, s32 v_uncom_temperature_s32);

/**************************************************************/
/**\name	FUNCTION FOR FLOAT OUTPUT PRESSURE*/
/**************************************************************/
/*!
 * @brief Reads actual pressure from uncompensated pressure
 * @note Returns pressure in Pa as double.
 * @note Output value of "96386.2"
 * equals 96386.2 Pa = 963.862 hPa.
 *
 *
 *  @param bme280 structure pointer.
 *  @param v_uncom_pressure_s32 : value of uncompensated pressure
 *
 *  @return  Return the actual pressure in floating point
 *
*/
double bme280_compensate_pressure_double(struct bme280_t *bme280, s32 v_uncom_pressure_s32);

/**************************************************************/
/**\name	FUNCTION FOR FLOAT OUTPUT HUMIDITY*/
/**************************************************************/
/*!
 * @brief Reads actual humidity from uncompensated humidity
 * @note returns the value in relative humidity (%rH)
 * @note Output value of "42.12" equals 42.12 %rH
 *
 * 
 * @param bme280 structure pointer. 
 * @param v_uncom_humidity_s32 : value of uncompensated humidity
 *
 *
 *  @return Return the actual humidity in floating point
 *
*/
double bme280_compensate_humidity_double(struct bme280_t *bme280, s32 v_uncom_humidity_s32);
#endif





/**************************************************************/
/**\name	FUNCTION FOR 64BIT OUTPUT PRESSURE*/
/**************************************************************/
#if defined(BME280_ENABLE_INT64) && defined(BME280_64BITSUPPORT_PRESENT)
/*!
 * @brief Reads actual pressure from uncompensated pressure
 * @note Returns the value in Pa as unsigned 32 bit
 * integer in Q24.8 format (24 integer bits and
 * 8 fractional bits).
 * @note Output value of "24674867"
 * represents 24674867 / 256 = 96386.2 Pa = 963.862 hPa
 *
 *
 *	@param bme280 structure pointer.
 *  @param  v_uncom_pressure_s32 : value of uncompensated temperature
 *
 *
 *  @return Return the actual pressure in u32
 *
*/
u32 bme280_compensate_pressure_int64(struct bme280_t *bme280, s32 v_uncom_pressure_s32);
/**************************************************************/
/**\name	FUNCTION FOR 24BIT OUTPUT PRESSURE*/
/**************************************************************/
/*!
 * @brief Reads actual pressure from uncompensated pressure
 * @note Returns the value in Pa.
 * @note Output value of "12337434"
 * @note represents 12337434 / 128 = 96386.2 Pa = 963.862 hPa
 *
 *
 *	@param bme280 structure pointer.
 *  @param v_uncom_pressure_s32 : value of uncompensated pressure
 *
 *
 *  @return the actual pressure in u32
 *
*/
u32 bme280_compensate_pressure_int64_twentyfour_bit_output(struct bme280_t *bme280, s32 v_uncom_pressure_s32);
#endif



/**************************************************************/
/**\name	FUNCTION FOR WAIT PERIOD*/
/**************************************************************/
/*!
 * @brief Computing waiting time for sensor data read
 *
 *
 *
 *	@param bme280 structure pointer.
 *  @param v_delaytime_u8 : The value of delay time for force mode
 *
 *
 *	@retval 0 -> Success
 *
 *
 */
BME280_RETURN_FUNCTION_TYPE bme280_compute_wait_time(struct bme280_t *bme280, u8 *v_delaytime_u8);



/**************************************************************/
/**\name	FUNCTION FOR SETTING DEVICE HEIGHT ABOVE SEA LEVEL*/
/**************************************************************/
/*!
 * @brief This function is used to adjust device height above sea level
 * in order to get accurate barometric pressure value
 *
 *
 *  @param bme280 structure pointer.
 *	@param altitude: The value of height above sea level in meters, unsigned int 
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
BME280_RETURN_FUNCTION_TYPE bme280_set_altitude(struct bme280_t *bme280, u16 altitude);
#endif // BME280_H