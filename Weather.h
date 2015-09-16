unsigned long lastWeatherSync = 0;
boolean weatherReceived = false;

char weatherTemps[8][7];
char weatherTypes[8][4];

void gotWeatherData(const char *name, const char *data){
  Serial.println("Got weather data:");

  Serial.println(data);

  // we'll get data like this:
  // 82.85~800~84.22~500~70.48~800~66.09~800~78.67~800~85.84~800~85.05~800~
  // which is seven sets of temp and weather types, seperated by ~
  // so the temp for the first day (today) is 82.85
  // and the weather type is 800

	int stringLength = strlen((const char *)data);

  Serial.print("Weather data length:");
  Serial.println(stringLength);

	memset(&weatherTemps,0,8*7);
	memset(&weatherTypes,0,8*4);

	int dayCounter = 0;
	int itemCounter = 0;
	int tempStringLoc = 0;
	boolean dropChar = false;

  Serial.println("Parsing weather data...");

	for (int i = 0; i < stringLength; i++) {
    // Serial.print("i: ");
    // Serial.println(i);
    //
    // Serial.print("itemCounter: ");
    // Serial.println(itemCounter);
    //
    // Serial.print("tempStringLoc: ");
    // Serial.println(tempStringLoc);
    //
    // Serial.print("dayCounter: ");
    // Serial.println(dayCounter);
    //
    // Serial.print("dropChar: ");
    // Serial.println(dropChar);

		if(data[i]=='~'){
			itemCounter++;
			tempStringLoc = 0;
			dropChar = false;
			if(itemCounter > 1){
				dayCounter++;
				itemCounter=0;
			}
		}
		else if(data[i] == '.' || data[i] == '"') {
			//if we get a . we want to drop all characters until the next ~
			dropChar = true;
		}
		else{
			if(!dropChar) {
				switch(itemCounter) {
				case 0:
          Serial.print("Temperature data: ");
          Serial.println(data[i]);
					weatherTemps[dayCounter][tempStringLoc++] = data[i];
					break;
				case 1:
          Serial.print("Weather type data: ");
          Serial.println(data[i]);
					weatherTypes[dayCounter][tempStringLoc++] = data[i];
					break;
				}
			}
		}
	}

  Serial.print("Finished parsing weather data.");

	lastWeatherSync = millis();
	weatherReceived = true;
}

int rain[12];

