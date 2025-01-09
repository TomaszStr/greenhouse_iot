#include "bme280.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include <math.h>

static const char *TAG = "BME280_ESP"; 

s64 adjust_pressure_value_by_altitude_s64(struct bme280_t *bme280, s64 pressure){
    if(bme280->altitude > 0){
        const double scale_height = 44330.0;
        const double exponent = -5.255;
        double adjusted_pressure = (double)pressure * pow(1.0 - ((double)bme280->altitude / scale_height), exponent);
        return (int64_t)adjusted_pressure;
    }
    return pressure;
}

double adjust_pressure_value_by_altitude_double(struct bme280_t *bme280, double pressure){
    if(bme280->altitude > 0){
        const double scale_height = 44330.0;
        const double exponent = -5.255;
        double adjusted_pressure = (double)pressure * pow(1.0 - ((double)bme280->altitude / scale_height), exponent);
        return (int64_t)adjusted_pressure;
    }
    return pressure;
}

BME280_RETURN_FUNCTION_TYPE bme280_write_register(struct bme280_t *bme280, u8 v_addr_u8, u8 *v_data_u8, u8 v_len_u8)
{
	BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
	if (bme280 == NULL) {
		return E_BME280_NULL_PTR;
		} else {
			com_rslt = bme280->BME280_BUS_WRITE_FUNC(
			bme280->dev_addr,
			v_addr_u8, v_data_u8, v_len_u8);
		}
	return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_read_register(struct bme280_t *bme280, u8 v_addr_u8, u8 *v_data_u8, u8 v_len_u8)
{
	BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
	if (bme280 == NULL) {
		return E_BME280_NULL_PTR;
		} else {
			com_rslt = bme280->BME280_BUS_READ_FUNC(
			bme280->dev_addr,
			v_addr_u8, v_data_u8, v_len_u8);
		}
	return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_get_calib_param(struct bme280_t *bme280)
{
    esp_err_t err;
    uint8_t calib_data[26];
    uint8_t calib_reg_addr = 0x88;

    err = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, calib_reg_addr, calib_data, 26);
    if (err != 0)
        return -1;

    bme280->cal_param.dig_T1 = (u16)(calib_data[0] | (calib_data[1] << 8));
    bme280->cal_param.dig_T2 = (s16)(calib_data[2] | (calib_data[3] << 8));
    bme280->cal_param.dig_T3 = (s16)(calib_data[4] | (calib_data[5] << 8));
    
    bme280->cal_param.dig_P1 = (u16)(calib_data[6] | (calib_data[7] << 8));
    bme280->cal_param.dig_P2 = (s16)(calib_data[8] | (calib_data[9] << 8));
    bme280->cal_param.dig_P3 = (s16)(calib_data[10] | (calib_data[11] << 8));
    bme280->cal_param.dig_P4 = (s16)(calib_data[12] | (calib_data[13] << 8));
    bme280->cal_param.dig_P5 = (s16)(calib_data[14] | (calib_data[15] << 8));
    bme280->cal_param.dig_P6 = (s16)(calib_data[16] | (calib_data[17] << 8));
    bme280->cal_param.dig_P7 = (s16)(calib_data[18] | (calib_data[19] << 8));
    bme280->cal_param.dig_P8 = (s16)(calib_data[20] | (calib_data[21] << 8));
    bme280->cal_param.dig_P9 = (s16)(calib_data[22] | (calib_data[23] << 8));

    bme280->cal_param.dig_H1 = calib_data[25];

    // Read data from 0xE1 - 0xE6
    calib_reg_addr = 0xE1;

    err = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, calib_reg_addr, calib_data, 7);
    if (err != 0)
        return -1;

    bme280->cal_param.dig_H2 = (s16)(((s8)calib_data[1] << 8) | calib_data[0]);
    bme280->cal_param.dig_H3 = calib_data[2];
    bme280->cal_param.dig_H4 = (s16)((((s16)((s8)calib_data[3])) << 4) | (((u8)0x0F) & calib_data[4]));
    bme280->cal_param.dig_H5 = (s16)((((s16)((s8)calib_data[5])) << 4) | (calib_data[4] >> 4));
    bme280->cal_param.dig_H6 = calib_data[6];

    return 0;
}


