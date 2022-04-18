#include "player.hpp"

#include "field.hpp"
#include "core.hpp"

#include <cstring>
#include <cstdlib>

Player::Player(Field* field, signed char INPUT, short x)
: field(field), INPUT(INPUT), posX(x), gameOver(false), score(0)
{
	timerCounter = 0;
	color = field->playerColor[field->playerNum];

	nextType = rand() % 7;
	GetTetromino();

	/* Control aux */
	down = false;
	counter = 0;
	speed = 0;
	speedDown = 0;
	dir = 0;
}

void Player::Update(void)
{
	Control();

	/* Pause game */
	if (START)
	{
		if (!field->gamePaused)
		{
			PauseMusicStream(field->core->music);
			PlaySound(field->core->sfx[7]);
		}
		else
			ResumeMusicStream(field->core->music);

		field->gamePaused = !field->gamePaused;
	}

	/* Player actions */

	if (!field->gamePaused)
	{
		/*** Direction to move / stop if changed ***/
		if (RIGHT)
		{
			if (dir != 1)
			{
				down = false;
				counter = 0;
				speed = 0;
			}
			dir = 1;
		}
		else if (LEFT)
		{
			if (dir != -1)
			{
				down = false;
				counter = 0;
				speed = 0;
			}
			 dir = -1;
		}

		/*** Move left / right, hold button ***/
		if (RIGHT or LEFT)
		{
			if (!down)
			{
				down = true;

				if (Move(dir, 0))
					PlaySound(field->core->sfx[0]);
			}
			else
			{
				if (counter >= 15)
				{
					if (speed >= 4)
					{
						speed = 0;

						if (Move(dir, 0))
							PlaySound(field->core->sfx[0]);
					}
					else speed++;
				}
				else counter++;
			}
		}
		else
		{
			down = false;
			counter = 0;
			speed = 0;
		}

		/*** Move piece down ***/
		if (DOWN)
		{
			if (speedDown >= 1)
			{
				speedDown = 0;

				Move(0, 1);
			}
			else speedDown++;
		}else speedDown = 0;

		/*** Rotations ***/
		if (ROTATE_LEFT)
		{
			if (Rotate('L'))
				PlaySound(field->core->sfx[1]);
		}

		if (ROTATE_RIGHT)
		{
			if (Rotate('R'))
				PlaySound(field->core->sfx[1]);
		}

		if (DROP) Drop();

		/* Gravity */
		if (timerCounter >= field->speed)
		{
			timerCounter = 0;

			if (!Move(0, 1))
				Put();
		}
		else
			timerCounter++;
	}
}

