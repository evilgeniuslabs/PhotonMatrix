unsigned long lastGmailSync = 0;
boolean gmailReceived = false;

uint16_t unreadCount = 0;

String extractString(String myString, const char* start, const char* end)
{
    if (myString == NULL)
    {
        return NULL;
    }
    int idx = myString.indexOf(start);
    if (idx < 0) {
        return NULL;
    }
    int endIdx = myString.indexOf(end);
    if (endIdx < 0) {
        return NULL;
    }
    return myString.substring(idx + strlen(start), endIdx);
}

void gotGmailData(const char *name, const char *data){
  Serial.println("Got Gmail data:");

  Serial.println(data);

  // we'll get data like this:
  // <?xml version="1.0" encoding="UTF-8"?>
  // <feed version="0.3" xmlns="http://purl.org/atom/ns#">
  //   <title>Gmail - Inbox for username@gmail.com</title>
  //   <tagline>New messages in your Gmail Inbox</tagline>
  //   <fullcount>1</fullcount>
  // ...

	int stringLength = strlen((const char *)data);

  Serial.print("Gmail data length:");
  Serial.println(stringLength);

  Serial.println("Parsing Gmail data...");

  String buffer = String(data);
  String emailReturn = extractString(buffer, "<fullcount>", "</fullcount>");
  if (emailReturn != NULL)
  {
    unreadCount = emailReturn.toInt();
    Serial.println(unreadCount);
  }

  Serial.println("Finished parsing Gmail data.");

	lastGmailSync = millis();
	gmailReceived = true;
}

static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char	 pixel_data[16 * 11 * 3 + 1];
} gmail_image = {
  16, 11, 3,
  "\33288\33288\33288\362\201\201\371\247\247\371\247\247\371\247\247\371\247"
  "\247\371\247\247\371\247\247\371\247\247\371\247\247\362\201\201\33288\332"
  "88\33288\33288\33288\33288\33288\377\266\266\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\266\266\33288\33288\332"
  "88\33288\33288\33288\362\201\201\351ZZ\33288\377\266\266\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\266\266\33288\351ZZ\362\201\201\332"
  "88\33288\33288\33288\377\377\377\362\201\201\351ZZ\33288\377\266\266\377"
  "\377\377\377\377\377\377\266\266\33288\351ZZ\362\201\201\377\377\377\332"
  "88\33288\33288\33288\377\377\377\377\377\377\362\201\201\351ZZ\33288\377"
  "\266\266\377\266\266\33288\351ZZ\362\201\201\377\377\377\377\377\377\332"
  "88\33288\33288\33288\377\377\377\377\377\377\377\266\266\362\201\201\351"
  "ZZ\33288\33288\351ZZ\362\201\201\377\266\266\377\377\377\377\377\377\332"
  "88\33288\33288\33288\377\377\377\377\342\342\371\247\247\377\266\266\362"
  "\201\201\351ZZ\351ZZ\362\201\201\377\266\266\371\247\247\377\342\342\377"
  "\377\377\33288\33288\33288\33288\377\342\342\371\247\247\377\342\342\377"
  "\377\377\377\377\377\362\201\201\362\201\201\377\377\377\377\377\377\377"
  "\342\342\371\247\247\377\342\342\33288\33288\33288\33288\371\247\247\377"
  "\342\342\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\342\342\371\247\247\33288\33288\332"
  "88\33288\377\342\342\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\342\342\33288\33288\33288\33288\371\247\247\371\247\247\371\247\247\371"
  "\247\247\371\247\247\371\247\247\371\247\247\371\247\247\371\247\247\371"
  "\247\247\371\247\247\371\247\247\33288\33288",
};

uint8_t drawGmail(uint8_t xOffset, uint8_t yOffset)
{
  const uint16_t length = gmail_image.width * gmail_image.height * gmail_image.bytes_per_pixel;

  uint8_t x = 0;
  uint8_t y = 0;

  for(uint16_t i = 0; i < length; i += 3) {
    unsigned char r = gmail_image.pixel_data[i];
    unsigned char g = gmail_image.pixel_data[i + 1];
    unsigned char b = gmail_image.pixel_data[i + 2];

    matrix.drawPixel(x + xOffset, y + yOffset, matrix.Color888(r, g, b, true));

    x++;
    if(x >= gmail_image.width)
    {
      x = 0;
      y++;
    }

    if(y >= gmail_image.height)
      break;
  }

  vectorNumber(unreadCount, 18, 1, White, 1, 1);
}
