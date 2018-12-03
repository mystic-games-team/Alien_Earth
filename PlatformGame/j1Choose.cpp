#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "j1Choose.h"
#include "Player.h"
#include "j1Collision.h"
#include "Entity.h"
#include "j1FadeToBlack.h"
#include "EntityManager.h"
#include "Button.h"
#include "UI_Element.h"
#include "Image.h"
#include "UI_Manager.h"

#include "Brofiler/Brofiler.h"

j1Choose::j1Choose() : j1Module()
{
	name.create("choose");
}

// Destructor
j1Choose::~j1Choose()
{}

// Called before render is available
bool j1Choose::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	file_texture = config.child("Start").text().as_string();
	MinY_ChooseRect = config.child("MinY_ChooseRect").attribute("value").as_int();
	MaxY_ChooseRect = config.child("MaxY_ChooseRect").attribute("value").as_int();
	MinX_RectChoosePlayer1 = config.child("MinX_RectChoosePlayer1").attribute("value").as_int();
	MaxX_RectChoosePlayer1 = config.child("MaxX_RectChoosePlayer1").attribute("value").as_int();
	MinX_RectChoosePlayer2 = config.child("MinX_RectChoosePlayer2").attribute("value").as_int();
	MaxX_RectChoosePlayer2 = config.child("MaxX_RectChoosePlayer2").attribute("value").as_int();
	MinX_RectChoosePlayer3 = config.child("MinX_RectChoosePlayer3").attribute("value").as_int();
	MaxX_RectChoosePlayer3 = config.child("MaxX_RectChoosePlayer3").attribute("value").as_int();
	PlayerNumber1 = config.child("PlayerNumber1").attribute("value").as_int();
	PlayerNumber2 = config.child("PlayerNumber2").attribute("value").as_int();
	PlayerNumber3 = config.child("PlayerNumber3").attribute("value").as_int();
	ChooseFx= config.child("ChooseFx").text().as_string();
	IntroFx = config.child("IntroFx").text().as_string();
	YellowStand = LoadGigantAliensAnimations(0, config, "Stand");
	PinkStand = LoadGigantAliensAnimations(1, config, "Stand");
	BlueStand = LoadGigantAliensAnimations(2, config, "Stand");
	YellowWalk = LoadGigantAliensAnimations(0, config, "Walk");
	PinkWalk = LoadGigantAliensAnimations(1, config, "Walk");
	BlueWalk = LoadGigantAliensAnimations(2, config, "Walk");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1Choose::Start()
{
	App->scene->active = false;
	//App->player->active = false;
	App->collision->active = false;
	App->map->active = false;
	GameOn = false;
	App->render->camera.x = 0;
	App->render->camera.y = 0;
	start = false;
	ScreenStart = App->tex->Load(file_texture.GetString());
	yellow = App->tex->Load(App->entitymanager->GetPlayerData()->sprites_name[0].GetString());
	pink = App->tex->Load(App->entitymanager->GetPlayerData()->sprites_name[1].GetString());
	blue = App->tex->Load(App->entitymanager->GetPlayerData()->sprites_name[2].GetString());
	Settings = App->tex->Load("textures/Settings.png");
	choosefx = App->audio->LoadFx(ChooseFx.GetString());
	introfx = App->audio->LoadFx(IntroFx.GetString());
	CreateButtonsTypePlayer();
	CreateMainMenu();
	CreateSettingsButtons();
	CreatehacksButtons();
	CreateIntro();
	WantToDisappearMainMenu(true);
	WantToDisappearButtonsTypePlayer(true);
	
	return true;
}

// Called each loop iteration
bool j1Choose::PreUpdate()
{
	BROFILER_CATEGORY("Menu: PreUpdate", Profiler::Color::Aquamarine);
	App->input->GetMousePosition(mouse.x, mouse.y);

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !GameOn && !start) {
		start = true;
		App->audio->PlayFx(introfx);	
		App->ui_manager->DeleteUI_Element(Title);
		App->ui_manager->DeleteUI_Element(sentence);
		WantToDisappearMainMenu(false);
		//CreateMainMenuButtons();
		//CreateButtons();
	}
	return true;
}

