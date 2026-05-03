#ifndef __ILI9486_DISPLAY_INIT_H__
#define __ILI9486_DISPLAY_INIT_H__

#include <esp_err.h>
#include <esp_lcd_types.h>
#include <freertos/idf_additions.h>

esp_err_t
displayInit(esp_lcd_panel_handle_t *ret_panel,
			esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done,
			void *on_trans_done_user_ctx);

#endif // __ILI9486_DISPLAY_INIT_H__
