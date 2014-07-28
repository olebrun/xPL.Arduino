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
 
#include "xPL.h"

#define XPL_LINE_MESSAGE_BUFFER_MAX			128	// max length of a line			// maximum command in a xpl message
#define XPL_END_OF_LINE						10

// define the line number identifier
#define XPL_MESSAGE_TYPE_IDENTIFIER	        1
#define XPL_OPEN_HEADER						2
#define XPL_HOP_COUNT						3
#define XPL_SOURCE							4
#define XPL_TARGET							5
#define XPL_CLOSE_HEADER					6
#define XPL_SCHEMA_IDENTIFIER		        7
#define XPL_OPEN_SCHEMA						8

// Heartbeat request class definition
//prog_char XPL_HBEAT_REQUEST_CLASS_ID[] PROGMEM = "hbeat";
//prog_char XPL_HBEAT_REQUEST_TYPE_ID[] PROGMEM = "request";
//prog_char XPL_HBEAT_ANSWER_CLASS_ID[] PROGMEM = "hbeat";
//prog_char XPL_HBEAT_ANSWER_TYPE_ID[] PROGMEM = "basic";  //app, basic
#define XPL_HBEAT_REQUEST_CLASS_ID  "hbeat"
#define XPL_HBEAT_REQUEST_TYPE_ID  "request"
#define XPL_HBEAT_ANSWER_CLASS_ID  "hbeat"
#define XPL_HBEAT_ANSWER_TYPE_ID  "app"

/* xPL Class */
xPL::xPL()
{
  udp_port = XPL_UDP_PORT;
  
  SendExternal = NULL;

#ifdef ENABLE_PARSING
  AfterParseAction = NULL;

  last_heartbeat = 0;
  hbeat_interval = XPL_DEFAULT_HEARTBEAT_INTERVAL;
  xpl_accepted = XPL_ACCEPT_ALL;
#endif
}

xPL::~xPL()
{
}

/// Set the source of outgoing xPL messages
void xPL::SetSource_P(const PROGMEM char * _vendorId, const PROGMEM char * _deviceId, const PROGMEM char * _instanceId)
{
	memcpy_P(source.vendor_id, _vendorId, XPL_VENDOR_ID_MAX);
	memcpy_P(source.device_id, _deviceId, XPL_DEVICE_ID_MAX);
	memcpy_P(source.instance_id, _instanceId, XPL_INSTANCE_ID_MAX);
}

/**
 * \brief       Send an xPL message
 * \details   There is no validation of the message, it is sent as is.
 * \param    buffer         buffer containing the xPL message.
 */
void xPL::SendMessage(char *_buffer)
{
	(*SendExternal)(_buffer);
}

/**
 * \brief       Send an xPL message
 * \details   There is no validation of the message, it is sent as is.
 * \param    message         			An xPL message.
 * \param    _useDefaultSource	if true, insert the default source (defined in SetSource) on the message.
 */
void xPL::SendMessage(xPL_Message *_message, bool _useDefaultSource)
{
	if(_useDefaultSource)
	{
		_message->SetSource(source.vendor_id, source.device_id, source.instance_id);
	}

    SendMessage(_message->toString());
}

#ifdef ENABLE_PARSING

/**
 * \brief       xPL Stuff
 * \details   Send heartbeat messages at "hbeat_interval" interval
 */
void xPL::Process()
{
	static bool bFirstRun = true;

	// Check heartbeat + send
	if ((millis()-last_heartbeat >= (unsigned long)hbeat_interval * 1000)
		  || (bFirstRun && millis() > 3000))
	{
		SendHBeat();
		bFirstRun = false;
	}
}

/**
 * \brief       Parse an ingoing xPL message
 * \details   Parse a message, check for hearbeat request and call user defined callback for post processing.
 * \param    buffer         buffer of the ingoing UDP Packet
 */