// Called each loop iteration
bool j1Choose::Update(float dt)
{
	BROFILER_CATEGORY("Menu: Update", Profiler::Color::Aquamarine);
	if (start) {
		if (!GameOn) {
			if (InSettings)
				SettingsMenu(dt);
			if (InMainMenu)
				MainMenu();
			if (StartChoosing)
				MenuChoosePlayer(dt);
			if (InHacks)
				HacksMenu(dt);
		}
	}
	return true;
}

// Called each loop iteration
bool j1Choose::PostUpdate()
{
	BROFILER_CATEGORY("Menu: PostUpdate", Profiler::Color::Aquamarine);
	bool ret = true;
	if (!GameOn) {
		if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || Exit)
			ret = false;
	}
	if (GoStart) {
		if (App->fade->current_step==App->fade->fade_from_black) {
			GoStart = false;
			InMainMenu = true;
			StartChoosing = false;
			App->scene->active = !App->scene->active;
			App->collision->active = !App->collision->active;
			App->map->active = !App->map->active;
			App->scene->KnowMap = 0;
			App->map->ChangeMap(App->scene->map_name[App->scene->KnowMap]);
			App->entitymanager->ActiveGame = true;
			App->entitymanager->GetPlayerData()->Start();
			App->entitymanager->GetPlayerData()->ChangePlayer(playernumber);
			App->scene->SpawnEnemies();
			App->entitymanager->GetPlayerData()->SetUI();
			GameOn = true;
		}
	}
	if (GoStartSaved) {
		if (App->fade->current_step == App->fade->fade_from_black) {
			GoStartSaved = false;
			App->ui_manager->DeleteAllUI();
			InMainMenu = true;
			App->scene->active = !App->scene->active;
			App->collision->active = !App->collision->active;
			App->map->active = !App->map->active;
			App->scene->KnowMap = 0;
			App->entitymanager->ActiveGame = true;
			App->LoadGame("save_game.xml");
			App->entitymanager->GetPlayerData()->SetUI();
			GameOn = true;
		}
	}
	return ret;
}

// Called before quitting
bool j1Choose::CleanUp()
{
	App->tex->UnLoad(ScreenStart);
	LOG("Freeing scene");
	return true;
}

Animation j1Choose::LoadGigantAliensAnimations(int playernumber, pugi::xml_node& config, p2SString NameAnim) const
{
	p2SString XML_Name_Player_Anims;
	SDL_Rect rect;
	Animation anim;
	switch (playernumber) {
	case 0:
		XML_Name_Player_Anims = "AnimationsPlayerYellow";
		break;
	case 1:
		XML_Name_Player_Anims = "AnimationsPlayerPink";
		break;
	case 2:
		XML_Name_Player_Anims = "AnimationsPlayerBlue";
		break;
	}

	for (pugi::xml_node frames = config.child(XML_Name_Player_Anims.GetString()).child(NameAnim.GetString()).child("frame"); frames; frames = frames.next_sibling("frame")) {
		rect.x = frames.attribute("x").as_int();
		rect.y = frames.attribute("y").as_int();
		rect.w = frames.attribute("w").as_int();
		rect.h = frames.attribute("h").as_int();
		anim.PushBack({ rect.x,rect.y,rect.w,rect.h });
	}
	anim.speed = config.child(XML_Name_Player_Anims.GetString()).child(NameAnim.GetString()).attribute("speed").as_float();
	anim.loop = config.child(XML_Name_Player_Anims.GetString()).child(NameAnim.GetString()).attribute("loop").as_bool();

	return anim;
}


//MENU
void j1Choose::CreateIntro()
{
	Background = App->ui_manager->CreateImage(0, 0, false);
	Background->SetSpritesData({ 0,1158,1024,768 });
	Title = App->ui_manager->CreateImage((App->win->width / 2) - (354 / 2), 70, false);
	Title->SetSpritesData({ 0,783,354,305 });
	sentence = App->ui_manager->CreateImage((App->win->width / 2) - (268 / 2), 550, false);
	sentence->SetSpritesData({ 0,1105,268,35 });
}

