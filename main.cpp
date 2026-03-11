#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <format>
#include <vector>
#include <chrono>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <filesystem>

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

bool within(std::vector<ComplexNumber> vect, ComplexNumber val) { for (ComplexNumber c : vect) { if (c == val) { return true; } } return false; }

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
/*u_int8_t mod(int n, int k) {
	return (u_int8_t) round(255*(((float) n)/((float) k)));
}*/

uint8_t iterate_mandelbrot(ComplexNumber c, int max_i) {
	ComplexNumber oz = ComplexNumber(0, 0);
	int ot = 0;
	int nt = 1;
	int iter = 0;
	ComplexNumber z = ComplexNumber(0, 0);
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
	return mod(iter, 50);
}

uint8_t iterate_julia(ComplexNumber z, int max_i) {
	ComplexNumber oz = ComplexNumber(0, 0);
	int ot = 0;
	int nt = 1;
	int iter = 0;
	ComplexNumber c = ComplexNumber(-0.8, 0.156);
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
	return mod(iter, 50);
}


std::string colour(std::string_view text, int r, int g, int b) { return std::format("\x1b[38;2;{};{};{}m{}\x1b[0m", r, g, b, text); }

void run(float lim_l, float lim_h, float lim_i, int max_i, std::string which, std::string f_name) {
	uint8_t w = 0;
	if (which == "mandelbrot") {
		w = 1;
	} else if (which == "julia") {
		w = 2;
	}
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<uint8_t> image;
	int s = std::round((lim_h - lim_l) / lim_i) + 1;
	image.assign(s * s * 3, 0);
	#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < s; ++y) {
		float imag = lim_l + y * lim_i;
		for (int x = 0; x < s; ++x) {
			float real = lim_l + x * lim_i;
			uint8_t val;
			if (w == 1) {
				val = iterate_mandelbrot(ComplexNumber(real, imag), max_i);
			} else if (w == 2) {
				val = iterate_julia(ComplexNumber(real, imag), max_i);
			}
			image[(y * s + x) * 3] = val;
		}
	}
	std::filesystem::create_directory("Fractals");
	stbi_write_png(("Fractals/" + f_name + ".png").c_str(), s, s, 3, image.data(), 0);
	auto end = std::chrono::high_resolution_clock::now();
	auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	long long seconds = total_ms / 1000;
	long long remainder_ms = total_ms % 1000;
	std::cout << "\rTime: " << seconds << "s " << remainder_ms << "ms" << std::endl;
}

int main() {
	//run(-2.5, 2.5, 0.00025, 1000000, "mandelbrot", "mandelbrot_set");
	run(-2.5, 2.5, 0.00025, 1000000, "julia", "julia_set");
}