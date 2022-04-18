#include "field.hpp"

#include "player.hpp"
#include "core.hpp"

#include <cstring>
#include <cstdlib>

Field::Field(Core* core, unsigned short width, unsigned short height) : core(core), width(width), height(height)
{
	block = (unsigned char*)::operator new(sizeof(unsigned char) * (width * height));
	memset(block, 0, sizeof(unsigned char) * (width * height));

	renderField = LoadRenderTexture(width * 8, height * 8);
	renderPlayer = LoadRenderTexture(width * 8, height * 8);

	playerNum = 0;
	player = (Player*)::operator new(sizeof(Player) * 4);

	/* Colors */
	playerColor[0] = Color{255, 255, 0, 255}; /* Yellow */
	playerColor[1] = Color{255, 0, 0, 255}; /* Red */
	playerColor[2] = Color{0, 0, 255, 255}; /* Blue */
	playerColor[3] = Color{0, 255, 0, 255}; /* Green */

	/* Mix colors */
	for (unsigned short i = 0; i < 4; i++)
		std::swap(playerColor[i], playerColor[rand() % 4]);

	keyboardPlayerNum = 0;
	lines = 0;

	UpdateColors();
	GetSpeed();

	/* Misc */
	core->SetMusic(0);
	MUSIC_SELECTION = 0; // Default
	startingLevel = core->level;
	endGameTimeout = 180;
	endGame = false;
	gamePaused = false;
}

Field::~Field()
{
	::operator delete(block);
	::operator delete(player);
	UnloadRenderTexture(renderField);
	UnloadRenderTexture(renderPlayer);
}

void Field::Update(void)
{
	if (endGame)
	{
		if (endGameTimeout > 0)
			endGameTimeout--;
		else
		{
			core->ResetGame();
			return;
		}
	}
	else
	{
		for (unsigned char i = 0; i < playerNum; i++)
		{
			if (!player[i].gameOver)
				player[i].Update();
		}

		if ((endGame = CheckGameOver()) == 1)
			core->SetMusic(0);
		else
			CheckNewPlayer();
	}
}

void Field::CheckNewPlayer(void)
{
	signed char INPUT = -1;
	bool START = false;

	if (playerNum >= 4)
		return;

	/* Add Keyboard player */
	if (IsKeyPressed(KEY_ENTER))
	{
		if (keyboardPlayerNum == 0)
		{
			keyboardPlayerNum++;
			INPUT = 'A';
		}
		else if (keyboardPlayerNum == 1)
		{
			keyboardPlayerNum++;
			INPUT = 'B';
		}
	}

	/* Add Gamepad player */

	for (unsigned char i = 0; i < 4; i++)
	{
		if (IsGamepadAvailable(i))
		{
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_MIDDLE_RIGHT)) START = true;
			if (IsGamepadButtonPressed(i, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) START = true;

			if (START)
			{
				for (unsigned char j = 0; j < playerNum; j++)
					if (player[j].INPUT == i) return;

				INPUT = i;
				break;
			}
		}
	}

	/* */
	if (INPUT != -1)
	{
		if (playerNum == 0)
			core->SetMusic(MUSIC_SELECTION);

		new(&player[playerNum]) Player(this, INPUT, (width / 2) - 1);
		playerNum++;

		RedrawPlayer();
		RedrawGui();
	}
}

void Field::RedrawField(void)
{
	unsigned char b;
	/* 0: Empty, 1: B1, 2: B2, 3: B3, 4: B4*/

	BeginTextureMode(renderField);
	ClearBackground(BLACK);

		for (unsigned short y = 0; y < height; y++)
		for (unsigned short x = 0; x < width; x++)
		{
			b = block[x + (y * width)];
			
			if (b)
				DrawRectangle(x*8, y*8, 8, 8, WHITE);

			switch (b)
			{
				case 1: DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x)*8, float(y)*8}, color1); break;
				case 2: DrawTextureRec(core->texture, Rectangle{8, 0, 8, 8}, Vector2{float(x)*8, float(y)*8}, color2); break;
				case 3: DrawTextureRec(core->texture, Rectangle{8, 0, 8, 8}, Vector2{float(x)*8, float(y)*8}, color1); break;
				case 4: DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x)*8, float(y)*8}, color2); break;
				default: break;
			}
		}

	EndTextureMode();
}

