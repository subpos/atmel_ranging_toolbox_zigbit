# atmel_ranging_toolbox_zigbit
Modifications to allow the Atmel RTB to run on an Atmel ATxmega256A3U-and-AT86RF233-ZigBit:

http://www.atmel.com/devices/ATxmega256A3U-and-AT86RF233-ZigBit-Wireless-Module.aspx

The original Ranging Toolbox evaluation application is located here:

http://www.atmel.com/tools/REB233SMAD-EK.aspx?tab=documents

Details:
The Atmel Ranging Toolbox evaluation application is designed to run on the REB233SMAD-EK.
However the code is portable and can run on similar hardware, in particular the AT86RF233
Zigbit modules. This code provides modifications to allow the Toolbox to run on the
ATxmega256A3U-and-AT86RF233-ZigBit module. Note that this module only has a single antenna
and as such doesn't support antenna diversity, so there is no ability to detect and 
reject bad results from multipath interference. This code is provided as a proof of 
concept demonstration only.

The system clock is sourced differently to the RTB defaults due to the way the CLKM pin from 
the AT86RF233 is connected to the ATxemga on the Zigbit module. The pin it connects to isn't 
supported by the event system in the xmega, so instead this CLKM pin is used as the main clock 
source for the xmega and the event timer is fed by the prescaler instead.

Some minor fixes are included as well to prevent a lockup which occurs when the AT86RF233 doesn't
wake up from sleep occasionally (this would normally happen randomly, but more than once an
hour or so).

You can adjust CONT_RANGING_PERIOD_MS   (100UL) in rtb_eval_app_param.h if you want faster 
continuous ranging measurements.

Licence - please read the Atmel EULA before using.
