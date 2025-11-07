# Rotary Dial Test

Simple test program to verify your rotary dial wiring before building the full RetroBell phone system.

## Features

- Tests both pulse and shunt switches
- Real-time pulse counting with visual feedback
- Displays dialed digits (0-9)
- Uses same GPIO pins as RetroBell project
- Works with both 3-wire and 4-wire rotary dials

## Hardware Requirements

- ESP32-S3-DevKitC-1 (or any ESP32)
- Rotary dial (3-wire or 4-wire)
- USB cable for programming and serial output

## Wiring

### 3-Wire Rotary Dial
```
Common/Ground → GND
Pulse Switch  → GPIO 15
Shunt/NC      → GPIO 14
```

### 4-Wire Rotary Dial
```
Pulse Contact A → GPIO 15
Pulse Contact B → GND
Shunt Contact A → GPIO 14
Shunt Contact B → GND
```

## How to Use

1. Wire your rotary dial according to the diagram above
2. Open this project in PlatformIO
3. Build and upload: `pio run --target upload`
4. Open serial monitor: `pio device monitor`
5. Dial digits and watch the output!

## Expected Output

```
[Dial started turning]
..........
[Dial returned to rest]

✓ Digit dialed: 0 (10 pulses)

[Dial started turning]
.....
[Dial returned to rest]

✓ Digit dialed: 5 (5 pulses)
```

## Troubleshooting

**No pulses detected:**
- Check that pulse switch is connected to GPIO 15 and GND
- Verify the switch closes/opens when dial rotates
- Try swapping the two wires on the pulse switch

**Wrong pulse count:**
- Check for loose connections
- May need to adjust `DEBOUNCE_MS` in code
- Verify dial returns fully to rest position

**Shunt not working:**
- Check that shunt switch is connected to GPIO 14 and GND
- Verify the switch opens when you start turning the dial
- Try swapping the two wires on the shunt switch

**Random pulses:**
- Increase `DEBOUNCE_MS` from 10 to 20 or 30
- Check for electrical noise near the dial
- Ensure good ground connection

## Next Steps

Once your dial is working correctly, you're ready to build the full RetroBell system!
