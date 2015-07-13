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
#include "mimecode.h"
#include "mimechar.h"
#include "mime.h"

bool cMimeEnvironment::m_autofolding = false;
std::string cMimeEnvironment::m_charset;
std::list<cMimeEnvironment::CODER_PAIR> cMimeEnvironment::m_listcoders;
std::list<cMimeEnvironment::FIELD_CODER_PAIR> cMimeEnvironment::m_listfieldcoders;
std::list<cMimeEnvironment::MEDIA_TYPE_PAIR> cMimeEnvironment::m_listmediatypes;
cMimeEnvironment cMimeEnvironment::m_globalmanager;

cMimeEnvironment::cMimeEnvironment() {
  REGISTER_MIMECODER("quoted-printable",      cMimeCodeQP);
  REGISTER_MIMECODER("base64",                cMimeCodeBase64);

  // initialize header fields encoding
  REGISTER_FIELDCODER("Subject",              cFieldCodeText);
  REGISTER_FIELDCODER("Comments",             cFieldCodeText);
  REGISTER_FIELDCODER("Content-Description",  cFieldCodeText);

  REGISTER_FIELDCODER("From",                 cFieldCodeAddress);
  REGISTER_FIELDCODER("To",                   cFieldCodeAddress);
  REGISTER_FIELDCODER("Resent-To",            cFieldCodeAddress);
  REGISTER_FIELDCODER("Cc",                   cFieldCodeAddress);
  REGISTER_FIELDCODER("Resent-Cc",            cFieldCodeAddress);
  REGISTER_FIELDCODER("Bcc",                  cFieldCodeAddress);
  REGISTER_FIELDCODER("Resent-Bcc",           cFieldCodeAddress);
  REGISTER_FIELDCODER("Reply-To",             cFieldCodeAddress);
  REGISTER_FIELDCODER("Resent-Reply-To",      cFieldCodeAddress);

  REGISTER_FIELDCODER("Content-Type",         cFieldCodeParameter);
  REGISTER_FIELDCODER("Content-Disposition",  cFieldCodeParameter);
};

bool cMimeEnvironment::autoFolding () {
  return m_autofolding;
}

void cMimeEnvironment::autoFolding (bool b_autofolding) {
  m_autofolding = b_autofolding;
  if (!b_autofolding) {
    DEREGISTER_MIMECODER("7bit");
    DEREGISTER_MIMECODER("8bit");
  } else {
    REGISTER_MIMECODER("7bit", cMimeCode7bit);
    REGISTER_MIMECODER("8bit", cMimeCode7bit);
  }
}

const char* cMimeEnvironment::globalCharset () {
  return m_charset.c_str();
}

void cMimeEnvironment::globalCharset (const char* p_charset) {
  m_charset = p_charset;
}

void cMimeEnvironment::registerCoder (const char* p_codingname, 
    CODER_BUILD p_createobject) {
  ASSERT(p_codingname != NULL);
  std::list<CODER_PAIR>::iterator it = m_listcoders.begin();

  while (it != m_listcoders.end()) {
    std::list<CODER_PAIR>::iterator it2 = it;
    it++;
    if (!strcmp(p_codingname, (*it2).first)) {
      m_listcoders.erase(it2);
    }
  }
  
  if (p_createobject != NULL) {
    CODER_PAIR newPair(p_codingname, p_createobject);
    m_listcoders.push_front(newPair);
  }
}

cMimeCodeBase* cMimeEnvironment::registerCoder (const char* p_codename) {
  if (!p_codename || !strlen(p_codename)) {
    p_codename = "7bit";
  }

  for (std::list<CODER_PAIR>::iterator it=m_listcoders.begin();
      it!=m_listcoders.end(); it++) {
    ASSERT((*it).first != NULL);
    if (!strcmp(p_codename, (*it).first)) {
      CODER_BUILD p_createobject = (*it).second;
      ASSERT(p_createobject != NULL);
      return p_createobject();
    }
  }
  return new cMimeCodeBase;
}

