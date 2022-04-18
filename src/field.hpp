#ifndef FIELD_HPP
#define FIELD_HPP

class Core;
class Player;

class Field
{
public:
	Core* core;

	unsigned short width;
	unsigned short height;

	unsigned char* block;
	Color color1;
	Color color2;

	RenderTexture renderField;
	RenderTexture renderPlayer;

	unsigned char playerNum;
	Player* player;
	Color playerColor[4];

	unsigned char keyboardPlayerNum;

	int lines;
	unsigned char speed;

	unsigned char MUSIC_SELECTION;
	int startingLevel;
	unsigned short endGameTimeout;
	bool endGame;
	bool gamePaused;

	Field(Core* core, unsigned short width, unsigned short height);
	~Field();

	void Update(void);
	void CheckNewPlayer(void);
	void RedrawField(void);
	void RedrawPlayer(void);
	void RedrawGui(void);
	void DrawNextPiece(short x, short y, unsigned char type, Color color);
	void UpdateColors(void);
	unsigned char UpdateLines(void);
	void GetSpeed(void);
	void UpdateLevel(void);
	bool CheckGameOver(void);
};

#endif