void Field::RedrawPlayer(void)
{
	BeginTextureMode(renderPlayer);
	ClearBackground(BLANK);

		for (unsigned char i = 0; i < playerNum; i++)
		{
			if (!player[i].gameOver)
			{
				player[i].MakeGhost();

				for (unsigned char j = 0; j < 4; j++)
					DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(player[i].tGhost[j].x)*8, float(player[i].tGhost[j].y)*8}, WHITE);

				for (unsigned char j = 0; j < 4; j++)
					DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(player[i].t[j].x)*8, float(player[i].t[j].y)*8}, player[i].color);
			}
		}

	EndTextureMode();
}

void Field::RedrawGui(void)
{
	BeginTextureMode(core->background);

		if (playerNum >= 1)
		{
			/* Score */
			DrawRectangle(8, 8, 8*7, 16, BLACK);
			DrawTextEx(core->font, "P1", Vector2{8, 8}, 8, 0, WHITE);
			DrawTextEx(core->font, TextFormat("%07u", player[0].score), Vector2{8, 16}, 8, 0, WHITE);

			/* Next piece */
			DrawRectangle(core->renderFieldPlayerXPos - 40, core->renderFieldPlayerYPos - 8, 32, 24, BLACK);
			DrawNextPiece(core->renderFieldPlayerXPos - 40, core->renderFieldPlayerYPos - 8, player[0].nextType, player[0].color);
		}

		if (playerNum >= 2)
		{
			/* Score */
			DrawRectangle(core->gameWidth - (8*8), 8, 8*7, 16, BLACK);
			DrawTextEx(core->font, "P2", Vector2{(float)core->gameWidth - (8*8), 8}, 8, 0, WHITE);
			DrawTextEx(core->font, TextFormat("%07u", player[1].score), Vector2{(float)core->gameWidth - (8*8), 16}, 8, 0, WHITE);

			/* Next piece */
			DrawRectangle(core->renderFieldPlayerXPos + ((width + 1) * 8), core->renderFieldPlayerYPos - 8, 32, 24, BLACK);
			DrawNextPiece(core->renderFieldPlayerXPos + ((width + 1) * 8), core->renderFieldPlayerYPos - 8, player[1].nextType, player[1].color);
		}

		if (playerNum >= 3)
		{
			/* Score */
			DrawRectangle(8, core->gameHeight - (8*3), 8*7, 16, BLACK);
			DrawTextEx(core->font, "P3", Vector2{8, (float)core->gameHeight - (8*3)}, 8, 0, WHITE);
			DrawTextEx(core->font, TextFormat("%07u", player[2].score), Vector2{8, (float)core->gameHeight - (8*2)}, 8, 0, WHITE);			

			/* Next piece */
			DrawRectangle(core->renderFieldPlayerXPos - 40, (core->renderFieldPlayerYPos + ((height + 1) * 8)) - 24, 32, 24, BLACK);
			DrawNextPiece(core->renderFieldPlayerXPos - 40, (core->renderFieldPlayerYPos + ((height + 1) * 8)) - 24, player[2].nextType, player[2].color);
		}

		if (playerNum >= 4)
		{
			/* Score */
			DrawRectangle(core->gameWidth - (8*8), core->gameHeight - (8*3), 8*7, 16, BLACK);
			DrawTextEx(core->font, "P4", Vector2{(float)core->gameWidth - (8*8), (float)core->gameHeight - (8*3)}, 8, 0, WHITE);
			DrawTextEx(core->font, TextFormat("%07u", player[3].score), Vector2{(float)core->gameWidth - (8*8), (float)core->gameHeight - (8*2)}, 8, 0, WHITE);

			/* Next piece */
			DrawRectangle(core->renderFieldPlayerXPos + ((width + 1) * 8), (core->renderFieldPlayerYPos + ((height + 1) * 8)) - 24, 32, 24, BLACK);
			DrawNextPiece(core->renderFieldPlayerXPos + ((width + 1) * 8), (core->renderFieldPlayerYPos + ((height + 1) * 8)) - 24, player[3].nextType, player[3].color);
		}

		DrawRectangle((core->gameWidth - (8*7)) / 2, core->gameHeight - (8*3), 8*7, 16, BLACK);
		DrawTextEx(core->font, "LINES", Vector2{(float)(core->gameWidth - (8*7)) / 2, (float)core->gameHeight - (8*3)}, 8, 0, WHITE);
		DrawTextEx(core->font, TextFormat("%07u", lines), Vector2{(float)(core->gameWidth - (8*7)) / 2, (float)core->gameHeight - (8*2)}, 8, 0, WHITE);

		DrawRectangle((core->gameWidth - (8*3)) / 2, 8, 8*3, 16, BLACK);
		DrawTextEx(core->font, "LVL", Vector2{(float)(core->gameWidth - (8*3)) / 2, 8}, 8, 0, WHITE);
		DrawTextEx(core->font, TextFormat("%03i", core->level), Vector2{(float)((core->gameWidth - (8*3)) / 2), 16}, 8, 0, WHITE);

	EndTextureMode();
}

