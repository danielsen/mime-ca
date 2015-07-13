/* MIME message encoding, decoding, and compilation. See mime.h for more 
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
#include <time.h>
#include <string>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mimecode.h"
#include "mimechar.h"
#include "mime.h"

using namespace std;

const int O_BINARY = 0;

/* Utility functions */

/* lineFind - Search for a character in the current line (before CRLF) */
static const char* lineFind (const char* p_string, int ch) {
  ASSERT(p_string != NULL);
  while (*p_string != 0 && *p_string != ch && *p_string != '\r'
      && *p_string != '\n')
    p_string++;
  return *p_string == ch ? p_string : NULL;
}

/* findString - Search for string2 in string1, i.e. strstr */
static const char* findString (const char* p_s1, const char* p_s2, 
    const char* p_end) {
  p_end -= strlen(p_s2);
  const char *s1, *s2;
  while (p_s1 <= p_end) {
    s1 = p_s1;
    s2 = p_s2;
    while (*s1 == *s2 && *s2) s1++, s2++;
    if (!*s2) return p_s1;
    p_s1++;
  }
  return NULL;
}

/* End utility fnuctions */

/* cMimeField definitions */

void cMimeField::value (string& p_value) const {
  string::size_type end = m_value.find(';');
  if (end != string::npos) {
    while (end > 0 && cMimeChar::isSpace((unsigned char)m_value[end-1])) {
      end--;
    }
    p_value.assign(m_value.c_str(), end);
  } else {
    p_value = m_value;
  }
}

void cMimeField::parameter (const char* p_attr, const char* p_value) {
  int size = p_value ? (int)strlen(p_value) : 0;
  string value;
  value.reserve(size+3);
  if (!p_value || *p_value != '"') {
    value = "\"";
  }
  if (p_value != NULL) {
    value += p_value;
  }
  if (size < 2 || p_value[size-1] != '"') {
    value += "\"";
  }

  int pos;
  if (!findParameter(p_attr, pos, size)) {
    // Add a new parameter
    m_value.reserve(m_value.size() + strlen(p_attr) + value.size() + 5);
    m_value += "; ";
    m_value += p_attr;
    m_value += "=";
    m_value += value;
  } else {
    m_value.replace(pos, size, value);
  }
}

bool cMimeField::parameter (const char* p_attr, string& p_value) const {
  int pos, size;
  if (!findParameter(p_attr, pos, size)){
    p_value.clear();
    return false;
  }
  if (m_value[pos] == '"') {
    pos++;
    size--;
    if (size > 0 && m_value[pos + size-1] == '"') {
      size--;
    }
  }
  p_value.assign(m_value.data()+pos, size);
  return true;
}

int cMimeField::getLength() const {
  int len = (int) m_name.size() + 4;
  cFieldCodeBase* coder = cMimeEnvironment::registerFieldCoder(name());
  coder->charset(m_charset.c_str());
  coder->setInput(m_value.c_str(), (int)m_value.size(), true);
  len += coder->getOutputLength();
  delete coder;
  return len;
}

int cMimeField::store(char* p_data, int maxsize) const {
  ASSERT(p_data != NULL);
  int minsize = (int)m_name.size() + 4;
  if (maxsize < minsize)
    return 0;
  strcpy(p_data, m_name.c_str());
  p_data += m_name.size();
  *p_data++ = ':';
  *p_data++ = ' ';

  cFieldCodeBase* coder = cMimeEnvironment::registerFieldCoder(name());
  coder->charset(m_charset.c_str());
  coder->setInput(m_value.c_str(), (int)m_value.size(), true);
  int encoded = coder->getOutput((unsigned char*) p_data, maxsize - minsize);
  delete coder;
  p_data += encoded;

  *p_data++ = '\r';
  *p_data++ = '\n';
  return minsize + encoded;
}