void xPL::ParseInputMessage(char* _buffer)
{
	xPL_Message* xPLMessage = new xPL_Message();
	Parse(xPLMessage, _buffer);

	// check if the message is an hbeat.request to send a heartbeat
	if (CheckHBeatRequest(xPLMessage))
	{
		SendHBeat();
	}

	// call the user defined callback to execute an action
	if(AfterParseAction != NULL)
	{
	  (*AfterParseAction)(xPLMessage);
	}

	delete xPLMessage;
}

/**
 * \brief       Check the xPL message target
 * \details   Check if the xPL message is for us
 * \param    _message         an xPL message
 */
bool xPL::TargetIsMe(xPL_Message * _message)
{
  if (memcmp(_message->target.vendor_id, source.vendor_id, strlen(source.vendor_id)) != 0)
    return false;

  if (memcmp(_message->target.device_id, source.device_id, strlen(source.device_id)) != 0)
    return false;

  if (memcmp(_message->target.instance_id, source.instance_id, strlen(source.instance_id)) != 0)
    return false;

  return true;
}

/**
 * \brief       Send a heartbeat message
  */
void xPL::SendHBeat()
{
  last_heartbeat = millis();
  char buffer[XPL_MESSAGE_BUFFER_MAX];

//  sprintf_P(buffer, PSTR("xpl-stat\n{\nhop=1\nsource=%s-%s.%s\ntarget=*\n}\n%s.%s\n{\ninterval=%d\n}\n"), source.vendor_id, source.device_id, source.instance_id, XPL_HBEAT_ANSWER_CLASS_ID, XPL_HBEAT_ANSWER_TYPE_ID, hbeat_interval);

  sprintf_P(buffer, PSTR("xpl-stat\n{\nhop=1\nsource=%s-%s.%s\ntarget=*\n}\n%s.%s\n{\ninterval=%d\nport=3865\nremote-ip=192.168.4.133\nversion=1.0\n}\n"), source.vendor_id, source.device_id, source.instance_id, XPL_HBEAT_ANSWER_CLASS_ID, XPL_HBEAT_ANSWER_TYPE_ID, hbeat_interval);

  //(*SendExternal)(buffer);
  SendMessage(buffer);
}

/**
 * \brief       Check if the message is a heartbeat request
  * \param    _message         an xPL message
 */
inline bool xPL::CheckHBeatRequest(xPL_Message* _message)
{
  if (!TargetIsMe(_message))
    return false;

  return _message->IsSchema(XPL_HBEAT_REQUEST_CLASS_ID, XPL_HBEAT_REQUEST_TYPE_ID);
}

/**
 * \brief       Parse a buffer and generate a xPL_Message
 * \details	  Line based xPL parser
 * \param    _xPLMessage    the result xPL message
 * \param    _message         the buffer
 */
void xPL::Parse(xPL_Message* _xPLMessage, char* _buffer)
{
    int len = strlen(_buffer);

    byte j=0;
    byte line=0;
    int result=0;
    char lineBuffer[XPL_LINE_MESSAGE_BUFFER_MAX+1];

    // read each character of the message
    for(byte i = 0; i < len; i++)
    {
        // load byte by byte in 'line' buffer, until '\n' is detected
        if(_buffer[i] == XPL_END_OF_LINE) // is it a linefeed (ASCII: 10 decimal)
        {
            ++line;
            lineBuffer[j]='\0';	// add the end of string id

            if(line <= XPL_OPEN_SCHEMA)
            {
                // first part: header and schema determination
            	// we analyse the line, function of the line number in the xpl message
                result = AnalyseHeaderLine(_xPLMessage, lineBuffer ,line);
            }

            if(line > XPL_OPEN_SCHEMA)
            {
                // second part: command line
            	// we analyse the specific command line, function of the line number in the xpl message
                result = AnalyseCommandLine(_xPLMessage, lineBuffer, line-9, j);

                if(result == _xPLMessage->command_count+1)
                    break;
            }

            if (result < 0) break;

            j = 0; // reset the buffer pointer
            clearStr(lineBuffer); // clear the buffer
        }
        else
        {
            // next character
        	lineBuffer[j++] = _buffer[i];
        }
    }
}