void j1Choose::CreateMainMenu()
{
	AlreadyChoosen = false;
	MainTitle = App->ui_manager->CreateImage((App->win->width / 2) - (844 / 2), 70, false);
	MainTitle->SetSpritesData({ 401,784,844	,165 });
	MainTitle->type = BUTTON;
	buttonSTART = App->ui_manager->CreateButton(400, 270, 1, "START", 30);
	buttonCONTINUE = App->ui_manager->CreateButton(400, 350, 1, "CONTINUE", 30);
	buttonSETTINGS = App->ui_manager->CreateButton(400, 430, 1, "SETTINGS", 30);
	buttonHACKS = App->ui_manager->CreateButton(400, 510, 1, "HACKS", 30);
	buttonCREDITS = App->ui_manager->CreateButton(400, 590, 1, "CREDITS", 30);
	buttonEXIT = App->ui_manager->CreateButton(400, 670, 1, "EXIT", 30);
}

void j1Choose::MainMenu()
{
	if (buttonSTART->pressed) {
		WantToDisappearMainMenu(true);
		WantToDisappearButtonsTypePlayer(false);
		buttonJEFF->pressed = false;
		buttonJANE->pressed = false;
		buttonJERRY->pressed = false;
		InMainMenu = false;
		StartChoosing = true;
	}
	if (buttonCONTINUE->pressed) {
		App->ui_manager->DeleteAllUI();
		App->entitymanager->GetPlayerData()->Intro = false;
		App->fade->FadeToBlack(3.0f);
		GoStartSaved = true;
	}
	if (buttonSETTINGS->pressed) {
		SettingMenuDone = false;
		Positioned = false;
		buttonSTART->NoUse = true;
		buttonCONTINUE->NoUse = true;
		buttonSETTINGS->NoUse = true;
		buttonHACKS->NoUse = true;
		buttonEXIT->NoUse = true;
		buttonCREDITS->NoUse = true;
		InMainMenu = false;
		InSettings = true;
		if (App->capactivated)
			checkboxFPS->pressed = true;
		else checkboxFPS->pressed = false;

	}
	if (buttonHACKS->pressed) {
		HacksMenuDone = false;
		positioned = false;
		buttonSTART->NoUse = true;
		buttonCONTINUE->NoUse = true;
		buttonSETTINGS->NoUse = true;
		buttonHACKS->NoUse = true;
		buttonEXIT->NoUse = true;
		buttonCREDITS->NoUse = true;
		InMainMenu = false;
		InHacks = true;
		if (App->entitymanager->GetPlayerData()->God)
			checkboxGODMODE->pressed = true;
		else checkboxGODMODE->pressed = false;
	}
	if (buttonEXIT->pressed) {
		Exit = true;
	}
}


void j1Choose::WantToDisappearMainMenu(bool Disappear)
{
	if (Disappear) {
		buttonSTART->NoUse = true;
		buttonCONTINUE->NoUse = true;
		buttonSETTINGS->NoUse = true;
		buttonEXIT->NoUse = true;
		buttonCREDITS->NoUse = true;
		buttonHACKS->NoUse = true;
		buttonSTART->WantToRender = false;
		buttonCONTINUE->WantToRender = false;
		buttonSETTINGS->WantToRender = false;
		buttonHACKS->WantToRender = false;
		buttonEXIT->WantToRender = false;
		buttonCREDITS->WantToRender = false;
		MainTitle->WantToRender = false;
	}
	else {
		buttonSTART->NoUse = false;
		buttonCONTINUE->NoUse = false;
		buttonSETTINGS->NoUse = false;
		buttonEXIT->NoUse = false;
		buttonHACKS->NoUse = false;
		buttonCREDITS->NoUse = false;
		buttonSTART->WantToRender = true;
		buttonCONTINUE->WantToRender = true;
		buttonSETTINGS->WantToRender = true;
		buttonHACKS->WantToRender = true;
		buttonEXIT->WantToRender = true;
		buttonCREDITS->WantToRender = true;
		MainTitle->WantToRender = true;
	}
}