BME280_RETURN_FUNCTION_TYPE bme280_init(struct bme280_t *bme280)
{
	BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
	u8 v_data_u8 = 0;
	u8 v_chip_id_read_count = 5;

	struct bme280_t *p_bme280 = bme280;

	while (v_chip_id_read_count > 0)
	{

		/* read Chip Id */
		com_rslt = p_bme280->BME280_BUS_READ_FUNC(p_bme280->dev_addr, BME280_REG_ID, &v_data_u8, 1);
		/* Check for the correct chip id */
		if (v_data_u8 == BME280_CHIP_ID){
            break;
        }
        ESP_LOGW(TAG, "Failed to read chip ID. Chip: %d Error: %d", v_data_u8, com_rslt);
		v_chip_id_read_count--;
		/* Delay added concerning the low speed of power up system to
		facilitate the proper reading of the chip ID */
		p_bme280->delay_msec(1);
	}
	p_bme280->chip_id = v_data_u8;
	com_rslt = (v_chip_id_read_count == 0) ? BME280_CHIP_ID_READ_FAIL : BME280_CHIP_ID_READ_SUCCESS;

	if (com_rslt == BME280_CHIP_ID_READ_SUCCESS)
	{
		/* readout bme280 calibparam structure */
		com_rslt += bme280_get_calib_param(p_bme280);
	}
	return com_rslt;
}



BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_temperature(struct bme280_t *bme280, s32 *v_uncomp_temperature_s32)
{
    uint8_t reg_addr = BME280_REG_TEMP_MSB;
    uint8_t temp_data[3] = {0};
    esp_err_t err;

    if (bme280 == NULL || v_uncomp_temperature_s32 == NULL) {
        return -1;
    }

    err = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, reg_addr, temp_data, 3);
    if (err != ESP_OK) {
        return -1;
    }

    // Combine the 20-bit temperature value
    *v_uncomp_temperature_s32 = (s32)(((u32)temp_data[0] << 12) |
                                         ((u32)temp_data[1] << 4) |
                                         ((u32)temp_data[2] >> 4));

    return 0;
}

s32 bme280_compensate_temperature_int32(struct bme280_t *bme280, s32 v_uncomp_temperature_s32)
{
    s32 var1, var2, temperature;

    var1 = ((((v_uncomp_temperature_s32 >> 3) - ((s32)bme280->cal_param.dig_T1 << 1))) * ((s32)bme280->cal_param.dig_T2)) >> 11;

    var2 = (((((v_uncomp_temperature_s32 >> 4) - ((s32)bme280->cal_param.dig_T1)) * ((v_uncomp_temperature_s32 >> 4) - ((s32)bme280->cal_param.dig_T1))) >> 12) * ((s32)bme280->cal_param.dig_T3)) >> 14;

    bme280->cal_param.t_fine = var1 + var2;

    temperature = (bme280->cal_param.t_fine * 5 + 128) >> 8;

    return temperature;
}

s16 bme280_compensate_temperature_int32_sixteen_bit_output(struct bme280_t *bme280, s32 v_uncomp_temperature_s32)
{
    s16 temperature = 0;

	bme280_compensate_temperature_int32(bme280, v_uncomp_temperature_s32);
	temperature  = (s16)((((bme280->cal_param.t_fine - 122880) * 25) + 128) >> 8);

	return temperature;
}



BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_pressure(struct bme280_t *bme280, s32 *v_uncomp_pressure_s32)
{
    uint8_t reg_addr = BME280_REG_PRESS_MSB;
    uint8_t data[3];
    esp_err_t err;

    err = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, reg_addr, data, 3);
    if (err != ESP_OK) {
        return -1;
    }

    *v_uncomp_pressure_s32 = (s32)(((u32)data[0] << 12) |
                                ((u32)data[1] << 4) |
                                ((u32)(data[2] >> 4)));

    return 0;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
u32 bme280_compensate_pressure_int32(struct bme280_t *bme280, s32 v_uncomp_pressure_s32)
{
    s64 var1, var2;
    s64 pressure;

    var1 = ((s64)bme280->cal_param.t_fine) - 128000;
    var2 = var1 * var1 * (s64)bme280->cal_param.dig_P6;
    var2 = var2 + ((var1*(s64)bme280->cal_param.dig_P5)<<17);
    var2 = var2 + (((s64)bme280->cal_param.dig_P4)<<35);
    var1 = ((var1 * var1 * (s64)bme280->cal_param.dig_P3)>>8) + ((var1 * (s64)bme280->cal_param.dig_P2)<<12);
    var1 = (((((s64)1)<<47)+var1))*((s64)bme280->cal_param.dig_P1)>>33; 
    if (var1 == 0)  {   
        return 0;
    }
    pressure = 1048576-v_uncomp_pressure_s32;
    pressure = (((pressure<<31)-var2)*3125)/var1;
    var1 = (((s64)bme280->cal_param.dig_P9) * (pressure>>13) * (pressure>>13)) >> 25;
    var2 = (((s64)bme280->cal_param.dig_P8) * pressure) >> 19;
    pressure = ((pressure + var1 + var2) >> 8) + (((s64)bme280->cal_param.dig_P7)<<4);

    return (s64)adjust_pressure_value_by_altitude_s64(bme280, pressure);
}

BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_humidity(struct bme280_t *bme280, s32 *v_uncomp_humidity_s32) {
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 a_data_u8[2] = {0, 0};

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    }

    com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_REG_HUM_MSB, a_data_u8, 2);

    *v_uncomp_humidity_s32 = (s32)(((s32)(a_data_u8[0]) << 8) | (s32)(a_data_u8[1]));

    return com_rslt;
}

u32 bme280_compensate_humidity_int32(struct bme280_t *bme280, s32 v_uncomp_humidity_s32)
{
    s32 var1;

    var1 = (bme280->cal_param.t_fine - ((s32)76800));
    var1 = (((((v_uncomp_humidity_s32 << 14) - (((s32)bme280->cal_param.dig_H4) << 20) - (((s32)bme280->cal_param.dig_H5) * 
        var1)) + ((s32)16384)) >> 15) * (((((((var1 * 
        ((s32)bme280->cal_param.dig_H6)) >> 10) * (((var1 * ((s32)bme280->cal_param.dig_H3)) >> 11) + 
        ((s32)32768))) >> 10) + ((s32)2097152)) * ((s32)bme280->cal_param.dig_H2) + 
        8192) >> 14)); 
    var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * 
        ((s32)bme280->cal_param.dig_H1)) >> 4));
    var1 = (var1 < 0 ? 0 : var1);
    var1 = (var1 > 419430400 ? 419430400 : var1);
    return (u32)(var1>>12);

}

u16 bme280_compensate_humidity_int32_sixteen_bit_output(struct bme280_t *bme280, s32 v_uncomp_humidity_s32)
{
    u32 var1 = 0;
	u16 var2 = 0;

	var1 =  bme280_compensate_humidity_int32(bme280, v_uncomp_humidity_s32);
	var2 = (u16)(var1 >> 1);

    return var2;
}



BME280_RETURN_FUNCTION_TYPE bme280_read_uncomp_pressure_temperature_humidity(
    struct bme280_t *bme280,
    s32 *v_uncomp_pressure_s32,
    s32 *v_uncomp_temperature_s32,
    s32 *v_uncomp_humidity_s32)
{
    uint8_t reg_addr = BME280_REG_PRESS_MSB;
    uint8_t data[8] = {0,0,0,0, 0,0,0,0};
    esp_err_t err;

    err = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, reg_addr, data, 8);
    if (err != ESP_OK) {
        return -1;
    }

    *v_uncomp_pressure_s32 = (s32)(((u32)data[0] << 12) | ((u32)data[1] << 4) | ((u32)data[2] >> 4));  // Pressure 3 bytes
    *v_uncomp_temperature_s32 = (s32)(((u32)data[3] << 12) | ((u32)data[4] << 4) | ((u32)data[5] >> 4));  // Temperature 3 bytes
    *v_uncomp_humidity_s32 = (s32)(((u32)data[6] << 8) | ((u32)(data[7])));  // Humidity 2 bytes

    return 0;
}

BME280_RETURN_FUNCTION_TYPE bme280_read_pressure_temperature_humidity(
    struct bme280_t *bme280,
    u32 *v_pressure_u32,
    s32 *v_temperature_s32,
    u32 *v_humidity_u32
)
{
    s32 uncomp_pressure;
    s32 uncomp_temperature;
    s32 uncomp_humidity;
    BME280_RETURN_FUNCTION_TYPE result;

    result = bme280_read_uncomp_pressure_temperature_humidity(bme280, &uncomp_pressure, &uncomp_temperature, &uncomp_humidity);
    if (result != 0) {
        return result;
    }

    *v_temperature_s32 = bme280_compensate_temperature_int32(bme280, uncomp_temperature);
    *v_pressure_u32 = bme280_compensate_pressure_int32(bme280, uncomp_pressure);
    *v_humidity_u32 = bme280_compensate_humidity_int32(bme280, uncomp_humidity);

    return 0;
}



