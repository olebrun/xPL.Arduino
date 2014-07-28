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
 
#ifndef xPLutil_h
#define xPLutil_h

#include "Arduino.h"
#include <string.h>

#define XPL_VENDOR_ID_MAX		8
#define XPL_DEVICE_ID_MAX		8
#define XPL_INSTANCE_ID_MAX		16
#define	XPL_CLASS_ID_MAX		8
#define	XPL_TYPE_ID_MAX			8
#define XPL_NAME_LENGTH_MAX		16
#define XPL_VALUE_LENGTH_MAX	32  // should be 128 but need to spare RAM

#define	XPL_HOP_COUNT_PARSER	PSTR("hop=%d")
#define	XPL_SOURCE_PARSER		PSTR("source=%8[^-]-%8[^'.'].%16s")
#define	XPL_TARGET_PARSER		PSTR("target=%8[^-]-%8[^'.'].%16s")
#define	XPL_SCHEMA_PARSER		PSTR("%8[^'.'].%8s")
#define XPL_COMMAND_PARSER		PSTR("%16[^'=']=%32s")    // 32 shall match XPL_VALUE_LENGTH_MAX

typedef struct struct_id struct_id;
struct struct_id			// source or target
{
    char vendor_id[XPL_VENDOR_ID_MAX+1];		// vendor id
    char device_id[XPL_DEVICE_ID_MAX+1];		// device id
    char instance_id[XPL_INSTANCE_ID_MAX+1];		// instance id
};

typedef struct struct_xpl_schema struct_xpl_schema;
struct struct_xpl_schema
{
    char class_id[XPL_CLASS_ID_MAX+1];	// class of schema (x10, alarm...)
    char type_id[XPL_TYPE_ID_MAX+1];	// type of schema (basic...)
};

typedef struct struct_command struct_command;
struct struct_command			// source or target
{
    char name[XPL_NAME_LENGTH_MAX+1];		// vendor id
    char value[XPL_VALUE_LENGTH_MAX+1];		// device id
};

void clearStr (char* str);

#endif
