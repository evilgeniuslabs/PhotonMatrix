#define Paddle1Pin A6
#define Paddle2Pin A5

int paddle1PinState = 0;
int paddle2PinState = 0;

#define Paddle1ButtonPin RX
#define Paddle2ButtonPin TX

int paddle1ButtonPinState = 0;
int paddle2ButtonPinState = 0;

uint8_t paddle1x = 1;
uint8_t paddle2x = 30;

uint8_t paddle1y = 15;
uint8_t paddle2y = 15;

uint8_t paddleWidth = 2;
uint8_t paddleHeight = 8;

float ballX = 15;
float ballY = 15;

float ballVx = 0.6f;
float ballVy = 0.7f;

bool paddle1ButtonPressed = false;
bool paddle2ButtonPressed = false;

uint8_t scoreY = 0;
uint8_t player1score = 0;
uint8_t player2score = 0;

bool isPaused = true;

void updatePongGame()
{
  // update ball position
  if(!isPaused)
  {
    ballX += ballVx;
    ballY += ballVy;
  }

  bool restartBall = false;

  // check for border collisions
  if(ballX < 0)
  {
    // player 2 wins
    player2score++;
    restartBall = true;
  }
  else if (ballX >= MATRIX_WIDTH)
  {
    // palyer 1 wins
    player1score++;
    restartBall = true;
  }

  if(restartBall)
  {
    ballX = 15;
    ballVx *= -1;
    isPaused = true;
    return;
  }

  bool ballCollisionX = false;
  bool ballCollisionY = false;

  if(ballY < 0)
  {
    ballY = 0;
    ballCollisionY = true;
  }
  else if (ballY >= MATRIX_HEIGHT)
  {
    ballY = MATRIX_HEIGHT - 1;
    ballCollisionY = true;
  }

  // check for paddle collisions
  if(ballX < paddle1x + paddleWidth && ballY >= paddle1y && ballY < paddle1y + paddleHeight)
  {
    ballX = paddle1x + paddleWidth;
    ballCollisionX = true;
  }

  if(ballX >= paddle2x && ballY >= paddle2y && ballY < paddle2y + paddleHeight)
  {
    ballX = paddle2x - 1;
    ballCollisionX = true;
  }

  if(ballCollisionY)
  {
    ballVy *= -1;
  }

  if(ballCollisionX)
  {
    ballVx *= -1;
  }
}

void handlePongGameInput()
{
  // read paddle rotary analog pins
  paddle1PinState = analogRead(Paddle1Pin);
  paddle2PinState = analogRead(Paddle2Pin);

  // update paddle positions
  paddle1y = paddle1PinState / (4096 / MATRIX_HEIGHT);
  paddle2y = paddle2PinState / (4096 / MATRIX_HEIGHT);

  // read paddle button digital pins
  paddle1ButtonPinState = digitalRead(Paddle1ButtonPin);
  paddle2ButtonPinState = digitalRead(Paddle2ButtonPin);

  // update paddle button states
  paddle1ButtonPressed = paddle1ButtonPinState == LOW;
  paddle2ButtonPressed = paddle2ButtonPinState == LOW;

  if(isPaused && (paddle1ButtonPressed || paddle2ButtonPressed))
  {
    isPaused = false;
  }
}

void drawPongGame()
{
  //draw pitch centre line
  for (byte i = 0; i < MATRIX_HEIGHT; i += 2) {
    matrix.drawPixel(16, i, matrix.Color333(0,4,0));
  }

  // draw paddles
  matrix.fillRect(paddle1x, paddle1y, paddleWidth, paddleHeight, paddle1ButtonPressed ? matrix.Color333(1, 1, 1) : matrix.Color333(0,0,4));
  matrix.fillRect(paddle2x, paddle2y, paddleWidth, paddleHeight, paddle2ButtonPressed ? matrix.Color333(1, 1, 1) : matrix.Color333(0,0,4));

  // draw score
	char buffer[3];

	itoa(player1score,buffer,10);
	//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
	if (player1score < 10) {
		buffer[1] = buffer[0];
		buffer[0] = '0';
	}
	vectorNumber(buffer[0]-'0',8,1 + scoreY,matrix.Color333(1,1,1),1,1);
	vectorNumber(buffer[1]-'0',12,1 + scoreY,matrix.Color333(1,1,1),1,1);

	itoa(player2score,buffer,10);
	if (player2score < 10) {
		buffer[1] = buffer[0];
		buffer[0] = '0';
	}
	vectorNumber(buffer[0]-'0',18,1 + scoreY,matrix.Color333(1,1,1),1,1);
	vectorNumber(buffer[1]-'0',22,1 + scoreY,matrix.Color333(1,1,1),1,1);

  // draw ball
  matrix.drawPixel(ballX, ballY, matrix.Color333(4, 0, 0));

}

void runPongGame()
{
  handlePongGameInput();

  updatePongGame();

  drawPongGame();
}