BME280_RETURN_FUNCTION_TYPE bme280_get_power_mode(struct bme280_t *bme280, u8 *v_power_mode_u8) {
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_mode_u8r = 0;
    
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    }

    com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_CTRL_MEAS_REG, &v_mode_u8r, 1);

    if (com_rslt == ERROR) {
        return com_rslt;
    }

    *v_power_mode_u8 = ((v_mode_u8r & (0x03)) >> (0));

    return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_set_power_mode(struct bme280_t *bme280, u8 v_power_mode_u8) {
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_mode_u8r = 0;
	u8 v_prev_pow_mode_u8 = 0;
	u8 v_pre_ctrl_hum_value_u8 = 0;
	u8 v_pre_config_value_u8 = 0;
	u8 v_data_u8 = 0;

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    }

    if (v_power_mode_u8 <= BME280_NORMAL_MODE) {
        v_mode_u8r = bme280->ctrl_meas_reg;
        v_mode_u8r =((v_mode_u8r & ~(0x03)) | ((v_power_mode_u8<<(0))&(0x03)));
        com_rslt = bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);
        if (v_prev_pow_mode_u8 != BME280_SLEEP_MODE) {
            com_rslt += bme280_set_soft_rst(bme280);
            bme280->delay_msec(3);
            /* write previous value of
            configuration register*/
            v_pre_config_value_u8 =
            bme280->config_reg;
            com_rslt = bme280_write_register(bme280, BME280_CONFIG_REG, &v_pre_config_value_u8, 1);
            /* write previous value of
            humidity oversampling*/
            v_pre_ctrl_hum_value_u8 =
            bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_pre_ctrl_hum_value_u8, 1);
            /* write previous and updated value of
            control measurement register*/
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &v_mode_u8r, 1);
        } else {
            com_rslt = bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &v_mode_u8r, 1);
        }
        /* read the control measurement register value*/
        com_rslt = bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;
        /* read the control humidity register value*/
        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;
        /* read the config register value*/
        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;
    } else {
    com_rslt = E_BME280_OUT_OF_RANGE;
    }

    return com_rslt;
}



BME280_RETURN_FUNCTION_TYPE bme280_get_oversamp_temperature(struct bme280_t *bme280, u8 *v_value_u8) {
    /* used to return the communication result*/
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    }

    com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_CTRL_MEAS_REG, &v_data_u8, 1);

    if (com_rslt != ERROR) {
        *v_value_u8 = ((v_data_u8 & (0xE0)) >> (5));
        bme280->oversamp_temperature = *v_value_u8;
    }

    return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_set_oversamp_temperature(struct bme280_t *bme280, u8 v_value_u8)
{
    /* used to return the communication result */
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;  /* Variable to store the current control measurement register value */
    u8 v_prev_pow_mode_u8 = 0;  /* Variable to store previous power mode */
    u8 v_pre_ctrl_hum_value_u8 = 0;  /* Variable to store previous control humidity register value */
    u8 v_pre_config_value_u8 = 0;  /* Variable to store previous config register value */

    /* Check if the structure pointer is NULL */
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Read the current value of the control measurement register */
        v_data_u8 = bme280->ctrl_meas_reg;

        v_data_u8 = ((v_data_u8 & ~(0xE0)) | ((v_value_u8<<(5))&(0xE0)));  // 0x1F = 00011111 in binary, clears the upper 3 bits

        com_rslt = bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);
        if (com_rslt != SUCCESS) {
            return com_rslt;
        }

        /* If not in sleep mode, perform a soft reset and update the registers */
        if (v_prev_pow_mode_u8 != BME280_SLEEP_MODE) {
            com_rslt += bme280_set_soft_rst(bme280);
            bme280->delay_msec(3);

            /* Write the previous value of the configuration register */
            v_pre_config_value_u8 = bme280->config_reg;
            com_rslt += bme280_write_register(bme280, BME280_CONFIG_REG, &v_pre_config_value_u8, 1);

            /* Write the previous value of the humidity oversampling */
            v_pre_ctrl_hum_value_u8 = bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_pre_ctrl_hum_value_u8, 1);

            /* Write the updated control measurement register */
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        } else {
            /* If in sleep mode, write directly to the register */
            com_rslt = bme280->BME280_BUS_WRITE_FUNC(bme280->dev_addr, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        }

        /* Update the oversampling temperature in the structure */
        bme280->oversamp_temperature = v_value_u8;

        /* Read the control measurement register value */
        com_rslt = bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;

        /* Read the control humidity register value */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;

        /* Read the configuration register value */
        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;
    }

    return com_rslt;
}


