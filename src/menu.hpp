#ifndef MENU_HPP
#define MENU_HPP

class Core;

class Menu
{
public:
	Menu(Core* core);

	void Update(void);
	void Draw(void);

	void Control(void);

	bool UP, LEFT, RIGHT, DOWN;
	bool OK, BACK;

	Core* core;

	/* Aux */
	bool title;
	unsigned char flash;
	
	unsigned char musicSelection;
	unsigned char levelSelection;

	unsigned short widthSelection;
	unsigned short heightSelection;

	unsigned char subMenu;
};

#endif