void j1Choose::CreateButtonsTypePlayer()
{
	CHOOSE = App->ui_manager->CreateImage((App->win->width / 2) - (608 / 2), 70, false);
	CHOOSE->SetSpritesData({ 0,1928,608,72 });
	CHOOSE->type = BUTTON;
	JEFFNAME = App->ui_manager->CreateImage((App->win->width / 4) - (68 / 2), 620, false);
	JEFFNAME->SetSpritesData({ 611,1928,68,38 });
	JEFFNAME->type = BUTTON;
	JANENAME = App->ui_manager->CreateImage((App->win->width / 4) * 2 - (72 / 2), 620, false);
	JANENAME->SetSpritesData({ 681,1928,72,38 });
	JANENAME->type = BUTTON;
	JERRYNAME = App->ui_manager->CreateImage((App->win->width / 4) * 3 - (92 / 2), 620, false);
	JERRYNAME->SetSpritesData({ 754,1928,92,38 });
	JERRYNAME->type = BUTTON;
	buttonJEFF = App->ui_manager->CreateButton((App->win->width / 4)-112, 159, 2);
	buttonJEFF->SetSpritesData({ 0,0,0,0 }, { 0,0,225,441 }, { 0,0,225,441 });
	buttonJANE = App->ui_manager->CreateButton((App->win->width / 4)*2 - 112, 159, 2);
	buttonJANE->SetSpritesData({ 0,0,0,0 }, { 0,0,225,441 }, { 0,0,225,441 });
	buttonJERRY = App->ui_manager->CreateButton((App->win->width / 4) *3-112, 159, 2);
	buttonJERRY->SetSpritesData({ 0,0,0,0	}, { 0,0,225,441 }, { 0,0,225,441 });
	buttonGOBACK = App->ui_manager->CreateButton(50, 25, 3);
	buttonGOBACK->SetSpritesData({ 559,0,39,31 }, { 652,0,39,31 }, { 608,0,39,28 });
}

void j1Choose::MenuChoosePlayer(float dt)
{
	if (buttonJEFF->mouseOn) {
		App->render->Blit(yellow, (App->win->width / 4) - (195 / 2), 265, &(YellowWalk.GetCurrentFrame(dt)));
		App->render->Blit(pink, (App->win->width / 4) * 2 - (160/ 2), 265, &(PinkStand.GetCurrentFrame(dt)));
		App->render->Blit(blue, (App->win->width / 4) * 3 - (169 / 2), 265, &(BlueStand.GetCurrentFrame(dt)));
		if (buttonJEFF->pressed && !AlreadyChoosen) {
			AlreadyChoosen = true;
			playernumber = PlayerNumber1;
			StartLevel();
		}
	}
	else if (buttonJANE->mouseOn) {
		App->render->Blit(yellow, (App->win->width / 4) - (184 / 2), 265, &(YellowStand.GetCurrentFrame(dt)));
		App->render->Blit(blue, (App->win->width / 4) * 3 - (168/2), 265, &(BlueStand.GetCurrentFrame(dt)));
		App->render->Blit(pink, (App->win->width / 4) * 2 - (168 / 2), 265, &(PinkWalk.GetCurrentFrame(dt)));
		if (buttonJANE->pressed && !AlreadyChoosen) {
			AlreadyChoosen = true;
			playernumber = PlayerNumber2;
			StartLevel();
		}
	}
	else if (buttonJERRY->mouseOn) {
		App->render->Blit(yellow, (App->win->width / 4) - (184 / 2), 265, &(YellowStand.GetCurrentFrame(dt)));
		App->render->Blit(pink, (App->win->width / 4) * 2 - (160 / 2), 265, &(PinkStand.GetCurrentFrame(dt)));
		App->render->Blit(blue, (App->win->width / 4) * 3 - (163/2), 265, &(BlueWalk.GetCurrentFrame(dt)));
		if (buttonJERRY->pressed && !AlreadyChoosen) {
			AlreadyChoosen = true;
			playernumber = PlayerNumber3;
			StartLevel();
		}
	}
	else {
		App->render->Blit(yellow, (App->win->width / 4) - (184 / 2), 265, &(YellowStand.GetCurrentFrame(dt)));
		App->render->Blit(pink, (App->win->width / 4) * 2 - (160 / 2), 265, &(PinkStand.GetCurrentFrame(dt)));
		App->render->Blit(blue, (App->win->width / 4) * 3 - (168 / 2), 265, &(BlueStand.GetCurrentFrame(dt)));
		repeat = false;
	}
	if (buttonGOBACK->pressed) {
		WantToDisappearButtonsTypePlayer(true);
		WantToDisappearMainMenu(false);
		StartChoosing = false;
		InMainMenu = true;
	}

}

