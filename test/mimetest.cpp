#include <iostream>

#include "../src/mime.h"

using namespace std;

int main (void) {
  cMimeMessage mail;

  mail.date();
  mail.setVersion();
  mail.fieldValue("Return-Path", "errors@whatever.com");
  mail.fieldValue("To", "Jane Jones <jane@whatever.com>");
  mail.fieldValue("From", "Sara Smith <sara@whatever.com>");
  mail.fieldValue("Subject", "Meeting today");
  mail.fieldValue("Message-Id", "123987");

  mail.contentType("multipart/alternative");
  mail.boundary(NULL);

  cMimeBody* payload = mail.createPart();
  payload->contentType("text/plain");
  payload->payload("Testing ...");

  payload = mail.createPart();
  payload->contentType("text/html");
  payload->payload("<p>Testing ...</p>");

  int msize = mail.getLength();
  char* mbuff = new char[msize];
  msize = mail.store(mbuff, msize);

  cout << mbuff << endl;
  delete mbuff;

  return 0;
}
