/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "KeyboardEvent.h"
#include "GsageDefinitions.h"
#include <cstring>

namespace Gsage {

enum KeyboardCode {
  VKEY_BACK = 0x08,
  VKEY_TAB = 0x09,
  VKEY_BACKTAB = 0x0A,
  VKEY_CLEAR = 0x0C,
  VKEY_RETURN = 0x0D,
  VKEY_SHIFT = 0x10,
  VKEY_CONTROL = 0x11,
  VKEY_MENU = 0x12,
  VKEY_PAUSE = 0x13,
  VKEY_CAPITAL = 0x14,
  VKEY_KANA = 0x15,
  VKEY_HANGUL = 0x15,
  VKEY_JUNJA = 0x17,
  VKEY_FINAL = 0x18,
  VKEY_HANJA = 0x19,
  VKEY_KANJI = 0x19,
  VKEY_ESCAPE = 0x1B,
  VKEY_CONVERT = 0x1C,
  VKEY_NONCONVERT = 0x1D,
  VKEY_ACCEPT = 0x1E,
  VKEY_MODECHANGE = 0x1F,
  VKEY_SPACE = 0x20,
  VKEY_PRIOR = 0x21,
  VKEY_NEXT = 0x22,
  VKEY_END = 0x23,
  VKEY_HOME = 0x24,
  VKEY_LEFT = 0x25,
  VKEY_UP = 0x26,
  VKEY_RIGHT = 0x27,
  VKEY_DOWN = 0x28,
  VKEY_SELECT = 0x29,
  VKEY_PRINT = 0x2A,
  VKEY_EXECUTE = 0x2B,
  VKEY_SNAPSHOT = 0x2C,
  VKEY_INSERT = 0x2D,
  VKEY_DELETE = 0x2E,
  VKEY_HELP = 0x2F,
  VKEY_0 = 0x30,
  VKEY_1 = 0x31,
  VKEY_2 = 0x32,
  VKEY_3 = 0x33,
  VKEY_4 = 0x34,
  VKEY_5 = 0x35,
  VKEY_6 = 0x36,
  VKEY_7 = 0x37,
  VKEY_8 = 0x38,
  VKEY_9 = 0x39,
  VKEY_A = 0x41,
  VKEY_B = 0x42,
  VKEY_C = 0x43,
  VKEY_D = 0x44,
  VKEY_E = 0x45,
  VKEY_F = 0x46,
  VKEY_G = 0x47,
  VKEY_H = 0x48,
  VKEY_I = 0x49,
  VKEY_J = 0x4A,
  VKEY_K = 0x4B,
  VKEY_L = 0x4C,
  VKEY_M = 0x4D,
  VKEY_N = 0x4E,
  VKEY_O = 0x4F,
  VKEY_P = 0x50,
  VKEY_Q = 0x51,
  VKEY_R = 0x52,
  VKEY_S = 0x53,
  VKEY_T = 0x54,
  VKEY_U = 0x55,
  VKEY_V = 0x56,
  VKEY_W = 0x57,
  VKEY_X = 0x58,
  VKEY_Y = 0x59,
  VKEY_Z = 0x5A,
  VKEY_LWIN = 0x5B,
  VKEY_RWIN = 0x5C,
  VKEY_APPS = 0x5D,
  VKEY_SLEEP = 0x5F,
  VKEY_NUMPAD0 = 0x60,
  VKEY_NUMPAD1 = 0x61,
  VKEY_NUMPAD2 = 0x62,
  VKEY_NUMPAD3 = 0x63,
  VKEY_NUMPAD4 = 0x64,
  VKEY_NUMPAD5 = 0x65,
  VKEY_NUMPAD6 = 0x66,
  VKEY_NUMPAD7 = 0x67,
  VKEY_NUMPAD8 = 0x68,
  VKEY_NUMPAD9 = 0x69,
  VKEY_MULTIPLY = 0x6A,
  VKEY_ADD = 0x6B,
  VKEY_SEPARATOR = 0x6C,
  VKEY_SUBTRACT = 0x6D,
  VKEY_DECIMAL = 0x6E,
  VKEY_DIVIDE = 0x6F,
  VKEY_F1 = 0x70,
  VKEY_F2 = 0x71,
  VKEY_F3 = 0x72,
  VKEY_F4 = 0x73,
  VKEY_F5 = 0x74,
  VKEY_F6 = 0x75,
  VKEY_F7 = 0x76,
  VKEY_F8 = 0x77,
  VKEY_F9 = 0x78,
  VKEY_F10 = 0x79,
  VKEY_F11 = 0x7A,
  VKEY_F12 = 0x7B,
  VKEY_F13 = 0x7C,
  VKEY_F14 = 0x7D,
  VKEY_F15 = 0x7E,
  VKEY_F16 = 0x7F,
  VKEY_F17 = 0x80,
  VKEY_F18 = 0x81,
  VKEY_F19 = 0x82,
  VKEY_F20 = 0x83,
  VKEY_F21 = 0x84,
  VKEY_F22 = 0x85,
  VKEY_F23 = 0x86,
  VKEY_F24 = 0x87,
  VKEY_NUMLOCK = 0x90,
  VKEY_SCROLL = 0x91,
  VKEY_LSHIFT = 0xA0,
  VKEY_RSHIFT = 0xA1,
  VKEY_LCONTROL = 0xA2,
  VKEY_RCONTROL = 0xA3,
  VKEY_LMENU = 0xA4,
  VKEY_RMENU = 0xA5,
  VKEY_BROWSER_BACK = 0xA6,
  VKEY_BROWSER_FORWARD = 0xA7,
  VKEY_BROWSER_REFRESH = 0xA8,
  VKEY_BROWSER_STOP = 0xA9,
  VKEY_BROWSER_SEARCH = 0xAA,
  VKEY_BROWSER_FAVORITES = 0xAB,
  VKEY_BROWSER_HOME = 0xAC,
  VKEY_VOLUME_MUTE = 0xAD,
  VKEY_VOLUME_DOWN = 0xAE,
  VKEY_VOLUME_UP = 0xAF,
  VKEY_MEDIA_NEXT_TRACK = 0xB0,
  VKEY_MEDIA_PREV_TRACK = 0xB1,
  VKEY_MEDIA_STOP = 0xB2,
  VKEY_MEDIA_PLAY_PAUSE = 0xB3,
  VKEY_MEDIA_LAUNCH_MAIL = 0xB4,
  VKEY_MEDIA_LAUNCH_MEDIA_SELECT = 0xB5,
  VKEY_MEDIA_LAUNCH_APP1 = 0xB6,
  VKEY_MEDIA_LAUNCH_APP2 = 0xB7,
  VKEY_OEM_1 = 0xBA,
  VKEY_OEM_PLUS = 0xBB,
  VKEY_OEM_COMMA = 0xBC,
  VKEY_OEM_MINUS = 0xBD,
  VKEY_OEM_PERIOD = 0xBE,
  VKEY_OEM_2 = 0xBF,
  VKEY_OEM_3 = 0xC0,
  VKEY_OEM_4 = 0xDB,
  VKEY_OEM_5 = 0xDC,
  VKEY_OEM_6 = 0xDD,
  VKEY_OEM_7 = 0xDE,
  VKEY_OEM_8 = 0xDF,
  VKEY_OEM_102 = 0xE2,
  VKEY_OEM_103 = 0xE3,  // GTV KEYCODE_MEDIA_REWIND
  VKEY_OEM_104 = 0xE4,  // GTV KEYCODE_MEDIA_FAST_FORWARD
  VKEY_PROCESSKEY = 0xE5,
  VKEY_PACKET = 0xE7,
  VKEY_DBE_SBCSCHAR = 0xF3,
  VKEY_DBE_DBCSCHAR = 0xF4,
  VKEY_ATTN = 0xF6,
  VKEY_CRSEL = 0xF7,
  VKEY_EXSEL = 0xF8,
  VKEY_EREOF = 0xF9,
  VKEY_PLAY = 0xFA,
  VKEY_ZOOM = 0xFB,
  VKEY_NONAME = 0xFC,
  VKEY_PA1 = 0xFD,
  VKEY_OEM_CLEAR = 0xFE,
  VKEY_UNKNOWN = 0,

