class PongGame : public Drawable {
public:
  uint8_t paddleWidth = 2;
  uint8_t paddleHeight = 8;

  Movable ball = Movable(15, 15, 0.6, 0.7);

  uint8_t scoreY = 0;
  uint8_t player1score = 0;
  uint8_t player2score = 0;

  bool isPaused = true;

  void update()
  {
    // update ball position
    if(!isPaused)
    {
      ball.location.x += ball.velocity.x;
      ball.location.y += ball.velocity.y;
    }

    bool restartBall = false;

    int newBallDirectionX = 0;

    // check for border collisions
    if(ball.location.x < 0)
    {
      // player 2 wins
      newBallDirectionX = 1;
      player2score++;
      restartBall = true;
    }
    else if (ball.location.x >= MATRIX_WIDTH)
    {
      // player 1 wins
      newBallDirectionX = -1;
      player1score++;
      restartBall = true;
    }

    if(restartBall)
    {
      if(player1score == 10 || player2score == 10)
      {
        player1score = 0;
        player2score = 0;
      }
      ball.location.x = 15;
      ball.velocity.x = 0.6f * newBallDirectionX;
      ball.velocity.y = 0.7f;
      isPaused = true;
      return;
    }

    if(ball.location.y < 0)
    {
      ball.location.y = 0;
        ball.velocity.y *= -1;
    }
    else if (ball.location.y >= MATRIX_HEIGHT)
    {
      ball.location.y = MATRIX_HEIGHT - 1;
      ball.velocity.y *= -1;
    }

    // check for paddle collisions
    if(ball.location.x < paddle1.location.x + paddleWidth && ball.location.y >= paddle1.location.y && ball.location.y < paddle1.location.y + paddleHeight)
    {
      ball.location.x = paddle1.location.x + paddleWidth;
      ball.velocity.x *= -1.0f;
      ball.velocity.y += paddle1.velocity.y / 64;
      // ball.velocity.x -= paddle1.velocity.y / 4;
    }
    else if(ball.location.x >= paddle2.location.x && ball.location.y >= paddle2.location.y && ball.location.y < paddle2.location.y + paddleHeight)
    {
      ball.location.x = paddle2.location.x - 1;
      ball.velocity.x *= -1.0f;
      ball.velocity.y += paddle2.velocity.y / 64;
      // ball.velocity.x -= paddle2.velocity.y / 4;
    }
  }

  void handleInput()
  {
    if(isPaused && (paddle1Button.clicks == 1 || paddle2Button.clicks == 1))
    {
      isPaused = false;
    }

    // update paddle positions
    float newY = paddle1PinState / (4096 / (MATRIX_HEIGHT));
    if(newY > MATRIX_HEIGHT - paddleHeight)
      newY = MATRIX_HEIGHT - paddleHeight;
    paddle1.velocity.y = paddle1.location.y + newY;
    paddle1.location.y = newY;

    newY = paddle2PinState / (4096 / (MATRIX_HEIGHT));
    if(newY > MATRIX_HEIGHT - paddleHeight)
      newY = MATRIX_HEIGHT - paddleHeight;
    paddle2.velocity.y = paddle2.location.y + newY;
    paddle2.location.y = newY;
  }

  void draw()
  {
    //draw pitch centre line
    for (byte i = 0; i < MATRIX_HEIGHT; i += 2) {
      matrix.drawPixel(16, i, matrix.Color333(0,4,0));
    }

    // draw paddles
    matrix.fillRect(paddle1.location.x, paddle1.location.y, paddleWidth, paddleHeight, paddle1Button.clicks != 0 ? matrix.Color333(1, 1, 1) : matrix.Color333(0,0,4));
    matrix.fillRect(paddle2.location.x, paddle2.location.y, paddleWidth, paddleHeight, paddle2Button.clicks != 0 ? matrix.Color333(1, 1, 1) : matrix.Color333(0,0,4));

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
    matrix.drawPixel(ball.location.x, ball.location.y, matrix.Color333(4, 0, 0));

  }

  uint8_t drawFrame()
  {
    handleInput();

    update();

    draw();

    return 40;
  }
};
