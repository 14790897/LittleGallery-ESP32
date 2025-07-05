#ifndef CONFIG_H
#define CONFIG_H

// ==================== 系统配置 ====================

// WiFi配置 - 请修改为您的WiFi信息
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Web服务器配置
#define WEB_SERVER_PORT 80

// 显示屏硬件配置
#define TFT_CS   7   // 片选引脚
#define TFT_DC   6   // 数据/命令引脚
#define TFT_RST  10  // 复位引脚

// 显示屏尺寸
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// 系统参数配置
#define IMAGE_UPDATE_INTERVAL 1000  // 图片更新检查间隔（毫秒）
#define MAX_IMAGES 50               // 支持的最大图片数量
#define MAX_FILE_SIZE (2 * 1024 * 1024)  // 最大文件大小 2MB

// 调试配置
#define SERIAL_BAUD_RATE 115200
#define DEBUG_ENABLED true

// 支持的图片格式
#define SUPPORT_JPEG true
#define SUPPORT_BMP true

#endif // CONFIG_H