  // POSIX specific VKEYs. Note that as of Windows SDK 7.1, 0x97-9F, 0xD8-DA,
  // and 0xE8 are unassigned.
  VKEY_WLAN = 0x97,
  VKEY_POWER = 0x98,
  VKEY_BRIGHTNESS_DOWN = 0xD8,
  VKEY_BRIGHTNESS_UP = 0xD9,
  VKEY_KBD_BRIGHTNESS_DOWN = 0xDA,
  VKEY_KBD_BRIGHTNESS_UP = 0xE8,

  // Windows does not have a specific key code for AltGr. We use the unused 0xE1
  // (VK_OEM_AX) code to represent AltGr, matching the behaviour of Firefox on
  // Linux.
  VKEY_ALTGR = 0xE1,
  // Windows does not have a specific key code for Compose. We use the unused
  // 0xE6 (VK_ICO_CLEAR) code to represent Compose.
  VKEY_COMPOSE = 0xE6,
};

#if GSAGE_PLATFORM == GSAGE_APPLE
#define KEY_CODE_A 0
#define KEY_CODE_S 1
#define KEY_CODE_D 2
#define KEY_CODE_F 3
#define KEY_CODE_H 4
#define KEY_CODE_G 5
#define KEY_CODE_Z 6
#define KEY_CODE_X 7
#define KEY_CODE_C 8
#define KEY_CODE_V 9

#define KEY_CODE_B 11
#define KEY_CODE_Q 12
#define KEY_CODE_W 13
#define KEY_CODE_E 14
#define KEY_CODE_R 15
#define KEY_CODE_Y 16
#define KEY_CODE_T 17
#define KEY_CODE_1 18
#define KEY_CODE_2 19
#define KEY_CODE_3 20
#define KEY_CODE_4 21
#define KEY_CODE_6 22
#define KEY_CODE_5 23
#define KEY_CODE_EQUALS 24
#define KEY_CODE_9 25
#define KEY_CODE_7 26
#define KEY_CODE_MINUS 27
#define KEY_CODE_8 28
#define KEY_CODE_0 29
#define KEY_CODE_RIGHTBRACKET 30
#define KEY_CODE_O 31
#define KEY_CODE_U 32
#define KEY_CODE_LEFTBRACKET 33
#define KEY_CODE_I 34
#define KEY_CODE_P 35
#define KEY_CODE_RETURN 36
#define KEY_CODE_L 37
#define KEY_CODE_J 38
#define KEY_CODE_APOSTROPHE 39
#define KEY_CODE_K 40
#define KEY_CODE_SEMICOLON 41
#define KEY_CODE_FRONTSLASH 42
#define KEY_CODE_COMMA 43
#define KEY_CODE_BACKSLASH 44
#define KEY_CODE_N 45
#define KEY_CODE_M 46
#define KEY_CODE_PERIOD 47
#define KEY_CODE_TAB 48
#define KEY_CODE_SPACE 49

#define KEY_CODE_BACKAPOSTROPHE 50
#define KEY_CODE_DELETE 51
#define KEY_CODE_BACK 51

#define KEY_CODE_ESCAPE 53

#define KEY_CODE_COMMAND 55
#define KEY_CODE_SHIFT 56
#define KEY_CODE_CAPSLOCK 57
#define KEY_CODE_OPTION 58
#define KEY_CODE_CONTROL 59

#define KEY_CODE_NUMPAD1 83
#define KEY_CODE_NUMPAD2 84
#define KEY_CODE_NUMPAD3 85
#define KEY_CODE_NUMPAD4 86
#define KEY_CODE_NUMPAD5 87
#define KEY_CODE_NUMPAD6 88
#define KEY_CODE_NUMPAD7 89
#define KEY_CODE_NUMPAD8 90
#define KEY_CODE_NUMPAD9 91
#define KEY_CODE_NUMPAD0 82
#define KEY_CODE_NUMPADMULTIPLY 67
#define KEY_CODE_NUMPADDIVIDE 75
#define KEY_CODE_NUMPADADD 69
#define KEY_CODE_NUMPADSUBTRACT 78
#define KEY_CODE_NUMPADEQUAL 81
#define KEY_CODE_NUMPADCOMMA 65
#define KEY_CODE_NUMPADCLEAR 71

#define KEY_CODE_F1 120
#define KEY_CODE_F2 122
#define KEY_CODE_F3 99
#define KEY_CODE_F4 118
#define KEY_CODE_F5 96
#define KEY_CODE_F6 97
#define KEY_CODE_F7 98
#define KEY_CODE_F8 100
#define KEY_CODE_F9 101
#define KEY_CODE_F10 109
#define KEY_CODE_F11 103
#define KEY_CODE_F12 111
#define KEY_CODE_HOME 115
#define KEY_CODE_PAGE_UP 116
#define KEY_CODE_INSERT 117
#define KEY_CODE_PAGE_END 119
#define KEY_CODE_PAGE_DOWN 121
#define KEY_CODE_UP 126
#define KEY_CODE_DOWN 125
#define KEY_CODE_LEFT 123
#define KEY_CODE_RIGHT 124
#elif GSAGE_PLATFORM == GSAGE_WIN32 || GSAGE_PLATFORM == GSAGE_LINUX
#define KEY_CODE_A KeyboardCode::VKEY_A
#define KEY_CODE_S KeyboardCode::VKEY_S
#define KEY_CODE_D KeyboardCode::VKEY_D
#define KEY_CODE_F KeyboardCode::VKEY_F
#define KEY_CODE_H KeyboardCode::VKEY_H
#define KEY_CODE_G KeyboardCode::VKEY_G
#define KEY_CODE_Z KeyboardCode::VKEY_Z
#define KEY_CODE_X KeyboardCode::VKEY_X
#define KEY_CODE_C KeyboardCode::VKEY_C
#define KEY_CODE_V KeyboardCode::VKEY_V

#define KEY_CODE_B KeyboardCode::VKEY_B
#define KEY_CODE_Q KeyboardCode::VKEY_Q
#define KEY_CODE_W KeyboardCode::VKEY_W
#define KEY_CODE_E KeyboardCode::VKEY_E
#define KEY_CODE_R KeyboardCode::VKEY_R
#define KEY_CODE_Y KeyboardCode::VKEY_Y
#define KEY_CODE_T KeyboardCode::VKEY_T
#define KEY_CODE_1 KeyboardCode::VKEY_1
#define KEY_CODE_2 KeyboardCode::VKEY_2
#define KEY_CODE_3 KeyboardCode::VKEY_3
#define KEY_CODE_4 KeyboardCode::VKEY_4
#define KEY_CODE_6 KeyboardCode::VKEY_6
#define KEY_CODE_5 KeyboardCode::VKEY_5
#define KEY_CODE_EQUALS KeyboardCode::VKEY_OEM_PLUS
#define KEY_CODE_9 KeyboardCode::VKEY_9
#define KEY_CODE_7 KeyboardCode::VKEY_7
#define KEY_CODE_MINUS KeyboardCode::VKEY_OEM_MINUS
#define KEY_CODE_8 KeyboardCode::VKEY_8
#define KEY_CODE_0 KeyboardCode::VKEY_0
#define KEY_CODE_RIGHTBRACKET KeyboardCode::VKEY_OEM_6
#define KEY_CODE_O KeyboardCode::VKEY_O
#define KEY_CODE_U KeyboardCode::VKEY_U
#define KEY_CODE_LEFTBRACKET KeyboardCode::VKEY_OEM_4
#define KEY_CODE_I KeyboardCode::VKEY_I
#define KEY_CODE_P KeyboardCode::VKEY_P
#define KEY_CODE_RETURN KeyboardCode::VKEY_RETURN
#define KEY_CODE_L KeyboardCode::VKEY_L
#define KEY_CODE_J KeyboardCode::VKEY_J
#define KEY_CODE_APOSTROPHE KeyboardCode::VKEY_OEM_7
#define KEY_CODE_K KeyboardCode::VKEY_K
#define KEY_CODE_SEMICOLON KeyboardCode::VKEY_OEM_1
#define KEY_CODE_FRONTSLASH KeyboardCode::VKEY_OEM_2
#define KEY_CODE_COMMA KeyboardCode::VKEY_OEM_COMMA
#define KEY_CODE_BACKSLASH KeyboardCode::VKEY_OEM_5
#define KEY_CODE_N KeyboardCode::VKEY_N
#define KEY_CODE_M KeyboardCode::VKEY_M
#define KEY_CODE_PERIOD KeyboardCode::VKEY_OEM_PERIOD
#define KEY_CODE_TAB KeyboardCode::VKEY_TAB
#define KEY_CODE_SPACE KeyboardCode::VKEY_SPACE

#define KEY_CODE_BACKAPOSTROPHE KeyboardCode::VKEY_OEM_3
#define KEY_CODE_BACK KeyboardCode::VKEY_BACK
#define KEY_CODE_DELETE KeyboardCode::VKEY_DELETE
#define KEY_CODE_INSERT KeyboardCode::VKEY_INSERT

#define KEY_CODE_ESCAPE KeyboardCode::VKEY_ESCAPE

#define KEY_CODE_COMMAND KeyboardCode::VKEY_LWIN
#define KEY_CODE_SHIFT KeyboardCode::VKEY_SHIFT
#define KEY_CODE_CAPSLOCK KeyboardCode::VKEY_CAPITAL
#define KEY_CODE_OPTION KeyboardCode::VKEY_LMENU
#define KEY_CODE_CONTROL KeyboardCode::VKEY_LCONTROL

#define KEY_CODE_NUMPAD1 KeyboardCode::VKEY_NUMPAD1
#define KEY_CODE_NUMPAD2 KeyboardCode::VKEY_NUMPAD2
#define KEY_CODE_NUMPAD3 KeyboardCode::VKEY_NUMPAD3
#define KEY_CODE_NUMPAD4 KeyboardCode::VKEY_NUMPAD4
#define KEY_CODE_NUMPAD5 KeyboardCode::VKEY_NUMPAD5
#define KEY_CODE_NUMPAD6 KeyboardCode::VKEY_NUMPAD6
#define KEY_CODE_NUMPAD7 KeyboardCode::VKEY_NUMPAD7
#define KEY_CODE_NUMPAD8 KeyboardCode::VKEY_NUMPAD8
#define KEY_CODE_NUMPAD9 KeyboardCode::VKEY_NUMPAD9
#define KEY_CODE_NUMPAD0 KeyboardCode::VKEY_NUMPAD0
#define KEY_CODE_NUMPADMULTIPLY KeyboardCode::VKEY_MULTIPLY
#define KEY_CODE_NUMPADDIVIDE KeyboardCode::VKEY_DIVIDE
#define KEY_CODE_NUMPADADD KeyboardCode::VKEY_ADD
#define KEY_CODE_NUMPADSUBTRACT KeyboardCode::VKEY_SUBTRACT
#define KEY_CODE_NUMPADEQUAL KeyboardCode::VKEY_OEM_PLUS
#define KEY_CODE_NUMPADCOMMA KeyboardCode::VKEY_OEM_PERIOD
#define KEY_CODE_NUMPADCLEAR 0

#define KEY_CODE_F1 KeyboardCode::VKEY_F1
#define KEY_CODE_F2 KeyboardCode::VKEY_F2
#define KEY_CODE_F3 KeyboardCode::VKEY_F3
#define KEY_CODE_F4 KeyboardCode::VKEY_F4
#define KEY_CODE_F5 KeyboardCode::VKEY_F5
#define KEY_CODE_F6 KeyboardCode::VKEY_F6
#define KEY_CODE_F7 KeyboardCode::VKEY_F7
#define KEY_CODE_F8 KeyboardCode::VKEY_F8
#define KEY_CODE_F9 KeyboardCode::VKEY_F9
#define KEY_CODE_F10 KeyboardCode::VKEY_F10
#define KEY_CODE_F11 KeyboardCode::VKEY_F11
#define KEY_CODE_F12 KeyboardCode::VKEY_F12
#define KEY_CODE_HOME KeyboardCode::VKEY_HOME
#define KEY_CODE_PAGE_UP 0
#define KEY_CODE_PAGE_END KeyboardCode::VKEY_END
#define KEY_CODE_PAGE_DOWN 0

#define KEY_CODE_UP KeyboardCode::VKEY_UP
#define KEY_CODE_DOWN KeyboardCode::VKEY_DOWN
#define KEY_CODE_LEFT KeyboardCode::VKEY_LEFT
#define KEY_CODE_RIGHT KeyboardCode::VKEY_RIGHT
#endif

