#include "menu.hpp"
#include "core.hpp"
#include "field.hpp"

Menu::Menu(Core* core) : core(core)
{
	title = true;
	flash = 0;
	musicSelection = 0;
	levelSelection = 0;

	widthSelection = 10;
	heightSelection = 20;

	/*0: Music, 1: Level, 2: Width, 3: Height, */
	subMenu = 0;
}

void Menu::Update(void)
{
	if (flash) flash--;
	else flash = 3;

	Control();

	/* Tittle screen */
	if ((OK) and (title))
	{
		PlaySound(core->sfx[7]);
		core->SetMusic(1);
		title = false;
		OK = false;
	}

	/* Next / Previous submenu */
	if (OK)
	{
		if (subMenu < 3) {subMenu++; PlaySound(core->sfx[7]);}
		else
		{
			/* START GAME !!! */
			PlaySound(core->sfx[7]);
			core->StartGame(widthSelection, heightSelection, musicSelection, levelSelection);

			return;
		}
	}
	else if ((BACK) and (!title))
	{
		if (subMenu > 0) {subMenu--; PlaySound(core->sfx[1]);}
		else PlaySound(core->sfx[3]);
	}

	switch (subMenu)
	{
		/* Music selection */
		case 0:
		{
			bool musicChange = false;

			if (UP)
			{
				if (musicSelection > 0) {musicSelection--; PlaySound(core->sfx[8]); musicChange = true;}
				else PlaySound(core->sfx[3]);
			}
			else if (DOWN)
			{
				if (musicSelection < 3) {musicSelection++; PlaySound(core->sfx[8]); musicChange = true;}
				else PlaySound(core->sfx[3]);		
			}

			/* Update preview music */
			if (musicChange)
			{
				switch (musicSelection)
				{
					case 0: core->SetMusic(1); break;
					case 1: core->SetMusic(2); break;
					case 2: core->SetMusic(3); break;
					case 3: core->SetMusic(0); break;
				}
			}
		}break;

		/* Level selection */
		case 1:
		{
			if (RIGHT)
			{
				if (levelSelection < 9) {levelSelection++; PlaySound(core->sfx[8]);}
				else PlaySound(core->sfx[3]);
			}else if (LEFT)
			{
				if (levelSelection > 0) {levelSelection--; PlaySound(core->sfx[8]);}
				else PlaySound(core->sfx[3]);	
			}else if (DOWN)
			{
				if ((levelSelection + 5) <= 9) {levelSelection += 5; PlaySound(core->sfx[8]);}
				else PlaySound(core->sfx[3]);	
			}else if (UP)
			{
				if ((levelSelection - 5) >= 0) {levelSelection -= 5; PlaySound(core->sfx[8]);}
				else PlaySound(core->sfx[3]);	
			}
		}break;
	}

	if ((subMenu == 2) or (subMenu == 3))
	{
		unsigned short* SIZE;

		switch (subMenu)
		{
			case 2: SIZE = &widthSelection; break;
			case 3: SIZE = &heightSelection; break;
		}

		if (RIGHT)
		{
			if ((*SIZE) < 255) {(*SIZE)++; PlaySound(core->sfx[8]);}
			else PlaySound(core->sfx[3]);
		} else if (LEFT)
		{
			if ((*SIZE) > 5) {(*SIZE)--; PlaySound(core->sfx[8]);}
			else PlaySound(core->sfx[3]);
		}
	}
}

void Menu::Draw(void)
{
	Color color;
	
	if (title)
		DrawTexture(core->titleTexture, 0, 0, WHITE);
	else
	{
		DrawTexture(core->setupTexture, 0, 0, WHITE);

		/* Music selection */
		unsigned short posy;
		switch (musicSelection)
		{
			case 0: posy = 59; break;
			case 1: posy = 75; break;
			case 2: posy = 90; break;
			case 3: posy = 106; break;
			default: posy = 0;
		}

		if (subMenu == 0) color = Color{234, 158, 34, 255};
		else color = WHITE;

		if (flash)
			{DrawTexturePro(core->texture, Rectangle{32, 0, 8, 8}, Rectangle{133, (float)posy, 8, 8}, Vector2{0, 0}, 0, color);
			DrawTexturePro(core->texture, Rectangle{32, 0, -8, 8}, Rectangle{207, (float)posy, 8, 8}, Vector2{0, 0}, 0, color);}

		/* Level selection */
		if (subMenu == 1) color = Color{234, 158, 34, 255};
		else color = WHITE;

		if (flash)
			DrawRectangle(((levelSelection % 5) * 16) + 36, ((levelSelection >= 5 ? 1 : 0) * 16) + 70, 16, 16, color);

		for (unsigned char i = 0; i <= 4; i++)
		{
			DrawTextEx(core->font, TextFormat("%u", i), Vector2{(float)(16 * i) + 41, 75}, 8, 0, Color{181, 49, 32, 255});
			DrawTextEx(core->font, TextFormat("%u", i + 5), Vector2{(float)(16 * i) + 41, 91}, 8, 0, Color{181, 49, 32, 255});
		}

		/* WIDTH selection */
		if (subMenu == 2) color = Color{234, 158, 34, 255};
		else color = WHITE;

		DrawTextEx(core->font, TextFormat("%03u", widthSelection), Vector2{84, 170}, 8, 0, WHITE);

		if (flash)
			{DrawTexturePro(core->texture, Rectangle{32, 0, 8, 8}, Rectangle{72, 170, 8, 8}, Vector2{0, 0}, 0, color);
			DrawTexturePro(core->texture, Rectangle{32, 0, -8, 8}, Rectangle{111, 170, 8, 8}, Vector2{0, 0}, 0, color);}


		/* HEIGHT selection */
		if (subMenu == 3) color = Color{234, 158, 34, 255};
		else color = WHITE;

		DrawTextEx(core->font, TextFormat("%03u", heightSelection), Vector2{152, 170}, 8, 0, WHITE);

		if (flash)
			{DrawTexturePro(core->texture, Rectangle{32, 0, 8, 8}, Rectangle{140, 170, 8, 8}, Vector2{0, 0}, 0, color);
			DrawTexturePro(core->texture, Rectangle{32, 0, -8, 8}, Rectangle{179, 170, 8, 8}, Vector2{0, 0}, 0, color);}
	}
}

void Menu::Control(void)
{
	UP = false; LEFT = false; RIGHT = false;
	DOWN = false; OK = false; BACK = false;

	if (IsKeyPressed(KEY_D)) RIGHT = true;
	if (IsKeyPressed(KEY_A)) LEFT = true;
	if (IsKeyPressed(KEY_S)) DOWN = true;
	if (IsKeyPressed(KEY_W)) UP = true;
	if (IsKeyPressed(KEY_ENTER)) OK = true;
	if (IsKeyPressed(KEY_ESCAPE)) BACK = true;

	if (IsKeyPressed(KEY_RIGHT)) RIGHT = true;
	if (IsKeyPressed(KEY_LEFT)) LEFT = true;
	if (IsKeyPressed(KEY_DOWN)) DOWN = true;
	if (IsKeyPressed(KEY_UP)) UP = true;
	if (IsKeyPressed(KEY_BACKSPACE)) BACK = true;

	for (unsigned char i = 0; i < 4; i++)
	{
		if (IsGamepadAvailable(i))
		{
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) OK = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_MIDDLE_RIGHT)) OK = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) RIGHT = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) LEFT = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_LEFT_FACE_UP)) UP = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) DOWN = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) BACK = true;
		}
	}
}