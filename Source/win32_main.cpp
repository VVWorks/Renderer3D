//Author: Vornicescu Vasile

#include <Windows.h>
#include <math.h>
#include <ctime>
#include <algorithm>
#include <list>
#include <sstream>

static bool Running = true;
static bool DEBUG = false;

struct Renderer_state {
	int heigth = 700, width = 700;
	void* memory;
	float* z_buffer = nullptr;
	int memsize;

	BITMAPINFO bitmap_info;
};

HDC hdc;
static Renderer_state Renderer;

#include "Math.cpp"
#include "tools.cpp"
#include "renderer.cpp"
#include "Renderer3D.cpp"
#include "Input.cpp"

LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT rezult = 0;

	switch (uMsg) {
	case WM_CLOSE:
	case WM_DESTROY: {
		Running = false;
	} break;

	case WM_SIZE: {
		RECT rect;
		GetClientRect(hwnd, &rect);
		Renderer.width = rect.right - rect.left;
		Renderer.heigth = rect.bottom - rect.top;

		//z_buffer
		delete[] Renderer.z_buffer;
		Renderer.z_buffer = new float[Renderer.width * Renderer.heigth];

		//buffer
		Renderer.memsize = Renderer.width * Renderer.heigth * sizeof(unsigned int);

		if (Renderer.memory) {
			VirtualFree(Renderer.memory, 0, MEM_RELEASE);
		}
		Renderer.memory = VirtualAlloc(0, Renderer.memsize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		Renderer.bitmap_info.bmiHeader.biSize = sizeof(Renderer.bitmap_info.bmiHeader);
		Renderer.bitmap_info.bmiHeader.biWidth = Renderer.width;
		Renderer.bitmap_info.bmiHeader.biHeight = Renderer.heigth;
		Renderer.bitmap_info.bmiHeader.biPlanes = 1;
		Renderer.bitmap_info.bmiHeader.biBitCount = 32;
		Renderer.bitmap_info.bmiHeader.biCompression = BI_RGB;
	} break;

	default: {
		rezult = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
	return rezult;
}

long double deltaTime = 0.0f;

std::vector<Object> Objects;

vec3d light_direction = { 0.0f, 0.0f, -1.0f };
INPUTs inp = { };
Camera mainCamera;
matrix4x4 ProjectionMatrix, matRotZ, matRotX, matRotY;

POINT middleP;

float fNear;
float fFar;
float fFov;
float fAstepctRatio;
float fFovRad;
float q;

void Start() {
	Mesh mesh; mesh.loadMesh("Meshes/Earth/Earth.obj");
	Object o1; o1.mesh = mesh; o1.pos = vec3d(7.0f, 0.0f, 12.0f); o1.rotation = vec3d(0.0f, 0.0f, 0.0f);
	o1.mesh.Texture.Load("Meshes/Earth/Earth.jpg");

	o1.rotation.x = 0.3f;

	mainCamera.fYaw = -0.45f;

	Objects.push_back(o1);
}

void Update() {

	if (is_down(BUTTON_ESC)) {
		Running = false;
	}

	Objects[0].rotation.y += 0.1f * deltaTime;

	ShowCursor(false);
	POINT mouse;
	if (GetCursorPos(&mouse)) {
		float deltaX = Renderer.width / 2 - mouse.x; 
		float deltaY = Renderer.heigth / 2 - mouse.y;

		if (deltaX != 0.0f) {
			deltaX = (deltaX / abs(deltaX)) * 3.0f;
		}
		if (deltaY != 0.0f) {
			deltaY = (deltaY / abs(deltaY)) * 3.0f;
		}

		mainCamera.fYaw -= (deltaX * deltaTime);
		mainCamera.fPitch -= (deltaY * deltaTime);

		//clamp pitch to pi/2
		if (mainCamera.fPitch <= -1.5) {
			mainCamera.fPitch = -1.5;
		}

		if (mainCamera.fPitch >= 1.5) {
			mainCamera.fPitch = 1.5;
		}
	}

	SetCursorPos(Renderer.width / 2, Renderer.heigth / 2);

	if (is_down(BUTTON_TAB)) {
		DEBUG = true;
	}
	else {
		DEBUG = false;
	}

	if (is_down(BUTTON_D)) {
		vec3d mov{ -mainCamera.lookDir.z, 0, mainCamera.lookDir.x }; mov.Normalize();
		mainCamera.cameraPos += mov * (5.0f * deltaTime);
	}

	if (is_down(BUTTON_A)) {
		vec3d mov{ mainCamera.lookDir.z, 0, -mainCamera.lookDir.x }; mov.Normalize();
		mainCamera.cameraPos += mov * (5.0f * deltaTime);
	}

	if (is_down(BUTTON_W)) {
		vec3d vForward = mainCamera.lookDir * (5.0f * deltaTime);

		mainCamera.cameraPos += vForward;
	}

	if (is_down(BUTTON_S)) {
		vec3d vForward = mainCamera.lookDir * (5.0f * deltaTime);

		mainCamera.cameraPos -= vForward;
	}

	if (is_down(BUTTON_SPACE)) {
		mainCamera.cameraPos.y += 5.0f * deltaTime;
	}

	if (is_down(BUTTON_SHIFT)) {
		mainCamera.cameraPos.y -= 5.0f * deltaTime;
	}
}

int  WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	fNear = 0.1f;
	fFar = 1000.0f;
	fFov = 90.0f;
	fAstepctRatio = (float)Renderer.heigth / (float)Renderer.width;
	fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);
	q = fFar / (fFar - fNear);

	ProjectionMatrix.m[0][0] = fAstepctRatio * fFovRad;
	ProjectionMatrix.m[1][1] = fFovRad;
	ProjectionMatrix.m[2][2] = q;
	ProjectionMatrix.m[3][2] = -q * fNear;
	ProjectionMatrix.m[2][3] = 1.0f;
	ProjectionMatrix.m[3][3] = 0.0f;
	
	//win class

	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = L"Main_Wind_Class";
	window_class.lpfnWndProc = window_callback;

	//register class
	RegisterClass(&window_class);

	//create window
	HWND window = CreateWindow(window_class.lpszClassName, L"3D Renderer", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 700, 700, 0, 0, hInstance, 0);
	ShowWindow(window, SW_MAXIMIZE);
	
	hdc = GetDC(window);

	Start();

	Renderer.z_buffer = new float[Renderer.width * Renderer.heigth];
	renderer_clear_screen(Black);

	std::clock_t c_start = std::clock();
	//main loop
	while (Running) {
		//input
		MSG message;

		for (int i = 0; i < BUTTON_COUNT; i++) {
			inp.buttons[i].changed = false;
		}

		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			switch (message.message){
				case WM_KEYUP:
				case WM_KEYDOWN: {
					unsigned int vk_code = (unsigned int)message.wParam;
					bool is_down = ((message.lParam & (1 << 31)) == 0);

					Evaluate_Message(vk_code, is_down, inp);
				} break;

				default: {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
		}

		//Logic
		renderer_clear_screen(Black);
		Update();

		//Renderer
		fAstepctRatio = (float)Renderer.heigth / (float)Renderer.width;
		ProjectionMatrix.m[0][0] = fAstepctRatio * fFovRad;

		for (int k = 0; k < Objects.size(); k++) {

			RotateZ(Objects[k].rotation.z, matRotZ);
			RotateX(Objects[k].rotation.x, matRotX);
			RotateY(Objects[k].rotation.y, matRotY);

			matrix4x4 matTrans;
			matTrans = Matrix_Translation(Objects[k].pos.x, Objects[k].pos.y, Objects[k].pos.z);

			matrix4x4 matWorld;
			matWorld = Matrix_identity();
			matWorld = matRotZ * matRotX * matRotY;
			matWorld = matWorld * matTrans;

			//camera view
			vec3d vUp = { 0.0f, 1.0f, 0.0f };
			vec3d vTarget = { 0.0f, 0.0f, 1.0f };
			matrix4x4 matCameraRotY, matCameraRotX, finalCamera;
			RotateY(mainCamera.fYaw, matCameraRotY);
			RotateX(mainCamera.fPitch, matCameraRotX);

			finalCamera = matCameraRotX * matCameraRotY;

			mainCamera.lookDir = Matrix_MuliplyVector(finalCamera, vTarget);
			vTarget = mainCamera.cameraPos + mainCamera.lookDir;

			matrix4x4 matCamera = Matrix_PointAt(mainCamera.cameraPos, vTarget, vUp);
			matrix4x4 matView = Matrix_PseudeInverse(matCamera);

			//object tris:
			for (auto tri : Objects[k].mesh.tris) {

				triangle triProjected, triTranslated, triViewed;

				triTranslated.p[0] = Matrix_MuliplyVector(matWorld, tri.p[0]);
				triTranslated.p[1] = Matrix_MuliplyVector(matWorld, tri.p[1]);
				triTranslated.p[2] = Matrix_MuliplyVector(matWorld, tri.p[2]);

				vec3d normal, line1, line2;

				line1 = triTranslated.p[1] - triTranslated.p[0];
				line2 = triTranslated.p[2] - triTranslated.p[0];

				normal = line1 * line2; normal.Normalize();

				vec3d cameraRay = triTranslated.p[0] - mainCamera.cameraPos;

				if (DotProduct(normal, cameraRay) < 0.0f) {
					//illumination
					
					light_direction.Normalize();

					float lum = DotProduct(normal, light_direction);

					if (lum < 0.4f) { lum = 0.4f; }
					
						//world space to view space:
						triViewed.p[0] = Matrix_MuliplyVector(matView, triTranslated.p[0]);
						triViewed.p[1] = Matrix_MuliplyVector(matView, triTranslated.p[1]);
						triViewed.p[2] = Matrix_MuliplyVector(matView, triTranslated.p[2]);

						//copy uv cords
						triViewed.texture[0] = tri.texture[0];
						triViewed.texture[1] = tri.texture[1];
						triViewed.texture[2] = tri.texture[2];

						//world clipping
						int nClippedTris = 0;
						triangle clipped[2];
						nClippedTris = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

						for (int n = 0; n < nClippedTris; n++) {
							//project points in 2D
							triProjected.p[0] = Matrix_MuliplyVector(ProjectionMatrix, clipped[n].p[0]);
							triProjected.p[1] = Matrix_MuliplyVector(ProjectionMatrix, clipped[n].p[1]);
							triProjected.p[2] = Matrix_MuliplyVector(ProjectionMatrix, clipped[n].p[2]);
							triProjected.p[0] *= 1.0f/triProjected.p[0].w;
							triProjected.p[1] *= 1.0f/triProjected.p[1].w;
							triProjected.p[2] *= 1.0f/triProjected.p[2].w;

							//textures:
							triProjected.texture[0] = clipped[n].texture[0];
							triProjected.texture[1] = clipped[n].texture[1];
							triProjected.texture[2] = clipped[n].texture[2];


							triProjected.texture[0].u = triProjected.texture[0].u / triProjected.p[0].w;
							triProjected.texture[1].u = triProjected.texture[1].u / triProjected.p[1].w;
							triProjected.texture[2].u = triProjected.texture[2].u / triProjected.p[2].w;

							triProjected.texture[0].v = triProjected.texture[0].v / triProjected.p[0].w;
							triProjected.texture[1].v = triProjected.texture[1].v / triProjected.p[1].w;
							triProjected.texture[2].v = triProjected.texture[2].v / triProjected.p[2].w;

							triProjected.texture[0].w = 1.0f / triProjected.p[0].w;
							triProjected.texture[1].w = 1.0f / triProjected.p[1].w;
							triProjected.texture[2].w = 1.0f / triProjected.p[2].w;

							//scale back to screen
							triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
							triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
							triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

							triProjected.p[0].x *= 0.5f * (float)Renderer.width;
							triProjected.p[0].y *= 0.5f * (float)Renderer.heigth;

							triProjected.p[1].x *= 0.5f * (float)Renderer.width;
							triProjected.p[1].y *= 0.5f * (float)Renderer.heigth;

							triProjected.p[2].x *= 0.5f * (float)Renderer.width;
							triProjected.p[2].y *= 0.5f * (float)Renderer.heigth;

							//DRAWING and screen clipping:

							triangle clipped[2];
							std::list<triangle> listTriangles;

							listTriangles.push_back(triProjected);
							int nNewTriangles = 1;

							for (int p = 0; p < 4; p++)
							{
								int nTrisToAdd = 0;
								while (nNewTriangles > 0)
								{
									triangle test = listTriangles.front();
									listTriangles.pop_front();
									nNewTriangles--;

									switch (p)
									{
									case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
									case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)Renderer.heigth - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
									case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
									case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)Renderer.width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
									}

									for (int w = 0; w < nTrisToAdd; w++)
										listTriangles.push_back(clipped[w]);
								}
								nNewTriangles = listTriangles.size();
							}

							for (auto& t : listTriangles) {
								/*//draw triangle (no textures):
								renderer_draw_triangle_px(
									t.p[0].x, t.p[0].y,
									t.p[1].x, t.p[1].y,
									t.p[2].x, t.p[2].y,
									t.color, true,
									(t.p[0].z + t.p[1].z + t.p[2].z) / 3.0f
								);*/

								//draw with textures
								renderer_draw_tri_texture_px(t.p[0].x, t.p[0].y, t.texture[0].u, t.texture[0].v, t.texture[0].w,
									t.p[1].x, t.p[1].y, t.texture[1].u, t.texture[1].v, t.texture[1].w,
									t.p[2].x, t.p[2].y, t.texture[2].u, t.texture[2].v, t.texture[2].w, &Objects[k].mesh.Texture, lum, Objects[k].mesh.Texture.loaded);
								
								//debug lines:
								if (DEBUG)
									renderer_draw_triangle_px(
										t.p[0].x, t.p[0].y,
										t.p[1].x, t.p[1].y,
										t.p[2].x, t.p[2].y,
										RGBc(0, 0, 0), false,
										100.0f
									);
							
							}
						}
				}
			}
		}

		StretchDIBits(hdc, 0, 0, Renderer.width, Renderer.heigth, 0, 0, Renderer.width, Renderer.heigth, Renderer.memory, &Renderer.bitmap_info, DIB_RGB_COLORS, SRCCOPY);
	
		//delta time
		std::clock_t c_end = std::clock();
		deltaTime = long double(c_end - c_start) / CLOCKS_PER_SEC;
		c_start = c_end;

		std::wstringstream wss(L"");
		wss << (int)(1.0f / deltaTime)<<" FPS";

		SetWindowText(window, wss.str().c_str());
	}
}