void cMimeEnvironment::registerFieldCoder(const char* p_fieldname,
    FIELD_CODER_BUILD p_createobject) {
 ASSERT(p_fieldname != NULL);
  std::list<FIELD_CODER_PAIR>::iterator it = m_listfieldcoders.begin();
  while (it != m_listfieldcoders.end()) {
    std::list<FIELD_CODER_PAIR>::iterator it2 = it;
    it++;
    if (!strcmp(p_fieldname, (*it2).first))
      m_listfieldcoders.erase(it2);
  }
  if (p_createobject != NULL) {
    FIELD_CODER_PAIR newPair(p_fieldname, p_createobject);
    m_listfieldcoders.push_front(newPair);
  }
}
 
cFieldCodeBase* cMimeEnvironment::registerFieldCoder(const char* p_fieldname) {
  ASSERT(p_fieldname != NULL);
  for (std::list<FIELD_CODER_PAIR>::iterator it=m_listfieldcoders.begin(); 
      it!=m_listfieldcoders.end(); it++) {
    ASSERT((*it).first != NULL);
    if (!strcmp(p_fieldname, (*it).first)) {
      FIELD_CODER_BUILD p_createobject = (*it).second;
      ASSERT(p_createobject != NULL);
      return p_createobject();
    }
  }
  return new cFieldCodeBase;    // default coder for unregistered header fields
}

void cMimeEnvironment::registerMediaType (const char* p_mediatype, 
    BODY_PART_BUILD p_createobject) {
  ASSERT(p_mediatype != NULL);
  std::list<MEDIA_TYPE_PAIR>::iterator it = m_listmediatypes.begin();
  while (it != m_listmediatypes.end()) {
    std::list<MEDIA_TYPE_PAIR>::iterator it2 = it;
    it++;
    if (!strcmp(p_mediatype, (*it2).first))
      m_listmediatypes.erase(it2);
  }
  if (p_createobject != NULL) {
    MEDIA_TYPE_PAIR newPair(p_mediatype, p_createobject);
    m_listmediatypes.push_front(newPair);
  }
}

cMimeBody* cMimeEnvironment::createBodyPart (const char* p_mediatype) {
  if (!p_mediatype || !::strlen(p_mediatype))
    p_mediatype = "text";

  ASSERT(p_mediatype != NULL);
  for (std::list<MEDIA_TYPE_PAIR>::iterator it=m_listmediatypes.begin(); 
    it!=m_listmediatypes.end(); it++) {
    ASSERT((*it).first != NULL);
    if (!strcmp(p_mediatype, (*it).first)) {
      BODY_PART_BUILD p_createobject = (*it).second;
      ASSERT(p_createobject != NULL);
      return p_createobject();
    }
  }
  return new cMimeBody;     // default body part for unregistered media type
}

cMimeCodeBase::cMimeCodeBase() : 
  m_input(NULL),
  m_inputsize(0),
  m_isencoding(false) {}

void cMimeCodeBase::setInput (const char* p_input, int p_inputsize,
    bool p_encoding) {
  m_input = (const unsigned char*)p_input;
  m_inputsize = p_inputsize;
  m_isencoding = p_encoding;
}

int cMimeCodeBase::getOutputLength () const {
  return m_isencoding ? getEncodeLength() : getDecodeLength();
}

int cMimeCodeBase::getOutput (unsigned char* p_output, int p_maxsize) {
  return m_isencoding ? encode(p_output, p_maxsize) :
    decode(p_output, p_maxsize);
}

int cMimeCodeBase::getEncodeLength() const {
  return m_inputsize;
}

int cMimeCodeBase::getDecodeLength() const {
  return m_inputsize;
}

int cMimeCodeBase::encode(unsigned char* p_output,int p_maxsize) const {
  int size = std::min(p_maxsize, m_inputsize);
  memcpy(p_output, m_input, size);
  return size;
}

int cMimeCodeBase::decode(unsigned char* p_output,int p_maxsize) {
  return cMimeCodeBase::encode(p_output, p_maxsize);
}

/* cMimeCode7bit */
int cMimeCode7bit::getEncodeLength() const {
  int size = m_inputsize + m_inputsize / MAX_MIME_LINE_LEN * 4;
  size += 4;
  return size;
}