BME280_RETURN_FUNCTION_TYPE bme280_get_oversamp_pressure(struct bme280_t *bme280, u8 *v_value_u8)
{
	BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
	u8 v_data_u8 = 0;

	if (bme280 == NULL) {
		return E_BME280_NULL_PTR;
	} else {
		com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_CTRL_MEAS_REG, &v_data_u8, 1);

		*v_value_u8 = ((v_data_u8 & (0x1C)) >> (2));

		bme280->oversamp_pressure = *v_value_u8;
	}

	return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_set_oversamp_pressure(struct bme280_t *bme280, u8 v_value_u8)
{
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;
    u8 v_prev_pow_mode_u8 = 0;
    u8 v_pre_ctrl_hum_value_u8 = 0;
    u8 v_pre_config_value_u8 = 0;

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Get the current measurement control register value */
        v_data_u8 = bme280->ctrl_meas_reg;

        /* Modify the oversampling bits for pressure (bits 2-4) */
        // Clear bits 2-4 (masking)
        // Set the new oversampling value (shifted into bits 2-4)
        v_data_u8 = ((v_data_u8 & ~(0x1C)) | ((v_value_u8<<(2))&(0x1C)));

        /* Get the current power mode */
        com_rslt = bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);

        if (v_prev_pow_mode_u8 != BME280_SLEEP_MODE) {
            /* If the sensor is not in sleep mode, perform a soft reset */
            com_rslt += bme280_set_soft_rst(bme280);
            /* Simple delay (using a basic busy-wait loop or appropriate delay function) */
            bme280->delay_msec(3);  // Use a 3ms delay

            /* Save the previous configuration register value */
            /* Write the previous configuration register value */
            v_pre_config_value_u8 = bme280->config_reg;
            com_rslt = bme280_write_register(bme280, BME280_CONFIG_REG, &v_pre_config_value_u8, 1);  // Assuming BME280_CONFIG_REG is the config register

            /* Save the previous humidity oversampling value */
            /* Write the previous humidity oversampling value */
            v_pre_ctrl_hum_value_u8 = bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_pre_ctrl_hum_value_u8, 1);  // Assuming BME280_CTRL_HUMIDITY_REG is the humidity control register

            /* Write the updated value of the measurement control register */
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        } else {
            /* If the sensor is in sleep mode, just write the new oversampling value */
            // Same register for control of measurements
            com_rslt = bme280->BME280_BUS_WRITE_FUNC(bme280->dev_addr, BME280_CTRL_MEAS_REG,  &v_data_u8, 1);
        }

        /* Update the oversampling value in the internal state */
        bme280->oversamp_pressure = v_value_u8;

        /* Read the control measurement register value */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;

        /* Read the control humidity register value */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;

        /* Read the control configuration register value */
        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;
    }

    return com_rslt;
}



BME280_RETURN_FUNCTION_TYPE bme280_get_oversamp_humidity(struct bme280_t *bme280, u8 *v_value_u8)
{
    /* Used to return the communication result */
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;

    /* Check if the bme280 pointer is NULL */
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Read the humidity control register (BME280_CTRL_HUMIDITY_REG) */
        com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);  // Read 1 byte

        /* Check for communication errors */
        if (com_rslt != 0) {
            return com_rslt;
        }

        /* Extract the oversampling bits (bits 0-2) */
        *v_value_u8 = ((v_data_u8 & (0x07)) >> (0));  // Mask the first 3 bits

        /* Update the internal oversampling value */
        bme280->oversamp_humidity = *v_value_u8;
    }

    return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_set_oversamp_humidity(struct bme280_t *bme280, u8 v_value_u8)
{
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;
    u8 pre_ctrl_meas_value = 0;
    u8 v_pre_config_value_u8 = 0;
    u8 v_prev_pow_mode_u8 = 0;

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Get the current humidity control register value */
        com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);

        /* Modify the oversampling bits for humidity (bits 0-2) */
        v_data_u8 = ((v_data_u8 & ~(0x07)) | ((v_value_u8)&(0x07)));
        
        /* Check the current power mode */
        com_rslt += bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);

        if (v_prev_pow_mode_u8 != BME280_SLEEP_MODE) {
            /* If not in sleep mode, reset the sensor and configure */
            com_rslt += bme280_set_soft_rst(bme280);
            bme280->delay_msec(3);  // 3ms delay for soft reset

            /* write previous value of
            configuration register*/
            v_pre_config_value_u8 = bme280->config_reg;
            com_rslt += bme280_write_register(bme280, BME280_CONFIG_REG, &v_pre_config_value_u8, 1);
            /* write the value of control humidity*/
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
            /* write previous value of
            control measurement register*/
            pre_ctrl_meas_value = bme280->ctrl_meas_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &pre_ctrl_meas_value, 1);
        }
        else {
            com_rslt = bme280->BME280_BUS_WRITE_FUNC(bme280->dev_addr, BME280_CTRL_HUMIDITY_REG,  &v_data_u8, 1);
            // com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        }

        /* Update the internal oversampling humidity value */
        bme280->oversamp_humidity = v_value_u8;

        /* Read the control measurement, humidity, and configuration registers */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;
    }

    return com_rslt;
}




