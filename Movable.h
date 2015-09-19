class Movable {
public:
  Vector location;
  Vector velocity;

  Movable() {
  }

  Movable(float x, float y) {
    location.x = x;
    location.y = y;
  }

  Movable(float x, float y, float velocityX, float velocityY) {
    location.x = x;
    location.y = y;

    velocity.x = velocityX;
    velocity.y = velocityY;
  }
};