int cMimeCode7bit::encode(unsigned char* p_output, int p_maxsize) const {
  const unsigned char* p_data = m_input;
  const unsigned char* p_end = m_input + m_inputsize;
  unsigned char* p_outstart = p_output;
  unsigned char* p_outend = p_output + p_maxsize;
  unsigned char* p_space = NULL;
  int linelen = 0;
  while (p_data < p_end) {
    if (p_output >= p_outend)
      break;

    unsigned char ch = *p_data;
    if (ch == '\r' || ch == '\n')
    {
      linelen = -1;
      p_space = NULL;
    }
    else if (linelen > 0 && cMimeChar::isSpace(ch))
      p_space = p_output;

    // fold the line if it's longer than 76
    if (linelen >= MAX_MIME_LINE_LEN && p_space != NULL &&
      p_output+2 <= p_outend)
    {
      int size = (int)(p_output - p_space);
      ::memmove(p_space+2, p_space, size);
      *p_space++ = '\r';
      *p_space = '\n';
      p_space = NULL;
      linelen = size;
      p_output += 2;
    }

    *p_output++ = ch;
    p_data++;
    linelen++;
  }

  return (int)(p_output - p_outstart);
}
// end cMimeCode7bit

// cMimeCodeQP
cMimeCodeQP::cMimeCodeQP() :
  m_quotelinebreak(false) {}

void cMimeCodeQP::quoteLineBreak (bool p_quote) {
  m_quotelinebreak = p_quote;
}

int cMimeCodeQP::getEncodeLength() const {
  int length = m_inputsize;
  const unsigned char* p_data = m_input;
  const unsigned char* p_end = m_input + m_inputsize;
  while (p_data < p_end) {
    if (!cMimeChar::isPrintable(*p_data++)) { length += 2; }
  }
  length += length / (MAX_MIME_LINE_LEN - 2) * 6;
  return length;
}

int cMimeCodeQP::encode (unsigned char* p_output, int p_maxsize) const {
  static const char* s_qptable = "0123456789ABCDEF";
  const unsigned char* p_data = m_input;
  const unsigned char* p_end = m_input + m_inputsize;
  unsigned char* p_outstart = p_output;
  unsigned char* p_outend = p_output + p_maxsize;
  unsigned char* p_space = NULL;
  int linelen = 0;
  while (p_data < p_end) {
    if (p_output >= p_outend) { break; }

    unsigned char ch = *p_data;
    bool b_quote = false, b_copy = false;

    // According to RFC 2045, TAB and SPACE MAY be represented as 
    // ASCII characters, but MUST NOT be so represented at the end of an 
    // encoded line.
    if (ch == '\t' || ch == ' ') {
      if (p_data == p_end-1 || (!m_quotelinebreak && *(p_data+1) == '\r')) {
        // quote the SPACE/TAB
        b_quote = true;    
      } else {
        // copy the SPACE/TAB
        b_copy = true;   
      }
      if (linelen > 0) {
        p_space = (unsigned char*) p_output;
      }
    } else if (!m_quotelinebreak && (ch == '\r' || ch == '\n')) {
      // keep 'hard' line break
      b_copy = true;     
      linelen = -1;
      p_space = NULL;
    } else if (!m_quotelinebreak && ch == '.') {
      if (p_data-m_input >= 2 &&
        *(p_data-2) == '\r' && *(p_data-1) == '\n' &&
        *(p_data+1) == '\r' && *(p_data+2) == '\n') {
        // avoid confusing with SMTP's message end flag
        b_quote = true;    
      } else {
        b_copy = true;
      }
    } else if (ch < 33 || ch > 126 || ch == '=') {
      // quote this character
      b_quote = true;      
    } else {
      // copy this character
      b_copy = true;     
    }

    if (linelen+(b_quote ? 3 : 1) >= MAX_MIME_LINE_LEN 
      && p_output+3 <= p_outend) {
      if (p_space != NULL && p_space < p_output) {
        p_space++;
        int p_size = (int)(p_output - p_space);
        memmove(p_space+3, p_space, p_size);
        linelen = p_size;
      } else {
        p_space = p_output;
        linelen = 0;
      }
      memcpy(p_space, "=\r\n", 3);
      p_output += 3;
      p_space = NULL;
    }

    if (b_quote && p_output+3 <= p_outend) {
      *p_output++ = '=';
      *p_output++ = s_qptable[(ch >> 4) & 0x0f];
      *p_output++ = s_qptable[ch & 0x0f];
      linelen += 3;
    } else if (b_copy) {
      *p_output++ = (char) ch;
      linelen++;
    }

    p_data++;
  }

  return (int)(p_output - p_outstart);
}