BME280_RETURN_FUNCTION_TYPE bme280_set_soft_rst(struct bme280_t *bme280)
{
    /* Used to return the communication result */
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = BME280_SOFT_RESET_CODE;

    /* Check if the bme280 pointer is NULL */
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Write the reset code to the reset register (0xE0) */
        com_rslt = bme280->BME280_BUS_WRITE_FUNC(bme280->dev_addr, 0xE0, &v_data_u8, 1);
    }

    return com_rslt;
}




BME280_RETURN_FUNCTION_TYPE bme280_get_filter(struct bme280_t *bme280, u8 *v_value_u8)
{
    /* Used to return the communication result */
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;

    /* Check if the bme280 pointer is NULL */
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Read the value from the filter register (BME280_CONFIG_REG) */
        com_rslt = bme280->BME280_BUS_READ_FUNC(bme280->dev_addr, BME280_CONFIG_REG, &v_data_u8, 1);

        /* Extract the filter setting (bits 3 and 4) */
        *v_value_u8 = ((v_data_u8 & (0x1C)) >> (2));
    }

    return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_set_filter(struct bme280_t *bme280, u8 v_value_u8)
{
    /* Used to return the communication result */
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;
    u8 pre_ctrl_meas_value = 0;
    u8 v_prev_pow_mode_u8 = 0;
    u8 v_pre_ctrl_hum_value_u8 = 0;

    /* Check if the bme280 pointer is NULL */
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Get the current configuration register value */
        v_data_u8 = bme280->config_reg;

        /* Set the filter setting */
        v_data_u8 = ((v_data_u8 & ~(0x1C)) | ((v_value_u8<<(2))&(0x1C)));  // Setting the filter bits (3 and 4)

        /* Check the current power mode */
        com_rslt = bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);
        if (v_prev_pow_mode_u8 != 0x01) {  // If not in sleep mode
            /* Perform soft reset if not in sleep mode */
            com_rslt += bme280_set_soft_rst(bme280);
            bme280->delay_msec(3);  // A small delay for reset

            /* Write the updated configuration register value */
            com_rslt += bme280_write_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);

            /* Write the previous values of other registers */
            v_pre_ctrl_hum_value_u8 = bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG,&v_pre_ctrl_hum_value_u8, 1);

            pre_ctrl_meas_value = bme280->ctrl_meas_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &pre_ctrl_meas_value, 1);
        } else {
            /* If in sleep mode, directly write the updated filter setting */
            com_rslt = bme280->BME280_BUS_WRITE_FUNC(bme280->dev_addr, BME280_CONFIG_REG, &v_data_u8, 1);
        }

        /* Read and update register values */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;
    }

    return com_rslt;
}



BME280_RETURN_FUNCTION_TYPE bme280_get_standby_durn(struct bme280_t *bme280, u8 *v_standby_durn_u8)
{
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Read the current value of the configuration register */
        com_rslt = bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        
        /* Extract the bits 5 to 7 for standby duration */
        *v_standby_durn_u8 = ((v_data_u8 & (0xE0)) >> (5));
    }

    return com_rslt;
}

BME280_RETURN_FUNCTION_TYPE bme280_set_standby_durn(struct bme280_t *bme280, u8 v_standby_durn_u8)
{
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;
    u8 pre_ctrl_meas_value = 0;
    u8 v_prev_pow_mode_u8 = 0;
    u8 v_pre_ctrl_hum_value_u8 = 0;

    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Read the current value of the configuration register */
        v_data_u8 = bme280->config_reg;
        
        /* Set the standby duration bits (bits 5-7) */
        v_data_u8 = ((v_data_u8 & ~(0xE0)) | ((v_standby_durn_u8<<(5))&(0xE0)));

        /* Read the current power mode to handle sleep mode */
        com_rslt = bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);
        
        if (v_prev_pow_mode_u8 != BME280_SLEEP_MODE) {
            /* If the sensor is not in sleep mode, perform a soft reset and update registers */
            com_rslt += bme280_set_soft_rst(bme280);
            bme280->delay_msec(3);  // 3 ms delay for reset

            /* Write updated configuration register value */
            com_rslt += bme280_write_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
            
            /* Write the previous values of humidity and measurement control registers */
            v_pre_ctrl_hum_value_u8 = bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_pre_ctrl_hum_value_u8, 1);

            pre_ctrl_meas_value = bme280->ctrl_meas_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &pre_ctrl_meas_value, 1);
        } else {
            /* If the sensor is in sleep mode, directly update the configuration register */
            com_rslt += bme280->BME280_BUS_WRITE_FUNC(bme280->dev_addr, BME280_CONFIG_REG, &v_data_u8, 1);
        }

        /* Read and update the registers after modification */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;
    }

    return com_rslt;
}





