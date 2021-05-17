# A second life for an old microwave oven

<p align="center">
  <img src="images/1.jpg?raw=true" width="600" title="Microwave after modification">
</p>

I have a 28 years old oven, Samsung M7136. It so old, than this image is probably only one of this model on the Internet :)

<p align="center">
  <img src="images/5.jpg?raw=true" width="400" title="Samsung M7136">
</p>

The microwave had been working great, but but one day it stopped. One gear become broken:

<p align="center">
  <img src="images/4.jpg?raw=true" width="400" title="broken mechanism">
</p>

It was a pity to loose such a great oven an I did not want to multiply waste as well, so I decided to replace the mecanical timer with an electronic one.
If you decide to repeat this experience, please, do not remove any switches or termal protections from your oven :)

## Components:

1. Arduino NANO;
2. 2 pcs. 5V 10A x 250V relays, but it is recommended to use 30A relay for the microwave switch instead of 10A one;
3. KY-040 encoder module;
4. TM1637 display module;
5. 5V active buzzer;
6. 5V power supply.

## Connection schematic:

<p align="center">
  <img src="images/schem.png?raw=true" width="600" title="Microwave after modification">
</p>

## Functionality:
Turn the encoder knob clockwise to start timer and heeting process, turn the knob counterclockwise to decrease time and turn it off. Short press on the knob in turned off state to show the current power level. Long press on the knob in turned off state to change the power level.

Arduino project code is included in this repo.
<p align="center">
  <img src="images/2.jpg?raw=true" width="400" title="Samsung M7136">
</p>

<p align="center">
  <img src="images/3.jpg?raw=true" width="400" title="Samsung M7136">
</p>

 Remember about safety and have a fun :)
