
#include "stdafx.h"

#include<stdlib.h>
#include<iostream.h>
#include<conio.h>
#include<stdio.h>
#include<winsock2.h>
#include<windows.h>
#include<winuser.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "usb_hid_keys.h"

void mprintf( char* format,... );

// Common-use modifiers
#define CTRL 0x01
#define SHIFT 0x02
#define ALT 0x04
#define GUI 0x08


// Look-up table stored in code memory that we use to convert from ASCII
// characters to HID key codes.
char hidKeyCode[128] =
{
    0,                 // 0x00 Null
    0,                 // 0x01
    0,                 // 0x02
    0,                 // 0x03
    0,                 // 0x04
    0,                 // 0x05
    0,                 // 0x06
    0,                 // 0x07
    KEY_BACKSPACE,     // 0x08 Backspace
    KEY_TAB,           // 0x09 Horizontal Tab
    KEY_ENTER,        // 0x0A Line Feed
    0,                 // 0x0B
    0,                 // 0x0C
    KEY_ENTER,        // 0x0D Carriage return
    0,                 // 0x0E
    0,                 // 0x0F
    0,                 // 0x10
    0,                 // 0x11
    0,                 // 0x12
    0,                 // 0x13
    0,                 // 0x14
    0,                 // 0x15
    0,                 // 0x16
    0,                 // 0x17
    0,                 // 0x18
    0,                 // 0x19
    0,                 // 0x1A
    KEY_ESC,        // 0x1B Escape
    0,                 // 0x1C
    0,                 // 0x1D
    0,                 // 0x1E
    0,                 // 0x1F

    KEY_SPACE,         // 0x20
    KEY_1,             // 0x21 !
    KEY_APOSTROPHE,    // 0x22 "
    KEY_3,             // 0x23 #
    KEY_4,             // 0x24 $
    KEY_5,             // 0x25 %
    KEY_7,             // 0x26 &
    KEY_APOSTROPHE,    // 0x27 '
    KEY_9,             // 0x28 (
    KEY_0,             // 0x29 )
    KEY_8,             // 0x2A *
    KEY_EQUAL,         // 0x2B +
    KEY_COMMA,         // 0x2C ,
    KEY_MINUS,         // 0x2D -
    KEY_DOT,        // 0x2E .
    KEY_SLASH,         // 0x2F /
    KEY_0,             // 0x30 0
    KEY_1,             // 0x31 1
    KEY_2,             // 0x32 2
    KEY_3,             // 0x33 3
    KEY_4,             // 0x34 4
    KEY_5,             // 0x35 5
    KEY_6,             // 0x36 6
    KEY_7,             // 0x37 7
    KEY_8,             // 0x38 8
    KEY_9,             // 0x39 9
    KEY_SEMICOLON,     // 0x3A :
    KEY_SEMICOLON,     // 0x3B ;
    KEY_COMMA,         // 0x3C <
    KEY_EQUAL,         // 0x3D =
    KEY_DOT,        // 0x3E >
    KEY_SLASH,         // 0x3F ?

    KEY_3,             // 0x40 @
    KEY_A,             // 0x41 A
    KEY_B,             // 0x42 B
    KEY_C,             // 0x43 C
    KEY_D,             // 0x44 D
    KEY_E,             // 0x45 E
    KEY_F,             // 0x46 F
    KEY_G,             // 0x47 G
    KEY_H,             // 0x48 H
    KEY_I,             // 0x49 I
    KEY_J,             // 0x4A J
    KEY_K,             // 0x4B K
    KEY_L,             // 0x4C L
    KEY_M,             // 0x4D M
    KEY_N,             // 0x4E N
    KEY_O,             // 0x4F O
    KEY_P,             // 0x50 P
    KEY_Q,             // 0x51 Q
    KEY_R,             // 0x52 R
    KEY_S,             // 0x53 S
    KEY_T,             // 0x55 T
    KEY_U,             // 0x55 U
    KEY_V,             // 0x56 V
    KEY_W,             // 0x57 W
    KEY_X,             // 0x58 X
    KEY_Y,             // 0x59 Y
    KEY_Z,             // 0x5A Z
    KEY_LEFTBRACE,  // 0x5B [
    KEY_BACKSLASH,     // 0x5C '\'
    KEY_RIGHTBRACE, // 0x5D ]
    KEY_6,             // 0x5E ^
    KEY_MINUS,         // 0x5F _

    KEY_GRAVE,         // 0x60 `
    KEY_A,             // 0x61 a
    KEY_B,             // 0x62 b
    KEY_C,             // 0x63 c
    KEY_D,             // 0x66 d
    KEY_E,             // 0x65 e
    KEY_F,             // 0x66 f
    KEY_G,             // 0x67 g
    KEY_H,             // 0x68 h
    KEY_I,             // 0x69 i
    KEY_J,             // 0x6A j
    KEY_K,             // 0x6B k
    KEY_L,             // 0x6C l
    KEY_M,             // 0x6D m
    KEY_N,             // 0x6E n
    KEY_O,             // 0x6F o
    KEY_P,             // 0x70 p
    KEY_Q,             // 0x71 q
    KEY_R,             // 0x72 r
    KEY_S,             // 0x73 s
    KEY_T,             // 0x75 t
    KEY_U,             // 0x75 u
    KEY_V,             // 0x76 v
    KEY_W,             // 0x77 w
    KEY_X,             // 0x78 x
    KEY_Y,             // 0x79 y
    KEY_Z,             // 0x7A z
    KEY_LEFTBRACE,  // 0x7B {
    KEY_BACKSLASH,     // 0x7C |
    KEY_RIGHTBRACE, // 0x7D }
    KEY_GRAVE,         // 0x7E ~
    KEY_DELETE         // 0x7F Delete
};