BME280_RETURN_FUNCTION_TYPE bme280_get_forced_uncomp_pressure_temperature_humidity(
    struct bme280_t *bme280,
    s32 *v_uncom_pressure_s32,
    s32 *v_uncom_temperature_s32, 
    s32 *v_uncom_humidity_s32)
{
    /* Used to return the communication result */
    BME280_RETURN_FUNCTION_TYPE com_rslt = ERROR;
    u8 v_data_u8 = 0;
    u8 v_waittime_u8 = 0;
    u8 v_prev_pow_mode_u8 = 0;
    u8 v_mode_u8r = 0;
    u8 pre_ctrl_config_value = 0;
    u8 v_pre_ctrl_hum_value_u8 = 0;

    /* Check if the bme280 structure pointer is NULL */
    if (bme280 == NULL) {
        return E_BME280_NULL_PTR;
    } else {
        /* Read the current value of ctrl_meas register */
        v_mode_u8r = bme280->ctrl_meas_reg;

        /* Set the forced mode by clearing bits [0:1] and setting them to 01 */
        /* Clear bits 0 and 1 (mask 11111100) */
        /* Set to forced mode (01) */
        v_mode_u8r = ((v_mode_u8r & ~(0x03)) | (((0x01)<<(0))&(0x03)));

        /* Get the current power mode */
        com_rslt = bme280_get_power_mode(bme280, &v_prev_pow_mode_u8);
        if (v_prev_pow_mode_u8 != BME280_SLEEP_MODE) {
            /* Perform a soft reset */
            com_rslt += bme280_set_soft_rst(bme280);
            bme280->delay_msec(3);

            /* Restore previous values of configuration registers */
            pre_ctrl_config_value = bme280->config_reg;
            com_rslt += bme280_write_register(bme280, BME280_CONFIG_REG, &pre_ctrl_config_value, 1);

            v_pre_ctrl_hum_value_u8 = bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_pre_ctrl_hum_value_u8, 1);

            /* Write forced mode to ctrl_meas register */
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &v_mode_u8r, 1);
        } else {
            /* Write humidity oversampling first */
            v_pre_ctrl_hum_value_u8 = bme280->ctrl_hum_reg;
            com_rslt += bme280_write_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_pre_ctrl_hum_value_u8, 1);

            /* Write forced mode directly */
            com_rslt += bme280_write_register(bme280, BME280_CTRL_MEAS_REG, &v_mode_u8r, 1);
        }

        /* Compute the wait time and delay */
        bme280_compute_wait_time(bme280, &v_waittime_u8);
        bme280->delay_msec(v_waittime_u8);

        /* Read uncompensated pressure, temperature, and humidity */
        com_rslt += bme280_read_uncomp_pressure_temperature_humidity(bme280,  v_uncom_pressure_s32,  v_uncom_temperature_s32, v_uncom_humidity_s32);

        /* Read and store updated register values */
        com_rslt += bme280_read_register(bme280, BME280_CTRL_HUMIDITY_REG, &v_data_u8, 1);
        bme280->ctrl_hum_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CONFIG_REG, &v_data_u8, 1);
        bme280->config_reg = v_data_u8;

        com_rslt += bme280_read_register(bme280, BME280_CTRL_MEAS_REG, &v_data_u8, 1);
        bme280->ctrl_meas_reg = v_data_u8;
    }
    return com_rslt;
}





#ifdef BME280_ENABLE_FLOAT

double bme280_compensate_temperature_double(struct bme280_t *bme280, s32 v_uncom_temperature_s32)
{
    /* Check if the pointer to bme280 is NULL */
    if (bme280 == NULL) {
        return 0.0;
    }

    double v_x1_u32 = 0;
	double v_x2_u32 = 0;
	double temperature = 0;

	v_x1_u32  = (((double)v_uncom_temperature_s32) / 16384.0 -
	((double)bme280->cal_param.dig_T1) / 1024.0) *
	((double)bme280->cal_param.dig_T2);
	v_x2_u32  = ((((double)v_uncom_temperature_s32) / 131072.0 -
	((double)bme280->cal_param.dig_T1) / 8192.0) *
	(((double)v_uncom_temperature_s32) / 131072.0 -
	((double)bme280->cal_param.dig_T1) / 8192.0)) *
	((double)bme280->cal_param.dig_T3);
	bme280->cal_param.t_fine = (s32)(v_x1_u32 + v_x2_u32);
	temperature  = (v_x1_u32 + v_x2_u32) / 5120.0;


	return temperature;
}