int cMimeCodeQP::decode(unsigned char* p_output, int p_maxsize) {
  const unsigned char* p_data = m_input;
  const unsigned char* p_end = m_input + m_inputsize;
  unsigned char* p_outstart = p_output;
  unsigned char* p_outend = p_output + p_maxsize;

  while (p_data < p_end) {
    if (p_output >= p_outend) { break; }

    unsigned char ch = *p_data++;
    if (ch == '=') {
      if (p_data+2 > p_end) {
        // invalid endcoding
        break;        
      }
      ch = *p_data++;
      if (cMimeChar::isHexDigit(ch)) {
        ch -= ch > '9' ? 0x37 : '0';
        *p_output = ch << 4;
        ch = *p_data++;
        ch -= ch > '9' ? 0x37 : '0';
        *p_output++ |= ch & 0x0f;
      } else if (ch == '\r' && *p_data == '\n') {
        // Soft Line Break, eat it
        p_data++;     
      } else {          
        // invalid endcoding, let it go
        *p_output++ = ch;
      }
    } else {
      *p_output++ = ch;
    }
  }

  return (int)(p_output - p_outstart);
}
// end cMimeCodeQP

// cMimeCodeBase64
cMimeCodeBase64::cMimeCodeBase64() : m_addlinebreak(true) {}

int cMimeCodeBase64::getEncodeLength() const {
  int length = (m_inputsize + 2) / 3 * 4;
  if (m_addlinebreak) {
    length += (length / MAX_MIME_LINE_LEN + 1) * 2;
  }
  return length;
}

int cMimeCodeBase64::getDecodeLength() const {
  return m_inputsize * 3 / 4 + 2;
}

void cMimeCodeBase64::addLineBreak(bool add) { m_addlinebreak = add; }

int cMimeCodeBase64::encode (unsigned char* p_output, int p_maxsize) const {
  static const char* s_base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  unsigned char* p_outstart = p_output;
  unsigned char* p_outend = p_output + p_maxsize;
  int n_from, n_linelen = 0;
  unsigned char ch_high4bits = 0;

  for (n_from=0; n_from<m_inputsize; n_from++) {
    if (p_output >= p_outend) { break; }

    unsigned char ch = m_input[n_from];
    switch (n_from % 3) {
      case 0:
        *p_output++ = s_base64Table[ch >> 2];
        ch_high4bits = (ch << 4) & 0x30;
        break;

      case 1:
        *p_output++ = s_base64Table[ch_high4bits | (ch >> 4)];
        ch_high4bits = (ch << 2) & 0x3c;
        break;

      default:
        *p_output++ = s_base64Table[ch_high4bits | (ch >> 6)];
        if (p_output < p_outend) {
          *p_output++ = s_base64Table[ch & 0x3f];
          n_linelen++;
        }
    }

    n_linelen++;
    if (m_addlinebreak && n_linelen >= MAX_MIME_LINE_LEN 
      && p_output+2 <= p_outend) {
      *p_output++ = '\r';
      *p_output++ = '\n';
      n_linelen = 0;
    }
  }
  // 76 = 19 * 4, so the padding wouldn't exceed 76
  if (n_from % 3 != 0 && p_output < p_outend) {
    *p_output++ = s_base64Table[ch_high4bits];
    int n_pad = 4 - (n_from % 3) - 1;
    if (p_output+n_pad <= p_outend) {
      memset(p_output, '=', n_pad);
      p_output += n_pad;
    }
  }
  if (m_addlinebreak && n_linelen != 0 && p_output+2 <= p_outend) {
    *p_output++ = '\r';
    *p_output++ = '\n';
  }
  return (int)(p_output - p_outstart);
}

