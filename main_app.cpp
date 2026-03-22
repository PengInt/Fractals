// The app embodiment of main_png
// Version a1.0.1
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <format>
#include <vector>
#include <fstream>

#include "include/raylib.h"

struct ComplexNumber {
	float Real; float Imaginary;
	ComplexNumber(float r, float i) {
		Real = r; Imaginary = i;
	}

	ComplexNumber operator*(ComplexNumber other) {
		float r1 = Real * other.Real;
		float r2 = -(Imaginary * other.Imaginary);
		float i1 = Real * other.Imaginary;
		float i2 = Imaginary * other.Real;
		return ComplexNumber(r1 + r2, i1 + i2);
	}
	ComplexNumber operator+(ComplexNumber other) {
		float r1 = Real + other.Real;
		float i1 = Imaginary + other.Imaginary;
		return ComplexNumber(r1, i1);
	}
	ComplexNumber operator/(ComplexNumber other) {
		float r1 = other.Real / (other.Real*other.Real + other.Imaginary*other.Imaginary);
		float i1 = -other.Imaginary / (other.Real*other.Real + other.Imaginary*other.Imaginary);
		return *this * ComplexNumber(r1, i1);
	}
	ComplexNumber operator-(ComplexNumber other) {
		float r1 = Real - other.Real;
		float i1 = Imaginary - other.Imaginary;
		return ComplexNumber(r1, i1);
	}
	template<std::integral T>
	ComplexNumber operator^(T other) {
		ComplexNumber n = ComplexNumber(1, 0);
		for (uint8_t i = 0; i < other; i++) {
			n = n * *this;
		}
		return n;
	}

	ComplexNumber operator*(float other) {
		float r1 = Real * other;
		float i1 = Imaginary * other;
		return ComplexNumber(r1, i1);
	}
	ComplexNumber operator+(float other) {
		float r1 = Real + other;
		float i1 = Imaginary;
		return ComplexNumber(r1, i1);
	}
	ComplexNumber operator-(float other) {
		float r1 = Real - other;
		float i1 = Imaginary;
		return ComplexNumber(r1, i1);
	}

	bool operator==(const ComplexNumber& other) const { if (Real == other.Real && Imaginary == other.Imaginary) { return true; } return false; }

	float magnitude() {
		return std::sqrt(Real*Real + Imaginary*Imaginary);
	}
	float magnitude2() {
		return Real*Real + Imaginary*Imaginary;
	}
};
std::ostream& operator<<(std::ostream& os, ComplexNumber c) {
	if (c.Real != 0 && c.Imaginary != 0) { os << c.Real << " + " << c.Imaginary << "i"; }
	else if (c.Real == 0 && c.Imaginary != 0) { os << c.Imaginary << "i"; }
	else if (c.Real != 0) { os << c.Real; }
	else { os << 0; }
	return os;
}
ComplexNumber operator*(float other, ComplexNumber self) {
	float r1 = self.Real * other;
	float i1 = self.Imaginary * other;
	return ComplexNumber(r1, i1);
}
ComplexNumber operator+(float other, ComplexNumber self) {
	float r1 = self.Real + other;
	float i1 = self.Imaginary;
	return ComplexNumber(r1, i1);
}
ComplexNumber operator-(float other, ComplexNumber self) {
	float r1 = other - self.Real;
	float i1 = -self.Imaginary;
	return ComplexNumber(r1, i1);
}

const ComplexNumber i = ComplexNumber(0, 1);

uint8_t mod(int n, int k) {
	// Handle edge cases for infinity or non-positive values
	if (std::isinf(static_cast<float>(n))) return 255;
	if (n <= 0) return 0;

	// As n -> inf, intensity -> 255
	// Formula: intensity = 255 * (n / (k + n))
	float intensity = 255.0f * (static_cast<float>(n) / (static_cast<float>(k + n)));

	return static_cast<uint8_t>(std::clamp(intensity, 0.0f, 255.0f));
}

