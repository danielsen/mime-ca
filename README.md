mime-ca - Tools for constructing and deconstructing MIME messages. 

### Setup and Installation

mime-ca can be built as a library by using `make`. Run `make` to build mime-ca,
the resulting shared object, `libmime-ca.so` will be placed in `build/Release`. 
Copy `libmime-ca.so` to your application directory or to your system library
directory. Remember to run `ldconfig` if `libmime-ca.so` winds up in a 
system library directory or add it's final destination to the 
`LD_LIBRARY_PATH` environment variable. Either way, use `-lmime-ca` to include
the library in your application.

    $ make
    $ cp build/Release/libmime-ca.so /application-x/lib
    $ export LD\_LIBRARY\_PATH=/application-x/lib:${LD\_LIBRARY\_PATH}

    $ g++ -lmime-ca application-x.cpp -o appx

Alternatively, copy the files from the `src` directory to your application 
directory and include them directly when building your application.

    $ g++ mime.cpp mimecode.cpp mimetype.cpp mimechar.cpp application-x.cpp -o apx

### Constructing a Message

This is a brief example of how to construct a message. A more complete example
is available in `test/mimetest.cpp`.

    cMimeMessage mail;

    mail.date()
    mail.setVersion();
    mail.from("me@local.com");
    mail.to("you@remote.com");
    mail.subject("A message from me to you");
    mail.fieldValue("X-Custom-Field", "custom value");
    mail.fieldValue("X-Priortiy", "3");

    mail.contentType("multipart/mixed");
    mail.boundary(NULL);

    // Add a plain text part
    cMimeBody\* payload = mail.createPart();
    payload->contentType("text/plain");
    payload->payload("This is a message");

    // Compose a simple message
    cMimeMessage mail1;
    mail1.from("user1@local.com");
    mail1.to("user2@remote.com");
    mail1.subject("An attached message");
    mail1.text("Attached message content.\r\n");

    // Attach the message
    sp = mail.createPart();
    sp->contentDescription("enclosed message");
    sp->transferEncoding("7bit");
    sp->message(&mail1);

    // Save the message to a buffer and fold any long lines
    cMimeEnvironment::autoFolding(true);
    int msize = mail.length();
    char \*buff = new char[msize];
    msize = mail.store(buff, msize);

    ...

    // Delete when finished
    delete buff;

### Deconstructing a message

Deconstructing is simple, mostly being the reverse of constructing the message

    cMimeMessage mail;
    int loadsize = mail.load(buff, mailsize);
  
    // Inspect the message headers
    const char \*mfield;
    mfield = mail.subject();
    if (mfield != NULL)
      printf("Subject: %s\n", mfield);
    mfield = mail.from();
    if (mfield != NULL)
      printf("From: %s\n", mfield);

    // Iterate through the message body parts
    cMimeBody::cBodyList bodies;
    int count = mail.bodyPartList(bodies);
    cMimeBody::cBodyList::const\_iterator it;
    for (it=bodies.begin(); it!=bodies.end(); it++) {
      cMimeBody \*bpart = \*it;

      // Iterate through the header fields
      cMimeHeader::cFieldList &fields = bpart->fields();
      cMimeHeader::cFieldList::const_iterator itfld;
      for (itfld=fields.begin(); itfld!=fields.end(); itfld++) {
        const cMimeField &fd = *itfd;
        printf("%s: %s\n", fd.name(), fd.value());
      }

      if (bpart->text()) {
        string stext;
        bpart->text(stext);
        printf("Content: %s\n", stext.c_str());
      } else if (bpart->attachment()) {
        string sname = bpart->name();
        printf("File name: %s\n", sname.c_str());
        printf("File size: %s\n", bpart->contentLength());
        sname = "./" + sname;
        bpart->writeToFile(sname.c_str());
      }
    }