int cMimeCodeBase64::decode(unsigned char* p_output, int p_maxsize)
{
  const unsigned char* p_data = m_input;
  const unsigned char* p_end = m_input + m_inputsize;
  unsigned char* p_outstart = p_output;
  unsigned char* p_outend = p_output + p_maxsize;

  int n_from = 0;
  unsigned char ch_highbits = 0;

  while (p_data < p_end) {
    if (p_output >= p_outend)
      break;

    unsigned char ch = *p_data++;
    if (ch == '\r' || ch == '\n')
      continue;
    ch = (unsigned char) decodeBase64Char(ch);
    if (ch >= 64)       // invalid encoding, or trailing pad '='
      break;

    switch ((n_from++) % 4) {
      case 0:
        ch_highbits = ch << 2;
        break;

      case 1:
        *p_output++ = ch_highbits | (ch >> 4);
        ch_highbits = ch << 4;
        break;

      case 2:
        *p_output++ = ch_highbits | (ch >> 2);
        ch_highbits = ch << 6;
        break;

      default:
        *p_output++ = ch_highbits | ch;
    }
  }

  return (int)(p_output - p_outstart);
}
// end cMimeCodeBase64

// cMimeEncodedWord
cMimeEncodedWord::cMimeEncodedWord() : m_encoding(0) {}

int cMimeEncodedWord::encoding() const { return m_encoding; }

void cMimeEncodedWord::encoding(int p_encoding, const char* p_charset) {
  m_charset = p_charset;
  m_encoding = p_encoding;
}

const char* cMimeEncodedWord::charset () const { return m_charset.c_str(); }

int cMimeEncodedWord::getEncodeLength() const {
  if (!m_inputsize || m_charset.empty())
    return cMimeCodeBase::getEncodeLength();

  int n_length, n_codelen = (int) m_charset.size() + 7;
  if (tolower(m_encoding) == 'b') {
    cMimeCodeBase64 base64;
    base64.setInput((const char*)m_input, m_inputsize, true);
    base64.addLineBreak(false);
    n_length = base64.getOutputLength();
  } else {
    cMimeCodeQP qp;
    qp.setInput((const char*)m_input, m_inputsize, true);
    qp.quoteLineBreak(false);
    n_length = qp.getOutputLength();
  }

  n_codelen += 4;
  ASSERT(n_codelen < MAX_ENCODEDWORD_LEN);
  return (n_length / (MAX_ENCODEDWORD_LEN - n_codelen) + 1) 
    * n_codelen + n_length;
}

int cMimeEncodedWord::encode (unsigned char* p_output, int p_maxsize) const {
  if (m_charset.empty()) { 
    return cMimeCodeBase::encode(p_output, p_maxsize);
  }

  if (!m_inputsize) {
    return 0;
  }

  if (tolower(m_encoding) == 'b') {
    return base64Encode(p_output, p_maxsize);
  }
  return QPEncode(p_output, p_maxsize);
}