void j1Choose::WantToDisappearButtonsTypePlayer(bool Disappear)
{
	if (Disappear) {
		CHOOSE->WantToRender = false;
		JEFFNAME->WantToRender = false;
		JANENAME->WantToRender = false;
		JERRYNAME->WantToRender = false;
		buttonJEFF->WantToRender = false;
		buttonJANE->WantToRender = false;
		buttonJERRY->WantToRender = false;
		buttonGOBACK->WantToRender = false;
		CHOOSE->NoUse = true;
		JEFFNAME->NoUse = true;
		JANENAME->NoUse = true;
		JERRYNAME->NoUse = true;
		buttonJEFF->NoUse = true;
		buttonJANE->NoUse = true;
		buttonJERRY->NoUse = true;
		buttonGOBACK->NoUse = true;
	}
	else {
		CHOOSE->WantToRender = true;
		JEFFNAME->WantToRender = true;
		JANENAME->WantToRender = true;
		JERRYNAME->WantToRender = true;
		buttonJEFF->WantToRender = true;
		buttonJANE->WantToRender = true;
		buttonJERRY->WantToRender = true;
		buttonGOBACK->WantToRender = true;
		CHOOSE->NoUse = false;
		JEFFNAME->NoUse = false;
		JANENAME->NoUse = false;
		JERRYNAME->NoUse = false;
		buttonJEFF->NoUse = false;
		buttonJANE->NoUse = false;
		buttonJERRY->NoUse = false;
		buttonGOBACK->NoUse = false;
	}
}

void j1Choose::CreateSettingsButtons()
{
	SettingMenuDone = false;
	imageSETTINGS = App->ui_manager->CreateImage(170, 1300, true);
	imageSETTINGS->SetSpritesData({ 758,0,705,671 });
	imageSETTINGS->type = BUTTON;
	buttonGOBACKSETTINGS = App->ui_manager->CreateButton(200, 1335, 3);
	buttonGOBACKSETTINGS->SetSpritesData({ 559,0,39,31 }, { 652,0,39,31 }, { 608,0,39,28 });
	checkboxFPS = App->ui_manager->CreateCheckBox(550, 1407);
	labelFPS = App->ui_manager->CreateLabel(270, 1400, "CAP FPS TO 30", 50, true);
	sliderVOLUMEMUSIC = App->ui_manager->CreateSlider(550, 1507, App->audio->volume);
	labelMUSICVOLUME = App->ui_manager->CreateLabel(270, 1507, "MUSIC VOLUME", 50, true);
	sliderVOLUMEFX = App->ui_manager->CreateSlider(550, 1607, App->audio->fxvolume);
	labelVOLUMEFX = App->ui_manager->CreateLabel(270, 1607, "FX VOLUME", 50, true);
	
	if (App->capactivated)
		checkboxFPS->pressed = true;
}


