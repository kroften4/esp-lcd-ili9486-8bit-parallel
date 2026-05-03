#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_lcd_io_i80.h>
#include <esp_lcd_panel_dev.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_types.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/idf_additions.h>
#include <freertos/projdefs.h>
#include <hal/gpio_types.h>
#include <soc/clk_tree_defs.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_ili9486_panel.h"
#include "ili9486_display_init.h"
#include "sdkconfig.h"

static const char *TAG = "ili9486_display_init";

esp_err_t
ili9486_display_init(esp_lcd_panel_handle_t *ret_panel,
					 esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done,
					 void *on_trans_done_user_ctx)
{
	// TODO: wire backlight to gpio and turn on only after init
	gpio_reset_pin(CONFIG_ILI9486_PIN_RD);
	gpio_set_direction(CONFIG_ILI9486_PIN_RD, GPIO_MODE_OUTPUT);
	gpio_set_level(CONFIG_ILI9486_PIN_RD, 1);
	vTaskDelay(pdMS_TO_TICKS(10));

	esp_lcd_i80_bus_handle_t i80_bus = NULL;
	esp_lcd_i80_bus_config_t bus_config = {
		.clk_src = LCD_CLK_SRC_DEFAULT,
		.dc_gpio_num = CONFIG_ILI9486_PIN_DC,
		.wr_gpio_num = CONFIG_ILI9486_PIN_WR,
		.data_gpio_nums = {
			CONFIG_ILI9486_PIN_D0,
			CONFIG_ILI9486_PIN_D1,
			CONFIG_ILI9486_PIN_D2,
			CONFIG_ILI9486_PIN_D3,
			CONFIG_ILI9486_PIN_D4,
			CONFIG_ILI9486_PIN_D5,
			CONFIG_ILI9486_PIN_D6,
			CONFIG_ILI9486_PIN_D7,
		},
		.bus_width = 8,
		.max_transfer_bytes = CONFIG_ILI9486_MAX_TRANSFER_BYTES,
		.dma_burst_size = CONFIG_ILI9486_DMA_BURST_SIZE,
	};
	ESP_RETURN_ON_ERROR(esp_lcd_new_i80_bus(&bus_config, &i80_bus), TAG,
						"Failed to initialize i80 bus");

	esp_lcd_panel_io_handle_t io_handle = NULL;
	esp_lcd_panel_io_i80_config_t io_config = {
		.on_color_trans_done = on_color_trans_done,
		.user_ctx = on_trans_done_user_ctx,
		.cs_gpio_num = CONFIG_ILI9486_PIN_CS,
		.pclk_hz = CONFIG_ILI9486_CLOCK_HZ,
		.trans_queue_depth = CONFIG_ILI9486_TRANS_QUEUE_DEPTH,
		.dc_levels = {
#if CONFIG_ILI9486_DC_DATA_ACTIVE_HIGH
			.dc_data_level = 1,
			.dc_cmd_level = 0,
#else
			.dc_data_level = 0,
			.dc_cmd_level = 1,
#endif
			.dc_dummy_level = 0,
			.dc_idle_level = 0,
		},
		.lcd_cmd_bits = 8,
		.lcd_param_bits = 8,
		.flags = {
#if CONFIG_ILI9486_CS_ACTIVE_HIGH
			.cs_active_high =  1,
#else
			.cs_active_high =  0,
#endif
#if CONFIG_ILI9486_WR_ACTIVE_NEG
			.pclk_active_neg = 1,
#else
			.pclk_active_neg = 0,
#endif
			.pclk_idle_low = 0,
#if CONFIG_ILI9486_REVERSE_COLOR_BITS
			.reverse_color_bits = 1,
#else
			.reverse_color_bits = 0,
#endif
#if CONFIG_ILI9486_SWAP_COLOR_BYTES
			.swap_color_bytes = 1,
#else
			.swap_color_bytes = 0,
#endif
		}
	};
	ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i80(i80_bus, &io_config,
												 &io_handle),
						TAG, "Failed to initialize panel io");

	esp_lcd_panel_dev_config_t panel_config = {
		.reset_gpio_num = CONFIG_ILI9486_PIN_RST,
	// .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
#if CONFIG_ILI9486_16_BPP
		.bits_per_pixel = 16,
#elif CONFIG_ILI9486_18_BPP || CONFIG_ILI9486_18_BPP_SPI
		.bits_per_pixel = 18,
#elif CONFIG_ILI9486_3_BPP
		.bits_per_pixel = 3,
#endif
	};

	ESP_RETURN_ON_ERROR(esp_lcd_new_panel_ili9486(io_handle, &panel_config,
												  ret_panel),
						TAG, "Failed to initialize panel");
	ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(*ret_panel), TAG,
						"Failed to reset panel");
	ESP_RETURN_ON_ERROR(esp_lcd_panel_init(*ret_panel), TAG,
						"Panel init sequence failed");
	ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(*ret_panel, true), TAG,
						"Failed to turn the display on");

	return ESP_OK;
}
