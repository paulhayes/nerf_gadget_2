Nerf Gadget 2
=============

Improved version of the nerf gadget, now with a 4 digit display. Cheaper and easier to make than my original nerf gadget.

The Nerf Gadget acts as both a magazine counter, and also a bullet speed measuring device.

Requires

+ Multimeter
+ [FTDI Programmer](https://www.sparkfun.com/products/9716)
+ Hot glue gun


Circuit Components

+ 1x 7-Segment Serial Display by Sparkfun Electronics ( https://www.sparkfun.com/products/11441 )
+ 1x QRD1114 Reflective Object Sensor
+ 3x Microswitches
+ 1x 1kΩ trimming potentiometer
+ 1x 220Ω resistor
+ 4x 10kΩ resistor
+ 1x 15kΩ resistor
+ [lithium polymer battery](https://www.sparkfun.com/products/10718)

Sensor Housing

+ 15mm x 10cm PVC plumbing pipe
+ Hardboard tube 20cm

#### Step 1
Following [these instructions](https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Customizing%20the%20Display), for setting up Arduino IDE for use with Seven Segment Display. 

### Step 2
Make up your circuit board according to the [fritzing](http://fritzing.org/home/) files, provided in the hardware directory. 

#### Step 3
Seperate the QRD1114 sensor and emitter from their housing. 

#### Step 4
Drill a 5mm hole through either side of the PVC pipe.

#### Step 5 
Hot glue the ir sensor into one side of the pvc tube, being careful to not let any hot glue protrude into the tubes interior. Repeat the process on the other side of the tube with the ir emitter.

#### Step 6
Solder 4 wires from the sensor and emitter to your circuit board. 




![prototype](https://raw2.github.com/paulhayes/nerf_gadget_2/master/images/NerfGadget%202%20closeup.jpg) 