void j1Choose::SettingsMenu(float dt)
{
	Title->NoRenderLabel = true;
	sentence->NoRenderLabel = true;
	if (imageSETTINGS->position.y <= buttonEXIT->position.y + buttonEXIT->height) {
		buttonEXIT->WantToRender = false;
	}
	if (imageSETTINGS->position.y <= buttonHACKS->position.y + buttonHACKS->height) {
		buttonHACKS->WantToRender = false;
	}
	if (imageSETTINGS->position.y <= buttonCREDITS->position.y + buttonCREDITS->height) {
		buttonCREDITS->WantToRender = false;
	}
	if (imageSETTINGS->position.y <= buttonSETTINGS->position.y + buttonSETTINGS->height) {
		buttonSETTINGS->WantToRender = false;
	}
	if (imageSETTINGS->position.y <= buttonCONTINUE->position.y + buttonCONTINUE->height) {
		buttonCONTINUE->WantToRender = false;
	}
	if (imageSETTINGS->position.y <= buttonSTART->position.y + buttonSTART->height) {
		buttonSTART->WantToRender = false;
	}
	if (imageSETTINGS->position.y >= buttonEXIT->position.y) {
		buttonEXIT->WantToRender = true;
	}
	if (imageSETTINGS->position.y >= buttonCREDITS->position.y) {
		buttonCREDITS->WantToRender = true;
	}
	if (imageSETTINGS->position.y >= buttonHACKS->position.y) {
		buttonHACKS->WantToRender = true;
	}
	if (imageSETTINGS->position.y >= buttonSETTINGS->position.y) {
		buttonSETTINGS->WantToRender = true;
	}
	if (imageSETTINGS->position.y >= buttonCONTINUE->position.y) {
		buttonCONTINUE->WantToRender = true;
	}
	if (imageSETTINGS->position.y >= buttonSTART->position.y) {
		buttonSTART->WantToRender = true;
	}
	if (!Positioned && !SettingMenuDone) { //MENU GOING UP

		buttonGOBACKSETTINGS->position.y -= 1000 * dt;
		imageSETTINGS->position.y -= 1000 * dt;
		checkboxFPS->position.y -= 1000 * dt;
		labelFPS->position.y -= 1000 * dt;
		sliderVOLUMEMUSIC->position.y -= 1000 * dt;
		labelMUSICVOLUME->position.y -= 1000 * dt;
		sliderVOLUMEFX->position.y -= 1000 * dt;
		labelVOLUMEFX->position.y -= 1000 * dt;
		
		if (buttonGOBACKSETTINGS->position.y <= 100) {
			Positioned = true;
			buttonGOBACKSETTINGS->pressed = false;
			SettingMenuDone = true;
		}
	}
	if (!Positioned && SettingMenuDone) { //MENU GOING DOWN

		buttonGOBACKSETTINGS->position.y += 2000 * dt;
		imageSETTINGS->position.y += 2000 * dt;
		checkboxFPS->position.y += 2000 * dt;
		labelFPS->position.y += 2000 * dt;
		sliderVOLUMEMUSIC->position.y += 2000 * dt;
		labelMUSICVOLUME->position.y += 2000 * dt;
		sliderVOLUMEFX->position.y += 2000 * dt;
		labelVOLUMEFX->position.y += 2000 * dt;
		if (buttonGOBACKSETTINGS->position.y >= 1225) {
			buttonSTART->NoUse = false;
			buttonCONTINUE->NoUse = false;
			buttonSETTINGS->NoUse = false;
			buttonEXIT->NoUse = false;
			buttonHACKS->NoUse = false;
			buttonCREDITS->NoUse = false;
			InSettings = false;
			InMainMenu = true;
		}
	}
	if (SettingMenuDone) { //MENU LOGIC BUTTONS
		if (buttonGOBACKSETTINGS->pressed) {
			Positioned = false;
		}
		if (checkboxFPS->pressed) {
			App->capactivated = true;
		}
		if (!checkboxFPS->pressed) {
			App->capactivated = false;
		}
		
		App->audio->volume = sliderVOLUMEMUSIC->Value;
		App->audio->fxvolume = sliderVOLUMEFX->Value;

	}
}

void j1Choose::CreatehacksButtons()
{
	HacksMenuDone = false;
	imageHACKS = App->ui_manager->CreateImage(170, -700, true);
	imageHACKS->SetSpritesData({ 758,0,705,671 });
	imageHACKS->type = BUTTON;
	buttonGOBACKHACKS = App->ui_manager->CreateButton(200, -665, 3);
	buttonGOBACKHACKS->SetSpritesData({ 559,0,39,31 }, { 652,0,39,31 }, { 608,0,39,28 });
	labelGODMODE = App->ui_manager->CreateLabel(270, -600, "GODMODE", 50, true);
	checkboxGODMODE = App->ui_manager->CreateCheckBox(550, -600);

}