int cMimeField::load (const char* p_data, int datasize) {
  clear();
  ASSERT(p_data != NULL);

  const char *end, *start = p_data;
  while (cMimeChar::isSpace((unsigned char)*start)) {
    if (*start == '\r')
      return 0;
    start = findString(start, "\r\n", p_data+datasize);
    if (!start)
      return 0;
    start += 2;
  }

  end = lineFind(start, ':');
  if (end != NULL) {
    m_name.assign(start, (end - start));
    start = end + 1;
  }

  while (*start == ' ' || *start == '\t')
    start++;
  end = start;
  do {
    end = findString(end, "\r\n", p_data + datasize);
    if (!end)
      return 0;
    end += 2;
  } while (*end == '\t' || *end == ' ');

  cFieldCodeBase* coder = cMimeEnvironment::registerFieldCoder(name());
  coder->setInput(start, (int)(end - start)-2, false);
  m_value.resize(coder->getOutputLength());
  int size = coder->getOutput((unsigned char*) m_value.c_str(), 
      (int)m_value.capacity());
  m_value.resize(size);
  m_charset = coder->charset();
  delete coder;
  return (int)(end - start);
}

bool cMimeField::findParameter (const char* p_attr, int& pos, int& size) const {
  ASSERT(p_attr != NULL);
  const char* params = strchr(m_value.data(), ';');
  int attrsize = (int)strlen(p_attr);
  while (params != NULL) {
    while (cMimeChar::isSpace((unsigned char)*params) || *params == ';')
      params++;

    const char* name = params;
    params = strchr(params, '=');
    if (!params)
      break;

    params++;
    const char* paramend = NULL;
    if (*params == '"')
      paramend = strchr(params + 1, '"');
    if (!paramend) {
      paramend = params;
      while (cMimeChar::isToken(*paramend))
        paramend++;
    } else {
      paramend++;
    }

    if (!memcmp(p_attr, name, attrsize) && (
        cMimeChar::isSpace((unsigned char)name[attrsize]) ||
        name[attrsize] == '=')) {
      pos = (int)(params - m_value.data());
      size = (int)(paramend - params);
      return true;
    }

    params = paramend;
  }
  return false;
}
      
/* End cMimeField definitions */
/* cMimeHeader definitions */
cMimeHeader::media cMimeHeader::mediaType () const {
  const char* type = contentType();
  if (!type)
    type = "text";

  int index = 0;
  while (m_typetable[index] != NULL && 
      memcmp(type, m_typetable[index], strlen(m_typetable[index])) != 0)
    index++;
  return (media)index;
}

string cMimeHeader::mainType () const {
  string type;
  const char* p_type = contentType();
  if (p_type != NULL) {
    const char* slash = strchr(p_type, '/');
    if (slash != NULL) {
      type.assign(p_type, slash - p_type);
    } else {
      type = p_type;
    }
  } else {
    type = "text";
  }
  return type;
}

string cMimeHeader::subType() const {
  string subtype;
  const cMimeField *fd = field(cMimeConst::contentType());
  if (fd != NULL) {
    string type;
    fd->value(type);
    string::size_type slash = type.find('/');
    if (slash > 0)
      subtype = type.substr(slash+1);
  } else {
    subtype = "plain";
  }
  return subtype;
}

void cMimeHeader::charset (const char* p_charset) {
  cMimeField *fd = field(cMimeConst::contentType());
  if (!fd) {
    cMimeField fd;
    fd.name(cMimeConst::contentType());
    fd.value("text/plain");
    fd.parameter(cMimeConst::charset(), p_charset);
  } else {
    fd->parameter(cMimeConst::charset(), p_charset);
  }
}

void cMimeHeader::name (const char* p_name) {
  cMimeField *fd = field(cMimeConst::contentType());
  if (!fd) {
    ASSERT(p_name != NULL);
    string type;
    const char* p_type = "application/octet-stream";
    const char* p_filext = strrchr(p_name, '.');
    if (p_filext != NULL) {
      p_filext++;
      int index = 0;
      while (m_typecvttable[index].media_type != MEDIA_UNKNOWN) {
        if (!strcmp(p_filext, m_typecvttable[index].file_ext)) {
          type = m_typetable[m_typecvttable[index].media_type];
          type += '/';
          type += m_typecvttable[index].sub_type;
          p_type = type.c_str();
          break;
        }
        index++;
      }
    }

    cMimeField fd;
    fd.name(cMimeConst::contentType());
    fd.value(p_type);
    fd.parameter(cMimeConst::name(), p_name);
    m_listfields.push_back(fd);
  } else {
    fd->parameter(cMimeConst::name(), p_name);
  }
}

