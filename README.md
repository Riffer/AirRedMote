
# Simple and solid sketch to check IR modules  

* Arduino Nano (cheap and simple with the advantage of 5v levels)
    * any arduino compatible devices should do but you need to adjust the interrupt routines memory
* A receiving module (TSOP 3 pin like VS1838B) at D2
* A sending IR module (with power LED and transistor) or just a single IR LED with a resistor depending on the current and voltage at D3
* (optional) Matek buzzer sound module for send and receive feedback at A0 connected to B-.
* a simple push button at A1 (pin to ground)

By using the interrupt the system will also detect its own sending signal.
To have distance tests, just set up two items and you can send vice versa.