int cMimeEncodedWord::decode (unsigned char* p_output, int p_maxsize) {
  m_charset.clear();
  const char* p_data = (const char*) m_input;
  const char* p_end = p_data + m_inputsize;
  unsigned char* p_outstart = p_output;
  while (p_data < p_end) {
    const char* p_headerend = p_data;
    const char* p_codeend = p_end;
    int n_coding = 0, n_codelen = (int)(p_end - p_data);
    // it might be an encoded-word
    if (p_data[0] == '=' && p_data[1] == '?') {
      p_headerend = strchr(p_data+2, '?');
      if (p_headerend != NULL && p_headerend[2] == '?' 
          && p_headerend+3 < p_end) {
        n_coding = tolower(p_headerend[1]);
        p_headerend += 3;
        p_codeend = strstr(p_headerend, "?=");  // look for the tailer
        if (!p_codeend || p_codeend >= p_end)
          p_codeend = p_end;
        n_codelen = (int)(p_codeend - p_headerend);
        p_codeend += 2;
        if (m_charset.empty()) {
          m_charset.assign(p_data+2, p_headerend-p_data-5);
          m_encoding = n_coding;
        }
      }
    }

    int n_decoded;
    if (n_coding == 'b') {
      cMimeCodeBase64 base64;
      base64.setInput(p_headerend, n_codelen, false);
      n_decoded = base64.getOutput(p_output, p_maxsize);
    } else if (n_coding == 'q') {
      cMimeCodeQP qp;
      qp.setInput(p_headerend, n_codelen, false);
      n_decoded = qp.getOutput(p_output, p_maxsize);
    } else {
      p_codeend = strstr(p_data+1, "=?");  // find the next encoded-word
      if (!p_codeend || p_codeend >= p_end) {
        p_codeend = p_end;
      } else if (p_data > (const char*) m_input) {
        const char* p_space = p_data;
        while (cMimeChar::isSpace((unsigned char)*p_space))
          p_space++;
          // ignore white-space between adjacent encoded words
          if (p_space == p_codeend) 
          p_data = p_codeend;
      }
      n_decoded = std::min((int)(p_codeend - p_data), p_maxsize);
      memcpy(p_output, p_data, n_decoded);
    }

    p_data = p_codeend;
    p_output += n_decoded;
    p_maxsize -= n_decoded;
    if (p_maxsize <= 0)
      break;
  }

  return (int)(p_output - p_outstart);
} 

int cMimeEncodedWord::base64Encode (unsigned char* p_output, 
    int p_maxsize) const {
  int n_charsetlen = (int)m_charset.size();
  // a single encoded-word cannot exceed 75 bytes
  int n_blocksize = MAX_ENCODEDWORD_LEN - n_charsetlen - 7; 
  n_blocksize = n_blocksize / 4 * 3;
  ASSERT(n_blocksize > 0);

  unsigned char* p_outstart = p_output;
  int n_input = 0;
  for (;;) {
    if (p_maxsize < n_charsetlen+7)
      break;
    // encoded-word header
    *p_output++ = '=';      
    *p_output++ = '?';
    memcpy(p_output, m_charset.c_str(), n_charsetlen);
    p_output += n_charsetlen;
    *p_output++ = '?';
    *p_output++ = 'B';
    *p_output++ = '?';

    p_maxsize -= n_charsetlen + 7;
    cMimeCodeBase64 base64;
    base64.setInput((const char*)m_input+n_input, 
      std::min(m_inputsize-n_input, n_blocksize), true);
    base64.addLineBreak(false);
    int n_encoded = base64.getOutput(p_output, p_maxsize);
    p_output += n_encoded;
    // encoded-word tail
    *p_output++ = '?';      
    *p_output++ = '=';

    n_input += n_blocksize;
    p_maxsize -= n_encoded + n_charsetlen + 7;
    if (n_input >= m_inputsize)
      break;
    // add a liner-white-space between adjacent encoded words
    *p_output++ = ' ';      
    p_maxsize--;
  }
  return (int)(p_output - p_outstart);
}

int cMimeEncodedWord::QPEncode (unsigned char* p_output, int p_maxsize) const {
  static const char* s_qptable = "0123456789ABCDEF";
  const unsigned char* p_data = m_input;
  const unsigned char* p_end = m_input + m_inputsize;
  unsigned char* p_outstart = p_output;
  unsigned char* p_outend = p_output + p_maxsize;
  int n_codelen, n_charsetlen = (int)m_charset.size();
  int n_linelen = 0, n_maxline = MAX_ENCODEDWORD_LEN - n_charsetlen - 7;

  while (p_data < p_end) {
    unsigned char ch = *p_data++;
    if (ch < 33 || ch > 126 || ch == '=' || ch == '?' || ch == '_') {
      n_codelen = 3;
    } else {
      n_codelen = 1;
    }

    if (n_linelen+n_codelen > n_maxline) {
      if (p_output+3 > p_outend)
        break;
      *p_output++ = '?';
      *p_output++ = '=';
      *p_output++ = ' ';
      n_linelen = 0;
    }

    if (!n_linelen) {
      if (p_output+n_charsetlen+7 > p_outend)
        break;
      *p_output++ = '=';
      *p_output++ = '?';
      memcpy(p_output, m_charset.c_str(), n_charsetlen);
      p_output += n_charsetlen;
      *p_output++ = '?';
      *p_output++ = 'Q';
      *p_output++ = '?';
    }

    n_linelen += n_codelen;
    if (p_output+n_codelen > p_outend)
      break;
    if (n_codelen > 1) {
      *p_output++ = '=';
      *p_output++ = s_qptable[(ch >> 4) & 0x0f];
      *p_output++ = s_qptable[ch & 0x0f];
    } else {
      *p_output++ = ch;
    }
  }

  if (p_output+2 <= p_outend) {
    *p_output++ = '?';
    *p_output++ = '=';
  }
  return (int)(p_output - p_outstart);
}