void j1Choose::HacksMenu(float dt)
{
	Title->NoRenderLabel = true;
	sentence->NoRenderLabel = true;
	if (imageHACKS->position.y + imageHACKS->height >= buttonEXIT->position.y) {
		buttonEXIT->WantToRender = false;
	}
	if (imageHACKS->position.y + imageHACKS->height >= buttonHACKS->position.y) {
		buttonHACKS->WantToRender = false;
	}
	if (imageHACKS->position.y + imageHACKS->height >= buttonCREDITS->position.y) {
		buttonCREDITS->WantToRender = false;
	}
	if (imageHACKS->position.y + imageHACKS->height >= buttonSETTINGS->position.y) {
		buttonSETTINGS->WantToRender = false;
	}
	if (imageHACKS->position.y + imageHACKS->height >= buttonCONTINUE->position.y) {
		buttonCONTINUE->WantToRender = false;
	}
	if (imageHACKS->position.y + imageHACKS->height >= buttonSTART->position.y) {
		buttonSTART->WantToRender = false;
	}
	if (imageHACKS->position.y + imageHACKS->height <= buttonEXIT->position.y) {
		buttonEXIT->WantToRender = true;
	}
	if (imageHACKS->position.y + imageHACKS->height <= buttonCREDITS->position.y) {
		buttonCREDITS->WantToRender = true;
	}
	if (imageHACKS->position.y + imageHACKS->height <= buttonHACKS->position.y) {
		buttonHACKS->WantToRender = true;
	}
	if (imageHACKS->position.y + imageHACKS->height <= buttonSETTINGS->position.y) {
		buttonSETTINGS->WantToRender = true;
	}
	if (imageHACKS->position.y + imageHACKS->height <= buttonCONTINUE->position.y) {
		buttonCONTINUE->WantToRender = true;
	}
	if (imageHACKS->position.y + imageHACKS->height <= buttonSTART->position.y) {
		buttonSTART->WantToRender = true;
	}
	if (!positioned && !HacksMenuDone) { //MENU GOING UP

		buttonGOBACKHACKS->position.y += 1000 * dt;
		imageHACKS->position.y += 1000 * dt;
		labelGODMODE->position.y += 1000 * dt;
		checkboxGODMODE->position.y += 1000 * dt;
		if (imageHACKS->position.y + imageHACKS->height >= 700) {
			positioned = true;
			buttonGOBACKHACKS->pressed = false;
			HacksMenuDone = true;
		}
	}
	if (!positioned && HacksMenuDone) { //MENU GOING DOWN

		buttonGOBACKHACKS->position.y -= 2000 * dt;
		imageHACKS->position.y -= 2000 * dt;
		labelGODMODE->position.y -= 2000 * dt;
		checkboxGODMODE->position.y -= 2000 * dt;
		if (buttonGOBACKHACKS->position.y <= -700) {
			buttonSTART->NoUse = false;
			buttonCONTINUE->NoUse = false;
			buttonSETTINGS->NoUse = false;
			buttonHACKS->NoUse = false;
			buttonEXIT->NoUse = false;
			buttonCREDITS->NoUse = false;
			InHacks = false;
			InMainMenu = true;
		}
	}
	if (HacksMenuDone) { //MENU LOGIC BUTTONS
		if (buttonGOBACKHACKS->pressed) {
			positioned = false;
		}
		if (checkboxGODMODE->pressed && !App->entitymanager->GetPlayerData()->God) {
			App->entitymanager->GetPlayerData()->God = true;
		}
		if (!checkboxGODMODE->pressed && App->entitymanager->GetPlayerData()->God) {
			App->entitymanager->GetPlayerData()->God = false;
		}
	}
}

void j1Choose::StartLevel()
{
	App->ui_manager->DeleteAllUI();
	App->entitymanager->GetPlayerData()->Intro = true;
	App->fade->FadeToBlack(3.0f);
	GoStart = true;
}






