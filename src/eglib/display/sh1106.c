#include "sh1106.h"

//
// Defines from Datasheet
//

#define SH1106_RESET_LOW_MS 10
#define SH1106_RESET_HIGH_MS 2

#define SH1106_SET_LOWER_COLUMN_ADDRESS(addr) (0x00|((addr)&0x0F))
#define SH1106_SET_HIGHER_COLUMN_ADDRESS(addr) (0x10|((addr)>>4))

#define SHH1106_SET_PUMP_VOLTAGE(voltage) (0x30|((voltage)&0x3))

#define SH1106_SET_DISPLAY_START_LINE(line) (0x40|((line)&0x3F))

#define SH1106_SET_CONTRAST_CONTROL_REGISTER 0x81

#define SH1106_SET_SEGMENT_REMAP(dir) (0xA0|((dir)&0x1))

#define SH1106_SET_ENTIRE_DISPLAY_OFF 0xA4
#define SH1106_SET_ENTIRE_DISPLAY_ON 0xA5

#define SH1106_SET_NORMAL_DISPLAY 0xA6
#define SH1106_SET_REVERSE_DISPLAY 0xA7

#define SH1106_SET_MULTIPLEX_RATIO 0xA8
#define SH1106_SET_MULTIPLEX_RATIO_ARG(ratio) ((ratio)&0x3F)

#define SH1106_DC_DC_CONTROL_MODE_SET 0xAD
#define SH1106_DC_DC_OFF 0x8A
#define SH1106_DC_DC_ON 0x8B

#define SH1106_DISPLAY_OFF 0xAE
#define SH1106_DISPLAY_ON 0xAF
#define SH1106_DISPLAY_ON_MS 100

#define SH1106_SET_PAGE_ADDRESS(addr) (0xB0|((addr)&0x0F))

#define SH1106_SET_COMMON_OUTPUT_SCAN_DIRECTION(dir) (0xC0|(((dir)&0x1)<<3))

#define SH1106_SET_DISPLAY_OFFSET 0xD3

#define SH1106_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY 0xD5
#define SH1106_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY_ARG( \
	oscillator_frequency, \
	divite_ratio \
) ( \
	(0xF0&((oscillator_frequency)<<4)) | \
	(0xF&(divite_ratio)) \
)

#define SH1106_DISCHARGE_PRECHARGE_PERIOD_MODE_SET 0xD9
#define SH1106_DISCHARGE_PRECHARGE_PERIOD_MODE_SET_ARG( \
	discharge_period, \
	precharge_period \
) ( \
	(0xF0&((discharge_period)<<4)) | \
	(0xF&(precharge_period)) \
)

#define SH1106_COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET 0xDA
#define SH1106_COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET_ARG(value) (0x02|(((value)&0x1)<<4))

#define SH1106_VCOM_DESELECT_LEVEL_MODE_SET 0xDB

#define SH1106_READ_MODIFY_WRITE 0xE0

#define SH1106_END 0xEE

#define SH1106_NOP 0xE3

#define SH1106_I2C_CO 7
#define SH1106_I2C_DC 6

//
// Functions
//

// Helpers

static inline void set_column_address(
	eglib_t *eglib,
	coordinate_t column
) {
	sh1106_config_t *display_config;
	uint8_t buff[2];

	display_config = display_get_config(eglib);

	buff[0] = SH1106_SET_HIGHER_COLUMN_ADDRESS(column + display_config->column_offset);
	buff[1] = SH1106_SET_LOWER_COLUMN_ADDRESS(column + display_config->column_offset);
	hal_send_commands(eglib, buff, sizeof(buff));
}

static inline void display_on(eglib_t *eglib) {
	uint8_t buff[] = {
		SH1106_DISPLAY_ON,
	};

	hal_send_commands(eglib, buff, sizeof(buff));
	hal_delay_ms(eglib, SH1106_DISPLAY_ON_MS);
}

// display_t

static uint8_t get_7bit_slave_addr(eglib_t *eglib, hal_dc_t dc) {
	sh1106_config_t *display_config;

	(void)dc;
	display_config = display_get_config(eglib);

	return 0x3C | (display_config->sa0);
}

