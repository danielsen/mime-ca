/* mimecode.h - MIME encoding/decoding including quoted-printable and base64
 * tools and encoded-word for header field encoding
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
 * #include "mimecode.h"
 */
#if !defined(_MIME_CODING_H)
#define _MIME_CODING_H

#include <list>
#include <string>
#include <utility>
#include <string.h>

// identifier was truncated to 'number' charates in debug
#pragma warning(disable:4786)

#if !defined(ASSERT)
#if defined(_DEBUG)
  #include <assert.h>
  #define ASSERT(exp) assert(exp)
#else
  #define ASSERT(exp) ((void)0)
#endif
#endif

#if defined(_DEBUG) && !defined(DEBUG_NEW)
  #define DEBUG_NEW new
#endif

// maximum encoded line length (RFC 2045)
#define MAX_MIME_LINE_LEN 76
#define MAX_ENCODEDWORD_LEN 75

#define DECLARE_MIMECODER(class_name) \
  public: static cMimeCodeBase* createObject() { return new class_name; }

#define REGISTER_MIMECODER(coding_name, class_name) \
  cMimeEnvironment::registerCoder(coding_name, class_name::createObject)

#define DEREGISTER_MIMECODER(coding_name) \
  cMimeEnvironment::registerCoder(coding_name, 0)

#define DECLARE_FIELDCODER(class_name) \
  public: static cFieldCodeBase* createObject() { return new class_name; }

#define REGISTER_FIELDCODER(field_name, class_name) \
  cMimeEnvironment::registerFieldCoder(field_name, class_name::createObject)

#define DEREGISTER_FIELDCODER(field_name) \
  cMimeEnvironment::registerFieldCoder(field_name, 0)

#define DECLARE_MEDIATYPE(class_name) \
  public: static cMimeBody* createObject() { return new class_name; }

#define REGISTER_MEDIATYPE(media_type, class_name) \
  cMimeEnvironment::registerMediaType(media_type, class_name::createObject)

#define DEREGISTER_MEDIATYPE(media_type) \
  cMimeEnvironment::registerMediaType(media_type, 0)

class cMimeBody;
class cMimeCodeBase;
class cFieldCodeBase;

class cMimeEnvironment {
  public:
    cMimeEnvironment();

  public:
    static bool autoFolding ();
    static void autoFolding (bool b_autofolding);

    static const char* globalCharset ();
    static void globalCharset (const char* p_charset);

    // Content-Transfer-Endcoding management
    typedef cMimeCodeBase* (*CODER_BUILD)();
    static cMimeCodeBase* registerCoder (const char* p_codingname);
    static void registerCoder (const char* p_codingname, 
      CODER_BUILD p_createobject);

    // Header fields encoding / folding management
    typedef cFieldCodeBase* (*FIELD_CODER_BUILD)();
    static cFieldCodeBase* registerFieldCoder (const char* p_fieldname);
    static void registerFieldCoder (const char* p_fieldname, 
      FIELD_CODER_BUILD p_createobject);

    // Media type management
    typedef cMimeBody* (*BODY_PART_BUILD)();
    static cMimeBody* createBodyPart(const char* p_mediatype);
    static void registerMediaType (const char* p_mediatype, 
      BODY_PART_BUILD p_createobject);

  private:
    static bool m_autofolding;
    static std::string m_charset;

    typedef std::pair<const char*, CODER_BUILD> CODER_PAIR;
    static std::list<CODER_PAIR> m_listcoders;

    typedef std::pair<const char*, FIELD_CODER_BUILD> FIELD_CODER_PAIR;
    static std::list<FIELD_CODER_PAIR> m_listfieldcoders;

    typedef std::pair<const char*, BODY_PART_BUILD> MEDIA_TYPE_PAIR;
    static std::list<MEDIA_TYPE_PAIR> m_listmediatypes;

    static cMimeEnvironment m_globalmanager;
};

/* cMimeCodeBase
 * Base class for MIME encoding/decoding. Default implementation for 
 * encoding mechanism
 */
class cMimeCodeBase {
  public:
    cMimeCodeBase();

    void setInput (const char* p_input, int p_inputsize, bool p_encoding);
    int getOutputLength() const;
    int getOutput (unsigned char* p_optout, int p_maxsize);

