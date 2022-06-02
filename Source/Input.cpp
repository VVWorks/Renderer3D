#define is_down(b) inp.buttons[b].is_down
#define pressed(b) inp.buttons[b].is_down && inp.buttons[b].changed

struct Button_state{
	bool is_down;
	bool changed;
};

enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_W,
	BUTTON_A,
	BUTTON_D,
	BUTTON_S,
	BUTTON_SPACE,
	BUTTON_E,
	BUTTON_TAB,
	BUTTON_ESC,
	BUTTON_SHIFT,
	//....
	BUTTON_COUNT, //last
};

struct INPUTs{
	Button_state buttons[BUTTON_COUNT];
};

void Evaluate_Message(unsigned int vk_code, bool is_down, INPUTs& inp) {
	switch (vk_code){
		case VK_UP: {
			inp.buttons[BUTTON_UP].is_down = is_down;
			inp.buttons[BUTTON_UP].changed = true;
		}break;
		case VK_DOWN: {
			inp.buttons[BUTTON_DOWN].is_down = is_down;
			inp.buttons[BUTTON_DOWN].changed = true;
		}break;
		case VK_LEFT: {
			inp.buttons[BUTTON_LEFT].is_down = is_down;
			inp.buttons[BUTTON_LEFT].changed = true;
		}break;
		case VK_RIGHT: {
			inp.buttons[BUTTON_RIGHT].is_down = is_down;
			inp.buttons[BUTTON_RIGHT].changed = true;
		}break;
		case VK_SPACE: {
			inp.buttons[BUTTON_SPACE].is_down = is_down;
			inp.buttons[BUTTON_SPACE].changed = true;
		}break;
		case 'W': {
			inp.buttons[BUTTON_W].is_down = is_down;
			inp.buttons[BUTTON_W].changed = true;
		}break;
		case 'A': {
			inp.buttons[BUTTON_A].is_down = is_down;
			inp.buttons[BUTTON_A].changed = true;
		}break;
		case 'S': {
			inp.buttons[BUTTON_S].is_down = is_down;
			inp.buttons[BUTTON_S].changed = true;
		}break;
		case 'D': {
			inp.buttons[BUTTON_D].is_down = is_down;
			inp.buttons[BUTTON_D].changed = true;
		}break;
		case 'E': {
			inp.buttons[BUTTON_E].is_down = is_down;
			inp.buttons[BUTTON_E].changed = true;
		}break;
		case VK_TAB: {
			inp.buttons[BUTTON_TAB].is_down = is_down;
			inp.buttons[BUTTON_TAB].changed = true;
		}break;
		case VK_ESCAPE: {
			inp.buttons[BUTTON_ESC].is_down = is_down;
			inp.buttons[BUTTON_ESC].changed = true;
		}break;
		case VK_SHIFT: {
			inp.buttons[BUTTON_SHIFT].is_down = is_down;
			inp.buttons[BUTTON_SHIFT].changed = true;
		}break;
	}
}