void cMimeHeader::boundary (const char* p_boundary) {
  static int s_partnumber;
  char buf[80];
  if (!p_boundary) {
    srand(((unsigned)time(NULL)));
    sprintf(buf, "==_mimepart_%06d.%06d", rand(), rand());
    if (s_partnumber >= 9)
      s_partnumber = 0;
    p_boundary = buf;
  }

  cMimeField *pfd = field(cMimeConst::contentType());
  if (!pfd) {
    cMimeField fd;
    fd.name(cMimeConst::contentType());
    fd.value("multipart/mixed");
    fd.parameter(cMimeConst::boundary(), p_boundary);
    m_listfields.push_back(fd);
  } else {
    if (memcmp(pfd->value(), "multipart", 9) != 0)
      pfd->value("multipart/mixed");
    pfd->parameter(cMimeConst::boundary(), p_boundary);
  }
}

void cMimeHeader::clear() {
  m_listfields.clear();
}

int cMimeHeader::getLength() const {
  int len = 0;
  std::list<cMimeField>::const_iterator it;
  for (it = m_listfields.begin(); it != m_listfields.end(); it++)
    len += (*it).getLength();
  return len + 2;
}

int cMimeHeader::store (char* p_data, int maxsize) const {
  ASSERT(p_data != NULL);
  int output = 0;
  std::list<cMimeField>::const_iterator it;
  for (it = m_listfields.begin(); it != m_listfields.end(); it++) {
    const cMimeField& fd = *it;
    int size = fd.store(p_data+output, maxsize-output);
    if (size <= 0)
      return size;
    output += size;
  }

  p_data[output++] = '\r';
  p_data[output++] = '\n';
  return output;
}

int cMimeHeader::load (const char* p_data, int datasize) {
  ASSERT(p_data != NULL);
  int input = 0;
  while (p_data[input] != 0 && p_data[input] != '\r') {
    cMimeField fd;
    int size = fd.load(p_data + input, datasize - input);
    if (size <= 0)
      return size;
    input += size;
    m_listfields.push_back(fd);
  }
  return input + 2;
}

std::list<cMimeField>::const_iterator cMimeHeader::findField (
    const char* p_fieldname) const {
  std::list<cMimeField>::const_iterator it;
  for (it = m_listfields.begin(); it != m_listfields.end(); it++) {
    const cMimeField& fd = *it;
    if (!strcmp(fd.name(), p_fieldname))
      break;
  }
  return it;
}

std::list<cMimeField>::iterator cMimeHeader::findField (
    const char* p_fieldname) {
  std::list<cMimeField>::iterator it;
  for (it = m_listfields.begin(); it != m_listfields.end(); it++) {
    cMimeField& fd = *it;
    if (!strcmp(fd.name(), p_fieldname))
      break;
  }
  return it;
}

/* End cMimeHeader definitions */

/* cMimeBody definitions */
void cMimeBody::clear() {
  deleteAll();
  m_itfind = m_listbodies.end();
  freeBuffer();
  cMimeHeader::clear();
}

int cMimeBody::payload (const char* p_text, int length) {
  ASSERT(p_text != NULL);
  if (!length)
    length = (int)strlen((char*)p_text);

  if (!allocateBuffer(length+4)) 
    return -1;

  memcpy(m_text, p_text, length);
  m_text[length] = 0;
  m_textsize = length;
  return length;
}

int cMimeBody::payload (char* p_text, int maxsize) {
  int size = min(maxsize, m_textsize);
  if (m_text != NULL)
    memcpy(p_text, m_text, size);
  return size;
}

int cMimeBody::payload (string& p_text) {
  if (m_text != NULL)
    p_text.assign((const char*) m_text, m_textsize);
  return m_textsize;
}

