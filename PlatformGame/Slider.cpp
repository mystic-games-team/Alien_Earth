#include "Slider.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "p2Log.h"
#include "UI_Manager.h"
#include "j1Audio.h"
#include "j1Input.h"

Slider::Slider(int x, int y, int SliderPos) : UI_Element(x, y)
{

	width = 29;
	height = 31;

	NoPressedNoMouseOn = { 569,43,29,31 };
	Pressed = { 662,43,29,31 };
	MouseOn = { 618,43,29,31 };

	png_pos = NoPressedNoMouseOn;

	image = App->ui_manager->CreateImage(x, y, true);
	image->SetSpritesData({ 553,89,200,49 });
	image->type = SLIDER;
	position.y = y + image->height / 2 - height / 2;
	Value = SliderPos;
	int RelPos = image->width - 15;
	
	position.x = ((((Value / (100 / 78.5))+1)*image->width) / 100) + 5 + image->position.x;
}

Slider::~Slider()
{
}

bool Slider::Update(float dt)
{
	image->position.y = position.y - height/2 + 5;
	if (!pressed) {
		if (IsMouseOn()) {
			png_pos = MouseOn;
			mouseOn = true;
			if (repeataudio) {
				App->audio->PlayFx(fxOn);
				repeataudio = false;
			}
			if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
				App->input->GetMousePosition(mouse_pos.x, mouse_pos.y);
				distance = mouse_pos.x - position.x;
				pressed = true;
				if (repeat2) {
					repeat2 = false;
					App->audio->PlayFx(fxPressed);
				}
			}
			else pressed = false;
		}
		else {
			repeataudio = true;
			repeat2 = true;
			png_pos = NoPressedNoMouseOn;
			mouseOn = false;
			pressed = false;
		}
	}
	if (pressed) {
		if (IsMouseOn())
			png_pos = MouseOn;
		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
			png_pos = Pressed;
			App->input->GetMousePosition(mouse_pos.x, mouse_pos.y);
			position.x = mouse_pos.x - distance;
			LookLimits();
		}
	}
	if (!IsMouseOn())
		pressed = false;

	Value = (((100 * (position.x - 5 - image->position.x)) / (image->width))-1)*(100/78.5);
	
	
	return true;
}

void Slider::LookLimits()
{
	if (position.x <= image->position.x + MARGIN) {
		position.x = image->position.x + MARGIN;
	}
	if (position.x + width >= image->position.x + image->width - MARGIN) {
		position.x = -width + image->position.x + image->width - MARGIN;
	}

}