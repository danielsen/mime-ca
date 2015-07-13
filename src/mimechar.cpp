/*
 * Copyright (C) 2015 Dan Nielsen <dnielsen@fastmail.fm>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "mimechar.h"

const unsigned char cMimeChar::m_aCharTbl[128] = {
  CONTROL,                // 00 (NUL)
  CONTROL,                // 01 (SOH)
  CONTROL,                // 02 (STX)
  CONTROL,                // 03 (ETX)
  CONTROL,                // 04 (EOT)
  CONTROL,                // 05 (ENQ)
  CONTROL,                // 06 (ACK)
  CONTROL,                // 07 (BEL)
  CONTROL,                // 08 (BS)
  SPACE | CONTROL,        // 09 (HT)
  SPACE | CONTROL,        // 0A (LF)
  SPACE | CONTROL,        // 0B (VT)
  SPACE | CONTROL,        // 0C (FF)
  SPACE | CONTROL,        // 0D (CR)
  CONTROL,                // 0E (SI)
  CONTROL,                // 0F (SO)
  CONTROL,                // 10 (DLE)
  CONTROL,                // 11 (DC1)
  CONTROL,                // 12 (DC2)
  CONTROL,                // 13 (DC3)
  CONTROL,                // 14 (DC4)
  CONTROL,                // 15 (NAK)
  CONTROL,                // 16 (SYN)
  CONTROL,                // 17 (ETB)
  CONTROL,                // 18 (CAN)
  CONTROL,                // 19 (EM)
  CONTROL,                // 1A (SUB)
  CONTROL,                // 1B (ESC)
  CONTROL,                // 1C (FS)
  CONTROL,                // 1D (GS)
  CONTROL,                // 1E (RS)
  CONTROL,                // 1F (US)
  SPACE,                  // 20 SPACE
  PUNCT,                  // 21 !
  PUNCT | SPECIAL,        // 22 "
  PUNCT,                  // 23 #
  PUNCT,                  // 24 $
  PUNCT,                  // 25 %
  PUNCT,                  // 26 &
  PUNCT,                  // 27 '
  PUNCT | SPECIAL,        // 28 (
 PUNCT | SPECIAL,         // 29 )
  PUNCT,                  // 2A *
  PUNCT,                  // 2B +
  PUNCT | SPECIAL,        // 2C ,
  PUNCT,                  // 2D -
  PUNCT | SPECIAL,        // 2E .
  PUNCT,                  // 2F /
  PRINT | HEXDIGIT,       // 30 0
  PRINT | HEXDIGIT,       // 31 1
  PRINT | HEXDIGIT,       // 32 2
  PRINT | HEXDIGIT,       // 33 3
  PRINT | HEXDIGIT,       // 34 4
  PRINT | HEXDIGIT,       // 35 5
  PRINT | HEXDIGIT,       // 36 6
  PRINT | HEXDIGIT,       // 37 7
  PRINT | HEXDIGIT,       // 38 8
  PRINT | HEXDIGIT,       // 39 9
  PUNCT | SPECIAL,        // 3A :
  PUNCT | SPECIAL,        // 3B ;
  PUNCT | SPECIAL,        // 3C <
  PUNCT | SPECIAL,        // 3D =
  PUNCT | SPECIAL,        // 3E >
  PUNCT,                  // 3F ?
  PUNCT | SPECIAL,        // 40 @
  PRINT | HEXDIGIT,       // 41 A
  PRINT | HEXDIGIT,       // 42 B
  PRINT | HEXDIGIT,       // 43 C
  PRINT | HEXDIGIT,       // 44 D
  PRINT | HEXDIGIT,       // 45 E
  PRINT | HEXDIGIT,       // 46 F
  PRINT,                  // 47 G
  PRINT,                  // 48 H
  PRINT,                  // 49 I
  PRINT,                  // 4A J
  PRINT,                  // 4B K
  PRINT,                  // 4C L
  PRINT,                  // 4D M
  PRINT,                  // 4E N
  PRINT,                  // 4F O
  PRINT,                  // 50 P
  PRINT,                  // 51 Q
  PRINT,                  // 52 R
  PRINT,                  // 53 S
  PRINT,                  // 54 T
  PRINT,                  // 55 U
  PRINT,                  // 56 V
  PRINT,                  // 57 W
  PRINT,                  // 58 X
  PRINT,                  // 59 Y
  PRINT,                  // 5A Z
  PUNCT | SPECIAL,        // 5B [
  PUNCT | SPECIAL,        // 5C '\'
  PUNCT | SPECIAL,        // 5D ]
  PUNCT,                  // 5E ^
  PUNCT,                  // 5F 
  PUNCT,                  // 60 `
  PRINT,                  // 61 a
  PRINT,                  // 62 b
  PRINT,                  // 63 c
  PRINT,                  // 64 d
  PRINT,                  // 65 e
  PRINT,                  // 66 f
  PRINT,                  // 67 g
  PRINT,                  // 68 h
  PRINT,                  // 69 i
  PRINT,                  // 6A j
  PRINT,                  // 6B k
  PRINT,                  // 6C l
  PRINT,                  // 6D m
  PRINT,                  // 6E n
  PRINT,                  // 6F o
  PRINT,                  // 70 p
  PRINT,                  // 71 q
  PRINT,                  // 72 r
  PRINT,                  // 73 s
  PRINT,                  // 74 t
  PRINT,                  // 75 u
  PRINT,                  // 76 v
  PRINT,                  // 77 w
  PRINT,                  // 78 x
  PRINT,                  // 79 y
  PRINT,                  // 7A z
  PUNCT,                  // 7B {
  PUNCT,                  // 7C |
  PUNCT,                  // 7D }
  PUNCT,                  // 7E ~
  CONTROL,                // 7F (DEL)
};

bool cMimeChar::isNonAscii (unsigned char ch) {
  return (ch & 0x80) != 0;
}

bool cMimeChar::isControl (unsigned char ch) {
  return !(ch & 0x80) && (m_aCharTbl[ch] & CONTROL) != 0;
}

bool cMimeChar::isSpace (unsigned char ch) {
  return !(ch & 0x80) && (m_aCharTbl[ch] & SPACE) != 0;
}

bool cMimeChar::isPrintable (unsigned char ch) {
  return !(ch & 0x80) && (m_aCharTbl[ch] & PRINT) != 0;
}

bool cMimeChar::isSpecial (unsigned char ch) {
  return !(ch & 0x80) && (m_aCharTbl[ch] & SPECIAL) != 0;
}

bool cMimeChar::isHexDigit (unsigned char ch) {
  return !(ch & 0x80) && (m_aCharTbl[ch] & HEXDIGIT) != 0;
}

bool cMimeChar::isDelimiter (unsigned char ch) {
  return isSpace(ch) || isSpecial(ch);
}

bool cMimeChar::isToken (unsigned char ch) {
  return isNonAscii(ch) || (ch > ' ' && !isSpecial(ch));
}