uint8_t iterate_mandelbrot(ComplexNumber c, int max_i, float r0, float i0, int k) {
	ComplexNumber z = ComplexNumber(r0, i0);
	ComplexNumber oz = z;
	int ot = 0;
	int nt = 1;
	int iter = 0;
	while (iter < max_i) {
		z = z*z + c;
		iter++;
		if (z.magnitude2() > 4) { break; }
		if (oz == z) { return 0; }
		ot++;
		if (ot == nt) {
			nt *= 2;
			ot = 0;
			oz = z;
		}
	}
	return mod(iter, k);
}

Color Colour(float r, float g, float b, uint8_t mod_i) {
	return { (uint8_t) (r * mod_i), (uint8_t) (g * mod_i), (uint8_t) (b * mod_i), 255};
}

class Cam {
public:
	float real;
	float imag;
	float s_x;
	float s_y;
	Cam(int w, int h) {
		real = 0;
		imag = 0;
		s_x = 2.5;
		s_y = 2.5*(((float) h)/((float) w));
	}
	void RecalculateSize(int w, int h) {
		s_y = s_x*(((float) h)/((float) w));
	}
	std::vector<float> get_info(int w, int h) {
		float right = real + s_x;
		float left = real - s_x;
		float top = imag + s_y;
		float bottom = imag - s_y;
		float inc_x = (right - left)/((float) (w-1));
		float inc_y = (top - bottom)/((float) (h-1));
		return {left, right, bottom, top, inc_x, inc_y};
	}
	void move(int l_r, int d_u) {
		real += s_x*0.02*l_r;
		imag += s_y*0.02*d_u;
	}
	void zoom(bool in) {
		if (in) {
			s_x *= 0.9765625;
			s_y *= 0.9765625;
		} else {
			s_x *= 1.024;
			s_y *= 1.024;
		}
	}
};

