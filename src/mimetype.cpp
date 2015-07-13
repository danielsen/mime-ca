/* mimetype.cpp - Content-Type extension tables 
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

/* Content-Type header fields specifiy the media type of a payload, either 
 * discrete: text/image/audio/video/application or composite, message/multipart.
 * The default Content-Type is text/plain; charset=us-ascii (RFC 2046)
 */
#include "mime.h"

const char* cMimeHeader::m_typetable[] = {
  "text", "image", "audio", "video", "application", "multipart", "message", NULL
};

/* TODO: This table is terribly incomplete, need to continue to add sub-types
 * to it.
 */
const cMimeHeader::mediaTypeCVT cMimeHeader::m_typecvttable[] = {
  // media-type,        sub-type,             file extension
  { MEDIA_APPLICATION,  "xml",                "xml"   },
  { MEDIA_APPLICATION,  "msword",             "doc"   },
  { MEDIA_APPLICATION,  "rtf",                "rtf"   },
  { MEDIA_APPLICATION,  "vnd.ms-excel",       "xls"   },
  { MEDIA_APPLICATION,  "vnd.ms-powerpoint",  "ppt"   },
  { MEDIA_APPLICATION,  "pdf",                "pdf"   },
  { MEDIA_APPLICATION,  "zip",                "zip"   },
  { MEDIA_IMAGE,        "jpeg",               "jpeg"  },
  { MEDIA_IMAGE,        "jpeg",               "jpg"   },
  { MEDIA_IMAGE,        "gif",                "gif"   },
  { MEDIA_IMAGE,        "tiff",               "tiff"  },
  { MEDIA_IMAGE,        "tiff",               "tif"   },
  { MEDIA_AUDIO,        "basic",              "wav"   },
  { MEDIA_AUDIO,        "basic",              "mp3"   },
  { MEDIA_VIDEO,        "mpeg",               "mpg"   },
  { MEDIA_VIDEO,        "mpeg",               "mpeg"  },

  // add new subtypes before this entry
  { MEDIA_UNKNOWN,      "",                   "",     }
};
