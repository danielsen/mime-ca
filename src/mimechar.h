/* mimechar.h - Mime character classifications
 * 
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
 *
 * #include mimechar.h
 */
#if !defined(_MIME_CHAR_H)
#define _MIME_CHAR_H

class cMimeChar {

  public:
    static bool isNonAscii (unsigned char ch);
    static bool isControl (unsigned char ch);
    static bool isSpace (unsigned char ch);
    static bool isPrintable (unsigned char ch);
    static bool isSpecial (unsigned char ch);
    static bool isHexDigit (unsigned char ch);
    static bool isDelimiter (unsigned char ch);
    static bool isToken (unsigned char ch);

  private:
    enum { 
      CONTROL   = 0x01,
      SPACE     = 0x02,
      PRINT     = 0x04,
      PUNCT     = 0x10,
      SPECIAL   = 0x20,
      HEXDIGIT  = 0x40
    };

    static const unsigned char m_aCharTbl[128];
};
#endif // _MIME_CHAR_H