typedef enum {
		Left_Key		= 0x01,
		Right_Key		= 0x02,
		Mid_Kry			= 0x04,
}MouseKey;
	
typedef enum {
		Left_Control	= 0x01,
		Left_Shift		= 0x02,
		Left_Alt		= 0x04,
		Left_GUI		= 0x08,
		Right_Control	= 0x10,
		Right_Shift		= 0x20,
		Right_Alt		= 0x40,
		Right_GUI		= 0x80,
}FunctionKey;

static int Old_CursorX = 0;
static int Old_CursorY = 0;

int keyboard_fill_report(char report[8], char key)
{
	int key_state;
	
	report[0] = 0;
	report[1] = 0;
	report[2] = 0;
	report[3] = 0;
	report[4] = 0;
	report[5] = 0;
	report[6] = 0;
	report[7] = 0;

	key_state = GetKeyState (VK_CONTROL);
		if (key_state < 0) {
		report[0] = Left_Control;
	}

	key_state = GetKeyState (VK_SHIFT);
		if (key_state < 0) {
		report[0] = Left_Shift;
	}
	
	report[2] = hidKeyCode[key];

	return 0;
}


int mouse_fill_report(char report[8], int x, int y, unsigned int nFlags)
{
	report[0] = 0;
	report[1] = 0;
	report[2] = 0;
	report[3] = 0;
	
	int Differ_X,Differ_Y;
	
	if((!Old_CursorX)&&(!Old_CursorY))
	{
		Old_CursorX = x;		
		Old_CursorY = y;
		return 0;
	}

	if (nFlags & MK_LBUTTON) {
		report[0] |= 0x01;
	}
	
	if (nFlags & MK_RBUTTON) {
		report[0] |= 0x02;
	}
	
	if (nFlags & MK_MBUTTON) {
		report[0] |= 0x04;
	}
					
	Differ_X = x - Old_CursorX;
	Differ_Y = y - Old_CursorY;
	Old_CursorX = x;		
	Old_CursorY = y;
		
	if( Differ_X > 127 ) Differ_X = 127;
	if( Differ_X < -128 ) Differ_X = -128;

	if( Differ_Y > 127 ) Differ_Y = 127;
	if( Differ_Y < -128 ) Differ_Y = -128;
	
	report[1] = Differ_X;
	report[2] = Differ_Y;
	
	return 0;
}