// end cMimeEncodedWord

// cFieldCodeBase
const char* cFieldCodeBase::charset () const {
  return m_charset.c_str();
}

void cFieldCodeBase::charset (const char* p_charset) {
  m_charset = p_charset;
}

int cFieldCodeBase::findSymbol (const char* p_data, int p_size, 
  int& p_delimeter, int& p_nonascchars) const {

  p_nonascchars = 0;
  const char* p_datastart = p_data;
  const char* p_end = p_data + p_size;

  while (p_data < p_end) {
    char ch = *p_data;
    if (cMimeChar::isNonAscii((unsigned char)ch)) {
      p_nonascchars++;
    } else {
      if (ch == (char) p_delimeter) {
        p_delimeter = 0;   // stop at any delimeters (space or specials)
        break;
      }

      if (!p_delimeter && cMimeChar::isDelimiter(ch)) {
        switch (ch) {
          case '"':
            p_delimeter = '"'; // quoted-string, delimeter is '"'
            break;
          case '(':
            p_delimeter = ')'; // comment, delimeter is ')'
            break;
          case '<':
            p_delimeter = '>'; // address, delimeter is '>'
            break;
        }
        break;
      }
    }
    p_data++;
  }

  return (int)(p_data - p_datastart);
} 

int cFieldCodeBase::selectEncoding (int p_length, 
  int p_nonasciichars) const {

  int q_encodesize = p_length + p_nonasciichars * 2;
  int b_encodesize = (p_length + 2) / 3 * 4;
  return (q_encodesize <= b_encodesize 
    || p_nonasciichars * 5 <= p_length) ? 'Q' : 'B';
}

void cFieldCodeBase::unfoldField (std::string& p_field) const {
  for (;;) {
    std::string::size_type pos = p_field.rfind("\r\n");
    if (pos == std::string::npos) 
      break;

    p_field.erase(pos, 2);
    int spaces = 0;
    while (cMimeChar::isSpace((unsigned char)p_field[pos+spaces]))
      spaces++;
    p_field.replace(pos, spaces, " ");
  }
}

int cFieldCodeBase::getEncodeLength() const {
  // use the global charset if there's no specified charset
  std::string charset = m_charset;
  if (charset.empty())
    charset = cMimeEnvironment::globalCharset();
  if (charset.empty() && !cMimeEnvironment::autoFolding())
    return cMimeCodeBase::getEncodeLength();

  int n_length = 0;
  const char* p_data = (const char*) m_input;
  int inputsize = m_inputsize;
  int p_nonasciichars, n_delimeter = getDelimeter();

  // divide the field into syntactic units to calculate the output length
  do {
    int n_unitsize = findSymbol(p_data, inputsize, n_delimeter, 
      p_nonasciichars);
    if (!p_nonasciichars || charset.empty()) {
      n_length += n_unitsize;
    } else {
      cMimeEncodedWord coder;
      coder.encoding(selectEncoding(n_unitsize, p_nonasciichars), 
        charset.c_str());
      coder.setInput(p_data, n_unitsize, true);
      n_length += coder.getOutputLength();
    }

    p_data += n_unitsize;
    inputsize -= n_unitsize;
    if (isFoldingChar(*p_data)) 
      // the char follows the unit is a delimeter (space or special char)
      n_length += 3;
    n_length++;
    p_data++;
    inputsize--;
  } while (inputsize > 0);

  if (cMimeEnvironment::autoFolding())
    n_length += n_length / MAX_MIME_LINE_LEN * 6;
  return n_length;
}
