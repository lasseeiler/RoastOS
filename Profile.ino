// These three arrays will hold the profile in full
double profTemperatures[prof_profileArraySize];
int profProgressionTimes[prof_profileArraySize];
int profTotalTimes[prof_profileArraySize];

//This variable will hold the amount of steps in the profile
int profStepCount;

bool prof_ReadProfile(char rawProfileInput[]) //Read profile from raw profile string
{
	profStepCount = 0; //Reset step count, as we are reading a new profile
	char* inputPtr = rawProfileInput; //Init pointer to start of profile string
	while((inputPtr <= &rawProfileInput[netInputBufferMaxLength-1]) && (*inputPtr != '}')) //Loop profile from end to end
	{
		if(*inputPtr == '#') //Detect new step
		{
			profStepCount++;
		}
		inputPtr++; //Move pointer down the string
	}
	inputPtr = rawProfileInput; //Reset pointer to start of string

	if(profStepCount==0) //No steps were detected, this means something went wrong
	{
		// We could call error handling here, but we prefer to just try again.
		return false;
	}

	char tmp[3]; //Array to hold a value while parsing the profile (for example 030 degrees)
	char* tmpPtr = tmp; //Init pointer to the temporary array
	int arrayType = 1; //Int to designate which of our profile arrays we must read the tmp pointer value into when done parsing each value
	int tmpStepNumber = 0; //Keep track of which step we are currently parsing

	while((inputPtr <= &rawProfileInput[netInputBufferMaxLength-1]) && (*inputPtr != '}')) //Loop profile string from end to end
	{
		if(*inputPtr == '-' || *inputPtr == '#') //Either - or # means we must have accumulated a 3-digit value in tmp array
		{
			tmpPtr = tmp; //Reset pointer to start of temporary array
			switch(arrayType)
			{
				case 1: //double profTemperatures
					profTemperatures[tmpStepNumber] = strtod(tmp,NULL); //Actually storing the value of the tmp array
					break;
				case 2: //int profProgressionTimes
					sscanf(tmp,"%d",&profProgressionTimes[tmpStepNumber]); //Actually storing the value of the tmp array
					break;
				case 3: //int profTotalTimes
					sscanf(tmp,"%d",&profTotalTimes[tmpStepNumber]); //Actually storing the value of the tmp array
					break;
			}
			arrayType++; //Go to target next profile array
			if(*inputPtr == '#') //Step is over at #
			{
				tmpStepNumber++; //Increase step count because it's a new step
				arrayType = 1; //Reset the target array because it's a new step
			}
		}
		else
		{
			//We are here because we still just need to read a digit into the tmp array
			*tmpPtr = *inputPtr; //Set the value of the array at it's current position, to the char at the current position in the profile string
			if(tmpPtr < &tmp[2]) //This test is to make sure we don't go beyond the memory allocated for the array
			{
				tmpPtr++; //Move to next place in the tmp array
			}
		}

		inputPtr++; //Move to next char in the profile string
	}
	return true;
}

void prof_PrintProfileToSerial() //Prints the currently loaded profile to the serial output which must be already active
{
	for(int i = 0; i < profStepCount;i++)
	{
		Serial.print(int(profTemperatures[i]));
		Serial.print("-");
		Serial.print(profProgressionTimes[i]);
		Serial.print("-");
		Serial.println(profTotalTimes[i]);
	}
}

double prof_GetCurrentTargetTemp() //Returns the current target temperature, depending on how long we've been roasting
{
	int elapsedSeconds = helper_getElapsedSeconds();
	for(int i = 0; i < profStepCount; i++) //Loop steps in profile
	{
		if(elapsedSeconds < profTotalTimes[i]) // This is a test to see if "this" step is the current step
		{
			if(elapsedSeconds < profProgressionTimes[i])
			{

				double prevStepTemp = 0;
				if(i > 0)
				{
					prevStepTemp = profTemperatures[i-1];
				}
				return ((profTemperatures[i] - prevStepTemp) * ((double)elapsedSeconds / (double)profProgressionTimes[i])) + prevStepTemp;
			}
			else
			{
				return profTemperatures[i]; //Return the temperature of the step.
			}
			
		}
		else
		{
			elapsedSeconds -= profTotalTimes[i]; //Subtract the seconds of this step, from the elapsed time, so the elapsed keeps decreasing until we match a step
		}
	}
	return 0.0; //If we haven't returned by now, we're f'ed...
}