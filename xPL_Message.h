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

#ifndef xPLMessage_h
#define xPLMessage_h
 
#include "Arduino.h"
#include "xPL_utils.h"

#define XPL_CMND 1
#define XPL_STAT 2
#define XPL_TRIG 3

#define XPL_MESSAGE_BUFFER_MAX           256  // going over 256 would mean changing index from byte to int
#define XPL_MESSAGE_COMMAND_MAX          10

class xPL_Message
{
    public:
        short type;			        // 1=cmnd, 2=stat, 3=trig
        short hop;				// Hop count
        
		struct_id source;			// source identification
        struct_id target;			// target identification

        struct_xpl_schema schema;
        struct_command *command;
        byte command_count;

        bool AddCommand_P(const PROGMEM char *,const PROGMEM char *);
		bool AddCommand(char*, char*);
        
        xPL_Message();
        ~xPL_Message();

        char *toString();
        
        bool IsSchema(char*, char*);
        bool IsSchema_P(const PROGMEM char*, const PROGMEM char*);
	
	    void SetSource(char *,char *,char *);  // define my source
		void SetTarget_P(const PROGMEM char *,const PROGMEM char * = NULL,const PROGMEM char * = NULL);
		void SetSchema_P(const PROGMEM char *,const PROGMEM char *);
			
		
	private:
		bool CreateCommand();
};

#endif
