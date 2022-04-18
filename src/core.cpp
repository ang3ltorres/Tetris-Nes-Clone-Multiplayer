#include "core.hpp"
#include "field.hpp"
#include "menu.hpp"
#include "data.hpp"

#include <cstdio>

#include <chrono>
#include <thread>

Core::Core(void)
{
	gameWidth = 256;
	gameHeight = 240;

	displayWidth = gameWidth;
	displayHeight = gameHeight;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(displayWidth, displayHeight, "Tetris Multiplayer");
	SetExitKey(KEY_NULL);

	InitAudioDevice();

	render = LoadRenderTexture(gameWidth, gameHeight);

	CalcScale();

	/* Load data */

	/* Texture */
	void* buffer = ::operator new(D_FONT_SIZE);
	F = fopen("data", "rb");
	Image img;

	fseek((FILE*)F, D_TEXTURE_POS, SEEK_SET);
	fread(buffer, 1, D_TEXTURE_SIZE, (FILE*)F);
	img = LoadImageFromMemory(".png", (unsigned char*)buffer, D_TEXTURE_SIZE);
	texture = LoadTextureFromImage(img);
	UnloadImage(img);

	/* Background texture */
	fseek((FILE*)F, D_BACKGROUND_POS, SEEK_SET);
	fread(buffer, 1, D_BACKGROUND_SIZE, (FILE*)F);
	img = LoadImageFromMemory(".png", (unsigned char*)buffer, D_BACKGROUND_SIZE);
	backgroundTexture = LoadTextureFromImage(img);
	UnloadImage(img);

	/* Title texture */
	fseek((FILE*)F, D_TITLE_POS, SEEK_SET);
	fread(buffer, 1, D_TITLE_SIZE, (FILE*)F);
	img = LoadImageFromMemory(".png", (unsigned char*)buffer, D_TITLE_SIZE);
	titleTexture = LoadTextureFromImage(img);
	UnloadImage(img);

	/* Setup texture */
	fseek((FILE*)F, D_SETUP_POS, SEEK_SET);
	fread(buffer, 1, D_SETUP_SIZE, (FILE*)F);
	img = LoadImageFromMemory(".png", (unsigned char*)buffer, D_SETUP_SIZE);
	setupTexture = LoadTextureFromImage(img);
	UnloadImage(img);

	/* Font */
	fseek((FILE*)F, D_FONT_POS, SEEK_SET);
	fread(buffer, 1, D_FONT_SIZE, (FILE*)F);

	font = LoadFontFromMemory(".ttf", (unsigned char*)buffer, D_FONT_SIZE, 8, 0, 224);
	SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);

	/* SFX */
	Wave wave;

	for (unsigned short i = 0; i < 9; i++)
	{
		fseek((FILE*)F, D_SFX_POS[i], SEEK_SET);
		fread(buffer, 1, D_SFX_SIZE[i], (FILE*)F);
		wave = LoadWaveFromMemory(".ogg", (unsigned char*)buffer, D_SFX_SIZE[i]);
		sfx[i] = LoadSoundFromWave(wave);
		UnloadWave(wave);
	}

	::operator delete(buffer);
	fclose((FILE*)F);

	/* Music */
	musicStream = nullptr;
	music.looping = true;
	//SetMusic(0);

	/**/
	menuEnabled = true;
	field = nullptr;
	menu = new Menu(this);
	
	flash = 0;
	pressStartColor = 0;
}

Core::~Core()
{
	delete menu;

	if (!menuEnabled)
	{
		delete field;
		UnloadRenderTexture(background);
	}

	UnloadRenderTexture(render);

	UnloadTexture(texture);
	UnloadTexture(backgroundTexture);
	UnloadTexture(setupTexture);
	UnloadTexture(titleTexture);

	if (musicStream)
	{
		UnloadMusicStream(music);
		::operator delete(musicStream);		
	}

	UnloadFont(font);

	for (unsigned short i = 0; i < 9; i++)
		UnloadSound(sfx[i]);

	CloseAudioDevice();
	CloseWindow();
}

void Core::Loop(void)
{
	auto next_frame = std::chrono::steady_clock::now();

	while (!WindowShouldClose())
	{
		next_frame += std::chrono::microseconds(1000000 / 60);

		/* Update */
		CheckWindow();
		
		if (musicStream)
			UpdateMusicStream(music);

		if (menuEnabled)
			menu->Update();
		else
			field->Update();

		/* Draw */
		BeginDrawing();
		ClearBackground(BLACK);
			Draw();
			
		EndDrawing();

		std::this_thread::sleep_until(next_frame);
	}
}