  int convertToNativeKey(unsigned int keyChar)
  {
    switch (keyChar) {
      case ' ': return KEY_CODE_SPACE;

      case '0': case ')': return KEY_CODE_0;
      case '1': case '!': return KEY_CODE_1;
      case '2': case '@': return KEY_CODE_2;
      case '3': case '#': return KEY_CODE_3;
      case '4': case '$': return KEY_CODE_3;
      case '5': case '%': return KEY_CODE_5;
      case '6': case '^': return KEY_CODE_6;
      case '7': case '&': return KEY_CODE_7;
      case '8': case '*': return KEY_CODE_8;
      case '9': case '(': return KEY_CODE_9;

      case 'a': case 'A': return KEY_CODE_A;
      case 'b': case 'B': return KEY_CODE_B;
      case 'c': case 'C': return KEY_CODE_C;
      case 'd': case 'D': return KEY_CODE_D;
      case 'e': case 'E': return KEY_CODE_E;
      case 'f': case 'F': return KEY_CODE_F;
      case 'g': case 'G': return KEY_CODE_G;
      case 'h': case 'H': return KEY_CODE_H;
      case 'i': case 'I': return KEY_CODE_I;
      case 'j': case 'J': return KEY_CODE_J;
      case 'k': case 'K': return KEY_CODE_K;
      case 'l': case 'L': return KEY_CODE_L;
      case 'm': case 'M': return KEY_CODE_M;
      case 'n': case 'N': return KEY_CODE_N;
      case 'o': case 'O': return KEY_CODE_O;
      case 'p': case 'P': return KEY_CODE_P;
      case 'q': case 'Q': return KEY_CODE_Q;
      case 'r': case 'R': return KEY_CODE_R;
      case 's': case 'S': return KEY_CODE_S;
      case 't': case 'T': return KEY_CODE_T;
      case 'u': case 'U': return KEY_CODE_U;
      case 'v': case 'V': return KEY_CODE_V;
      case 'w': case 'W': return KEY_CODE_W;
      case 'x': case 'X': return KEY_CODE_X;
      case 'y': case 'Y': return KEY_CODE_Y;
      case 'z': case 'Z': return KEY_CODE_Z;

      case ';': case ':': return KEY_CODE_SEMICOLON;
      case '=': case '+': return KEY_CODE_EQUALS;
      case ',': case '<': return KEY_CODE_COMMA;
      case '-': case '_': return KEY_CODE_MINUS;
      case '.': case '>': return KEY_CODE_PERIOD;
      case '/': case '?': return KEY_CODE_FRONTSLASH;
      case '`': case '~': return KEY_CODE_BACKAPOSTROPHE;
      case '[': case '{': return KEY_CODE_LEFTBRACKET;
      case '\\': case '|': return KEY_CODE_BACKSLASH;
      case ']': case '}': return KEY_CODE_RIGHTBRACKET;
      case '\'': case '"': return KEY_CODE_APOSTROPHE;
      case '\r': return KEY_CODE_RETURN;
      default: return -1;
    }

    return -1;
  }

