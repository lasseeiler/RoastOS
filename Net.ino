#include <SPI.h>
#include <Ethernet.h>

int currentTask = 0;
bool callStarted = false;
long nextRetryMillis = 0;
bool readingInput = false;
char inputString[netInputBufferMaxLength];
char* inputStringPtr = inputString;
int statusCodeToSend = 0;
int tmpStatusCode = 0;
double temperatureToSend = 0;

//MAC address of the ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

char server[] = "webinterface.il-torrefattore.dk"; // Domain to use for DNS lookup



// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,1,177);

EthernetClient client;

void net_setup()
{
  //if (Ethernet.begin(mac) == 0) {
  //  Serial.println("Failed to configure Ethernet using DHCP");
    
    // Try manual IP
    Ethernet.begin(mac, ip);
  //}
  // Ethernet needs a second to initialize
  nextRetryMillis = millis()+1000;
}

void net_loop()
{

  if(currentTask > 0 && !callStarted && nextRetryMillis < millis())
  {
    //Disconnect any live connection
    client.stop();

    if (client.connect(server, 80)) {
      callStarted = true;

      switch(currentTask)
      {
        case 1: // Get/set status
          client.print("GET /RoastIO/RoasterStatus.aspx?code=");
          client.print(statusCodeToSend);
          break;
        case 2: // Load profile
          client.print("GET /RoastIO/GetProfile.aspx");
          break;
        case 3: // Send current temperature
          client.print("GET /RoastIO/ReceiveCurrentTemperature.aspx?temperature=");
          client.print(temperatureToSend);
          break;
        case 4: // Send current temperature
          client.print("GET /RoastIO/ReceiveRoastData.aspx?roastdata=");
          client.print(helper_getElapsedSeconds());
          client.print(";");
          client.print(tc_getLastReadTemperature());
          client.print(";");
          client.print(currentPace);
          client.print("-");
          client.print(currentTargetPace);
          client.print("-");
          client.print(currentRoastingEffect);
          break;
        case 5: // Get manual roast target temperature
          client.print("GET /RoastIO/GetManualRoastTemperature.aspx");
          break;
      }
      client.println(" HTTP/1.1");
      client.println("Host: webinterface.il-torrefattore.dk");
      client.println("Connection: close");
      client.println();        
      
      Serial.print("Connection initiated:            ");
      switch(currentTask)
      {
        case 1: // Get/set status
          Serial.println("#");
          break;
        case 2: // Load profile
          Serial.println("# #");
          break;
        case 3: // Send current temperature
          Serial.println("# # #");
          break;
        case 4: // Send current temperature
          Serial.println("# # # #");
          break;
        case 5: // Get man roast temp
          Serial.println("# # # # #");
          break;
      }
    } 
    else
    {
        nextRetryMillis = millis()+1000;
    }
  }

  if(callStarted && currentTask > 0 && client.available())
  {
    char c = client.read();
    if(c == '{')
    {
      readingInput = true;
    }
    if(readingInput && c != '{')
    {
      *inputStringPtr = c;
      inputStringPtr++; // Maybe make some check to ensure we don't exceed array size thus rebooting arduino
    }
    if(!client.available())
    {
      readingInput = false;

      switch (currentTask)
      {
          case 1:
            currentTask = 0;
            tmpStatusCode = helper_readStatus(inputString);
            if(tmpStatusCode>0)
            {
              handleReceivedStatus(tmpStatusCode);
            }
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
          case 2:
            currentTask = 0;
            if(prof_ReadProfile(inputString))
            {
              profileLoaded = true;
              setStatus(110);
              //Debug
              prof_PrintProfileToSerial();
            }
            else
            {
              net_profileLoadFailed();
            }
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
          case 3:
          case 4:
            currentTask = 0;
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
          case 5:
            currentTask = 0;
            manualRoastTargetTemperature = helper_readManualRoastTargetTemp(inputString);
            flushInputBuffer(inputString, netInputBufferMaxLength, ' ');
            inputStringPtr = inputString;
            break;
      }
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
  Serial.println("Error loading profile");
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
    Serial.print("Sending temperature: ");
    Serial.println(temperature);
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
    Serial.println("Sending roasting data"); 
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
    Serial.println("Getting man. roast target temp."); 
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