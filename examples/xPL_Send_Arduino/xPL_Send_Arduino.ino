/*
 * xPL.Arduino v0.1, xPL Implementation for Arduino
 *
 * This code is parsing a xPL message stored in 'received' buffer
 * - isolate and store in 'line' buffer each part of the message -> detection of EOL character (DEC 10)
 * - analyse 'line', function of its number and store information in xpl_header memory
 * - check for each step if the message respect xPL protocol
 * - parse each command line
 *
 * Copyright (C) 2012 johan@pirlouit.ch, olivier.lebrun@gmail.com
 * Original version by Gromain59@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
 
#include <SPI.h>        
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "xPL.h"

xPL xpl;

unsigned long timer = 0; 

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 177);
IPAddress broadcast(10, 0, 0, 255);
EthernetUDP Udp;

void SendUdPMessage(char *buffer)
{
    Udp.beginPacket(broadcast, xpl.udp_port);
    Udp.write(buffer);
    Udp.endPacket(); 
}

void setup()
{
  Serial.begin(115200);
  Ethernet.begin(mac,ip);
  Udp.begin(xpl.udp_port);  
  
  xpl.SendExternal = &SendUdPMessage;  // pointer to the send callback 
  xpl.SetSource_P(PSTR("xpl"), PSTR("arduino"), PSTR("test")); // parameters for hearbeat message
}

void loop()
{
  xpl.Process();  // heartbeat management
   
   // Example of sending an xPL Message every 10 second
   if ((millis()-timer) >= 10000)
   {
     xPL_Message msg;

     msg.hop = 1;
     msg.type = XPL_TRIG;

     msg.SetTarget_P(PSTR("*"));
     msg.SetSchema_P(PSTR("sensor"), PSTR("basic"));

     msg.AddCommand_P(PSTR("device"),PSTR("1"));
     msg.AddCommand_P(PSTR("type"),PSTR("temp"));
     msg.AddCommand_P(PSTR("current"),PSTR("22"));

     xpl.SendMessage(&msg);
     
     timer = millis();
   } 
}