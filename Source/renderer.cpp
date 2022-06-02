#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//hex values of some colors
#define Aqua 0xa4d8eb
#define Blue 0x1f3fbf
#define Black 0x000000
#define White 0xffffff
#define Red	0xff0000
#define Green 0x00ff0d
#define Yellow 0xeeff00
#define Purple 0xe603ff

//from RBG to hex
unsigned int getHex(RGBc col)
{
	return ((col.r & 0xff) << 16) + ((col.g & 0xff) << 8) + (col.b & 0xff);
}

//from hex to RGB
RGBc Hex_RGB(const unsigned int hexValue)//hext to rgb color
{
	struct RGBc rgbColor;
	rgbColor.r = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
	rgbColor.g = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
	rgbColor.b = ((hexValue) & 0xFF);        // Extract the BB byte

	return rgbColor;
}

//simple image structure that loads an image using the stbi_load from stb_image.h
struct Image {
	//RGB colors
	int width, heigth, n;
	unsigned char* img;
	bool loaded = false;

	bool Load(std::string path) {
		img = stbi_load(path.c_str(), &width, &heigth, &n, 3);
		if (img != nullptr) {
			loaded = true;
		}
		return (img != nullptr);
	}

	RGBc getColorN(float xf, float yf) {
		int x = width * xf;
		int y = heigth - heigth * yf;

		if (x >= width)
			x = width - 1;

		if (y >= heigth) {
			y = heigth - 1;
		}

		return getColor(x, y);
	}

	RGBc getColor(int x, int y) {
		int index = 3 * (y * width + x);
		return RGBc((img[index + 0]), (img[index + 1]), (img[index + 2]));
	}
};

//also updates the z-buffer
static void renderer_clear_screen(const unsigned int color) {
	unsigned int* pixel = (unsigned int*)Renderer.memory;

	for (int y = 0; y < Renderer.heigth; y++) {
		for (int x = 0; x < Renderer.width; x++) {
			*pixel++ = color;
			Renderer.z_buffer[y * Renderer.width + x] = 0.0f;
		}
	}
}

//independent of the z-buffer!
static void renderer_draw_pixel_nonz(int x, int y, const unsigned int color) {
	x = Renderer.width - x;
	if (x < Renderer.width && y < Renderer.heigth && x >= 0 && y >= 0) {
		unsigned int* pixel = (unsigned int*)Renderer.memory + x + y * Renderer.width;
		*pixel = color;
	}
}

//dependent of the z-buffer!
static void renderer_draw_pixel(int x, int y, const unsigned int color, float zb) {
	x = Renderer.width - x;
	if (x < Renderer.width && y < Renderer.heigth && x >= 0 && y >= 0) {
		unsigned int* pixel = (unsigned int*)Renderer.memory + x + y * Renderer.width;
		float* z = (float*)Renderer.z_buffer + x + y * Renderer.width;

		if (Renderer.z_buffer[y * Renderer.width + x] <= zb) {
			Renderer.z_buffer[y * Renderer.width + x] = zb;
			*pixel = color;
		}
	}
}

//simple draw the given texture on the screen
static void renderer_draw_texture(int x, int y, Image& Texture) {
	for (int xs = x; xs < x + Texture.width; xs++) {
		for (int ys = y; ys < y + Texture.heigth; ys++) {
			RGBc col = Texture.getColor(xs - x, ys - y);
			renderer_draw_pixel(xs, ys, getHex(col), -1.0f);
		}
	}
}

