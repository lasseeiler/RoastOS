#include <TimerOne.h>

volatile int _dcChopCount = 0;    // Variable to use as a counter (volatile because it's changed within interrupt code!)
volatile int _dcZeroCrossed = 0;  // Boolean to store a "switch" to tell us if we have crossed zero (volatile because it's changed within interrupt code!)
int _dcTriacOutputPin;            // Output to Opto Triac
int _dcDimmerLevel = 0;           // Dimming level (0-128)  0 = on, 128 = 0ff
int _dcInterruptId;               // Variable to hold which interrupt to use, will be set from main code

int _dcFreqInterval = 78;         // This is the delay-per-brightness step in microseconds.
                                  // It is calculated based on the frequency the voltage supply (50Hz or 60Hz)
                                  // and the number of brightness steps we want.

void initDimmerControl(int triacOutputPin, int interruptId)
{
  _dcTriacOutputPin = triacOutputPin;
  _dcInterruptId = interruptId;
  pinMode(triacOutputPin, OUTPUT);

  attachInterrupt(_dcInterruptId, dcZeroCrossDetect, RISING);   // Attach an Interupt to a valid interrupt pin for Zero Cross Detection
  Timer1.initialize(_dcFreqInterval);                           // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dcCheckDimmer, _dcFreqInterval);  
}

void dcZeroCrossDetect()
{
  _dcZeroCrossed = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  _dcChopCount = 0;
  digitalWrite(_dcTriacOutputPin, LOW);       // turn off TRIAC (thus AC)
}

void dcCheckDimmer()
{
  if(_dcZeroCrossed == true) 
  {
    if(_dcChopCount >= 128 - _dcDimmerLevel) 
    {
      digitalWrite(_dcTriacOutputPin, HIGH); // turn on light
      _dcChopCount = 0;  // reset time step counter
      _dcZeroCrossed = false; //reset zero cross detection
    }
    else 
    {
      _dcChopCount++; // increment time step counter
    }                                
  }                                  
}  

void dcSetLevel(int dimmerLevel)
{
  if(dimmerLevel < 0)
  {
    dimmerLevel = 0;
  }
  if(dimmerLevel > 100)
  {
    dimmerLevel = 100;  // When testing, the last steps from 100 up to 128 do not give any great effect,
                        // and thus we'd have to implement logic in the algorithm to take care of that.
                        // Instead we limit to max 100, where each step has a greater effect.
                        // Also change this in TC code: maxRoastingEffect
  }
  if(dimmerLevel == 0)
  {
    _dcDimmerLevel = dimmerLevel;
  }
  else
  {
    _dcDimmerLevel = dimmerLevel+12;
  }
  
}
