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
 
#include "xPL_Message.h"

xPL_Message::xPL_Message()
{
    command = NULL;
	command_count = 0;
}

xPL_Message::~xPL_Message()
{
	if(command != NULL)
	{
		free(command);
	}
}

/**
 * \brief       Set source of the message (optional)
 * \param    _vendorId         vendor id.
 * \param    _deviceId         device id.
 * \param    _instanceId      instance id.
 */
void xPL_Message::SetSource(char * _vendorId, char * _deviceId, char * _instanceId)
{
	memcpy(source.vendor_id, _vendorId, XPL_VENDOR_ID_MAX + 1);
	memcpy(source.device_id, _deviceId, XPL_DEVICE_ID_MAX + 1);
	memcpy(source.instance_id, _instanceId, XPL_INSTANCE_ID_MAX + 1);
}

/**
 * \brief       Set Target of the message
 * \details	  insert "*" into _vendorId to broadcast the message
 * \param    _vendorId         vendor id.
 * \param    _deviceId         device id.		(optional)
 * \param    _instanceId      instance id.    (optional)
 */
void xPL_Message::SetTarget_P(const PROGMEM char * _vendorId, const PROGMEM char * _deviceId, const PROGMEM char * _instanceId)
{
	memcpy_P(target.vendor_id, _vendorId, XPL_VENDOR_ID_MAX + 1);
	if(_deviceId != NULL) memcpy_P(target.device_id, _deviceId, XPL_DEVICE_ID_MAX + 1);
	if(_instanceId != NULL) memcpy_P(target.instance_id, _instanceId, XPL_INSTANCE_ID_MAX + 1);
}

/**
 * \brief       Set Schema of the message
  * \param   _classId       Class
 * \param    _typeId         Type
  */
void xPL_Message::SetSchema_P(const PROGMEM char * _classId, const PROGMEM char * _typeId)
{
	memcpy_P(schema.class_id, _classId, XPL_CLASS_ID_MAX + 1);
	memcpy_P(schema.type_id, _typeId, XPL_TYPE_ID_MAX + 1);
}

/**
 * \brief       Create a new command/value pair
 * \details	  Check if maximun command is reach and add memory to command array
 */
bool xPL_Message::CreateCommand()
{
	struct_command	*ncommand;

	// Maximun command reach
	// To avoid oom, we arbitrary accept only XPL_MESSAGE_COMMAND_MAX command
	if(command_count > XPL_MESSAGE_COMMAND_MAX)
		return false;
		
	ncommand = (struct_command*)realloc ( command, (command_count + 1) * sizeof(struct_command) );
	
	if (ncommand != NULL) {
		command = ncommand;
		command_count++;
		return true;
	}
	else
		return false;
}

/**
 * \brief       Add a command to the message's body
 * \details	  PROGMEM Version
 * \param    _name         name of the command
 * \param    _value         value of the command
 */
bool xPL_Message::AddCommand_P(const PROGMEM char* _name, const PROGMEM char* _value)
{
	if(!CreateCommand()) return false;

	struct_command newcmd;
	memcpy_P(newcmd.name, _name, XPL_NAME_LENGTH_MAX + 1);
	memcpy_P(newcmd.value, _value, XPL_VALUE_LENGTH_MAX + 1);
	command[command_count-1] = newcmd;
	return true;
}

/**
 * \brief       Add a command to the message's body
 * \details	  char* Version
 * \param    _name         name of the command
 * \param    _value         value of the command
 */
bool xPL_Message::AddCommand(char* _name, char* _value)
{
	if(!CreateCommand()) return false;

	struct_command newcmd;
	memcpy(newcmd.name, _name, XPL_NAME_LENGTH_MAX + 1);
	memcpy(newcmd.value, _value, XPL_VALUE_LENGTH_MAX + 1);
	command[command_count-1] = newcmd;
	return true;
}

/**
 * \brief       Convert xPL_Message to char* buffer
 */
char* xPL_Message::toString()
{
  char message_buffer[XPL_MESSAGE_BUFFER_MAX];
  int pos;

  clearStr(message_buffer);

  switch(type)
  {
    case (XPL_CMND):
      pos = sprintf_P(message_buffer, PSTR("xpl-cmnd"));
      break;
    case (XPL_STAT):
      pos = sprintf_P(message_buffer, PSTR("xpl-stat"));
      break;
    case (XPL_TRIG):
      pos = sprintf_P(message_buffer, PSTR("xpl-trig"));
      break;
  }

  pos += sprintf_P(message_buffer + pos, PSTR("\n{\nhop=1\nsource=%s-%s.%s\ntarget="), source.vendor_id, source.device_id, source.instance_id);

  if(memcmp(target.vendor_id,"*", 1) == 0)  // check if broadcast message
  {
    pos += sprintf_P(message_buffer + pos, PSTR("*\n}\n"));
  }
  else
  {
	pos += sprintf_P(message_buffer + pos, PSTR("%s-%s.%s\n}\n"),target.vendor_id, target.device_id, target.instance_id);
  }

  pos += sprintf_P(message_buffer + pos, PSTR("%s.%s\n{\n"),schema.class_id, schema.type_id);

  for (byte i=0; i<command_count; i++)
  {
	pos += sprintf_P(message_buffer + pos, PSTR("%s=%s\n"), command[i].name, command[i].value);
  }

  sprintf_P(message_buffer + pos, PSTR("}\n"));

  return message_buffer;
}

bool xPL_Message::IsSchema(char* _classId, char* _typeId)
{
  if (strcmp(schema.class_id, _classId) == 0)
  {
    if (strcmp(schema.type_id, _typeId) == 0)
    {
      return true;
    }   
  }  

  return false;
}

/**
 * \brief       Check the message's schema
  * \param   _classId        class
 * \param    _typeId         type
 */
bool xPL_Message::IsSchema_P(const PROGMEM char* _classId, const PROGMEM char* _typeId)
{
	if (strncmp_P(schema.class_id, _classId, XPL_CLASS_ID_MAX) == 0)
	  {
	    if (strncmp_P(schema.type_id, _typeId, XPL_TYPE_ID_MAX) == 0)
	    {
	      return true;
	    }
	  }

	  return false;
}