static void init(eglib_t *eglib) {
	sh1106_config_t *display_config;

	display_config = display_get_config(eglib);

	// Hardware reset

	hal_set_reset(eglib, 0);
	hal_delay_ms(eglib, SH1106_RESET_LOW_MS);
	hal_set_reset(eglib, 1);
	hal_delay_ms(eglib, SH1106_RESET_HIGH_MS);

	// comm begin

	hal_comm_begin(eglib);


	uint8_t commands_init[] = {
		// Display physical construction

		SH1106_SET_SEGMENT_REMAP(display_config->segment_remap),

		SH1106_SET_MULTIPLEX_RATIO,
		SH1106_SET_MULTIPLEX_RATIO_ARG(display_config->height-1),

		SH1106_COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET,
		SH1106_COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET_ARG(
			display_config->common_pads_hardware_configuration_mode
		),

		SH1106_SET_COMMON_OUTPUT_SCAN_DIRECTION(
			display_config->common_output_scan_direction
		),

		SH1106_SET_DISPLAY_OFFSET,
		display_config->display_offset,

		// Change period

		SH1106_DISCHARGE_PRECHARGE_PERIOD_MODE_SET,
		SH1106_DISCHARGE_PRECHARGE_PERIOD_MODE_SET_ARG(
			display_config->dis_charge_period,
			display_config->pre_charge_period
		),

		// VCOM deselect

		SH1106_VCOM_DESELECT_LEVEL_MODE_SET,
		display_config->vcom_deselect_level,

		// Internal display clocks

		SH1106_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY,
		SH1106_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY_ARG(
			display_config->oscillator_frequency,
			display_config->clock_divide
		),
	};

	hal_send_commands(eglib, commands_init, sizeof(commands_init));


	// Charge Pump Regulator
	if(display_config->dc_dc_enable) {
		hal_send_command_byte(eglib, SH1106_DC_DC_CONTROL_MODE_SET);
		hal_send_command_byte(eglib, SH1106_DC_DC_ON);
		hal_send_command_byte(eglib, SHH1106_SET_PUMP_VOLTAGE(display_config->dc_dc_voltage));
	} else {
		hal_send_command_byte(eglib, SH1106_DC_DC_CONTROL_MODE_SET);
		hal_send_command_byte(eglib, SH1106_DC_DC_OFF);
	}

	// Clear RAM
	for(uint8_t page=0 ; page < (display_config->height / 8) ; page++) {
		hal_send_command_byte(eglib, SH1106_SET_PAGE_ADDRESS(page));
		set_column_address(eglib, 0);
		for(coordinate_t column=0 ; column < display_config->width ; column ++)
			hal_send_data_byte(eglib, 0x00);
	}

	// Set display on
	display_on(eglib);

	hal_comm_end(eglib);
};

static void sleep_in(eglib_t *eglib) {
	// TODO
	(void)eglib;
};

static void sleep_out(eglib_t *eglib) {
	// TODO
	(void)eglib;
};

static void get_dimension(
	eglib_t *eglib,
	coordinate_t *width, coordinate_t*height
) {
	sh1106_config_t *display_config;

	display_config = display_get_config(eglib);

	*width = display_config->width;;
	*height = display_config->height;
};

static void get_pixel_format(eglib_t *eglib, pixel_format_t *pixel_format) {
	(void)eglib;

	*pixel_format = PIXEL_FORMAT_1BIT_BW_PAGED;
}

static void draw_pixel_color(
	eglib_t *eglib,
	coordinate_t x, coordinate_t y, color_t color
) {
	(void)eglib;
	(void)x;
	(void)y;
	(void)color;
};

static void send_buffer(
	eglib_t *eglib,
	void *buffer_ptr,
	coordinate_t x, coordinate_t y,
	coordinate_t width, coordinate_t height
) {
	uint8_t *buffer;
	coordinate_t display_width, display_height;

	buffer = (uint8_t *)buffer_ptr;

	display_get_dimension(eglib, &display_width, &display_height);

	hal_comm_begin(eglib);
	for(uint8_t page=y/8 ; page < ((y+height)/8+1) ; page++) {
		hal_send_command_byte(eglib, SH1106_SET_PAGE_ADDRESS(page));
		set_column_address(eglib, x);
		hal_send_data(eglib, (buffer + page * display_width + x), width);
	}
	hal_comm_end(eglib);
}

static bool refresh(eglib_t *eglib) {
	(void)eglib;
	return false;
}

// Custom Functions

void sh1106_set_start_line(
	eglib_t *eglib,
	uint8_t line
) {
	hal_comm_begin(eglib);
	hal_send_command_byte(eglib, SH1106_SET_DISPLAY_START_LINE(line));
	hal_comm_end(eglib);
}

void sh1106_set_contrast(
	eglib_t *eglib,
	uint8_t contrast
) {
	hal_comm_begin(eglib);
	hal_send_command_byte(eglib, SH1106_SET_CONTRAST_CONTROL_REGISTER);
	hal_send_command_byte(eglib, contrast);
	hal_comm_end(eglib);
}