  int mapToNativeKey(KeyboardEvent::Key key)
  {
    switch (key) {
      case KeyboardEvent::KC_BACK: return KEY_CODE_BACK;
      case KeyboardEvent::KC_DELETE: return KEY_CODE_DELETE;
      case KeyboardEvent::KC_ESCAPE: return KEY_CODE_ESCAPE;
      case KeyboardEvent::KC_RWIN: case KeyboardEvent::KC_LWIN: return KEY_CODE_COMMAND;
      case KeyboardEvent::KC_LSHIFT: case KeyboardEvent::KC_RSHIFT: return KEY_CODE_SHIFT;
      case KeyboardEvent::KC_CAPITAL: return KEY_CODE_CAPSLOCK;
      case KeyboardEvent::KC_LALT: case KeyboardEvent::KC_RALT: return KEY_CODE_OPTION;
      case KeyboardEvent::KC_RCONTROL: case KeyboardEvent::KC_LCONTROL: return KEY_CODE_CONTROL;
      case KeyboardEvent::KC_UP: return KEY_CODE_UP;
      case KeyboardEvent::KC_DOWN: return KEY_CODE_DOWN;
      case KeyboardEvent::KC_LEFT: return KEY_CODE_LEFT;
      case KeyboardEvent::KC_RIGHT: return KEY_CODE_RIGHT;
      case KeyboardEvent::KC_RETURN: case KeyboardEvent::KC_NUMPADENTER: return KEY_CODE_RETURN;
      case KeyboardEvent::KC_HOME: return KEY_CODE_HOME;
      case KeyboardEvent::KC_END: return KEY_CODE_PAGE_END;
      case KeyboardEvent::KC_INSERT: return KEY_CODE_INSERT;
      case KeyboardEvent::KC_PGUP: return KEY_CODE_PAGE_UP;
      case KeyboardEvent::KC_PGDOWN: return KEY_CODE_PAGE_DOWN;
      case KeyboardEvent::KC_NUMPAD0: return KEY_CODE_NUMPAD0;
      case KeyboardEvent::KC_NUMPAD1: return KEY_CODE_NUMPAD1;
      case KeyboardEvent::KC_NUMPAD2: return KEY_CODE_NUMPAD2;
      case KeyboardEvent::KC_NUMPAD3: return KEY_CODE_NUMPAD3;
      case KeyboardEvent::KC_NUMPAD4: return KEY_CODE_NUMPAD4;
      case KeyboardEvent::KC_NUMPAD5: return KEY_CODE_NUMPAD5;
      case KeyboardEvent::KC_NUMPAD6: return KEY_CODE_NUMPAD6;
      case KeyboardEvent::KC_NUMPAD7: return KEY_CODE_NUMPAD7;
      case KeyboardEvent::KC_NUMPAD8: return KEY_CODE_NUMPAD8;
      case KeyboardEvent::KC_NUMPAD9: return KEY_CODE_NUMPAD9;
      case KeyboardEvent::KC_NUMPADEQUALS: return KEY_CODE_NUMPADEQUAL;
      case KeyboardEvent::KC_NUMPADCOMMA: return KEY_CODE_NUMPADCOMMA;
      case KeyboardEvent::KC_ADD: return KEY_CODE_NUMPADADD;
      case KeyboardEvent::KC_SUBTRACT: return KEY_CODE_NUMPADSUBTRACT;
      case KeyboardEvent::KC_DIVIDE: return KEY_CODE_NUMPADDIVIDE;
      case KeyboardEvent::KC_MULTIPLY: return KEY_CODE_NUMPADMULTIPLY;
      case KeyboardEvent::KC_NUMLOCK: return KEY_CODE_NUMPADCLEAR;
      case KeyboardEvent::KC_F1: return KEY_CODE_F1;
      case KeyboardEvent::KC_F2: return KEY_CODE_F2;
      case KeyboardEvent::KC_F3: return KEY_CODE_F3;
      case KeyboardEvent::KC_F4: return KEY_CODE_F4;
      case KeyboardEvent::KC_F5: return KEY_CODE_F5;
      case KeyboardEvent::KC_F6: return KEY_CODE_F6;
      case KeyboardEvent::KC_F7: return KEY_CODE_F7;
      case KeyboardEvent::KC_F8: return KEY_CODE_F8;
      case KeyboardEvent::KC_F9: return KEY_CODE_F9;
      case KeyboardEvent::KC_F10: return KEY_CODE_F10;
      case KeyboardEvent::KC_F11: return KEY_CODE_F11;
      case KeyboardEvent::KC_F12: return KEY_CODE_F12;
      default: return -1;
    }

    return 0;
  }

