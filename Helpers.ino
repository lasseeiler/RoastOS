String doubleToString(double input)
{
	char output[10];
	dtostrf(input,3,2,output);
	return String(output);
}

void flushInputBuffer(char array[], int length, char charToInsert)
{	
	for(int i = 0; i<length; i++)
	{
		array[i] = charToInsert;
	}
}

int helper_readStatus(char rawInputString[])
{
	char* inputPtr = rawInputString; //Init pointer to start of profile string
	char tmp[3]; //Array to hold a value while parsing the profile (for example 030 degrees)
	char* tmpPtr = tmp; //Init pointer to the temporary array
	int tmpStatusCode = 0;

	while((inputPtr <= &rawInputString[netInputBufferMaxLength-1]) && (*inputPtr != '}')) //Loop profile string from end to end
	{
		
		*tmpPtr = *inputPtr; //Set the value of the array at it's current position, to the char at the current position in the profile string
		if(tmpPtr < &tmp[2]) //This test is to make sure we don't go beyond the memory allocated for the array
		{
			tmpPtr++; //Move to next place in the tmp array
		}
		inputPtr++; //Move to next char in the profile string
	}
	sscanf(tmp,"%d",&tmpStatusCode);
	return tmpStatusCode;
}

double helper_readManualRoastTargetTemp(char rawInputString[])
{
	char* inputPtr = rawInputString; //Init pointer to start of profile string
	char tmp[3]; //Array to hold a value while parsing the profile (for example 030 degrees)
	char* tmpPtr = tmp; //Init pointer to the temporary array
	int tmpTemperature = 0;

	while((inputPtr <= &rawInputString[netInputBufferMaxLength-1]) && (*inputPtr != '}')) //Loop profile string from end to end
	{
		
		*tmpPtr = *inputPtr; //Set the value of the array at it's current position, to the char at the current position in the profile string
		if(tmpPtr < &tmp[2]) //This test is to make sure we don't go beyond the memory allocated for the array
		{
			tmpPtr++; //Move to next place in the tmp array
		}
		inputPtr++; //Move to next char in the profile string
	}
	sscanf(tmp,"%d",&tmpTemperature);
	return (double)tmpTemperature;
}

int helper_getElapsedSeconds()
{
	return (millis() - roastingStartMillis) / 1000;
}

void helper_insertBulb()
{
	bulbDesiredPosition = 1;
	digitalWrite(bulbDesiredPositionPin, HIGH);
}

bool helper_isBulbInserted()
{
	return (digitalRead(bulbPositionPin) == HIGH) && (bulbDesiredPosition == 1);
}

void helper_ejectBulb()
{
	bulbDesiredPosition = 0;
	digitalWrite(bulbDesiredPositionPin, LOW);
}

bool helper_isBulbEjected()
{
	return (digitalRead(bulbPositionPin) == LOW) && (bulbDesiredPosition == 0);
}