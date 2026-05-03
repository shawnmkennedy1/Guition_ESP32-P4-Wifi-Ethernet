#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ek79007.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcd_init.h"

#define LCD_GPIO_GRB        GPIO_NUM_33   // RESET_LCD (active low)
#define LCD_GPIO_PWR_EN     GPIO_NUM_33   // if tied together
#define LCD_GPIO_BL_EN      GPIO_NUM_32   // backlight control

static const char *TAG = "lcd_init";

static void lcd_reset_sequence(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LCD_GPIO_GRB),
    };
    gpio_config(&io_conf);

    gpio_set_level(LCD_GPIO_GRB, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(LCD_GPIO_GRB, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
}

esp_err_t lcd_init(lcd_handle_t *out_lcd)
{
    ESP_RETURN_ON_FALSE(out_lcd, ESP_ERR_INVALID_ARG, TAG, "out_lcd is NULL");

    // Power enable
    gpio_config_t pwr_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LCD_GPIO_PWR_EN),
    };
    gpio_config(&pwr_conf);
    gpio_set_level(LCD_GPIO_PWR_EN, 1);

    // Backlight control
    gpio_config_t bl_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LCD_GPIO_BL_EN),
    };
    gpio_config(&bl_conf);
    gpio_set_level(LCD_GPIO_BL_EN, 0); // off until ready

    lcd_reset_sequence();

    // 1. Create DSI bus
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = 2,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = 500,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &out_lcd->dsi_bus),
                        TAG, "Failed to create DSI bus");

    // 2. DBI IO over DSI (Command Mode)
    esp_lcd_dbi_io_config_t dbi_cfg = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(out_lcd->dsi_bus, &dbi_cfg, &out_lcd->io),
                        TAG, "Failed to create DBI IO");

    // 3. DPI panel config via EK79007 helper
    esp_lcd_dpi_panel_config_t dpi_cfg =
        EK79007_1024_600_PANEL_60HZ_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB888);

    ek79007_vendor_config_t vendor_cfg = {
        .mipi_config = {
            .dsi_bus = out_lcd->dsi_bus,
            .dpi_config = &dpi_cfg,
            .lane_num = 2,
        },
    };

    const esp_lcd_panel_dev_config_t panel_dev_cfg = {
        .reset_gpio_num = LCD_GPIO_GRB,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_cfg,
    };

    // 4. Create the Panel
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_ek79007(out_lcd->io, &panel_dev_cfg, &out_lcd->panel),
                        TAG, "Failed to create EK79007 panel");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(out_lcd->panel), TAG, "panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(out_lcd->panel), TAG, "panel init failed");

    gpio_set_level(LCD_GPIO_BL_EN, 1); // Turn on backlight

    ESP_LOGI(TAG, "LCD init complete");
    return ESP_OK;
}