//draw a line, also used in draw triangle functions
static void renderer_draw_line_px(int x0, int y0, int x1, int y1, const unsigned int color, float z) {
	int xMax = max(x0, x1), xMin = min(x0, x1);
	int yMax = max(y0, y1), yMin = min(y0, y1);

	int dx, dy;
	if (x0 <= x1) {
		dx = (x0 - x1);
		dy = (y0 - y1);
	}
	else {
		dx = (x1 - x0);
		dy = (y1 - y0);
	}

	if (dx == 0) {
		for (int y = yMin; y <= yMax; y++) {
			renderer_draw_pixel(x0, y, color, z);
		}

		return;
	}

	if (dy == 0) {
		for (int x = xMin; x <= xMax; x++) {
			renderer_draw_pixel(x, y0, color, z);
		}

		return;
	}

	if (abs(dx) >= abs(dy)) {
		float m = (float)dy / dx;
		float n = y0 - m * x0;

		for (int r_x = xMin; r_x <= xMax; r_x++) {
			int r_y = m * r_x + n;

			renderer_draw_pixel(r_x, r_y, color, z);
		}
	}
	else {
		float m = (float)dy / dx;
		float n = y0 - m * x0;

		for (int r_y = yMin; r_y <= yMax; r_y++) {
			int r_x = (r_y - n) / m;

			renderer_draw_pixel(r_x, r_y, color, z);
		}
	}
}

//draw a rect (not useful in 3D rendering)
static void renderer_draw_rect_px(int x0, int y0, int x1, int y1, const unsigned int color, bool fill) {
	if (!fill) {
		renderer_draw_line_px(x0, y0, x0, y1, color, 0);
		renderer_draw_line_px(x0, y0, x1, y0, color, 0);
		renderer_draw_line_px(x1, y1, x0, y1, color, 0);
		renderer_draw_line_px(x1, y1, x1, y0, color, 0);
	}
	else {
		y0 = Renderer.heigth - y0;
		y1 = Renderer.heigth - y1;

		x0 = clamp(0, Renderer.width, x0);
		x1 = clamp(0, Renderer.width, x1);
		y0 = clamp(0, Renderer.heigth, y0);
		y1 = clamp(0, Renderer.heigth, y1);

		int xMax = max(x0, x1), xMin = min(x0, x1);
		int yMax = max(y0, y1), yMin = min(y0, y1);
		for (int y = yMin; y <= yMax; y++) {
			unsigned int* pixel = (unsigned int*)Renderer.memory + x0 + y * Renderer.width;
			for (int x = xMin; x <= xMax; x++) {
				*pixel++ = color;
			}
		}
	}
}

//draw a simple circle (not useful in 3D rendering)
static void renderer_draw_circle_px(int x0, int y0, int radius, const unsigned int color, bool fill) {

	for (int x = -radius; x <= radius; x++) {
		for (int y = -radius; y <= radius; y++) {
			int r_x = x + x0, r_y = y + y0;

			if (r_x >= 0 && r_y >= 0 && r_x < Renderer.width && r_y < Renderer.heigth) {
				if (fill) {
					if (x * x + y * y < radius * radius + radius) {
						renderer_draw_pixel(r_x, r_y, color, 0);
					}
				}
				else {
					if (x * x + y * y > radius * radius - radius && x * x + y * y < radius * radius + radius) {
						renderer_draw_pixel(r_x, r_y, color, 0);
					}
				}
			}
		}
	}
}