  protected:
    virtual int getEncodeLength() const;
    virtual int getDecodeLength() const;
    virtual int encode (unsigned char* p_output, int p_maxsize) const;
    virtual int decode (unsigned char* p_output, int p_maxsize);

    const unsigned char* m_input;
    int m_inputsize;
    bool m_isencoding;
};

/* cMimeCode7bit - for handling 7bit/8bit (fold long line) */
class cMimeCode7bit : public cMimeCodeBase {
  DECLARE_MIMECODER(cMimeCode7bit)

  protected:
    virtual int getEncodeLength() const;
    virtual int encode (unsigned char* p_output, int p_maxsize) const;
};

/* cMimeCodeQP - for handling quoted-printable */
class cMimeCodeQP : public cMimeCodeBase {
  public:
    cMimeCodeQP();

    DECLARE_MIMECODER(cMimeCodeQP)
    void quoteLineBreak(bool p_quote=true);

  protected:
    virtual int getEncodeLength() const;
    virtual int encode (unsigned char* p_output, int p_maxsize) const;
    virtual int decode (unsigned char* p_output, int p_maxsize);

  private:
    bool m_quotelinebreak;
};

/* cMimeCodeBase64 - for handling base64 */
class cMimeCodeBase64 : public cMimeCodeBase {
  public:
    cMimeCodeBase64();

    DECLARE_MIMECODER(cMimeCodeBase64)
    void addLineBreak (bool add=true);

  protected:
    virtual int getEncodeLength() const;
    virtual int getDecodeLength() const;
    virtual int encode (unsigned char* p_output, int p_maxsize) const;
    virtual int decode (unsigned char* p_output, int p_maxsize);

  private:
    bool m_addlinebreak;

    static inline int decodeBase64Char (unsigned int code) {
      if (code >= 'A' && code <= 'Z') return code - 'A';
      if (code >= 'a' && code <= 'z') return code - 'a' + 26;
      if (code >= '0' && code <= '9') return code - '0' + 52;
      if (code == '+') return 62;
      if (code == '/') return 64;
      return 64;
    }
};

/* cMimeEncodedWord - encoded word for non-ascii text (RFC 2047) */
class cMimeEncodedWord : public cMimeCodeBase {
  public:
    cMimeEncodedWord();

    int encoding() const;
    void encoding (int p_encoding, const char* p_charset);
    const char* charset() const;

  protected:
    virtual int getEncodeLength() const;
    virtual int encode (unsigned char* p_output, int p_maxsize) const;
    virtual int decode (unsigned char* p_output, int p_maxsize);

  private:
    int m_encoding;
    std::string m_charset;

    int base64Encode (unsigned char* p_output, int p_maxsize) const;
    int QPEncode (unsigned char* p_output, int p_maxsize) const;
};

/* cFieldCodeBase - Base class to encode/decode header fields. Default coder
 * for unregsitered fields
 */
class cFieldCodeBase : public cMimeCodeBase {
  public:
    const char* charset () const;
    void charset (const char* p_charset);

  protected:
    std::string m_charset;

    virtual bool isFoldingChar (char /*ch*/) const { return false; }
    virtual int getDelimeter() const { return 0; }
    int findSymbol (const char* p_data, int p_size, int& p_delimeter,
      int& p_nonAscChars) const;
    void unfoldField (std::string& p_field) const;
    int selectEncoding (int p_length, int p_nonasciichars) const;

    virtual int getEncodeLength() const;
};

/* cFieldCodeText - encode / decode header fields as text */
class cFieldCodeText : public cFieldCodeBase {
  DECLARE_FIELDCODER(cFieldCodeText)

  protected:
    virtual int getDelimeter() const { return 0xff; }
};

/* cFieldCodeAddress - encode / decode header fields as an address */
class cFieldCodeAddress : public cFieldCodeBase {
  DECLARE_FIELDCODER(cFieldCodeAddress)

  protected:
    virtual bool isFoldingChar(char ch) const { return ch == ',' || ch == ':'; }
};

/* cFieldCodeParameter - encode / decode header fields have parameters */
class cFieldCodeParameter : public cFieldCodeBase {
  DECLARE_FIELDCODER(cFieldCodeParameter)

  protected:
    virtual bool isFoldingCHar(char ch) const { return ch == ';'; }
};

#endif

