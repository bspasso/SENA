# SENA
Solar ENergy and Automation (Internet of Things - IoT)

Sun ENergy and Automation (SENA)
================================

[Open Source Offline Home Automation Systems](https://www.dropbox.com/s/vqv9f4tq64pkadt/Comp_Space_2019_SENA.pdf)

[link222](https://www.dropbox.com/s/xrgiy96et31nbw5/index.htm?dl=0)

* Setting HTTP headers appropriately can help to lock down and secure your web application. [read more...](https://github.com/helmetjs/helmet)

**TEST**

[link](https://github.com/bspasso/test)

<a href="http://www.youtube.com/watch?feature=player_embedded&v=YOUTUBE_VIDEO_ID_HERE
" target="_blank"><img src="http://img.youtube.com/vi/YOUTUBE_VIDEO_ID_HERE/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="240" height="180" border="10" /></a>

**SENA** is a Project which started from 6KWp Photostatic System for household usage.

### Harvesting the Sun's Energy 


This became the very first sub-system, based upon which I have elaborated another sub-systems of the overall SENA Project as described below:

1. Photostatic OFF-Grid systems with the following components:
	a) Two **strings** of 12 PV panels each: SHARP ND-RB270 = **24 panels** X 270 KWp = 6.48 KWp /2 MPP Trackers X 3 KWp each/ [link to SHARP ND-RB270](https://www.sharp.co.uk/cps/rde/xchg/gb/hs.xsl/-/html/product-details-solar-modules-2189.htm?product=NDRB270)
	b) Conversol S2 Off-Grid Inverter - 5kVA, 48V, DUO MPPT Charger [link](https://voltaconsolar.com/conversol-5kva-duo-mppt-inverter-charger.html)
	c) WEB and SNMP Box, Modbus - Remote Monitoring for Hybrid Inverters
		[link](https://voltaconsolar.com/solar-products/accessories/webox-modbus-remote-monitoring-hybrid-ongrid-inverters.html) and the concept of SNMP based upon which I've elaborated server side extract of energy related data:
		[link](https://www.dpstele.com/snmp/what-does-oid-network-elements.php) and a very useful tool: 
		[SNMP OID/MIB Browser](http://www.ireasoning.com/downloadmibbrowserfree.php)
	d) AccuForce 12 - 200 AGM VLRA BATTERY X 4 /4 X 12V = 48V X 200 Ah = 9.6 KWh/	
		[link](http://www.systems-sunlight.com/wp-content/uploads/2014/11/accuforce-12v-200ah.pdf)

The above PV OFF-Grid system has the advantage in a case of insufficient sun power to automatically switch in bypass mode for providing electrical energy directly from public Utility Grid /220V in EU/.
	
### Components and sub-systems for managing electrical energy consumption:

2. Hot water heating devices X 4pcs (two in the basement and one per 1st and 2nd floors) from commercially mass produced **ARISTON SHP ECO 100 V 1.8K** [link](http://www.ariston.com/sg/SHAPE_ECO_50-80-100)
		
	**Without** any interventions on the original ARISTON's electronics, I've equipped each of the water heaters with additional 10K thermistor for reading the temperature of the water in real time using: [Thermistor, interfacing with NodeMCU](http://www.electronicwings.com/nodemcu/thermistor-interfacing-with-nodemcu)
	Also the NodeMCU Arduino based ESP8266 controller is capable to switch ON and OFF the ARISTON water heater with emulating power ONOFF push button with a small DUAL IN LINE REED RELAY /1 normally open contact/ 
	
<a href="./images/IMG_20180407_130121.jpg" target="_blank"><img src="./images/IMG_20180407_130121.jpg" 
alt="MY IMAGE TEXT HERE" width="240" height="180" border="10" /></a>

		(detailed description and additionally used libraries/protocols like MQTT will be elaborated separately in Wiki part of the **SENA Project**)

3. Next category of mass used devices for heating and cooling the house is 6 X Mitsubishi Electric HVACs (4 X MUZ-FH25VEHZ; 1 X MUZ-FH35VEHZ; 1 X MUZ-FH50VEHZ).
	In addition to already pre-installed WiFi modules to connect arch of the devices to MELCloud for remote control over the Web, for each of the HVACs /per room of the house/ I have assembled small boxes with ESP8266 and DHT-22 sensor for monitoring temperature and humidity.
	The main feature of these boxes is an infrared beaming LED which /connected to digital output of ESP8266/ can switch the HVAC in each of the modes its operates /heating, cooling and dry mode/. Special **THANKS** to:
		[IR Remote Arduino Libraries](https://www.arduinolibraries.info/libraries/i-rremote-esp8266) and 
		[IR Remote Arduino Examples](https://github.com/markszabo/IRremoteESP8266/tree/master/examples) and
		[IR Remote Example for Mitsubishi Electric HVACs](https://github.com/markszabo/IRremoteESP8266/blob/1f9d0abc3d5494b05d9aaa29439634de1a2dc925/examples/TurnOnMitsubishiAC/TurnOnMitsubishiAC.ino)
		
### Monitoring and controlling components:

[test link to a test page](./pages/README2.md)

4. Monitoring UV Index, Visual Light and IR Light nearby photostatic strings.

5. Switching ON/OFF 220V Energy entry from Public Grid and bypassing PV Conversol module to directly supply Grid 220V over the nights.

### Central Processing Unit:

6. Raspberry PI with MQTT Brokerage function and Node-RED visual coding software environment.

### Disclaimer and comments

The project is 85% ready and in-production. The final 15% of Server side function and business logic are still to me finalized. Detailed description will follow.

### License

MIT 

### Acknowledgements


### Description


### Version

v. 1.9

### How to cite SENA

MIT license rules apply

### How to contribute to the development of SENA

ETHEREUM /ETH/ and BITCOIN /BTC/ addresses will be published soon.