void sh1106_entire_display_on(
	eglib_t *eglib,
	bool entire_display_on
) {
	hal_comm_begin(eglib);
	if(entire_display_on)
		hal_send_command_byte(eglib, SH1106_SET_ENTIRE_DISPLAY_ON);
	else
		hal_send_command_byte(eglib, SH1106_SET_ENTIRE_DISPLAY_OFF);
	hal_comm_end(eglib);
}

void sh1106_reverse(
	eglib_t *eglib,
	bool reverse
) {
	hal_comm_begin(eglib);
	if(reverse)
		hal_send_command_byte(eglib, SH1106_SET_REVERSE_DISPLAY);
	else
		hal_send_command_byte(eglib, SH1106_SET_NORMAL_DISPLAY);
	hal_comm_end(eglib);
}

//
// display_t
//

static void i2c_send(
	eglib_t *eglib,
	void (*i2c_write)(eglib_t *eglib, uint8_t byte),
	hal_dc_t dc,
	uint8_t *bytes,
	uint16_t length
) {
	// For more than 2 bytes it is more efficient to use Co=0
	if(length > 2) {
		// Control byte
		i2c_write(eglib, (0<<SH1106_I2C_CO) | (dc<<SH1106_I2C_DC));
		for (uint16_t i = 0; i < length; i++){
			// Data byte
			i2c_write(eglib, bytes[i]);
		}
		// ReStart
		hal_comm_begin(eglib);
	} else {
		for (uint16_t i = 0; i < length; i++){
			// Control byte
			i2c_write(eglib, (1<<SH1106_I2C_CO) | (dc<<SH1106_I2C_DC));
			// Data byte
			i2c_write(eglib, bytes[i]);
		}
	}
}

static hal_i2c_config_comm_t hal_i2c_config_comm = {
	.speed = EGLIB_HAL_I2C_400KHZ,
	.get_7bit_slave_addr = get_7bit_slave_addr,
	.send = i2c_send,
};

const display_t sh1106_vdd1_1_65_v = {
	.comm = {
		.four_wire_spi = &((hal_four_wire_spi_config_comm_t){
			.mode = 0,
			.bit_numbering = EGLIB_HAL_MSB_FIRST,
			.cs_setup_ns = 240,
			.cs_hold_ns = 120,
			.cs_disable_ns = 0,
			.dc_setup_ns = 300,
			.dc_hold_ns = 300,
			.sck_cycle_ns = 500,
		}),
		.i2c = &hal_i2c_config_comm,
	},
	.init = init,
	.sleep_in = sleep_in,
	.sleep_out = sleep_out,
	.get_dimension = get_dimension,
	.get_pixel_format = get_pixel_format,
	.draw_pixel_color = draw_pixel_color,
	.send_buffer = send_buffer,
	.refresh = refresh,
};

const display_t sh1106_vdd1_2_4_v = {
	.comm = {
		.four_wire_spi = &((hal_four_wire_spi_config_comm_t){
			.mode = 0,
			.bit_numbering = EGLIB_HAL_MSB_FIRST,
			.cs_setup_ns = 120,
			.cs_hold_ns = 60,
			.cs_disable_ns = 0,
			.dc_setup_ns = 150,
			.dc_hold_ns = 150,
			.sck_cycle_ns = 250,
		}),
		.i2c = &hal_i2c_config_comm,
	},
	.init = init,
	.sleep_in = sleep_in,
	.sleep_out = sleep_out,
	.get_dimension = get_dimension,
	.get_pixel_format = get_pixel_format,
	.draw_pixel_color = draw_pixel_color,
	.send_buffer = send_buffer,
	.refresh = refresh,
};

//
// sh1106_config_t
//

const sh1106_config_t sh1106_config_sparkfun_micro_oled = {
	// Display physical construction
	.width = 64,
	.height = 48,
	.segment_remap = SH1106_SEGMENT_REMAP_REVERSE,
	.common_pads_hardware_configuration_mode = SH1106_COMMON_PADS_HARDWARE_CONFIGURATION_ALTERNATIVE,
	.common_output_scan_direction = SH1106_COMMON_OUTPUT_SCAN_DIRECTION_ASC,
	.display_offset = 0,
	.column_offset = 0,

	// Change period
	.pre_charge_period = 1,
	.dis_charge_period = 15,

	// VCOM deselect
	.vcom_deselect_level = 0x40,

	// Internal display clocks
	.clock_divide = 0,
	.oscillator_frequency = SH1106_OSCILLATOR_FREQUENCY_PLUS_15_PCT,

	// Charge Pump Regulator
	.dc_dc_enable = true,
	.dc_dc_voltage = SHH1106_DC_DC_8_0_V,
};