void Core::Draw(void)
{

	BeginTextureMode(render);

		if (menuEnabled)
			menu->Draw();
		else
		{
			if (!field->gamePaused)
			{
				/* Render background */
				DrawTexturePro(background.texture,
					Rectangle{0, 0, float(background.texture.width), -float(background.texture.height)},
					Rectangle{0, 0, float(background.texture.width), float(background.texture.height)},
					Vector2{0, 0}, 0, WHITE);
				
				/* Render field */
				DrawTexturePro(field->renderField.texture,
					Rectangle{0, 0, float(field->renderField.texture.width), -float(field->renderField.texture.height)},
					Rectangle{float(renderFieldPlayerXPos), float(renderFieldPlayerYPos), float(field->renderField.texture.width), float(field->renderField.texture.height)},
					Vector2{0, 0}, 0, WHITE);

				/* Render field */
				DrawTexturePro(field->renderPlayer.texture,
					Rectangle{0, 0, float(field->renderPlayer.texture.width), -float(field->renderPlayer.texture.height)},
					Rectangle{float(renderFieldPlayerXPos), float(renderFieldPlayerYPos), float(field->renderPlayer.texture.width), float(field->renderPlayer.texture.height)},
					Vector2{0, 0}, 0, WHITE);
			}
			else
			{
				ClearBackground(BLACK);
				DrawTextEx(font, "PAUSE", Vector2{(float)((gameWidth - (5 * 8)) / 2), (float)(gameHeight - (1 * 8)) / 2}, 8, 0, Color{146, 144, 255, 255});
			}

			/* Draw "PRESS START" */
			if (field->playerNum == 0)
			{
				pressStartColor += 14;
				DrawTextEx(font, "PRESS START", Vector2{(float)((gameWidth - (11 * 8)) / 2) + 1, 20 + 1}, 8, 0, BLACK);
				DrawTextEx(font, "PRESS START", Vector2{(float)((gameWidth - (11 * 8)) / 2), 20}, 8, 0, ColorFromHSV(pressStartColor, 1, 1));
			}
		}

	EndTextureMode();

	DrawTexturePro(render.texture, Rectangle{0, 0, float(gameWidth), -float(gameHeight)}, Rectangle{float(offsetX), float(offsetY), float(gameWidth * scale), float(gameHeight * scale)}, Vector2{0, 0}, 0, WHITE);
	//DrawFPS(0, 0);
}

void Core::CalcScale(void)
{
	// Get display / window size
	if ((GetScreenWidth() >= gameWidth) and (GetScreenHeight() >= gameHeight))
	{
		displayWidth = GetScreenWidth();
		displayHeight = GetScreenHeight();
	}
	else
	{
		SetWindowSize(gameWidth, gameHeight);
		displayWidth = gameWidth;
		displayHeight = gameHeight;
	}

	// Set pixel perfect scale
	unsigned short scaleW = displayWidth / gameWidth;
	unsigned short scaleH = displayHeight / gameHeight;
	scale = ((scaleW < scaleH) ? scaleW : scaleH);

	// Set offset
	offsetX = (displayWidth - (gameWidth * scale)) / 2;
	offsetY = (displayHeight - (gameHeight * scale)) / 2;
}

void Core::CheckWindow(void)
{
	if (IsKeyPressed(KEY_F11))
	{
		if (IsWindowFullscreen())
		{
			ToggleFullscreen();
			SetWindowSize(gameWidth, gameHeight);
			SetWindowPosition((GetMonitorWidth(0) - gameWidth) / 2, (GetMonitorHeight(0) - gameHeight) / 2);
		}
		else
		{
			SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
			ToggleFullscreen();
		}
	}

	if (IsWindowResized())
		CalcScale();
}

void Core::SetMusic(unsigned char n)
{
	unsigned int size, pos;

	if (musicStream)
	{
		StopMusicStream(music);
		UnloadMusicStream(music);
		::operator delete(musicStream);
		musicStream = nullptr;
	}

	switch (n)
	{
		case 1: size = D_SONG1_SIZE; pos = D_SONG1_POS; break;
		case 2: size = D_SONG2_SIZE; pos = D_SONG2_POS; break;
		case 3: size = D_SONG3_SIZE; pos = D_SONG3_POS; break;
		default: return;
	}

	F = fopen("data", "rb");

	musicStream = ::operator new(size);
	fseek((FILE*)F, pos, SEEK_SET);
	fread(musicStream, 1, size, (FILE*)F);
	music = LoadMusicStreamFromMemory(".ogg", (unsigned char*)musicStream, size);

	PlayMusicStream(music);
	SetMusicVolume(music, 1);

	fclose((FILE*)F);
}

