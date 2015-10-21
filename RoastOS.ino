#include <math.h>
#include <Helpers.ino>
#include <DimmerControl.ino>
#include <Profile.ino>
#include <TemperatureControl.ino>
#include <Net.ino>

//Pins
const int tempSensorPin = A1;
const int bulbDesiredPositionPin = 6;
const int bulbPositionPin = 7;
const int triacOutputPin = 3;

const long tickInterval = 1000;
const long tickInterval_Status = 2000;
const long tickInterval_CurrentTemperature = 1000;
const long tickInterval_RoastingData = 5000;
const long tickInterval_ManualTemp = 2000;
const int prof_profileArraySize = 20;
long lastTickMillis = 0;
long lastTickMillis_Status = 0;
long lastTickMillis_CurrentTemperature = 0;
long lastTickMillis_RoastingData = 0;
long lastTickMillis_ManualTemp = 0;
long roastingStartMillis = 0;
bool profileLoaded = false;
int statusCode = 0;
double currentPace = 0;
double currentTargetPace = 0;
int currentRoastingEffect = 0;
int bulbDesiredPosition = 0;
double manualRoastTargetTemperature = 0;

const int netInputBufferMaxLength = 200;

void setup()
{
	Serial.begin(57600);
	
	pinMode(tempSensorPin, INPUT);
	pinMode(bulbPositionPin, INPUT);
	pinMode(bulbDesiredPositionPin, OUTPUT);
	initDimmerControl(triacOutputPin, 0); //0 = digital pin 2
	tc_resetTemperatures();
	net_setup();

	setStatus(30); //The first status after boot will be online & ready
}  

void loop()
{
	switch (statusCode) {
		case 30: //Online, ready
			sendCurrentTemperature();
			break;
		case 106: //Should load new profile
			getNewProfile();
			break;
		case 110: //Profile loaded, waiting
			sendCurrentTemperature();
			break;
		case 120: //Starting profile roast and inserting bulb
	    	sendCurrentTemperature();
	    	startProfileRoast();
	    	break;
	    case 130: //Roasting with profile
	    	runTemperatureCycle();
	    	sendCurrentTemperature();
	    	sendRoastingData();
	    	break;
	    case 210: //Initiating manual roast
	    	sendCurrentTemperature();
	    	startManualRoast();
	    	break;
	    case 220: //Roasting manually
	    	runTemperatureCycle();
	    	getManualRoastTargetTemperature();
	    	sendCurrentTemperature();
	    	sendRoastingData();
	    	break;
	    case 310: //Ending roast
	    	profileLoaded = false;
	    	dcSetLevel(0);
	    	if(!helper_isBulbEjected())
	    	{
	    		helper_ejectBulb();
	    	}
	    	else
	    	{
	    		setStatus(30);
	    	}
	    	break;
	    case 401: //Ending roast
	    	sendCurrentTemperature();
	    	break;
	}

	sendStatus(); //Send and receive status
	net_loop(); //Required for internet features to work
}
void startProfileRoast()
{
	if (!helper_isBulbInserted())
	{
		helper_insertBulb();
	}
	else
	{
		roastingStartMillis = millis();
		setStatus(130);
	}
}
void startManualRoast()
{
	dcSetLevel(0);
	manualRoastTargetTemperature = 0;
	if (!helper_isBulbInserted())
	{
		helper_insertBulb();
	}
	else
	{
		roastingStartMillis = millis();
		setStatus(220);
	}
}
void sendCurrentTemperature()
{
	if ((millis() > tickInterval_CurrentTemperature) && (millis() - tickInterval_CurrentTemperature) > lastTickMillis_CurrentTemperature)
	{
		if (net_sendCurrentTemperature(tc_getCurrentTemperature()))
		{
			lastTickMillis_CurrentTemperature = millis();
		}
	}
}
void getManualRoastTargetTemperature()
{
	if ((millis() > tickInterval_ManualTemp) && (millis() - tickInterval_ManualTemp) > lastTickMillis_ManualTemp)
	{
		if (net_getManualRoastTargetTemperature())
		{
			lastTickMillis_ManualTemp = millis();
		}
	}
}

void getNewProfile()
{
	net_startProfileLoad();
}
void runTemperatureCycle()
{
	if ((millis() > tickInterval) && (millis() - tickInterval) > lastTickMillis)
	{
		tc_temperatureAdjustmentCycle();
		lastTickMillis = millis();
	}
}
void sendStatus()
{
	if ((millis() > tickInterval_Status) && (millis() - tickInterval_Status) > lastTickMillis_Status)
	{
		if (net_sendStatusCode(statusCode))
		{
			Serial.print("Sending status code: ");
			Serial.println(statusCode);
			lastTickMillis_Status = millis();
		}
	}
}
void sendRoastingData()
{
	if ((millis() > tickInterval_RoastingData) && (millis() - tickInterval_RoastingData) > lastTickMillis_RoastingData)
	{
		if (net_sendRoastingData())
		{
			lastTickMillis_RoastingData = millis();
		}		
	}
}
void handleReceivedStatus(int newStatusCode)
{
	Serial.print("Received status code: ");
	Serial.println(newStatusCode);
	switch(newStatusCode)
	{
		case 30: //Online, ready
		case 106: //Loading profile
		case 110: //Profile loaded, waiting
		case 120: //Starting profile roast, inserting bulb
		case 130: //Roasting profile roast
		case 210: //Starting manual roast
		case 220: //Roasting with manual roast
		case 310: //Ending roast
			if (statusCode != newStatusCode)
			{
				setStatus(401); //Status code not allowed	
			}
			break;
		case 105: //Load profile
			if (statusCode == 30)
			{
				setStatus(106); //Get new profile
			}
			else
			{
				setStatus(401); //Status code not allowed
			}
			break;			
		case 115: //Start profile roast
			if (statusCode == 110)
			{
				setStatus(120); //Starting profile roast, inserting bulb
			}
			else
			{
				setStatus(401); //Status code not allowed
			}
			break;
		case 205: //Start manual roasting
			if (statusCode == 30 || statusCode == 110 || statusCode == 130)
			{
				setStatus(210); //Starting manual roast
			}
			else
			{
				setStatus(401); //Status code not allowed
			}
			break;
		case 305: //End roast and eject bulb
			if (statusCode == 105 || 
				statusCode == 106 || 
				statusCode == 110 || 
				statusCode == 115 || 
				statusCode == 120 || 
				statusCode == 130 || 
				statusCode == 205 || 
				statusCode == 210 || 
				statusCode == 220)
			{
				setStatus(310); //Ending roast and ejecting bulb
			}
			else
			{
				setStatus(401); //Status code not allowed
			}
			break;
		case 401: //Error
			setStatus(401);
			break;
	}
}

void setStatus(int newStatusCode)
{
	statusCode = newStatusCode;
}