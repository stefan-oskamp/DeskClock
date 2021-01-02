# DeskClock
## Analog clock toolbar for the Windows taskbar.

![Screenshot vertical taskbar](/images/screenshot-vertical-taskbar.png)
![Screenshot horizontal taskbar](/images/screenshot-horizontal-taskbar.png) 

## Pre-requisites
Version 2.0.0 has been tested under Windows 10 64 bit. A 32 bit version could be provided if needed.

## Installation
Download and run the MSI installer from the latest release.

To show the clock on the tasbar, right-click on the taskbar and expand "Toolbars >", then check the DeskClock entry.
Note, after installation, you have to open the Toolbars menu entry twice to actually see the new entry.

In case you want to manually install the application after compiling it yourself:
- Create a new folder "DeskClock" under C:\Program Files\ or somewhere else.
- Copy the DLL and the clockface PNG into that directory.
- Open an elevated CMD window (press the Windows key, type cmd, right-click and select "Run as Adminstrator")
- Navigate to the folder where the DLL is located.
- Run "regsvr32 DeskClock.dll" (without the qoutes).
- Display the clock in the taskbar as described above.

## Hiding the clock
- You can simply uncheck the DeskClock in the context menu of the Windows taskbar.

## Complete uninstall
Run the installer again to unintall the DeskClock completely.

To manually uninstall:
- Open an elevated CMD window (see above).
- Navigate to the folder where DeskClock.dll resides.
- Run "regsvr32 /u DeskClock.dll
- Log off and back on again and delete the folder containig the files.

## Hints
### Stopping and re-starting the Windows Desktop (explorer.exe) without logging off
- Open a regular (non-elevated) CMD window 
- Click on the taskbar or on the desktop and press Alt+F4
- Instead of selecting one of the options, press Ctrl-Shift-Alt+Esc
- To restart the desktop, just execute "explorer" in the CMD window you opened before closing the desktop.

## Why an analog clock on the Windows taskbar?
- Because there was none so far.
- Because I have a 47 cm version of this Siemens slave clock from the Sixties at home and I like it.

## Version history
### Version 2.0.1
Added an MSI installer. The .wxs source file for the WiX toolset has been added to the source files.

### Version 2.0.0
This is the first version that
- is based on the Deskband sample coede from the MS Windows Classic Samples on GitHub,
- works with high-DPI settings (e.g. 150 % scaling of the display),
- uses the theme of taskbar for its background

### Version 1
I created the initial vesion in 2004 based on different Deskband sample code.

## SVG version:
[SVG Version of the clock](https://www.stefan-oskamp.de/SiemensClock/SVG/fullscreen/SiemensClock.svg) 
