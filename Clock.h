void drawDateAndTime(uint8_t x, uint8_t y)
{
  int mins = Time.minute();
  int hours = Time.hour();
  int date = Time.day();
  int dow = Time.weekday() - 1;

  char endchar[16];

	// adjust the hour for AM/PM, if desired
	if(ampm == 1) {
	  if (hours > 12) {
	    hours -= 12;
	  }
	  else if (hours < 1) {
	    hours += 12;
	  }
	}

  //set final characters
  char buffer[3];
  itoa(hours,buffer,10);

  //fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
  if (hours < 10) {
    buffer[1] = buffer[0];
    buffer[0] = ampm == 1 ? ' ' : '0';
  }

  endchar[0] = buffer[0];
  endchar[1] = buffer[1];
  endchar[2] = Time.second() % 2 == 0 ? ':' : ' ';

  itoa (mins, buffer, 10);
  if (mins < 10) {
    buffer[1] = buffer[0];
    buffer[0] = '0';
  }

  endchar[3] = buffer[0];
  endchar[4] = buffer[1];

  itoa (date, buffer, 10);
  if (date < 10) {
    buffer[1] = buffer[0];
    buffer[0] = ' ';
  }

  //then work out date 2 letter suffix - eg st, nd, rd, th etc
  char suffix[4][3] = {
    "st", "nd", "rd", "th" };

  byte s = 3;
  if(date == 1 || date == 21 || date == 31) {
    s = 0;
  }
  else if (date == 2 || date == 22) {
    s = 1;
  }
  else if (date == 3 || date == 23) {
    s = 2;
  }

  //set topline
  endchar[5] = ' ';

  if(ampm == 1) {
    endchar[6] = Time.isAM() ? 'A' : 'P';
    endchar[7] = 'M';
  }
  else {
    endchar[6] = ' ';
    endchar[7] = ' ';
  }

  //set bottom line
  endchar[8] = DayNames[dow][0];
  endchar[9] = DayNames[dow][1];
  endchar[10] = DayNames[dow][2];
  endchar[11] = ' ';
  endchar[12] = buffer[0];
  endchar[13] = buffer[1];
  endchar[14] = suffix[s][0];
  endchar[15] = suffix[s][1];

  uint8_t x2 = x;
  uint8_t y2 = y;

  for (uint8_t i = 0; i < 16; i++) {
    if(i == 8) {
      x2 = x;
      y2 = y + 8;
    }

    drawChar(x2, y2, endchar[i], 51, matrix.Color444(0,0,1));

    x2 += 4;
  }
}