void Core::GenerateBackground(void)
{
	BeginTextureMode(background);

		for (short y = 0; y < (gameHeight / 88) + 1; y++)
		for (short x = 0; x < (gameWidth / 88) + 1; x++)
			DrawTexture(backgroundTexture, x * 88, y * 88, WHITE);		

		/* Corners */
		DrawTextureRec(texture, Rectangle{16, 0, 8, 8}, Vector2{float(renderFieldPlayerXPos - 8), float(renderFieldPlayerYPos - 8)}, WHITE);
		DrawTextureRec(texture, Rectangle{16, 0, -8, 8}, Vector2{float(renderFieldPlayerXPos - 8) + ((field->width + 1) * 8), float(renderFieldPlayerYPos - 8)}, WHITE);
		DrawTextureRec(texture, Rectangle{16, 0, -8, -8}, Vector2{float(renderFieldPlayerXPos - 8) + ((field->width + 1) * 8), float(renderFieldPlayerYPos - 8) + ((field->height + 1) * 8)}, WHITE);
		DrawTextureRec(texture, Rectangle{16, 0, 8, -8}, Vector2{float(renderFieldPlayerXPos - 8), float(renderFieldPlayerYPos - 8) + ((field->height + 1) * 8)}, WHITE);

		/* Border */
		for (short x = 1; x < field->width + 1; x++)
		{
			DrawTextureRec(texture, Rectangle{24, 0, 8, 8}, Vector2{float(renderFieldPlayerXPos - 8) + (x * 8), float(renderFieldPlayerYPos - 8)}, WHITE);
			DrawTextureRec(texture, Rectangle{24, 0, 8, -8}, Vector2{float(renderFieldPlayerXPos - 8) + (x * 8), float(renderFieldPlayerYPos - 8) + ((field->height + 1) * 8)}, WHITE);
		}

		for (short y = 1; y < field->height + 1; y++)
		{
			DrawTexturePro(texture, Rectangle{24, 0, 8, -8}, Rectangle{float(renderFieldPlayerXPos - 8) + 8, float(renderFieldPlayerYPos - 8) + (y * 8), 8, 8}, Vector2{0, 0}, 90, WHITE);
			DrawTexturePro(texture, Rectangle{24, 0, 8, 8}, Rectangle{float((renderFieldPlayerXPos) + ((field->width + 1) * 8)), float(renderFieldPlayerYPos - 8) + (y * 8), 8, 8}, Vector2{0, 0}, 90, WHITE);
		}


	EndTextureMode();
}

void Core::StartGame(
	unsigned short widthSelection,
	unsigned short heightSelection,

	unsigned short musicSelection,
	unsigned short levelSelection)
{
	menuEnabled = false;

	/**/
	gameWidth = (widthSelection*8) + (8*12);
	gameHeight = (heightSelection*8) + (8*10);
	/**/
	fieldWidth = widthSelection;
	fieldHeight = heightSelection;

	/**/
	UnloadRenderTexture(render);
	render = LoadRenderTexture(gameWidth, gameHeight);
	background = LoadRenderTexture(gameWidth, gameHeight);
	level = levelSelection;
	field = new Field(this, fieldWidth, fieldHeight);

	switch (musicSelection)
	{
		case 0: field->MUSIC_SELECTION = 1; break;
		case 1: field->MUSIC_SELECTION = 2; break;
		case 2: field->MUSIC_SELECTION = 3; break;
		case 3: field->MUSIC_SELECTION = 0; break;
	}

	/* Generate background */
	renderFieldPlayerXPos = (((gameWidth - (fieldWidth * 8)) / 2) / 8) * 8;
	renderFieldPlayerYPos = (((gameHeight - (fieldHeight * 8)) / 2) / 8) * 8;

	GenerateBackground();

	field->RedrawField();
	field->RedrawPlayer();

	/* Resize window (If not fullscreen) */
	if (!IsWindowFullscreen())
	{
		SetWindowSize(gameWidth, gameHeight);
		displayWidth = gameWidth;
		displayHeight = gameHeight;
	}
	
	CalcScale();
}


void Core::ResetGame(void)
{
	gameWidth = 256;
	gameHeight = 240;

	displayWidth = gameWidth;
	displayHeight = gameHeight;

	UnloadRenderTexture(background);
	UnloadRenderTexture(render);
	render = LoadRenderTexture(gameWidth, gameHeight);

	CalcScale();
	SetMusic(0);

	delete field;
	field = nullptr;
	menuEnabled = true;

	new(menu) Menu(this);
}