int main() {
	std::ifstream cfg("app-settings.cfg");
	if (!cfg.is_open()) {
		std::cerr << "Error opening file app-settings.cfg" << std::endl;
		return 1;
	}

	std::string line;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file app-settings.cfg" << std::endl; }
	int w = std::stoi(line);
	const int cw = w;
	std::cout << "Width: " << w << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file app-settings.cfg" << std::endl; }
	int h = std::stoi(line);
	const int ch = h;
	std::cout << "Height: " << h << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file app-settings.cfg" << std::endl; }
	const int max_i = std::stoi(line);
	std::cout << "Maximum iterations: " << max_i << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file app-settings.cfg" << std::endl; }
	const std::string t = line;
	std::cout << "Type: " << t << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file app-settings.cfg" << std::endl; }
	const std::string c = line;
	std::cout << "Colour: " << c << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file app-settings.cfg" << std::endl; }
	const int k = std::stoi(line);
	std::cout << "K: " << k << std::endl;

	uint8_t type = 0;
	if (t == "mandelbrot") {
		type = 1;
	} else if (t == "julia") {
		type = 3;
	} else if (t == "julia swirl") {
		type = 4;
	} else if (t == "julia crystal") {
		type = 5;
	} else if (t == "nova") {
		type = 2;
	}

	float c_r = 0;
	float c_g = 0;
	float c_b = 0;
	if (c == "white") {
		c_r = 1;
		c_g = 1;
		c_b = 1;
	} else if (c == "red") {
		c_r = 1;
	} else if (c == "green") {
		c_g = 1;
	} else if (c == "blue") {
		c_b = 1;
	} else if (c == "yellow") {
		c_r = 1;
		c_g = 1;
	} else if (c == "cyan") {
		c_g = 1;
		c_b = 1;
	} else if (c == "magenta") {
		c_r = 1;
		c_b = 1;
	} else if (c == "orange") {
		c_r = 1;
		c_g = 0.5;
	} else if (c == "turquoise") {
		c_g = 1;
		c_b = 0.5;
	} else if (c == "purple") {
		c_r = 0.5;
		c_b = 1;
	}

	const Vector3 C = {c_r, c_g, c_b};

	SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT);
	InitWindow(w, h, "Fractal Renderer");
	SetTargetFPS(60);

	Cam cam = Cam(w, h);

	Shader shader = { 0 };
	if (type == 1) shader = LoadShader(0, "mandelbrot.fs");
	else if (type == 3) shader = LoadShader(0, "julia.fs");
	else if (type == 4) shader = LoadShader(0, "julia_swirl.fs");
	else if (type == 5) shader = LoadShader(0, "julia_crystal.fs");
	else std::cerr << "no fs" << std::endl;
	int left_loc = GetShaderLocation(shader, "u_l");
	int top_loc = GetShaderLocation(shader, "u_t");
	int x_inc_loc = GetShaderLocation(shader, "u_i_x");
	int y_inc_loc = GetShaderLocation(shader, "u_i_y");
	int res_loc = GetShaderLocation(shader, "res");
	int max_i_loc = GetShaderLocation(shader, "u_max_i");
	int c_loc = GetShaderLocation(shader, "u_colour");
	int k_loc = GetShaderLocation(shader, "k");

	Vector2 res = {(float) w, (float) h};

	bool borderless = false;
	bool updateScreen = true;
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_F11)) {
			int monitor = GetCurrentMonitor();
			borderless = !borderless;
			if (borderless) {
				SetWindowState(FLAG_WINDOW_UNDECORATED);
				SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
				SetWindowPosition(0, 0);
			} else {
				ClearWindowState(FLAG_WINDOW_UNDECORATED);
				SetWindowSize(cw, ch);
				SetWindowPosition(GetMonitorWidth(monitor)/2-cw/2, GetMonitorHeight(monitor)/2-ch/2);
			}
			/*if (IsWindowFullscreen()) {
				ToggleFullscreen();
				SetWindowSize(cw, ch);
			} else {
				SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
				ToggleFullscreen();
			}*/
		}
		if (IsWindowResized()) {
			w = GetScreenWidth();
			h = GetScreenHeight();
			cam.RecalculateSize(w, h);
			res = {(float) w, (float) h};
			updateScreen = true;
		}
		if (IsKeyDown(KEY_W)) {
			cam.move(0, 1);
			updateScreen = true;
		}  if (IsKeyDown(KEY_A)) {
			cam.move(-1, 0);
			updateScreen = true;
		}  if (IsKeyDown(KEY_S)) {
			cam.move(0, -1);
			updateScreen = true;
		}  if (IsKeyDown(KEY_D)) {
			cam.move(1, 0);
			updateScreen = true;
		}  if (IsKeyDown(KEY_MINUS)) {
			cam.zoom(false);
			updateScreen = true;
		}  if (IsKeyDown(KEY_EQUAL)) {
			cam.zoom(true);
			updateScreen = true;
		}

		if (updateScreen) {
			std::vector<float> CI = cam.get_info(w, h);
			SetShaderValue(shader, left_loc, &CI[0], SHADER_UNIFORM_FLOAT);
			SetShaderValue(shader, top_loc, &CI[3], SHADER_UNIFORM_FLOAT);
			SetShaderValue(shader, x_inc_loc, &CI[4], SHADER_UNIFORM_FLOAT);
			SetShaderValue(shader, y_inc_loc, &CI[5], SHADER_UNIFORM_FLOAT);
			SetShaderValue(shader, res_loc, &res, SHADER_UNIFORM_VEC2);
			SetShaderValue(shader, max_i_loc, &max_i, SHADER_UNIFORM_INT);
			SetShaderValue(shader, c_loc, &C, SHADER_UNIFORM_VEC3);
			SetShaderValue(shader, k_loc, &k, SHADER_UNIFORM_INT);

			updateScreen = false;
		}

		BeginDrawing();
			ClearBackground(RAYWHITE);

			BeginShaderMode(shader);
				DrawRectangle(0, 0, w, h, WHITE);
			EndShaderMode();
		EndDrawing();
	}

	UnloadShader(shader);
	CloseWindow();
	return 0;
}