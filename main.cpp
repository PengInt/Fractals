#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <format>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
	else if (c.Real != 0 && c.Imaginary == 0) { os << c.Real; }
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

u_int8_t iterate(ComplexNumber c, int max_i) {
	ComplexNumber oz = ComplexNumber(0, 0);
	int ot = 0;
	int nt = 1;
	int i = 0;
	ComplexNumber z = ComplexNumber(0, 0);
	while (i < max_i) {
		z = z*z + c;
		i++;
		if (z.magnitude2() > 4) { break; }
		if (oz == z) { return 0; }
		ot++;
		if (ot == nt) {
			nt *= 2;
			ot = 0;
			oz = z;
		}
	}
	return mod(i, 50);
}


std::string colour(std::string_view text, int r, int g, int b) { return std::format("\x1b[38;2;{};{};{}m{}\x1b[0m", r, g, b, text); }
int index(float re, float im, float l_l, float l_h, float inc) {
	int s = (l_h-l_l)/inc;
	int real = (re-l_l)/inc;
	int imag = (im-l_l)/inc; 
	return 3*(real+imag*s);
}

void run(float lim_l, float lim_h, float lim_i, int max_i, std::string f_name) {
	std::vector<u_int8_t> image;
	int s = std::round((lim_h - lim_l) / lim_i) + 1;
	image.assign(s * s * 3, 0);
	for (int y = 0; y < s; ++y) {
	  float imag = lim_l + y * lim_i;
	  for (int x = 0; x < s; ++x) {
		  float real = lim_l + x * lim_i;
		  image[(y * s + x) * 3 + 1] = iterate(ComplexNumber(real, imag), max_i);
	  }
	}
	stbi_write_png((f_name + ".png").c_str(), s, s, 3, image.data(), s*3);
}

int main() {
	run(-2.5, 2.5, 0.00025, 10000, "mandelbrot_set");
}