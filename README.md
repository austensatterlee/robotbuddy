# robotbuddy
C code + extra tools for a TI MSP430-based self-balancing robot. 
(We used an MSP430G2553)

## C Code
All MSP430 code is located inside the `robot` folder

## Extra tools
By default, the C code provided will use pins 1.1 and 1.2 for UART over the MSP430G's USCI hardware interface. 
To use this feature, make sure to rotate the jumpers as shown on [this page](http://energia.nu/Guide_MSP430LaunchPad.html). 
The code will send both the current angle estimate and the PID output as binary data over the UART interface. 
This data can be captured over USB using the provided python script `serialReader.py` 
(which relies on the [PySerial](http://pyserial.sourceforge.net/) library), and optionally its accompanying `angleGui.py` 
(which relies on [PyQt4](http://www.riverbankcomputing.com/software/pyqt/intro) and [PyOpenGL](http://pyopengl.sourceforge.net/)).
To command-line syntax for this tool is as follows:
```
python serialReader.py [-h] [-o OUTFILE] [-d DEVICE] [-r RATE] formats
```
* `OUTFILE` is the name of a CSV file where the recorded data will be dumped (and overwritten if it already exists)
* `DEVICE` is a number specifying the COM port of the robot (in Windows, at least, this can be gleaned from the device manager under 'ports').
* `RATE` is the baud rate of the communication channel. By default this will be 9600.
* `FORMATS` is where this gets even _more_ fun (I know, as if!). This should be a string, taking one of the following forms:
  - `raw` Specifying "raw" can be used for debugging, as it will capture the data being transmitted, byte by byte, and print out the hex in neat little columns.
  - `gui` This option is specifically configured for the default setup, and is used to graphically display the robot's current angle estimate. It's great for testing out different filters or filter parameters.
  - The last option is a list of [struct](https://docs.python.org/2/library/struct.html#format-characters) formats, seperated by comma. This form can be used to interpret raw bytes sent over from the MSP430, and then formatted as a CSV for post-processing. For instance, `h,h,h,f` is three shorts and a float per row. This would correspond to the MSP430 sending 3 16-bit integers and a 32-bit float in some sort of a loop.
  
  ## Schematic
  This schematic shows our setup and pin locations, but of course there are many possible configurations (and probably better H-bridges) that could be used.
  ![Example schematic](https://raw.githubusercontent.com/austensatterlee/robotbuddy/master/H-Bridge/all-npn-hbridge.png)
