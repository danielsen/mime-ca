/* mime.h - MIME message encoding / decoding and composition
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
 * #include "mime.h"
 * 
 * Applied RFCs
 * RFC 822 - Standard for the Format of ARPA Internet Text Messages
 * RFC 1521 - Mechanisms for Specifying and Describing the Format of Internet
 *  Messages
 * RFC 2045 - Format of Internet Message Bodies
 * RFC 2046 - Media Types
 * RFC 2047 - Message Header Extensions for Non-ASCII Text
 * RFC 2049 - Conformance Criteria and Examples
 */
#if !defined(_MIME_H)
#define _MIME_H

#include <list>
#include <string>

class cMimeConst {
  public:
    // Field Names
    static inline const char* constentId() { return "Content-ID"; }
    static inline const char* mimeVersion() { return "MIME-Version"; }
    static inline const char* contentType() { return "Content-Type"; }
    static inline const char* transferEncoding() {
      return "Content-Transfer-Encoding"; }
    static inline const char* contentDescription() {
      return "Content-Description"; }
    static inline const char* contentDisposition() {
      return "Content-Disposition"; }

    // Param names
    static inline const char* name() { return "name"; }
    static inline const char* charset() { return "charset"; }
    static inline const char* filename() { return "filename"; }
    static inline const char* boundary() { return "boundary"; }

    // Param values
    static inline const char* encoding7Bit() { return "7bit"; }
    static inline const char* encoding8Bit() { return "8bit"; }
    static inline const char* encodingBinary() { return "binary"; }
    static inline const char* encodingBase64() { return "base64"; }
    static inline const char* encodingQP() { return "quoted-printable"; }

    static inline const char* mediaText() { return "text"; }
    static inline const char* mediaImage() { return "image"; }
    static inline const char* mediaAudio() { return "audio"; }
    static inline const char* mediaVedio() { return "vedio"; }
    static inline const char* mediaMessage() { return "message"; }
    static inline const char* mediaMultiPart() { return "multipart"; }
    static inline const char* mediaApplication() { return "application"; }
};

/* cMimeField - Abstraction of a field in a MIME body part header */
class cMimeField {
  public:
    cMimeField() {}
    ~cMimeField() {}

    const char* name() const;
    void name(const char* p_name);

    const char* value() const;
    void value (const char* p_value);
    void value (std::string& p_value) const;

    void parameter (const char* p_attr, const char* p_value);
    bool parameter (const char* p_attr, std::string& p_value) const;

    const char* charset () const;
    void charset (const char* p_charset);

    void clear();
    int getLength() const;
    int store (char* p_data, int p_maxsize) const;
    int load (const char* p_data, int p_datasize);

  private:
    std::string m_name;
    std::string m_value;
    std::string m_charset;

    bool findParameter (const char* p_attr, int& p_pos, int& p_size) const;
};

inline const char* cMimeField::name() const {
  return m_name.data();
}

inline void cMimeField::name (const char* p_name) {
  m_name = p_name;
}

inline const char* cMimeField::value () const {
  return m_value.data();
}

inline void cMimeField::value (const char* p_value) {
  m_value = p_value;
}

inline const char* cMimeField::charset () const {
  return m_charset.data();
}

inline void cMimeField::charset (const char* p_charset) {
  m_charset = p_charset;
}

inline void cMimeField::clear() {
  m_name.clear();
  m_value.clear();
  m_charset.clear();
}

/* cMimeHeader - Abstracts MIME body part headers */
class cMimeHeader {
  public:
    cMimeHeader() {}
    virtual ~cMimeHeader() { clear(); }

  public:
    enum media {
      MEDIA_TEXT, MEDIA_MESSAGE, MEDIA_AUDIO, MEDIA_VIDEO, MEDIA_APPLICATION,
      MEDIA_MULTIPART, MEDIA_UNKNOWN, MEDIA_IMAGE
    };

    media mediaType() const;