void Player::Control(void)
{

	DOWN = false; LEFT = false; RIGHT = false; START = false;
	DROP = false; ROTATE_LEFT = false; ROTATE_RIGHT = false;

	switch (INPUT)
	{
		case 'A':
		{
			if (IsKeyDown(KEY_D)) RIGHT = true;
			if (IsKeyDown(KEY_A)) LEFT = true;
			if (IsKeyDown(KEY_S)) DOWN = true;
			if (IsKeyPressed(KEY_SPACE)) DROP = true;
			if (IsKeyPressed(KEY_Q)) ROTATE_LEFT = true;
			if (IsKeyPressed(KEY_E)) ROTATE_RIGHT = true;
			if (IsKeyPressed(KEY_ESCAPE)) START = true;
		}break;

		case 'B':
		{
			if (IsKeyDown(KEY_RIGHT)) RIGHT = true;
			if (IsKeyDown(KEY_LEFT)) LEFT = true;
			if (IsKeyDown(KEY_DOWN)) DOWN = true;
			if (IsKeyPressed(KEY_KP_2)) DROP = true;
			if (IsKeyPressed(KEY_KP_1)) ROTATE_LEFT = true;
			if (IsKeyPressed(KEY_KP_3)) ROTATE_RIGHT = true;
		}break;

		default:
		{
			if (INPUT != -1)
			{
				if (IsGamepadButtonDown(INPUT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) RIGHT = true;
				if (IsGamepadButtonDown(INPUT, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) LEFT = true;
				if (IsGamepadButtonDown(INPUT, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) DOWN = true;
				if (IsGamepadButtonPressed(INPUT, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) DROP = true;
				if (IsGamepadButtonPressed(INPUT, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) ROTATE_LEFT = true;
				if (IsGamepadButtonPressed(INPUT, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) ROTATE_RIGHT = true;
				if (IsGamepadButtonPressed(INPUT, GAMEPAD_BUTTON_MIDDLE_RIGHT)) START = true;
			}
			else
				return;
		}break;
	}
}

char Player::CheckCollision(short x, short y)
{
	/* 0: No collision, 1: Block / Border collision, 2: Roof collision */
	unsigned char b;

	for (int i = 0; i < 4; i++)
	{
		b = field->block[(t[i].x + x) + ((t[i].y + y) * field->width)];

		if (t[i].y + y < 0)
			return 2;

		if ((b >= 1) or (t[i].x + x == field->width) or ((t[i].x + x) < 0) or ((t[i].y + y) >= field->height))
			return 1;
	}

	return 0;
}

bool Player::Move(short x, short y)
{
	bool move = false;


	if (!CheckCollision(x, y))
	{
		for (int i = 0; i < 4; i++)
		{
			t[i].x += x;
			t[i].y += y;
		}
		
		move = true;

		/* Extra time */
		if (CheckCollision(0, 1) and y)
			timerCounter = -20;

		field->RedrawPlayer();
	}

	return move;
}

bool Player::Rotate(char dir)
{
	/*temp*/
	if (type == 'O')
		return true;

	/* Save old pos */
	memcpy(tOld, t, sizeof(COORD) * 4);

	/* Subtract point of rotation for each point */
	short rotationPointX = t[0].x;
	short rotationPointY = t[0].y;

	/* Put on corner */
	for (int i = 0; i < 4; i++)
	{
		t[i].x -= rotationPointX;
		t[i].y -= rotationPointY;
	}

	COORD temp;

	/* Do rotation */
	switch (dir)
	{
		case 'R':
		{
			for (int i = 0; i < 4; i++)
			{
				temp = t[i];

				t[i].x = temp.y * -1;
				t[i].y = temp.x;
			}break;
		}

		case 'L':
		{
			for (int i = 0; i < 4; i++)
			{
				temp = t[i];

				t[i].x = temp.y;
				t[i].y = temp.x * -1;
			}break;
		}

		default: return false;
	}

	/* Recover original position */
	for (int i = 0; i < 4; i++)
	{
		t[i].x += rotationPointX;
		t[i].y += rotationPointY;
	}

	/* Special case for I */

	char facingOld = facing;

	if (type == 'I')
	{
		/* Change facing direction */
		switch (dir)
		{
			case 'R':
			{
				if (facing == 3) facing = 0;
				else facing++;
			}break;

			case 'L':
			{
				if (facing == 0) facing = 3;
				else facing--;
			}break;
		}

		/* Correct position */
		switch (facing)
		{
			case 0:
			{
				if (dir == 'R')
				{
					for (int i = 0; i < 4; i++)
						t[i].y--;
				}
				else
				{
					for (int i = 0; i < 4; i++)
						t[i].x--;
				}
			}break;

			case 1:
			{
				if (dir == 'R')
				{
					for (int i = 0; i < 4; i++)
						t[i].x++;
				}
				else
				{
					for (int i = 0; i < 4; i++)
						t[i].y--;
				}
			}break;

			case 2:
			{
				if (dir == 'R')
				{
					for (int i = 0; i < 4; i++)
						t[i].y++;
				}
				else
				{
					for (int i = 0; i < 4; i++)
						t[i].x++;
				}
			}break;

			case 3:
			{
				if (dir == 'R')
				{
					for (int i = 0; i < 4; i++)
						t[i].x--;
				}
				else
				{
					for (int i = 0; i < 4; i++)
						t[i].y++;
				}
			}break;
		}
	}

	/* Check for collision */
	if (CheckCollision(0, 0))
	{
		memcpy(t, tOld, sizeof(COORD) * 4);

		/* Recover old facing indicator if collide */
		if (type == 'I')
			facing = facingOld;

		return false;
	}

	field->RedrawPlayer();

	return true;
}

void Player::GetTetromino(void)
{
	type = nextType;
	nextType = rand() % 7;

	switch (type)
	{
		case 0:
		{
			type = 'I';
			t[0] = {short(posX + 1), 1};
			t[1] = {short(posX + 0), 1};
			t[2] = {short(posX + 2), 1};
			t[3] = {short(posX + 3), 1};
			facing = 0;
		}break;

		case 1:
		{
			type = 'O';
			t[0] = {short(posX + 0), 0};
			t[1] = {short(posX + 1), 0};
			t[2] = {short(posX + 0), 1};
			t[3] = {short(posX + 1), 1};
		}break;

		case 2:
		{
			type = 'T';
			t[0] = {short(posX + 1), 1};
			t[1] = {short(posX + 1), 0};
			t[2] = {short(posX + 2), 1};
			t[3] = {short(posX + 0), 1};
		}break;

		case 3:
		{
			type = 'J';
			t[0] = {short(posX + 1), 1};
			t[1] = {short(posX + 0), 1};
			t[2] = {short(posX + 2), 1};
			t[3] = {short(posX + 0), 0};
		}break;

		case 4:
		{
			type = 'L';
			t[0] = {short(posX + 1), 1};
			t[1] = {short(posX + 2), 1};
			t[2] = {short(posX + 0), 1};
			t[3] = {short(posX + 2), 0};
		}break;

		case 5:
		{
			type = 'S';
			t[0] = {short(posX + 1), 1};
			t[1] = {short(posX + 1), 0};
			t[2] = {short(posX + 2), 0};
			t[3] = {short(posX + 0), 1};
		}break;

		case 6:
		{
			type = 'Z';
			t[0] = {short(posX + 1), 1};
			t[1] = {short(posX + 1), 0};
			t[2] = {short(posX + 0), 0};
			t[3] = {short(posX + 2), 1};
		}break;
	}

	/* Check if tetromino is inside game field */
	signed char outboundX = 0;

	for (int i = 0; i < 4; i++)
	{
		if (t[i].x < 0) {outboundX = -1; break;}
		if (t[i].x >= field->width) {outboundX = 1; break;}
	}

	if (outboundX > 0)
	{
		while (CheckCollision(0, 0))
		for (int i = 0; i < 4; i++)
		{
			t[i].x--;
		}
	}
	else if (outboundX < 0)
	{
		while (CheckCollision(0, 0))
		for (int i = 0; i < 4; i++)
		{
			t[i].x++;
		}
	}

	if (CheckCollision(0, 0))
	{
		GameOver();
		return;
	}
}

void Player::Put(void)
{
	/* Block style */
	unsigned char style = (rand() % 4) + 1;

	/* Old position / Fix */
	if ((type == 'O'))
		posX = t[0].x;
	else
		posX = t[0].x - 1;

	/* Reset timer */
	timerCounter = 0;

	/* Put */

	/* Fix if collide with other player */
	while (CheckCollision(0, 0) == 1)
	{
		if (!CheckCollision(0, -1))
		{
			for (int i = 0; i < 4; i++)
				t[i].y--;
		}
		else
			break;
	}

	for (int i = 0; i < 4; i++)
		field->block[t[i].x + (t[i].y * field->width)] = style;

	/* Score */
	unsigned char lines = field->UpdateLines();
	AddScore(lines);


	if (lines == 4)
		PlaySound(field->core->sfx[4]);
	else if (lines >= 1)
		PlaySound(field->core->sfx[5]);
	else
		PlaySound(field->core->sfx[3]);
	/**/

	GetTetromino();

	field->RedrawPlayer();
	field->RedrawField();
	field->RedrawGui();	
}

void Player::AddScore(unsigned char lines)
{

	unsigned char p;

	switch (lines)
	{
		case 1: p = 4; break;
		case 2: p = 10; break;
		case 3: p = 30; break;
		case 4: p = 120; break;
		default: return;
	}

	score += (p * field->width) * (field->core->level + 1);

	/* Update global level */
	field->lines += lines;
	field->UpdateLevel();
}

void Player::Drop(void)
{
	while (Move(0, 1));
	Put();
}

void Player::GameOver(void)
{
	unsigned char style = (rand() % 4) + 1;
	gameOver = true;

	for (int i = 0; i < 4; i++)
		field->block[t[i].x + (t[i].y * field->width)] = style;

	PlaySound(field->core->sfx[6]);
}

void Player::MakeGhost(void)
{
	memcpy(tOld, t, sizeof(COORD) * 4);

	while (!CheckCollision(0, 1))
	{
		for (int i = 0; i < 4; i++)
			t[i].y++;
	}

	memcpy(tGhost, t, sizeof(COORD) * 4);
	memcpy(t, tOld, sizeof(COORD) * 4);
}