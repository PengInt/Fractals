#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <format>
#include <vector>
#include <chrono>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"
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

uint8_t iterate_julia(ComplexNumber z, int max_i, float cr, float ci, int k) {
	ComplexNumber oz = ComplexNumber(0, 0);
	int ot = 0;
	int nt = 1;
	int iter = 0;
	ComplexNumber c = ComplexNumber(cr, ci);
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

// FIX NOVA FRACTAL
uint8_t iterate_nova(ComplexNumber c, int max_i, float r0, float i0, int k) {
	//n=o-(o^p-1)/(po^(p-1))+c  {z0=1}
	uint8_t p = 4;
	ComplexNumber z = ComplexNumber(1+r0, 0+i0);
	ComplexNumber oz = z;
	int iter = 0;
	while (iter < max_i) {
		ComplexNumber zp_1 = z ^ (p - 1);
		ComplexNumber zp = zp_1 * z;
		z = z - (zp - 1)/(p * zp_1) + c;
		iter ++;
		ComplexNumber diff = oz - z;
		if (diff.magnitude2() < 0.0000001) { return mod(iter, k); }
		//if (z.magnitude2() > 1e10) { break; }
		// Reason - nova fractals don't care about escaping to infinity
		oz = z;
	}
	return 0;
}


std::string colour(std::string_view text, int r, int g, int b) { return std::format("\x1b[38;2;{};{};{}m{}\x1b[0m", r, g, b, text); }

void run(float lim_l, float lim_h, float lim_i, int max_i, std::string which, std::string colour, int k, std::string f_name, float r0, float i0) {
	uint8_t w = 0;
	if (which == "mandelbrot") {
		w = 1;
	} else if (which == "julia") {
		w = 2;
	} else if (which == "nova") {
		w = 3;
	}
	float c_r = 0;
	float c_g = 0;
	float c_b = 0;
	if (colour == "white") {
		c_r = 1;
		c_g = 1;
		c_b = 1;
	} else if (colour == "red") {
		c_r = 1;
	} else if (colour == "green") {
		c_g = 1;
	} else if (colour == "blue") {
		c_b = 1;
	} else if (colour == "yellow") {
		c_r = 1;
		c_g = 1;
	} else if (colour == "cyan") {
		c_g = 1;
		c_b = 1;
	} else if (colour == "magenta") {
		c_r = 1;
		c_b = 1;
	} else if (colour == "orange") {
		c_r = 1;
		c_g = 0.5;
	} else if (colour == "turquoise") {
		c_g = 1;
		c_b = 0.5;
	} else if (colour == "purple") {
		c_r = 0.5;
		c_b = 1;
	}
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<uint8_t> image;
	int s = std::round((lim_h - lim_l) / lim_i) + 1;
	image.assign(s * s * 3, 0);
	#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < s; ++y) {
		float imag = -(lim_l + y * lim_i);
		for (int x = 0; x < s; ++x) {
			float real = lim_l + x * lim_i;
			uint8_t val = 0;
			if (w == 1) {
				val = iterate_mandelbrot(ComplexNumber(real, imag), max_i, r0, i0, k);
			} else if (w == 2) {
				val = iterate_julia(ComplexNumber(real, imag), max_i, r0, i0, k);
			} else if (w == 3) {
				val = iterate_nova(ComplexNumber(real, imag), max_i, r0, i0, k);
			}
			image[(y * s + x) * 3] = val * c_r;
			image[(y * s + x) * 3 + 1] = val * c_g;
			image[(y * s + x) * 3 + 2] = val * c_b;
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
	std::ifstream cfg("config.cfg");
	if (!cfg.is_open()) {
		std::cerr << "Error opening file config.cfg" << std::endl;
		return 1;
	}

	std::string line;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	float min = std::stof(line);
	std::cout << "Minimum: " << min << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	float max = std::stof(line);
	std::cout << "Maximum: " << max << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	float inc = std::stof(line);
	std::cout << "Increment: " << inc << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	float max_i = std::stof(line);
	std::cout << "Maximum iterations: " << max_i << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	std::string type = line;
	std::cout << "Type: " << type << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	std::string c = line;
	std::cout << "Colour: " << c << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	int k = std::stoi(line);
	std::cout << "K: " << k << std::endl;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	std::string f_name = line;
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	float r0 = std::stof(line);
	if (!std::getline(cfg, line)) { std::cerr << "Error reading file config.cfg" << std::endl; }
	float i0 = std::stof(line);
	std::cout << "Number: " << r0 << " + " << i0 << "i" << std::endl;
	std::cout << "File name: (Fractals/) " << f_name << ".png" << std::endl;
	std::cout << "\nStarting...        " << std::flush;

	run(min, max, inc, max_i, type, c, k, f_name, r0, i0);
	return 0;
}