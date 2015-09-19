class BreakoutGame : public Drawable {
private:
	Rectangle ball;
  Rectangle paddle;
  Rectangle blocks[32];

	bool ballFellOutBottom = false;
  uint8_t lives;
  uint8_t score;
  char scoreText[8];

public:
	bool isPaused;

	BreakoutGame() {
		setup();
	}

  void resetBall() {
    ball.width = 1.0;
    ball.height = 1.0;
    ball.setLeft(11.0);
    ball.setTop(18.0);
    ball.speedX = 0.500;
    ball.speedY = 1.000;
    ball.color = White;
  }

  void setup() {
    isPaused = true;

    paddle.width = 6.0;
    paddle.height = 1.0;
    paddle.setLeft(16.0);
    paddle.setTop(29.0);
    paddle.color = White;

    reset();
  }

  void reset() {
    resetBall();

    lives = 2;

    score = 0;
    sprintf(scoreText, "%d", score);

    // regenerate blocks
    generateBlocks();
  }

  void generateBlocks() {
    int colorIndex = 0;
    int index = 0;
    for (float y = 0.0; y < 4.0; y++) {
      for (float x = 0.0; x < 8.0; x++) {
        Rectangle block;
        block.width = 4.0;
        block.height = 2.0;
        block.setLeft(x * 4.0);
        block.setTop((y * 2.0) + 5);
        block.inActive = false;
        switch (colorIndex)
        {
          case 0:
            block.color = LightGrey;
            break;
          case 1:
            block.color = Red;
            break;
          case 2:
            block.color = Yellow;
            break;
          case 3:
            block.color = Blue;
            break;
        }
        blocks[index] = block;
        index++;
      }
      colorIndex++;
    }
  }

  void handleInput() {
    // update paddle positions
    float newX = paddle1PinState / (4096 / (MATRIX_WIDTH));
    if(newX > MATRIX_WIDTH - paddle.width)
      newX = MATRIX_WIDTH - paddle.width;
    paddle.speedX = paddle.left + newX;
    paddle.setLeft(newX);

		if(isPaused && paddle1Button.clicks == 1)
		{
			isPaused = false;
		}
  }

  void update() {
    bool collisionOnX = false;
    bool collisionOnY = false;

    // move the ball on the x axis
    ball.setLeft(ball.left + ball.speedX);

    // check for collisions on the x axis in the new position
    collisionOnX = ball.left <= -1 || ball.right >= MATRIX_WIDTH || ball.intersectsWith(paddle);

    bool blocksLeft = false;

    for (int index = 0; index < 32; index++) {
      Rectangle &block = blocks[index];
      if (block.inActive == true)
        continue;

      blocksLeft = true;

      if (ball.intersectsWith(block)) {
        collisionOnX = true;
        block.inActive = true;
        score++;
        sprintf(scoreText, "%d", score);
      }
    }

    // cleared the level?
    if (!blocksLeft) {
      generateBlocks();
      resetBall();
      isPaused = true;
      return;
    }

    // we're testing for collisions on each axis independently, so
    // move the ball back to the old x position
    ball.setLeft(ball.left - ball.speedX);

    // move the ball on the y axis
    ball.setTop(ball.top + ball.speedY);

    ballFellOutBottom = ball.bottom >= 31.0;


		bool ballIntersectsWithPaddle = ball.intersectsWith(paddle);

    // check for collisions on the y axis in the new position
    collisionOnY = ball.top <= 5 || ball.bottom >= MATRIX_HEIGHT - 1 || ballIntersectsWithPaddle;

    for (int index = 0; index < 32; index++) {
      Rectangle &block = blocks[index];
      if (block.inActive == true)
        continue;

      if (ball.intersectsWith(block)) {
        collisionOnY = true;
        block.inActive = true;
        score++;
        sprintf(scoreText, "%d", score);
      }
    }

    // move the ball back on the y axis
    ball.setTop(ball.top - ball.speedY);

    if (ballFellOutBottom) {
      ballFellOutBottom = false;

      isPaused = true;

      if (lives > 0) {
      	lives--;
        resetBall();
        return;
      }
      else {
        reset();
        return;
      }
    }

    // handle any collisions
    if (collisionOnX) {
      // reflect the ball on the x axis
      ball.speedX *= -1.0;
    }
    else {
    }

    if (collisionOnY) {
      // reflect the ball on the y axis
      ball.speedY *= -1.0;

			if(ballIntersectsWithPaddle)
				ball.speedX += paddle.speedX / 64;
    }
    else {
    }

    // move the ball to the new x position
    ball.setLeft(ball.left + ball.speedX);

    // move the ball to the new y position
    ball.setTop(ball.top + ball.speedY);
  }

  void draw() {
    // draw score
    // matrix.drawString(0, 0, White, scoreText);

    // draw blocks
    for (int index = 0; index < 32; index++) {
      Rectangle block = blocks[index];
      if (block.inActive == true)
        continue;
      matrix.drawRect(block.left, block.top, block.width, block.height, block.color);
    }

    // draw lives indicator
    int lx = 1;
    for (int i = 0; i < lives; i++) {
      matrix.drawPixel(0 + lx, 31, Orange);
      matrix.drawPixel(1 + lx, 31, LightGrey);
      matrix.drawPixel(2 + lx, 31, LightGrey);
      matrix.drawPixel(3 + lx, 31, Orange);

      lx += 5;
    }

    // draw ball
    matrix.drawRect(ball.left, ball.top, ball.width, ball.height, ball.color);

    // draw paddle
    matrix.drawPixel(paddle.left, paddle.top, Orange);
    for (int x = 1; x < 5; x++) {
      matrix.drawPixel(paddle.left + x, paddle.top, LightGrey);
    }
    matrix.drawPixel(paddle.right, paddle.top, Orange);

    //matrix.drawRectangle(paddle.left, paddle.top, paddle.right, paddle.bottom, paddle.color);
  }

  uint8_t drawFrame() {
    handleInput();

    if (!isPaused) {
      update();
    }

    draw();

    return 40;
  }
};