    void field (const cMimeField& field);
    cMimeField* field (const char* p_fieldname);
    const cMimeField* field (const char* p_fieldname) const;

    void fieldValue (const char* p_fieldname, const char* p_fieldvalue,
      const char* p_charset=NULL);
    const char* fieldValue (const char* p_fieldname) const;
    
    bool parameter (const char* p_fieldname, const char* p_attr,
      const char* p_value);
    std::string parameter (const char* p_fieldname, const char* p_attr) const;

    const char* fieldCharset (const char* p_fieldname) const;
    void fieldCharset (const char* p_fieldname, const char* p_charset);

    // Helper functions for standard fields
    const char* contentType () const;
    void contentType (const char* p_value, const char* p_charset=NULL);

    std::string subType() const;
    std::string mainType() const;

    std::string charset() const;
    void charset (const char* p_charset);

    std::string name () const;
    void name (const char* p_name);

    std::string boundary() const;
    void boundary (const char* p_boundary=NULL);
    std::string getBoundary() const;

    const char* transferEncoding() const;
    void transferEncoding (const char* p_value);

    const char* disposition() const;
    void disposition(const char* p_value, const char* p_charset=NULL);

    std::string filename() const;

    const char* description() const;
    void description (const char* p_value, const char* p_charset=NULL);

    typedef std::list<cMimeField> cFieldList;
    cFieldList& fields() { return m_listfields; }

    // Overrides
    virtual void clear();
    virtual int getLength() const;
    // Serialization 
    virtual int store (char* p_data, int p_maxsize) const;
    virtual int load (const char* p_data, int p_datasize);

  protected:
    std::list<cMimeField> m_listfields;
    std::list<cMimeField>::const_iterator 
      findField(const char* p_fieldname) const;
    std::list<cMimeField>::iterator findField(const char* p_fieldname);

    struct mediaTypeCVT {
      int media_type;
      const char* sub_type;
      const char* file_ext;
    };
    static const mediaTypeCVT m_typecvttable[];
    static const char* m_typetable[];

  private:
    cMimeHeader& operator=(const cMimeHeader&);
};

/* cMimeHeader::field() - Add or update a field */
inline void cMimeHeader::field (const cMimeField& field) {
  std::list<cMimeField>::iterator it = findField(field.name());
  if (it != m_listfields.end()) {
    *it = field;
  } else {
    m_listfields.push_back(field);
  }
}

/* cMimeHeader::field() - Find a field by name */
inline const cMimeField* cMimeHeader::field (const char* p_fieldname) const {
  std::list<cMimeField>::const_iterator it = findField(p_fieldname);
  if (it != m_listfields.end())
    return &(*it);
  return NULL;
}

inline cMimeField* cMimeHeader::field (const char* p_fieldname) {
  std::list<cMimeField>::iterator it = findField(p_fieldname);
  if (it != m_listfields.end())
    return &(*it);
  return NULL;
}

/* cMimeHeader::fieldValue - Add or update a field with a value */
inline void cMimeHeader::fieldValue (const char* p_fieldname,
  const char* p_fieldvalue, const char* p_charset) {
  cMimeField fd;
  fd.name(p_fieldname);
  fd.value(p_fieldvalue);

  if (p_charset != NULL)
    fd.charset(p_charset);
  field(fd);
}

inline const char* cMimeHeader::fieldValue (const char* p_fieldname) const {
  const cMimeField* pfd = field(p_fieldname);
  return pfd != NULL ? pfd->value() : NULL;
}

/* cMimeHeader::fieldCharset - Update or get a header field charset */
inline void cMimeHeader::fieldCharset (const char* p_fieldname, 
    const char* p_charset) {
  cMimeField *fd = field(p_fieldname);
  if (fd) {
    fd->charset(p_charset);
  } else {
    cMimeField fd;
    fd.name(p_fieldname);
    fd.charset(p_charset);
    field(fd);
  }
}

