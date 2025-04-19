# PCB Testing System

This project provides a simple and efficient system for testing PCBs (Printed Circuit Boards) using an **M5Stack Core** and an **MCP23017 I/O expander**. The system checks for continuity within nets (groups of electrically connected pins) and detects shorts between different nets. It provides visual feedback on the M5Stack display and logs test results to the serial monitor.

## Features

-   **Continuity Testing**: Verifies that all pins in a net are electrically connected.
-   **Short Detection**: Ensures there are no shorts between different nets.
-   **Visual Feedback**: Displays pass/fail results for each net on the M5Stack screen using colored circles:
    -   **Green Circle**: Net passed (no issues detected).
    -   **Red Circle**: Net failed (continuity issue or short detected).
-   **Serial Logging**: Provides detailed failure information for debugging.
-   **Multiple PCB Layouts**: Supports testing up to three different PCB layouts, selectable using the M5Stack buttons.
-   **Clear Layout Identification**: Displays the name of the active PCB layout on the M5Stack screen.
-   **Single Pin Mapping**: Simplifies wiring by using a single pin mapping for all layouts.

## Hardware Requirements

1.  **M5Stack Core Basic** (or compatible model with a display).
2.  **MCP23017 I/O Expander** (connected via I2C).
3.  A PCB with nets to be tested.
4.  Jumper wires or connectors to interface with the PCB.

## Software Requirements

1.  Arduino IDE or PlatformIO.
2.  Required libraries:
    -   [M5Unified](https://github.com/m5stack/M5Unified)
    -   [Adafruit_MCP23X17](https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library)

## Wiring Information

Connect the MCP23017 to the M5Stack via I2C:

-   Default MCP23017 address: `0x27`.
-   SDA and SCL lines connected appropriately to the M5Stack.
-   Connect the PCB under test to the MCP23017 using jumper wires or connectors, following the pin mapping defined in the code.

## How It Works

1.  The program uses a single pin mapping (`pinMapping`) to define the connections between device pins and MCP23017 pins.
2.  The `netConfig1`, `netConfig2`, and `netConfig3` arrays define the net configurations for up to three different PCB layouts. Each configuration specifies the nets to be tested.
3.  During testing:
    -   The program sets the first pin of each net LOW.
    -   It then checks continuity for the remaining pins in the net.
    -   Additionally, it verifies that there are no shorts between different nets.
4.  The M5Stack buttons are used to select the PCB layout to test.
    -   **Button A:** Selects the first PCB layout.
    -   **Button B:** Selects the second PCB layout.
    -   **Button C:** Selects the third PCB layout.
5.  Test results are displayed on the M5Stack screen:
    -   A green circle indicates that all connections in a net are correct.
    -   A red circle indicates a continuity break or a short with another net.
6.  Detailed failure information is also printed to the serial monitor for debugging.

## Setup Instructions

1.  Connect the MCP23017 to the M5Stack via I2C:

    -   Default MCP23017 address: `0x27`.
    -   SDA and SCL lines connected appropriately.
2.  Connect the PCB under test to the MCP23017 using jumper wires or connectors, according to your `pinMapping`.
3.  Load the code (from the `/src` directory) onto your M5Stack using the Arduino IDE or PlatformIO.

## Configuration

The following sections detail how to configure the PCB testing system.

### Pin Mapping

The `pinMapping` variable maps device pin labels to MCP23017 pins. Ensure that this mapping accurately reflects your wiring:

```
std::map<std::string, int> pinMapping = {
{"2", 11},
{"4", 12},
{"6", 13},
{"10", 14},
{"12", 15},
{"1", 4},
{"3", 5},
{"5", 6},
{"7", 7},
{"9", 8},
{"11", 9},
{"CN1", 0},
{"CN2", 1},
{"CN3", 2},
{"CN4", 3}
};
```


### Net Configurations

Define the net configurations for each PCB layout using the `netConfig1`, `netConfig2`, and `netConfig3` arrays. Each configuration specifies the nets to be tested.

Example:

```
const char* netConfig1[][MAX_NET_SIZE + 1] = {
{"4-7", "4", "7"},
{"1-9-CN1|12V", "1", "9", "CN1"},
{"2-10-CN4|5V", "2", "10", "CN4"},
{"3-5-6-11-12|CN2-CN3 GND", "3", "5", "6", "11", "12", "CN2", "CN3"}
};
```

### PCB Layout Names

The `layoutNames` array provides descriptive names for each PCB layout. These names are displayed on the M5Stack screen to indicate the active layout.

```
const char* layoutNames[MAX_LAYOUTS] = {
"DEM w/ Power Header",
"Plain DEM",
"DEM w/ Pwr Hdr Tray State"
};
```

### Display Settings

You can adjust display settings such as circle size, spacing, and font size using these constants:

```
const int circleRadius = 20;
const int circleSpacingX = 120;
const int circleSpacingY = 100;
const int labelFontSize = 2;
```


## Usage

1.  Connect the MCP23017 to the M5Stack and the PCB to be tested, ensuring that the wiring matches the pin mapping in the code.
2.  Upload the code (from the `/src` directory) to your M5Stack using the Arduino IDE or PlatformIO.
3.  Power on the M5Stack.
4.  Select the PCB layout to test by pressing the corresponding button:
    -   **Button A:** Selects the first PCB layout.
    -   **Button B:** Selects the second PCB layout.
    -   **Button C:** Selects the third PCB layout.
5.  Observe the test results on the M5Stack screen:
    -   Green circles indicate that all connections in a net are correct.
    -   Red circles indicate a continuity break or a short with another net.
6.  Check the serial monitor for detailed failure information.

## Serial Output Example

If a failure is detected, you’ll see output similar to the following in the serial monitor:

```
Net continuity test failed: Pin 7 did not read LOW
Short detected: Net 2's first pin is LOW while testing Net 0
```

## Troubleshooting

### Common Issues

1.  **MCP23017 Not Found:**
    -   Ensure proper I2C connections (SDA, SCL) and verify the MCP23017 address (`0x27` by default).
    -   If not found, an error message will be displayed on the screen.
2.  **Incorrect Test Results:**
    -   Double-check the pin mapping in the code to ensure that it matches your hardware setup.
    -   Inspect the wiring for any loose or incorrect connections.

## Contributing

Feel free to submit issues or pull requests to improve this project!

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 Jason Hoomani



