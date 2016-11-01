# RoastOS
OS for IT2 coffee roaster

# THIS BRANCH!
The purpose of this branch is to try and switch the network hardware from an Arduino ethernet shield to an ESP8266 module.

# Notes so far
AT+CWDHCP=1,1 activates to get IP from DHCP
AT+CIPMUX=1 must always get called
AT+CIPSTA="192.168.0.215","192.168.0.1","255.255.255.0" will set static IP, gateway and subnet mask

When sending headers, line breaks are necessary, and remember to end with an extra line break to let the server know we're done.
# Random documentation I find useful
##### AT+CWDHCP - Enable/Disable DHCP  
    
|Type|Instruction|Response|    
|-----|:-----:|:-----:|     
|Set|AT+CWDHCP=mode,en|OK|    
 

**Parameter Description**    
mode   
0 : set ESP8266 softAP  
1 : set ESP8266 station  
2 : set both softAP and station  
en  
0 : Disable DHCP  
1 : Enable DHCP  