
Gabriel Staples
www.ElectricRCAircraftGuy.com    
25 Sept. 2017 

The hand in the lab was built and programmed previously by former students or researchers. It didn't work well, so I rewrote the code entirely from scratch, and replaced the Sharp 2Y0A02 F 5Y IR distance sensor with a basic LDR/photoresistor. I would have preferred to start with their code base, but I didn't have it. This folder contains the backup at least of the actual flash memory stored directly on the mcu, so that if I want to revert to the old code I can, even though I don't have the actual source code for it. 

All of this data was read from the mcu using AVRDude from the command line. The command used is stored in "AVRDude backup mcu commands.txt".


