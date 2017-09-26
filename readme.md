
By Gabriel Staples  
www.ElectricRCAircraftGuy.com  
25 Sept. 2017  

## Background:  

This is a gimmicky trinket I programmed at the Florida Institute of Technology (FIT). It sits in Lab 116 (Dynamic Systems, Controls, and Mechatronics lab) of the Olin Engineering Building, where I worked part time from approx. July 2017 to 25 Sept. 2017, on the RINGS firmware and project. 

When I first arrived in the lab there was this gimmicky hand made up of a pink rubber glove, mounted with straws to a couple 9g servos on top of a hunk of scrap aluminum. It had two axes of control: 1 to pop up the hand from a lying down position to an upright position, and the other to wave it back and forth. It had an ATmega328-based Arduino Pro Mini and a Sharp 2Y0A02 F 5Y IR distance sensor. The distance sensor sat behind the glass inside our lab, looking out into the hallway. There was a sign on the window just underneath the device that read:  

```
Observe:  
Engineers in their 
natural habitat. 

Please do not tap on glass, 
engineers are easily frightened
by outsiders.
```

The concept is that if someone stands in front of the IR distance sensor long enough, and then walks away, the hand will pop up and wave several times back and forth at them, then lay back down onto a pedestal. It worked very sporadically, however. It would rarely pop up, even if you tried to force it to by standing in front of it intentionally then jumping away, waving your hand, etc. It was really flaky. Even people who knew it was supposed to work couldn't hardly get it to work, and those who didn't...well they hardly got the chance to see it work. 98% of the people walking by would never see it do *anything.*

So, I decided to fix it. I want it to not only work, but work well. I want it to pretty much just wave at *everyone* who walks by. I thought about using an ultrasonic range finder to improve reliability, but quickly realized that will measure the distance to the glass, as it cannot see through the glass. Since I have a little bit of basic Digital Signal Processing (DSP) knowledge, I decided to instead make it work with a basic LDR (light dependent resistor)/photoresistor instead. Since there is good back-lighting in the hallway, from both the sun and the hall lights, I can see that people cast ligth shadows on the wall as they pass by, and I know I can make an LDR sensitive enough to detect these shadows, and just wave at everyone passing by. 

So let's get to work!  

## Tips:  

Many parts of this contraption are hot-glued together. As I learned from FliteTest a few years back, if you ever need to separate hot glue bonds, just drop denatured alcohol onto the joints and it will wick into the hot glue joints and cause them to easily separate and pop apart.  

## Does the new source code for the contraption work?  

TBD.  


