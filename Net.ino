int currentTask = 0;
bool callStarted = false;
bool readingInput = false;
char inputString[netInputBufferMaxLength];
char* inputStringPtr = inputString;
int statusCodeToSend = 0;
int tmpStatusCode = 0;
double temperatureToSend = 0;

void net_setup()
{
}

void net_loop()
{
  bool expectResponse = false;

  if(currentTask > 0 && !callStarted)
  {
    switch(currentTask)
    {
      case 1: // Get/set status
        Serial.print("1/");
        Serial.print(statusCodeToSend);
        Serial.print("/$");
        callStarted = true;
        break;
      case 2: // Load profile
        Serial.print("2$");
        callStarted = true;
        break;
      case 3: // Send current temperature
        Serial.print("3/");
        Serial.print(temperatureToSend);
        Serial.print("/$");
        callStarted = true;
        break;
      case 4: // Send roast data
        Serial.print("4/");
        Serial.print(helper_getElapsedSeconds());
        Serial.print(";");
        Serial.print(tc_getLastReadTemperature());
        Serial.print(";");
        Serial.print(currentPace);
        Serial.print("-");
        Serial.print(currentTargetPace);
        Serial.print("-");
        Serial.print(currentRoastingEffect);
        Serial.print("/$");
        callStarted = true;
        break;
      case 5: // Get manual roast target temperature
        Serial.print("5$");
        callStarted = true;
        break;
    }
  }

  if(callStarted && currentTask > 0 && Serial.available())
  {
    char c = Serial.read();
    if(c == '{')
    {
      readingInput = true;
    }
    if(readingInput && c != '{')
    {
      *inputStringPtr = c;
      inputStringPtr++; // Maybe make some check to ensure we don't exceed array size thus rebooting arduino
    }
    if(c=='%')
    {
      readingInput = false;

      switch (currentTask)
      {
          case 1:
            tmpStatusCode = helper_readStatus(inputString);
            if(tmpStatusCode>0)
            {
              handleReceivedStatus(tmpStatusCode);
            }
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
          case 2:
            if(prof_ReadProfile(inputString))
            {
              profileLoaded = true;
              setStatus(110);
              //Debug
              //prof_PrintProfileToSerial();
            }
            else
            {
              net_profileLoadFailed();
            }
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
          case 5:
            manualRoastTargetTemperature = helper_readManualRoastTargetTemp(inputString);
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
      }
      currentTask = 0;
      callStarted = false;
    }
  }
}

bool net_startProfileLoad()
{
  if(currentTask == 0)
  {
    currentTask = 2; //Load profile
    return true;
  }
  else
  {
    return false;
  }
}

void net_profileLoadFailed()
{
  //Serial.println("Error loading profile");
}

bool net_sendStatusCode(int statusCode)
{
  if(currentTask == 0)
  {
    currentTask = 1; //Load profile
    statusCodeToSend = statusCode;
    return true;
  }
  else
  {
    return false;
  }  
}

bool net_sendCurrentTemperature(double temperature)
{
  if(currentTask == 0)
  {
    currentTask = 3; //Send temperature
    temperatureToSend = temperature;
    //Serial.print("Sending temperature: ");
    //Serial.println(temperature);
    return true;
  }
  else
  {
    return false;
  }  
}

bool net_sendRoastingData()
{
  if(currentTask == 0)
  {
    currentTask = 4; //Send roasting data
    //Serial.println("Sending roasting data"); 
    return true;
  }
  else
  {
    return false;
  }  
}

bool net_getManualRoastTargetTemperature()
{
  if(currentTask == 0)
  {
    currentTask = 5; //Get manual roast temp.
    //Serial.println("Getting man. roast target temp."); 
    return true;
  }
  else
  {
    return false;
  }  
}

bool net_isBusy()
{
  return currentTask != 0;
}