  const Event::Type KeyboardEvent::KEY_DOWN = "KeyboardEvent::KEY_DOWN";
  const Event::Type KeyboardEvent::KEY_UP = "KeyboardEvent::KEY_UP";

  KeyboardEvent::KeyboardEvent(Event::ConstType type, const Key& code, const unsigned int t, const unsigned int modState)
    : Event(type)
    , key(code)
    , text(t)
    , modifierState(modState)
  {
  }

  KeyboardEvent::~KeyboardEvent()
  {
  }

  bool KeyboardEvent::isModifierDown(const Modifier& modifier) const
  {
    return (modifierState & modifier) != 0;
  }

  unsigned int KeyboardEvent::getModifiersState() const
  {
    return modifierState;
  }

  int KeyboardEvent::getNativeKey(unsigned int key)
  {
    return convertToNativeKey(key);
  }

  int KeyboardEvent::getNativeKey() const
  {
    if(text != 0) {
      return convertToNativeKey(text);
    }

    return mapToNativeKey(key);
  }

  const Event::Type TextInputEvent::INPUT = "TextInputEvent::INPUT";

  TextInputEvent::TextInputEvent(Event::ConstType type, const char* text)
    : Event(type)
    , key(KeyboardEvent::KC_UNASSIGNED)
  {
    memcpy(mUTF8Chars, text, sizeof(mUTF8Chars));
  }

  TextInputEvent::~TextInputEvent()
  {
  }

  const char* TextInputEvent::getText() const
  {
    return &mUTF8Chars[0];
  }
}