inline const char* cMimeHeader::fieldCharset (const char* p_fieldname) const {
  const cMimeField* fd = field(p_fieldname);
  return fd != NULL ? fd->charset() : NULL;
}

/* cMimeHeader::parameter - Set or get a header field parameter */
inline bool cMimeHeader::parameter (const char* p_fieldname, const char* p_attr,
    const char* p_value) {
  cMimeField *fd = field(p_fieldname);
  if (fd) {
    fd->parameter(p_attr, p_value);
    return true;
  }
  return false;
}

inline std::string cMimeHeader::parameter (const char* p_fieldname, 
    const char* p_attr) const {
  std::string s_val;
  const cMimeField *fd = field(p_fieldname);
  if (fd) 
    fd->parameter(p_attr, s_val);
  return s_val;
}

/* cMimeHeader::contentType - Set/get Content-Type of a header field */
inline void cMimeHeader::contentType (const char* p_value,
    const char* p_charset) {
  fieldValue(cMimeConst::contentType(), p_value, p_charset);
}

inline const char* cMimeHeader::contentType () const {
  return fieldValue(cMimeConst::contentType());
}

/* cMimeHeader::charset - Set/get charset parameter of a header field */
inline std::string cMimeHeader::charset() const {
  return parameter(cMimeConst::contentType(), cMimeConst::charset()); 
}

inline std::string cMimeHeader::name() const {
  return parameter(cMimeConst::contentType(), cMimeConst::name()); 
}

inline std::string cMimeHeader::boundary() const {
  return parameter(cMimeConst::contentType(), cMimeConst::boundary()); 
}

inline std::string cMimeHeader::getBoundary() const {
  return parameter(cMimeConst::contentType(), cMimeConst::boundary());
}

inline void cMimeHeader::transferEncoding (const char* p_value) {
  fieldValue(cMimeConst::transferEncoding(), p_value);
}

inline const char* cMimeHeader::transferEncoding() const {
  return fieldValue(cMimeConst::transferEncoding());
}

inline void cMimeHeader::disposition (const char* p_value, 
    const char* p_charset) {
  fieldValue(cMimeConst::contentDisposition(), p_value, p_charset);
}

inline const char* cMimeHeader::disposition() const {
  return fieldValue(cMimeConst::contentDisposition());
}

inline std::string cMimeHeader::filename() const {
  return parameter(cMimeConst::contentDisposition(), cMimeConst::filename());
}

inline void cMimeHeader::description(const char* p_value, 
    const char* p_charset) {
  fieldValue(cMimeConst::contentDescription(), p_value, p_charset);
}

inline const char* cMimeHeader::description() const {
  return fieldValue(cMimeConst::contentDescription());
}

/* cMimeBody - Abstract for MIME message payloads */
class cMimeMessage;

class cMimeBody : public cMimeHeader {
  protected:
    cMimeBody() : m_text(NULL), m_textsize(0) {} 
    virtual ~cMimeBody() { clear(); }

  public:
    int contentLength() const;
    const unsigned char* content() const;

    // Operations on 'text' or 'message' media
    bool isText() const;
    int payload (const char* p_text, int length=0);
    int payload (char* p_text, int p_maxsize);
    int payload (std::string& p_text);

    // Operations on 'message' media
    bool isMessage() const;
    bool message (const cMimeMessage* p_mm);
    void message (cMimeMessage* p_mm) const;

    // Operations on attachment media
    bool isAttachment() const;
    bool readFromFile (const char* p_filename);
    bool writeToFile (const char* p_filename);

    // Operations on 'multipart' media
    bool isMultipart() const;
    void deleteAll();
    cMimeBody* createPart(const char* p_mediatype=NULL, 
        cMimeBody* p_where=NULL);
    void erasePart (cMimeBody* p_bp);
    cMimeBody* findFirstPart();
    cMimeBody* findNextPart();

