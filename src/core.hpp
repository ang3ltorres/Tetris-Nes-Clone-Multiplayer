#ifndef CORE_HPP
#define CORE_HPP

class Field;
class Menu;

class Core
{
public:
	unsigned short displayWidth;
	unsigned short displayHeight;

	unsigned short gameWidth;
	unsigned short gameHeight;
	
	unsigned short fieldHeight;
	unsigned short fieldWidth;

	unsigned char scale;
	unsigned short offsetX, offsetY;

	void* F;

	RenderTexture render;
	Texture texture;

	int renderFieldPlayerYPos;
	int renderFieldPlayerXPos;

	RenderTexture background;
	Texture backgroundTexture;

	Music music;
	void* musicStream;

	Font font;

	/* SFX */
	Sound sfx[9];

	/* Menu */
	bool menuEnabled;
	Menu* menu;
	Texture titleTexture;
	Texture setupTexture;

	/**/
	Field* field;
	int level;

	/* Press start text */
	unsigned char flash;
	unsigned int pressStartColor;

	Core(void);
	~Core();

	void Loop(void);
	void Draw(void);

	void CalcScale(void);
	void CheckWindow(void);
	void SetMusic(unsigned char n);
	void GenerateBackground(void);

	void StartGame(
		unsigned short widthSelection,
		unsigned short heightSelection,

		unsigned short musicSelection,
		unsigned short levelSelection);


	void ResetGame(void);
};

#endif