//this is used to draw a triangle with one of the lines parallel to the x-axis
static void FillFlatTri_px(int x0, int y0, int x01, int y01, int x2, int y2, const unsigned int color, float z) {//call only if y0 == y01
	int dx0, dx1, dy0, dy1;

	if (x0 <= x2) {
		dx0 = x0 - x2;
		dy0 = y0 - y2;
	}
	else {
		dx0 = x2 - x0;
		dy0 = y2 - y0;
	}

	if (x01 <= x2) {
		dx1 = x01 - x2;
		dy1 = y01 - y2;
	}
	else {
		dx1 = x2 - x01;
		dy1 = y2 - y01;
	}

	if (dx0 == 0 && dx1 == 0) { return; }

	if (dx0 == 0) {
		float m1 = (float)dy1 / (float)dx1;
		if (m1 == 0) {
			return;
		}

		float n1 = y01 - m1 * x01;
		for (int r_y = min(y0, y2); r_y <= max(y0, y2); r_y++) {
			int r_x1 = (r_y - n1) / m1;

			renderer_draw_line_px(r_x1, r_y, x0, r_y, color, z);
		}

		return;
	}

	if (dx1 == 0) {
		float m0 = (float)dy0 / (float)dx0;
		if (m0 == 0) {
			return;
		}

		float n0 = y0 - m0 * x0;
		for (int r_y = min(y0, y2); r_y <= max(y0, y2); r_y++) {
			int r_x0 = (r_y - n0) / m0;

			renderer_draw_line_px(r_x0, r_y, x01, r_y, color, z);
		}

		return;
	}

	float m0 = (float)dy0 / (float)dx0;
	float m1 = (float)dy1 / (float)dx1;

	if (m0 == 0 || m1 == 0) {
		return;
	}

	float n0 = y0 - m0 * x0;
	float n1 = y01 - m1 * x01;

	for (int r_y = min(y0, y2); r_y <= max(y0, y2); r_y++) {
		int r_x0 = (r_y - n0) / m0;
		int r_x1 = (r_y - n1) / m1;

		renderer_draw_line_px(r_x0, r_y, r_x1, r_y, color, z);
	}
}

//draw a triangle, does not deal with textures!
static void renderer_draw_triangle_px(int x0, int y0, int x1, int y1, int x2, int y2, const RGBc colorRGB, bool fill, float z) {//the function used to draw the triangles
	unsigned int color = getHex(colorRGB);

	if(!fill) {
		renderer_draw_line_px(x0, y0, x1, y1, color, z);
		renderer_draw_line_px(x0, y0, x2, y2, color, z);
		renderer_draw_line_px(x1, y1, x2, y2, color, z);
	}
	else{
		if (y0 == y1) {
			FillFlatTri_px(x0, y0, x1, y1, x2, y2, color, z);
			return;
		}

		if (y0 == y2) {
			FillFlatTri_px(x0, y0, x2, y2, x1, y1, color, z);
			return;
		}

		if (y2 == y1) {
			FillFlatTri_px(x2, y2, x1, y1, x0, y0, color, z);
			return;
		}

		int yMiddle = middle(y0, y1, y2);
		int xMiddle; 

		if (yMiddle == y0) {
			xMiddle = x0;
		}
		else if (yMiddle == y1) {
			xMiddle = x1;
		}
		else {
			xMiddle = x2;
		}

		int yMax = max(max(y0, y1), y2);
		int yMin = min(min(y0, y1), y2);
		int xMax, xMin;

		if (yMax == y0) {
			xMax = x0;
		}
		else if (yMax == y1) {
			xMax = x1;
		}
		else {
			xMax = x2;
		}

		if (yMin == y0) {
			xMin = x0;
		}
		else if (yMin == y1) {
			xMin = x1;
		}
		else {
			xMin = x2;
		}

		int dy, dx;
		if (xMax <= xMin) {
			dx = xMax - xMin;
			dy = yMax - yMin;
		}
		else {
			dx = xMin - xMax;
			dy = yMin - yMax;
		}

		if (dx == 0 && dy == 0) {
			return;
		}

		int opX;

		if (dx == 0) {
			opX = xMax;
		}
		else {
			float m = (float)dy / (float)dx;
			float n = yMax - xMax * m;

			opX = (yMiddle - n) / m;
		}

		FillFlatTri_px(xMiddle, yMiddle, opX, yMiddle, xMax, yMax, color, z);
		FillFlatTri_px(xMiddle, yMiddle, opX, yMiddle, xMin, yMin, color, z);
	}
}