double bme280_compensate_pressure_double(struct bme280_t *bme280, s32 v_uncom_pressure_s32)
{
    if (bme280 == NULL) {
        return 0.0;
    }

    double v_x1_u32 = 0;
	double v_x2_u32 = 0;
	double pressure = 0;

	v_x1_u32 = ((double)bme280->cal_param.t_fine /
	2.0) - 64000.0;
	v_x2_u32 = v_x1_u32 * v_x1_u32 *
	((double)bme280->cal_param.dig_P6) / 32768.0;
	v_x2_u32 = v_x2_u32 + v_x1_u32 *
	((double)bme280->cal_param.dig_P5) * 2.0;
	v_x2_u32 = (v_x2_u32 / 4.0) +
	(((double)bme280->cal_param.dig_P4) * 65536.0);
	v_x1_u32 = (((double)bme280->cal_param.dig_P3) *
	v_x1_u32 * v_x1_u32 / 524288.0 +
	((double)bme280->cal_param.dig_P2) * v_x1_u32) / 524288.0;
	v_x1_u32 = (1.0 + v_x1_u32 / 32768.0) *
	((double)bme280->cal_param.dig_P1);
	pressure = 1048576.0 - (double)v_uncom_pressure_s32;
	/* Avoid exception caused by division by zero */
	if ((v_x1_u32 > 0) || (v_x1_u32 < 0))
		pressure = (pressure - (v_x2_u32 / 4096.0)) * 6250.0 / v_x1_u32;
	else
		return 0;
	v_x1_u32 = ((double)bme280->cal_param.dig_P9) *
	pressure * pressure / 2147483648.0;
	v_x2_u32 = pressure * ((double)bme280->cal_param.dig_P8) / 32768.0;
	pressure = pressure + (v_x1_u32 + v_x2_u32 +
	((double)bme280->cal_param.dig_P7)) / 16.0;

	return adjust_pressure_value_by_altitude_double(bme280, pressure);
}

double bme280_compensate_humidity_double(struct bme280_t *bme280, s32 v_uncom_humidity_s32)
{
    if (bme280 == NULL) {
        return 0.0;
    }
    double var_h = 0;

    var_h = (((double)bme280->cal_param.t_fine) - 76800.0);
	if ((var_h > 0) || (var_h < 0))
		var_h = (v_uncom_humidity_s32 -
		(((double)bme280->cal_param.dig_H4) * 64.0 +
		((double)bme280->cal_param.dig_H5) / 16384.0 * var_h))*
		(((double)bme280->cal_param.dig_H2) / 65536.0 *
		(1.0 + ((double) bme280->cal_param.dig_H6)
		/ 67108864.0 * var_h * (1.0 + ((double)
		bme280->cal_param.dig_H3) / 67108864.0 * var_h)));
	else
		return 0;
	var_h = var_h * (1.0 - ((double)
	bme280->cal_param.dig_H1)*var_h / 524288.0);
	if (var_h > 100.0)
		var_h = 100.0;
	else if (var_h < 0.0)
		var_h = 0.0;

	return var_h;
}

#endif

BME280_RETURN_FUNCTION_TYPE bme280_compute_wait_time(struct bme280_t *bme280, u8 *v_delaytime_u8)
{
    BME280_RETURN_FUNCTION_TYPE com_rslt = SUCCESS;

    if (bme280 == NULL || v_delaytime_u8 == NULL) {
        return E_BME280_NULL_PTR;
    }
    
    /* Calculate the waiting time using oversampling and setup values */
    *v_delaytime_u8 = (20 +
                       37 *
                       (((1 << bme280->oversamp_temperature) >> 1)
                       + ((1 << bme280->oversamp_pressure) >> 1) +
                       ((1 << bme280->oversamp_humidity) >> 1))
                       + ((bme280->oversamp_pressure > 0) ? 10 : 0) +
                       ((bme280->oversamp_humidity > 0) ? 10 : 0) + 15) / 16;

    return com_rslt;  // Return success
}


BME280_RETURN_FUNCTION_TYPE bme280_set_altitude(struct bme280_t *bme280, u16 altitude){
    bme280->altitude = altitude;
    return SUCCESS;
}