void Field::DrawNextPiece(short x, short y, unsigned char type, Color color)
{
	switch (type)
	{
		case 0:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 0), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 24), float(y + 8)}, color);
		}break;

		case 1:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 16)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 16)}, color);
		}break;

		case 2:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 0)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 16)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 8)}, color);
		}break;

		case 3:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 0)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 16)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 16)}, color);
		}break;

		case 4:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 0)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 16)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 16)}, color);
		}break;

		case 5:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 0)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 16)}, color);
		}break;

		case 6:
		{
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 0)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 16), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 8)}, color);
			DrawTextureRec(core->texture, Rectangle{0, 0, 8, 8}, Vector2{float(x + 8), float(y + 16)}, color);
		}break;

		default: return;
	}
}

void Field::UpdateColors(void)
{
	unsigned char level = core->level % 10;

	switch (level)
	{
		case 0: color1 = Color{66, 66, 255, 255}; color2 = Color{99, 173, 255, 255}; break;
		case 1: color1 = Color{16, 148, 0, 255}; color2 = Color{140, 214, 0, 255}; break;
		case 2: color1 = Color{156, 24, 206, 255}; color2 = Color{239, 107, 255, 255}; break;
		case 3: color1 = Color{66, 66, 255, 255}; color2 = Color{90, 231, 49, 255}; break;
		case 4: color1 = Color{181, 33, 123, 255}; color2 = Color{66, 222, 132, 255}; break;
		case 5: color1 = Color{66, 222, 132, 255}; color2 = Color{148, 148, 255, 255}; break;
		case 6: color1 = Color{160, 35, 30, 255}; color2 = Color{80, 80, 80, 255}; break;
		case 7: color1 = Color{115, 41, 255, 255}; color2 = Color{107, 0, 66, 255}; break;
		case 8: color1 = Color{66, 66, 255, 255}; color2 = Color{181, 49, 33, 255}; break;
		case 9: color1 = Color{181, 49, 33, 255}; color2 = Color{231, 156, 33, 255}; break;
	}
}

unsigned char Field::UpdateLines(void)
{
	unsigned char b;
	bool clear;
	unsigned short line = height - 1;

	unsigned char LINES = 0;

	while (true)
	{
		clear = true;

		for (unsigned short x = 0; x < width; x++)
		{
			b = block[x + (line * width)];
			if (b == 0)
			{
				clear = false;

				if (line > 0) line--;
				else return LINES;

				break;
			}
		}

		if (clear)
		{
			for (unsigned short y = line; y > 0; y--)
			for (unsigned short x = 0; x < width; x++)
				block[x + (y * width)] = block[x + ((y - 1) * width)];

			/* Clear first line */
			memset(block, 0, sizeof(unsigned char) * width);

			LINES++;
		}
	}
}

void Field::GetSpeed(void)
{
	unsigned char L = core->level;

	switch (L)
	{
		case 0: speed = 48; return;
		case 1: speed = 43; return;
		case 2: speed = 38; return;
		case 3: speed = 33; return;
		case 4: speed = 28; return;
		case 5: speed = 23; return;
		case 6: speed = 18; return;
		case 7: speed = 13; return;
		case 8: speed = 8; return;
		case 9: speed = 6; return;
		default: break;
	}

	if ((L >= 10) and (L <= 12)) speed = 5;
	else if ((L >= 13) and (L <= 15)) speed = 4;
	else if ((L >= 16) and (L <= 18)) speed = 3;
	else if ((L >= 19) and (L <= 28)) speed = 2;
	else if ((L >= 29) and (L <= 39)) speed = 1;
	else speed = 0;
}

void Field::UpdateLevel(void)
{
	/*fixme*/
	int requiredLines = ((core->level + 1) * 10) - (startingLevel * 10);

	if (lines >= requiredLines)
	{
		core->level++;
		UpdateColors();
		GetSpeed();
		PlaySound(core->sfx[2]);
	}
}

bool Field::CheckGameOver(void)
{
	if (playerNum == 0)
		return false;

	for (unsigned char i = 0; i < playerNum; i++)
		if (!player[i].gameOver) return false;
	
	return true;
}