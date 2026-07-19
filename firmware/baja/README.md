# Baja DAQ firmware

The firmware samples vehicle data at 100 Hz. Logging starts at 800 RPM, stops
after RPM remains below 500 for one second, rotates files every 15 minutes, and
syncs the active file at least once per second. Existing `RUN_####.CSV` files
are never overwritten.

## Sensor interfaces

- MLX90614 object temperature: I2C1, address `0x5A`, PB6/PB7, 100 kHz.
- ISM330DHCX: SPI1 mode 0, PB1 chip select, 104 Hz, +/-4 g and +/-500 dps.
- Suspension position: ADC1 channels 2 and 3 with a 32-sample startup zero.
- Engine and secondary speed: TIM5 input capture with a 500 ms pulse timeout.

CSV `Sensor_Status` bit 0 marks temperature valid and bit 1 marks IMU data
valid. Invalid values must be ignored even if the numeric field contains the
last successful sample.

## CAN frames

- `0x100`, 100 Hz: engine RPM (big-endian uint16), speed, temperature, flags.
  Temperature `0xFF` means invalid. Flag bits 0-3 are high temperature, high
  RPM, temperature invalid, and IMU invalid.
- `0x101`, 1 Hz: saturated uint8 counters for queue drops, SD mount/write/sync
  failures, MLX failures, IMU failures, CAN drops, and CAN bus-off events.

CAN is configured for 333.333 kbit/s from the 16 MHz APB1 clock.

## Bench validation

Before vehicle use, verify sensor axis orientation, pulses per revolution,
gearbox ratio, temperature field of view, CAN bitrate, SD-card removal, and
sudden-power-loss recovery on the assembled board.
