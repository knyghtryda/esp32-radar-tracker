# ESP32 Radar Tracker with HLK-LD2450

Real-time radar tracking system using the HLK-LD2450 24GHz mmWave radar sensor. The Arduino build targets the original LilyGO TTGO T-Display (ESP32 with onboard ST7789V color LCD). The CircuitPython files are earlier ESP32-S3 experiments and have not been ported to the T-Display.

## 🎯 Features

- **Multi-Target Tracking**: Simultaneously tracks up to 3 targets with X/Y coordinates
- **Real-time Visualization**: Onboard LCD with radar sweep and target positions
- **Dual Platform Support**: 
  - Arduino C++ with optimized performance
  - CircuitPython prototypes for the previous ESP32-S3 hardware
- **Multiple Visualization Modes**:
  - Classic radar sweep display
  - Alien-style animated tracking
  - Diagnostic data output
  - Raw protocol debugging

## 📡 Hardware Requirements

- **LilyGO TTGO T-Display** (original ESP32 version with 1.14-inch ST7789V LCD)
- **HLK-LD2450** 24GHz mmWave Radar Sensor
- USB-C cable for programming and power

### Wiring Diagram

**Radar to T-Display:**
- TX → GPIO 26 (ESP32 UART RX)
- RX → GPIO 27 (ESP32 UART TX)
- VCC → 5V
- GND → GND

The LCD is built into the board. GPIO26 and GPIO27 are used for radar UART so they do not overlap the LCD's SPI pins.

## 🚀 Quick Start

### Arduino Setup (PlatformIO)

1. Install PlatformIO Core or the VS Code PlatformIO extension
2. Open the `arduino/` folder as a PlatformIO project
3. Build and upload the `lilygo-t-display` environment

The PlatformIO configuration selects the original ESP32 T-Display and configures TFT_eSPI for its onboard ST7789V display.

### CircuitPython Setup

1. Install CircuitPython 9.x on an ESP32-S3 (this legacy implementation is not ported to the T-Display)
2. Install required libraries on device:
   - adafruit_displayio_ssd1306
   - adafruit_display_text
3. Choose a visualization mode from `circuitpython/` folder
4. Deploy using the utility tool:

```powershell
python utils/deploy_circuitpython.py
```

Available CircuitPython modes:
- `code_correct_protocol.py` - Standard radar sweep display
- `code_alien_style.py` - Sci-fi animated tracking
- `code_dual_both.py` - Dual protocol support
- `code_diagnostic.py` - Debug output
- `code_realtime_sweep.py` - Fast refresh radar sweep

## 🛠️ Utilities

### Serial Monitor
Monitor ESP32 serial output in real-time:
```powershell
python utils/serial_monitor.py
```

### Code Deployment
Deploy CircuitPython code to ESP32:
```powershell
python utils/deploy_circuitpython.py
```

See [utils/README.md](utils/README.md) for detailed utility documentation.

## 📊 HLK-LD2450 Sensor Details

- **Detection Range**: 0-6 meters
- **Angle Coverage**: ±60°
- **Maximum Targets**: 3 simultaneous
- **Protocol**: UART (256000 baud, 8N1)
- **Update Rate**: ~10 Hz
- **Technology**: 24GHz FMCW radar

## 🎨 Visualization Modes

### Arduino Version
- Clean radar sweep with target markers
- 180-degree forward radar on the 240x135 LCD, with blue range arcs and colored targets
- Logarithmic 0-10 m scale; targets beyond 10 m clamp to the outer edge
- Displays up to three valid targets simultaneously; the status strip shows the nearest
- A one-second semicircular pulse expands outward from the radar origin; each target marker appears as the pulse reaches it, then fades over one second
- Three fixed footer columns show each target's range to one decimal place and do not fade

### CircuitPython Versions

**Standard Mode** (`code_correct_protocol.py`)
- Classic radar sweep animation
- Target position markers
- Distance labels

**Alien Style** (`code_alien_style.py`)
- Animated target tracking
- Fade effects
- Sci-fi aesthetic

**Diagnostic Mode** (`code_diagnostic.py`)
- Raw data output
- Protocol debugging
- Sensor status monitoring

## 🔧 Troubleshooting

### No Radar Data
1. Check UART wiring (TX/RX)
2. Verify sensor power (5V)
3. Run diagnostic mode to check protocol

### Display Not Working
1. Confirm the board is the original T-Display with the onboard ST7789V LCD, not the T-Display-S3
2. Confirm TFT_eSPI is installed by PlatformIO and the selected environment is `lilygo-t-display`

### COM Port Issues
- Windows: Check Device Manager for COM port
- Linux/Mac: Use `ls /dev/tty*` to find port
- Ensure no other program is using the port

## �� Project Structure

```
esp32-radar-tracker/
├── arduino/              # Arduino C++ implementation
│   ├── include/
│   │   ├── HLK_LD2450.h  # Radar protocol parser
│   │   └── RadarDisplay.h # Radar display drawing
│   ├── src/main.cpp
│   └── platformio.ini
├── circuitpython/       # Multiple CircuitPython versions
│   ├── code_correct_protocol.py
│   ├── code_alien_style.py
│   ├── code_diagnostic.py
│   └── ...              # Various visualization modes
├── utils/               # Deployment and monitoring tools
│   ├── deploy_circuitpython.py
│   ├── serial_monitor.py
│   └── README.md
├── docs/                # Additional documentation
├── requirements.txt     # Python dependencies
└── README.md
```

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- Additional visualization modes
- Enhanced target tracking algorithms
- Multi-sensor support
- Data logging features

## 📝 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Adafruit**: Display libraries and CircuitPython support
- **Hi-Link**: HLK-LD2450 sensor and protocol documentation
- **ESP32 Community**: Hardware support and examples

## 📚 Resources

- [HLK-LD2450 Datasheet](https://www.hlktech.net/index.php?id=1163)
- [CircuitPython Documentation](https://circuitpython.org/)
- [ESP32-S3 Technical Reference](https://www.espressif.com/en/products/socs/esp32-s3)
- [Adafruit SSD1306 Guide](https://learn.adafruit.com/monochrome-oled-breakouts)

---

**Status**: Arduino code is being ported to the original LilyGO TTGO T-Display; hardware validation is pending.
