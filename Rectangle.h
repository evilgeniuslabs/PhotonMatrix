struct Rectangle {
	float left;
	float right;
	float top;
	float bottom;

	float width;
	float height;

	float speedX;
	float speedY;

	uint16_t color;

	bool inActive = false;

	void setLeft(float newLeft) {
		left = newLeft;
		right = left + (width - 1);
	}

	void setRight(float newRight) {
		right = newRight;
		left = right - (width - 1);
	}

	void setTop(float y) {
		top = y;
		bottom = top + (height - 1);
	}

	void setBottom(float newBottom) {
		bottom = newBottom;
		top = bottom - (height - 1);
	}

	bool intersectsWith(Rectangle rect) {
		return (rect.left <= right + 1 && rect.right >= left - 1 && rect.top <= bottom) && rect.bottom >= top - 1;
	}
};
