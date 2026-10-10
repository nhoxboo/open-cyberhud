# 🛡️ OpenCyberHUD

[![Deploy Studio to GitHub Pages](https://github.com/nhoxboo/open-cyberhud/actions/workflows/deploy.yml/badge.svg)](https://github.com/nhoxboo/open-cyberhud/actions/workflows/deploy.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**OpenCyberHUD** là dự án mã nguồn mở tái tạo hoàn chỉnh firmware hiển thị HUD trên kính lái ô tô / taplo xe máy, tương thích 100% với ứng dụng **Vietmap Live** và bộ đọc dữ liệu xe **OBD2 ELM327 BLE**.

🌐 **Dùng thử & Nạp Firmware Online 1-Click:** [https://nhoxboo.github.io/open-cyberhud/](https://nhoxboo.github.io/open-cyberhud/)

---

## ✨ Tính Năng Nổi Bật

- **Tương thích hoàn toàn Vietmap Live:**
  - Nhận diện như thiết bị phần cứng Vietmap H1V chính hãng qua BLE GATT (`0xFFF0`).
  - Hiển thị tốc độ GPS thực tế, giới hạn tốc độ đoạn đường, khoảng cách và loại camera phạt nguội (tốc độ, vượt đèn đỏ, đi sai làn).
  - Điều hướng rẽ trái/phải/vòng xuyến theo thời gian thực.
- **Hỗ trợ OBD2 ELM327 BLE:**
  - Tự động kết nối đọc Vòng tua máy (RPM), Nhiệt độ nước làm mát (Coolant Temp), Điện áp bình ắc-quy (Battery Voltage).
- **Hỗ trợ đa dạng phần cứng ESP32:**
  - 🎯 **ESP32-2424S012**: Màn hình tròn 1.28" IPS GC9A01 + Cảm ứng CST816D.
  - 📟 **ESP32-2432S024 (CYD 2.4")**: Màn hình 2.4" ILI9341 + Cảm ứng XPT2046 + Loa + Cảm biến quang LDR.
  - 🛠️ **ESP32 DevKit / ESP32-S3 / ESP32-C3**: Tự do nối dây với màn hình ST7789, GC9A01, ILI9341.
- **Đồ hoạ DMA siêu tốc:** Sử dụng `LovyanGFX` với Framebuffer DMA, đáp ứng 40-60 FPS mượt mà.
- **Cấu hình trực tuyến Web Bluetooth:** Chỉnh góc xoay màn hình, độ sáng ngày/đêm, âm lượng còi báo, màu LED qua giao diện Web không cần nạp lại code.

---

## 🚀 Hướng Dẫn Nạp Firmware 1-Click

Truy cập trực tiếp [**OpenCyberHUD Studio**](https://nhoxboo.github.io/open-cyberhud/):
1. Cắm cáp USB bo mạch ESP32 vào máy tính (sử dụng trình duyệt Google Chrome hoặc MS Edge).
2. Chọn loại bo mạch (`ESP32-2424S012` hoặc `ESP32-2432S024`).
3. Nhấn **Install / Connect** & chọn cổng COM để nạp trực tiếp qua Web Serial API.

---

## 🛠️ Biên Dịch Bằng PlatformIO

```bash
# Clone repository
git clone https://github.com/nhoxboo/open-cyberhud.git
cd open-cyberhud

# Biên dịch cho ESP32-2424S012 (Màn hình tròn 1.28")
pio run -e esp32-2424s012

# Biên dịch cho ESP32-2432S024 (CYD 2.4")
pio run -e esp32-2432s024

# Nạp trực tiếp qua PlatformIO
pio run -e esp32-2432s024 --target upload
```

---

## 🔌 Sơ Đồ Chân (Pinout Reference)

| Chức năng | ESP32-2424S012 (Round 1.28") | ESP32-2432S024 (CYD 2.4") | ESP32 DevKit Chuẩn |
| :--- | :--- | :--- | :--- |
| **MCU** | ESP32-C3 RISC-V | ESP32-WROOM-32 | ESP32-WROOM-32 |
| **Màn hình LCD** | GC9A01 (240x240) | ILI9341 (320x240) | ST7789 (240x240/240x320) |
| **SPI Bus** | MOSI:7, SCLK:6, CS:10, DC:2, BL:3 | MOSI:13, MISO:12, SCLK:14, CS:15, DC:2, BL:21 | MOSI:23, SCLK:18, CS:5, DC:2, BL:15 |
| **Touch** | CST816D (SDA:4, SCL:5, INT:0, RST:1) | XPT2046 (CS:33, CLK:25, MOSI:32, MISO:39, IRQ:36) | CST816 / FT6236 (SDA:21, SCL:22) |
| **Buzzer** | GPIO 8 (Pad S1) | GPIO 26 (Onboard Amp) | GPIO 25 |
| **LED** | WS2812B (GPIO 2) | RGB 3-Pin (R:4, G:16, B:17) | WS2812B (GPIO 27) |
| **Cảm biến LDR** | — | GPIO 34 (Auto Light) | GPIO 34 (Tùy chọn) |

---

## 📜 Giấy Phép

Dự án được phân phối dưới giấy phép [MIT License](LICENSE).
Mọi đóng góp (Pull Request, Issue) đều được chào đón!