//just the same algorithm as the one above but modified to deal with textures and z-depth 
//could use the Bresenham Algorithm as well for a simpler implementation but not as fast
static void renderer_draw_tri_texture_px(int x1, int y1, float u1, float v1, float w1,
	int x2, int y2, float u2, float v2, float w2,
	int x3, int y3, float u3, float v3, float w3,
	Image* tex, float lum, bool haveTexture)
{
	if (y2 < y1){
		std::swap(y1, y2);
		std::swap(x1, x2);
		std::swap(u1, u2);
		std::swap(v1, v2);
		std::swap(w1, w2);
	}

	if (y3 < y1){
		std::swap(y1, y3);
		std::swap(x1, x3);
		std::swap(u1, u3);
		std::swap(v1, v3);
		std::swap(w1, w3);
	}

	if (y3 < y2){
		std::swap(y2, y3);
		std::swap(x2, x3);
		std::swap(u2, u3);
		std::swap(v2, v3);
		std::swap(w2, w3);
	}

	int dy1 = y2 - y1;
	int dx1 = x2 - x1;
	float dv1 = v2 - v1;
	float du1 = u2 - u1;
	float dw1 = w2 - w1;

	int dy2 = y3 - y1;
	int dx2 = x3 - x1;
	float dv2 = v3 - v1;
	float du2 = u3 - u1;
	float dw2 = w3 - w1;

	float tex_u, tex_v, tex_w;

	float dax_step = 0, dbx_step = 0,
		du1_step = 0, dv1_step = 0,
		du2_step = 0, dv2_step = 0,
		dw1_step = 0, dw2_step = 0;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	if (dy2) du2_step = du2 / (float)abs(dy2);
	if (dy2) dv2_step = dv2 / (float)abs(dy2);
	if (dy2) dw2_step = dw2 / (float)abs(dy2);

	if (dy1){
		for (int i = y1; i <= y2; i++)
		{
			int ax = x1 + (float)(i - y1) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u1 + (float)(i - y1) * du1_step;
			float tex_sv = v1 + (float)(i - y1) * dv1_step;
			float tex_sw = w1 + (float)(i - y1) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx){
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}

			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++){
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;

				int t_x = Renderer.width - j, t_y = i;
				if (t_x < Renderer.width && t_y < Renderer.heigth && t_x >= 0 && t_y >= 0) {
					unsigned int* pixel = (unsigned int*)Renderer.memory + t_x + t_y * Renderer.width;

					if (tex_w > Renderer.z_buffer[t_y * Renderer.width + t_x]) {
						Renderer.z_buffer[t_y * Renderer.width + t_x] = tex_w;
						RGBc COL;

						if (!haveTexture) {
							COL = RGBc(255 * lum, 255 * lum, 255 * lum);
						}
						else {
							COL = tex->getColorN(tex_u / tex_w, tex_v / tex_w);
							COL.b *= lum;
							COL.r *= lum;
							COL.g *= lum;
						}

						*pixel = getHex(COL);
					}
				}
				t += tstep;
			}

		}
	}

	dy1 = y3 - y2;
	dx1 = x3 - x2;
	dv1 = v3 - v2;
	du1 = u3 - u2;
	dw1 = w3 - w2;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	du1_step = 0, dv1_step = 0;
	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	if (dy1){
		for (int i = y2; i <= y3; i++){
			int ax = x2 + (float)(i - y2) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u2 + (float)(i - y2) * du1_step;
			float tex_sv = v2 + (float)(i - y2) * dv1_step;
			float tex_sw = w2 + (float)(i - y2) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx){
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}

			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++){
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;

				int t_x = Renderer.width - j, t_y = i;
				if (t_x < Renderer.width && t_y < Renderer.heigth && t_x >= 0 && t_y >= 0) {
					unsigned int* pixel = (unsigned int*)Renderer.memory + t_x + t_y * Renderer.width;

					if (tex_w > Renderer.z_buffer[t_y * Renderer.width + t_x]) {
						Renderer.z_buffer[t_y * Renderer.width + t_x]  = tex_w;
						RGBc COL;

						if (!haveTexture) {
							COL = RGBc(255 * lum, 255 * lum, 255 * lum);
						}
						else {
							COL = tex->getColorN(tex_u / tex_w, tex_v / tex_w);
							COL.b *= lum;
							COL.r *= lum;
							COL.g *= lum;
						}

						*pixel = getHex(COL);
					}
				}
				t += tstep;
			}
		}
	}
}