bool cMimeBody::message (const cMimeMessage* p_mm) {
  ASSERT(p_mm != NULL);
  int size = p_mm->getLength();
  if (!allocateBuffer(size+4))
    return false;

  size = p_mm->store((char*)m_text, size);
  m_text[size] = 0;
  m_textsize = size;

  const char* type = contentType();
  if (!type || memcmp(type, "message", 7) != 0)
    contentType("message/rfc822");

  return true;
}

void cMimeBody::message (cMimeMessage* p_mm) const {
  ASSERT(p_mm != NULL);
  ASSERT(m_text != NULL);
  p_mm->load((const char*)m_text, m_textsize);
}

bool cMimeBody::readFromFile (const char* p_filename) {
  int file = open(p_filename, O_RDONLY | O_BINARY);
  if (file < 0)
    return false;

  try {
    int filesize = (int)lseek(file, 0L, SEEK_END);
    lseek(file, 0L, SEEK_SET);

    freeBuffer();
    if (filesize > 0) {
      allocateBuffer(filesize+4);
      unsigned char* p_data = m_text;

      for (;;) {
        int rd = read(file, p_data, 512);
        if (rd < 0) {
          close(file);
          return false;
        }
        p_data += rd;
        if (rd < 512)
          break;
      }
      *p_data = 0;
      m_textsize = filesize;
    }
  } catch (...) {
    close(file);
    throw;
  }

  close(file);
  const char* p_name = strrchr(p_filename, '\\');
  if (!p_name) {
    p_name = p_filename;
  } else {
    p_name++;
  }
  name(p_name);
  return true;
}

bool cMimeBody::writeToFile (const char* p_filename) {
  if (!m_textsize)
    return true;

  int file = open(p_filename, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 
    S_IREAD | S_IWRITE);
  if (file < 0) 
    return false;

  const unsigned char* p_data = m_text;
  int left = m_textsize;

  try {
    for (;;) {
      int written = write(file, p_data, min(512, left));
      if (written <= 0) {
        close(file);
        return false;
      }
      p_data += written;
      left -= written;
      if (left <= 0)
        break;
    }
  } catch (...) {
    close(file);
    throw;
  }

  close(file);
  return true;
}

void cMimeBody::deleteAll() {
  while (!m_listbodies.empty()) {
    cMimeBody* p_bp = m_listbodies.back();
    m_listbodies.pop_back();
    ASSERT(p_bp != NULL);
    delete p_bp;
  }
}

cMimeBody* cMimeBody::createPart(const char* p_mediatype, cMimeBody* p_where) {
  cMimeBody* p_bp = cMimeEnvironment::createBodyPart(p_mediatype);
  ASSERT(p_bp != NULL);
  if (p_where != NULL) {
    for (cBodyList::iterator it = m_listbodies.begin(); 
        it != m_listbodies.end(); it++) {
      if (*it == p_where) {
        m_listbodies.insert(it, p_bp);
        return p_bp;
      }
    }
  }
  m_listbodies.push_back(p_bp);
  return p_bp;
}

void cMimeBody::erasePart(cMimeBody* p_bp) {
  ASSERT(p_bp != NULL);
  m_listbodies.remove(p_bp);
  delete p_bp;
}

int cMimeBody::bodyPartList (cBodyList& p_list) const {
  int count = 0;
  int mediatype = mediaType();

  if (MEDIA_MULTIPART != mediatype) {
    p_list.push_back((cMimeBody*)this);
    count++;
  } else {
    std::list<cMimeBody*>::const_iterator it;
    for (it = m_listbodies.begin(); it != m_listbodies.end(); it++) {
      cMimeBody* p_bp = *it;
      ASSERT(p_bp != NULL);
      count += p_bp->bodyPartList(p_list);
    }
  }
  return count;
}

