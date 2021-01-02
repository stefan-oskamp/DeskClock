# DeskClock
## Analog clock toolbar for the Windows taskbar.

![Screenshot vertical taskbar](/images/screenshot-vertical-taskbar.png)
![Screenshot horizontal taskbar](/images/screenshot-horizontal-taskbar.png) 

## Pre-requisites
Version 2.0.0 has been tested under Windows 10 64 bit. A 32 bit version could be provided if needed.

## Installation
The binary relaese needs to be installed manually (for now):
- Create a new folder "DeskClock" under C:\Program Files\.
- Move the release zip file there and unpack it.
- Open an elevated CMD window (press the Windows key, type cmd, right-click and select "Run as Adminstrator")
- Navigate to the folder where you unpacked the release.
- Run "regsvr32 DeskClock.dll" (without the qoutes).
- Right-click on the taskbar and open "Toolbars >"
- If you see "DeskClock", activate it. If not, close the menu and retry. One retry is needed after the first insatllation.
- You can change the size of the toolbar. (You might have to right-click on the taskbar and "unlock" it first.)

## Hiding the clock
- You can simply uncheck the DeskClock in the context menu of the Windows taskbar.

## Complete uninstall
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
### Version 2.0.0
This is the first version that
- is based on the Deskband sample coede from the MS Windows Classic Samples on GitHub,
- works with high-DPI settings (e.g. 150 % scaling of the display),
- uses the theme of taskbar for its background

### Version 1
I created the initial vesion in 2004 based on different Deskband sample code.

## SVG version:
[SVG Version of the clock](https://www.stefan-oskamp.de/SiemensClock/SVG/fullscreen/SiemensClock.svg) 







