#ifndef PLAYER_HPP
#define PLAYER_HPP

class Field;

class Player
{
public:
	Field* field;

	signed short timerCounter;

	/* Keyboard / Gamepad ID */
	signed char INPUT;

	/* Start position / Old position */
	short posX;

	bool gameOver;
	unsigned int score;

	/* Player color */
	Color color;

	/* Piece type */
	char type;
	char nextType;

	/* For correct I rotation */
	char facing;

	/* Control aux */
	bool down;
	unsigned char counter;
	unsigned char speed;
	unsigned char speedDown;
	signed char dir;

	struct COORD
	{
		short x;
		short y;
	}t[4], tGhost[4], tOld[4];

	/* Control */
	bool DOWN, LEFT, RIGHT, START;
	bool DROP, ROTATE_LEFT, ROTATE_RIGHT;

	Player(Field* field, signed char INPUT, short x);

	void Update(void);
	void Control(void);
	char CheckCollision(short x, short y);
	bool Move(short x, short y);
	bool Rotate(char dir);
	void GetTetromino(void);
	void Put(void);
	void AddScore(unsigned char lines);
	void Drop(void);
	void GameOver(void);
	void MakeGhost(void);
};

#endif