void drawWeatherIcon(uint8_t x, uint8_t y, int id){
	int rainColor = matrix.Color333(0,0,1);

  byte intensity = id - (id / 10) * 10 + 1;

  static uint8_t currentFrameIndex = 0;

  // millis the last time we switched frames
  static unsigned long lastFrameSwitch = 0;

  unsigned long frameDelay = (50 -( intensity * 10 )) < 0 ? 0 : 50 - intensity * 10;

  // show the next frame after 2 seconds
  if(lastFrameSwitch == 0 || millis() > lastFrameSwitch + frameDelay) {
    currentFrameIndex++;
		if(currentFrameIndex > 6) currentFrameIndex = 0;

    lastFrameSwitch = millis();

    for(int r = 0; r < 12; r++) {
       rain[r] = random(9, 15);
    }
  }

  uint8_t deep = currentFrameIndex;
	boolean raining = false;

  switch(id / 100) {
		case 2:
			//Thunder
			matrix.fillRect(x, y, 16, 16, matrix.Color333(0, 0, 0));
			matrix.drawBitmap(x, y, cloud_outline, 16, 16, matrix.Color333(1, 1, 1));
			if(random(0, 10) == 3) {
				int pos = random(-5, 5);
				matrix.drawBitmap(pos + x, y, lightning, 16, 16, matrix.Color333(1, 1, 1));
			}
			raining = true;
			break;
		case 3:
			//drizzle
			matrix.fillRect(x, y, 16, 16, matrix.Color333(0, 0, 0));
			matrix.drawBitmap(x, y, cloud, 16, 16, matrix.Color333(1, 1, 1));
			raining = true;
			break;
		case 5:
			//rain was 5
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));

			if(intensity < 3){
				matrix.drawBitmap(x, y, cloud, 16, 16, matrix.Color333(1, 1, 1));
			}
			else{
				matrix.drawBitmap(x, y, cloud_outline, 16, 16, matrix.Color333(1, 1, 1));
			}
			raining = true;
			break;
		case 6:
			//snow was 6
			rainColor = matrix.Color333(4, 4, 4);
			matrix.fillRect(x, y, 16, 16, matrix.Color333(0, 0, 0));

			if(intensity < 3){
				matrix.drawBitmap(x, y, cloud, 16, 16, matrix.Color333(1, 1, 1));
				matrix.fillRect(x, y + 16 - deep / 2, 16, deep / 2, rainColor);
			}
			else{
				matrix.drawBitmap(x,y,cloud_outline,16,16,matrix.Color333(1,1,1));
				matrix.fillRect(x,y+16-(deep),16,deep,rainColor);
			}
			raining = true;
			break;
		case 7:
			//atmosphere
			matrix.drawRect(x,y,16,16,matrix.Color333(1,0,0));
			drawString(x+2,y+6,(char*)"FOG",51,matrix.Color333(1,1,1));
			break;
		case 8:
			//cloud
			matrix.fillRect(x, y + 1, 15, 15, matrix.Color333(0,0,1));

			if(id == 800) {
				matrix.drawBitmap(x,y,big_sun,16,16,matrix.Color333(2,2,0));
			}
			else{
				if(id==801){
					matrix.drawBitmap(x,y,big_sun,16,16,matrix.Color333(2,2,0));
					matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
				}
				else{
					if(id==802 || id ==803){
						matrix.drawBitmap(x,y,small_sun,16,16,matrix.Color333(1,1,0));
					}
					matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
					matrix.drawBitmap(x,y,cloud_outline,16,16,matrix.Color333(0,0,0));
				}
			}
			break;
		case 9:
			//extreme
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));
			matrix.drawRect(x,y,16,16,matrix.Color333(7,0,0));
			if(id==906){
				raining =true;
				intensity=3;
				matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
			};
			break;
		default:
			matrix.fillRect(x, y + 1, 15, 15, matrix.Color333(0, 1, 1));
			matrix.drawBitmap(x, y, big_sun, 16, 16, matrix.Color333(2, 2, 0));
			break;
  }

	if(raining){
		for(int r = 0; r<13; r++){
			matrix.drawPixel(x+r+2, rain[r]++, rainColor);
			// if(rain[r]==16) rain[r]=9;
			//if(rain[r]==20) rain[r]=9;
		}
  }
}

void drawWeather(){
  if(!weatherReceived) {
  	drawString(1, 1, "Getting", 51, Navy);
    drawString(1, 7, "Weather", 51, Navy);

    return;
  }

	byte dow = Time.weekday() - 1;

  static uint8_t currentDayIndex = 0;

  // millis the last time we switched days
  static unsigned long lastDaySwitch = 0;

  // show the next day after 2 seconds
  if(millis() > lastDaySwitch + 2000) {
    lastDaySwitch = millis();
    currentDayIndex++;
    if(currentDayIndex >= 7) currentDayIndex = 0;
  }

	int numTemp = atoi(weatherTemps[currentDayIndex]);

	//fix within range to generate colour value
	if (numTemp<-14) numTemp=-10;
	if (numTemp>34) numTemp =30;
	//add 14 so it falls between 0 and 48
	numTemp = numTemp +14;
	//divide by 3 so value between 0 and 16
	numTemp = numTemp / 3;

	int tempColor;
	if(numTemp < 8){
		tempColor = matrix.Color444(0, tempColor / 2, 7);
	}
	else{
		tempColor = matrix.Color444(7, 7 - numTemp / 2, 0);
	}

	//Display the day on the top line.
	if(currentDayIndex == 0){
		drawString(2, 2, (char*)"Now", 51, matrix.Color444(1, 1, 1));
	}
	else{
		drawString(2, 2, DayNames[(dow + currentDayIndex) % 7], 51, matrix.Color444(0, 1, 0));
	}

	//put the temp underneath
	boolean positive = !(weatherTemps[currentDayIndex][0] == '-');
	for(int t = 0; t < 7; t++) {
		if(weatherTemps[currentDayIndex][t] == '-') {
			matrix.drawLine(3, 10, 4, 10, tempColor);
		}
		else if(!(weatherTemps[currentDayIndex][t] == 0)) {
			vectorNumber(weatherTemps[currentDayIndex][t] - '0', t * 4 + 2 + (positive * 2), 8, tempColor, 1, 1);
		}
	}

  drawWeatherIcon(16, 0, atoi(weatherTypes[currentDayIndex]));
}