int cMimeBody::attachmentList (cBodyList& p_list) const {
  int count = 0;
  int mediatype = mediaType();

  if (MEDIA_MULTIPART != mediatype) {
    string s_name = name();
    if (s_name.size() > 0) {
      p_list.push_back((cMimeBody*)this);
      count++;
    }
  } else {
    std::list<cMimeBody*>::const_iterator it;
    for (it = m_listbodies.begin(); it != m_listbodies.end(); it++) {
      cMimeBody* p_bp = *it;
      ASSERT(p_bp != NULL);
      count += p_bp->attachmentList(p_list);
    }
  }
  return count;
}

int cMimeBody::getLength() const {
  int length = cMimeHeader::getLength();
  cMimeCodeBase* coder = cMimeEnvironment::registerCoder(transferEncoding());
  ASSERT(coder != NULL);
  coder->setInput((const char*)m_text, m_textsize, true);
  length += coder->getOutputLength();
  delete coder;

  if (m_listbodies.empty())
    return length;

  string s_boundary = boundary();
  int boundsize = (int)s_boundary.size();
  std::list<cMimeBody*>::const_iterator it;
  for (it = m_listbodies.begin(); it != m_listbodies.end(); it++) {
    length += boundsize + 6;
    cMimeBody* p_bp = *it;
    ASSERT(p_bp != NULL);
    length += p_bp->getLength();
  }
  length += boundsize + 8;
  return length;
}

int cMimeBody::store (char* p_data, int maxsize) const {
  int size = cMimeHeader::store(p_data, maxsize);
  int a_count = 0;
  if (size <= 0)
    return size;

  char* p_databegin = p_data;
  p_data += size;
  maxsize -= size;

  cMimeCodeBase* coder = cMimeEnvironment::registerCoder(transferEncoding());
  ASSERT(coder != NULL);
  coder->setInput((const char*)m_text, m_textsize, true);
  int output = coder->getOutput((unsigned char*)p_data, maxsize);
  delete coder;
  if (output < 0)
    return output;

  p_data += output;
  maxsize -= output;
  if (m_listbodies.empty())
    return (int)(p_data - p_databegin);

  string s_boundary = getBoundary();
  if (s_boundary.empty())
    return -1;

  int boundsize = (int)s_boundary.size() + 6;
  for (cBodyList::const_iterator it=m_listbodies.begin();
      it != m_listbodies.end(); it++) {

    if (maxsize < boundsize)
      break;
    if (m_listbodies.begin() == it && *(p_data-2) == '\r' 
        && *(p_data-1) == '\n') {
      p_data -= 2;
      maxsize += 2;
    }
    sprintf(p_data, "\r\n--%s\r\n", s_boundary.c_str());
    p_data += boundsize;
    maxsize -= boundsize;

    cMimeBody *p_bp = *it;
    ASSERT(p_bp != NULL);
    output = p_bp->store(p_data, maxsize);

    if (output < 0) 
      return output;
    p_data += output;
    maxsize -= output;
    //xx
    //if (p_bp->disposition() == NULL) {
    //  a_count++;
    //  if (a_count > 1) {
    //    sprintf(p_data, "\r\n--%s--", s_boundary.c_str());
    //    p_data += boundsize;
    //  }
    //}
    //xx
  }

  if (maxsize >= boundsize + 2) {
    sprintf(p_data, "\r\n--%s--\r\n", s_boundary.c_str());
    p_data += boundsize + 2;
  }
  return (int)(p_data - p_databegin);
}

