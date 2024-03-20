# esp8266_oled_weather
esp8266+ssd1306+和风天气API显示实时天气
使用nodeMCU的esp8266
oled是ssd1306的0.96寸128*64分辨率
天气接口使用的是和风天气API
## 基于[https://github.com/Ldufan/OLED_Weather](https://github.com/Ldufan/OLED_Weather)修改
## 修改了天气显示的字体
## 修改了时间显示的字体
## 去掉了倒计时页面
## 修改配网方式为输入WiFi的ssdid和密码进行连接（原作者没有上传AP配网的头文件）
## 修改OLED的引脚为ESP8266的默认引脚，D1和D2
## 修改http客户端为ESP8266HTTPClient
## 修改天气接口地址为http地址，无法访问https地址或者服务器报错无法解析
# OLEDDisplayUi库下载地址为 [https://github.com/ThingPulse/esp8266-oled-ssd1306/tree/master](https://github.com/ThingPulse/esp8266-oled-ssd1306/tree/master)
本地转发使用python写的，部署在任意可以访问的服务器上都可以，只能用http方式