/**
 * \brief       Parse the header part of the xPL message line by line
 * \param    _xPLMessage    the result xPL message
 * \param    _buffer         	   the line to parse
 * \param    _line         	       the line number
 */
byte xPL::AnalyseHeaderLine(xPL_Message* _xPLMessage, char* _buffer, byte _line)
{
    switch (_line)
    {
		case XPL_MESSAGE_TYPE_IDENTIFIER: //message type identifier

			if (memcmp_P(_buffer,PSTR("xpl-"),4)==0) //xpl
			{
				if (memcmp_P(_buffer+4,PSTR("cmnd"),4)==0) //command type
				{
					_xPLMessage->type=XPL_CMND;  //xpl-cmnd
				}
				else if (memcmp_P(_buffer+4,PSTR("stat"),4)==0) //statut type
				{
					_xPLMessage->type=XPL_STAT;  //xpl-stat
				}
				else if (memcmp_P(_buffer+4,PSTR("trig"),4)==0) // trigger type
				{
					_xPLMessage->type=XPL_TRIG;  //xpl-trig
				}
			}
			else
			{
				return 0;  //unknown message
			}

			return 1;

			break;

		case XPL_OPEN_HEADER: //header begin

			if (memcmp(_buffer,"{",1)==0)
			{
				return 2;
			}
			else
			{
				return -2;
			}

			break;

		case XPL_HOP_COUNT: //hop
			if (sscanf_P(_buffer, XPL_HOP_COUNT_PARSER, &_xPLMessage->hop))
			{
				return 3;
			}
			else
			{
				return -3;
			}

			break;

		case XPL_SOURCE: //source
			if (sscanf_P(_buffer, XPL_SOURCE_PARSER, &_xPLMessage->source.vendor_id, &_xPLMessage->source.device_id, &_xPLMessage->source.instance_id) == 3)
			{
			  return 4;
			}
			else
			{
			  return -4;
			}

			break;

		case XPL_TARGET: //target

			if (sscanf_P(_buffer, XPL_TARGET_PARSER, &_xPLMessage->target.vendor_id, &_xPLMessage->target.device_id, &_xPLMessage->target.instance_id) == 3)
			{
			  return 5;
			}
			else
			{
			  if(memcmp(_xPLMessage->target.vendor_id,"*", 1) == 0)  // check if broadcast message
			  {
				  return 5;
			  }
			  else
			  {
				  return -5;
			  }
			}
			break;

		case XPL_CLOSE_HEADER: //header end
			if (memcmp(_buffer,"}",1)==0)
			{
				return 6;
			}
			else
			{
				return -6;
			}

			break;

		case XPL_SCHEMA_IDENTIFIER: //schema
			sscanf_P(_buffer, XPL_SCHEMA_PARSER, &_xPLMessage->schema.class_id, &_xPLMessage->schema.type_id);
			return 7;

			break;

		case XPL_OPEN_SCHEMA: //header begin
			if (memcmp(_buffer,"{",1)==0)
			{
				return 8;
			}
			else
			{
				return -8;
			}

			break;
    }

    return -100;
}

/**
 * \brief       Parse the body part of the xPL message line by line
 * \param    _xPLMessage    				   the result xPL message
 * \param    _buffer         	  				   the line to parse
 * \param    _command_line       	       the line number
 */
byte xPL::AnalyseCommandLine(xPL_Message * _xPLMessage, char *_buffer, byte _command_line, byte line_length)
{
    if (memcmp(_buffer,"}",1) == 0) // End of schema
    {
        return _xPLMessage->command_count+1;
    }
    else	// parse the next command
    {
    	struct_command newcmd;
		
		sscanf_P(_buffer, XPL_COMMAND_PARSER, &newcmd.name, &newcmd.value);

        _xPLMessage->AddCommand(newcmd.name, newcmd.value);

        return _command_line;
    }
}
#endif