int cMimeBody::load (const char* p_data, int datasize) {
  int size = cMimeHeader::load(p_data, datasize);
  if (size <= 0)
    return size;

  const char* p_databegin = p_data;
  p_data += size;
  datasize -= size;
  freeBuffer();

  const char* p_end = p_data + datasize;
  int mediatype = mediaType();
  if (MEDIA_MULTIPART == mediatype) {
    string s_boundary = getBoundary();
    if (!s_boundary.empty()) {
      s_boundary = "\r\n--" + s_boundary;
      p_end = findString(p_data-2, s_boundary.c_str(), p_end);
      if (!p_end) {
        p_end = p_data + datasize;
      } else {
        p_end += 2;
      }
    }
  }
  size = (int)(p_end - p_data);

  if (size > 0) {
    cMimeCodeBase* coder = cMimeEnvironment::registerCoder(transferEncoding());
    ASSERT(coder != NULL);
    coder->setInput(p_data, size, false);
    int output = coder->getOutputLength();
    if (allocateBuffer(output+4)) {
      output = coder->getOutput(m_text, output);
    } else {
      output -= 1;
    }
    delete coder;

    if (output < 0)
      return output;

    ASSERT(output < m_textsize);
    m_text[output] = 0;
    m_textsize = output;
    p_data += size;
    datasize -= size;
  }

  if (datasize <= 0)
    return (int)(p_data - p_databegin);

  string s_boundary = getBoundary();
  ASSERT(s_boundary.size() > 0);
  s_boundary = "\r\n--" + s_boundary;

  p_data -= 2;
  datasize += 2;
  p_end = p_data + datasize;
  const char* p_bound1 = findString(p_data, s_boundary.c_str(), p_end);
  while (p_bound1 != NULL && p_bound1 < p_end) {
    const char* p_start = findString(p_bound1+2, "\r\n", p_end);
    if (!p_start)
      break;
    p_start += 2;
    if (p_bound1[s_boundary.size()] == '-' 
        && p_bound1[s_boundary.size()+1] == '-')
      return (int)(p_start - p_databegin);

    const char* p_bound2 = findString(p_start, s_boundary.c_str(), p_end);
    if (!p_bound2)
      p_bound2 = p_end;
    int entitysize = (int)(p_bound2 - p_start);

    cMimeHeader header;
    header.load(p_start, entitysize);
    string s_mediatype = header.mainType();
    cMimeBody* p_bp = createPart(s_mediatype.c_str());

    int inputsize = p_bp->load(p_start, entitysize);
    if (inputsize < 0) {
      erasePart(p_bp);
      return inputsize;
    }
    p_bound1 = p_bound2;
  }
  return (int)(p_end - p_databegin);
}
/* End cMimeBody */

/* cMimeMessage */
void cMimeMessage::date() {
  time_t timenow = time(NULL);
  struct tm *ptm = localtime(&timenow);
  date(ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour,
      ptm->tm_min, ptm->tm_sec);
}

void cMimeMessage::date(int year, int month, int day, int hour, int minute,
    int second) {
  static const char* s_monthnames[] = { "Jan", "Feb", "Mar", "Apr", "May",
    "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  static const char* s_daynames[] = { "Sun", "Mon", "Tue", "Wed", "Thu",
    "Fri", "Sat" };

  struct tm tmdate;
  memset(&tmdate, 0, sizeof(tmdate));
  tmdate.tm_year = year - 1900;
  tmdate.tm_mon = month - 1;
  tmdate.tm_mday = day;
  tmdate.tm_min = minute;
  tmdate.tm_sec = second;
  tmdate.tm_isdst = -1;

  time_t timedate = mktime(&tmdate);
  if (timedate < 0) {
    ASSERT(false);
    return;
  }

  tmdate = *localtime(&timedate);
  struct tm *ptmgmt = gmtime(&timedate);
  long timediff = tmdate.tm_mday - ptmgmt->tm_mday;
  if (timediff > 1)
    timediff = -1;
  else if (timediff < -1)
    timediff = 1;
  timediff *= 60 * 24;
  timediff += (
    tmdate.tm_hour - ptmgmt->tm_hour) * 60 + tmdate.tm_min - ptmgmt->tm_min;
  if (tmdate.tm_isdst > 0)
    timediff -= 60;

  char sdate[40];
  ASSERT(tmdate.tm_wday < 7);
  ASSERT(tmdate.tm_mon < 12);
  sprintf(sdate, "%s, %d %s %d %02d:%02d:%02d %c%02d%02d",
    s_daynames[tmdate.tm_wday], tmdate.tm_mday, s_monthnames[tmdate.tm_mon],
    tmdate.tm_year+1900, tmdate.tm_hour, tmdate.tm_min, tmdate.tm_sec,
    (timediff >= 0 ? '+' : '-'), abs(timediff / 60), abs(timediff % 60));

  fieldValue("Date", sdate);
}