    typedef std::list<cMimeBody*> cBodyList;
    int bodyPartList(cBodyList& p_list) const;
    int attachmentList(cBodyList& p_list) const;

    // Overrides
    virtual void clear();
    virtual int getLength() const;
    
    // Serialization
    virtual int store (char* p_data, int p_maxsize) const;
    virtual int load (const char* p_data, int p_datasize);

  protected:
    unsigned char* m_text;
    int m_textsize;
    cBodyList m_listbodies;
    cBodyList::iterator m_itfind;

    bool allocateBuffer (int p_bufsize);
    void freeBuffer();

    friend class cMimeEnvironment;
};

inline int cMimeBody::contentLength() const {
  return m_textsize;
}

inline const unsigned char* cMimeBody::content() const {
  return m_text;
}

inline bool cMimeBody::isText() const {
  return mediaType() == MEDIA_TEXT;
}

inline bool cMimeBody::isMessage() const {
  return mediaType() == MEDIA_MESSAGE;
}

inline bool cMimeBody::isAttachment() const {
  return name().size() > 0;
}

inline bool cMimeBody::isMultipart() const {
  return mediaType() == MEDIA_MULTIPART;
}

inline cMimeBody* cMimeBody::findFirstPart() {
  m_itfind = m_listbodies.begin();
  return findNextPart();
}

inline cMimeBody* cMimeBody::findNextPart() {
  if (m_itfind != m_listbodies.end())
    return *m_itfind++;
  return NULL;
}

inline bool cMimeBody::allocateBuffer (int bufsize) {
  freeBuffer();
  m_text = new unsigned char[bufsize];
  if (!m_text) 
    return false;
  m_textsize = bufsize;
  return true;
}

inline void cMimeBody::freeBuffer() {
  delete m_text;
  m_text = NULL;
  m_textsize = 0;
}

class cMimeMessage : public cMimeBody {
  public:
    cMimeMessage() { /*setVersion();*/ }
    virtual ~cMimeMessage() { clear(); }

    const char* from() const;
    void from (const char* p_from, const char* p_charset=NULL);

    const char* to() const;
    void to (const char* p_to, const char* p_charset=NULL);

    const char* cc() const;
    void cc (const char* p_cc, const char* p_charset=NULL);

    const char* bcc() const;
    void bcc (const char* p_bcc, const char* p_charset=NULL);

    const char* subject() const;
    void subject (const char* p_subject, const char* charset=NULL);

    void date ();
    const char* date() const;
    void date (int year, int month, int day, int hour, int minute, int second);

    void setVersion();
};

inline void cMimeMessage::from (const char* p_addr, const char* p_charset) {
  fieldValue("From", p_addr, p_charset);
}

inline const char* cMimeMessage::from() const {
  return fieldValue("From");
}

inline void cMimeMessage::to (const char* p_addr, const char* p_charset) {
  fieldValue("To", p_addr, p_charset);
}

inline const char* cMimeMessage::to () const {
  return fieldValue("To");
}

inline void cMimeMessage::cc (const char* p_addr, const char* p_charset) {
  fieldValue("CC", p_addr, p_charset);
}

inline const char* cMimeMessage::cc () const {
  return fieldValue("CC");
}

inline void cMimeMessage::bcc (const char* p_addr, const char* p_charset) {
  fieldValue("BCC", p_addr, p_charset);
}

inline const char* cMimeMessage::bcc () const {
  return fieldValue("BCC");
}

inline void cMimeMessage::subject (const char* p_subj, const char* p_charset) {
  fieldValue("Subject", p_subj, p_charset);
}

inline const char* cMimeMessage::subject () const {
  fieldValue("Subject");
}

inline const char* cMimeMessage::date() const {
  return fieldValue("date");
}

inline void cMimeMessage::setVersion() {
  fieldValue(cMimeConst::mimeVersion(), "1.0");
}
#endif // !defined(_MIME_H)
