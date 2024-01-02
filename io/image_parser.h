#pragma once

#include "image/bitmap_image.h"
#include "image/pixel.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <Windows.h>


class ImageParser {

public:
	using BitmapPixel = Pixel<std::uint8_t>;

	ImageParser() {}

	static BitmapImage read_bitmap(std::filesystem::path image_path) {

		std::ifstream input(image_path, std::ios::binary);

		// Check if the file is open
		if (!input.is_open()) {
			throw std::invalid_argument("Unable to open file: " + image_path.string());
		}

		int width, height;
		input.seekg(18); // Skip to the width and height information
		input.read(reinterpret_cast<char*>(&width), sizeof(int));
		input.read(reinterpret_cast<char*>(&height), sizeof(int));

		// Create a BitmapImage with the read width and height
		BitmapImage image(width, height);

		input.seekg(10);
		std::uint32_t dynamic_offset;
		input.read(reinterpret_cast<char*>(&dynamic_offset), sizeof(std::uint32_t));
		// Skip to the pixel data
		input.seekg(dynamic_offset); // originally 54 

		//std::cout << "Offset: " << dynamic_offset << std::endl;

		// Read and set pixels in the BitmapImage
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				// Read individual color channels
				std::uint8_t blue, green, red;

				input.read(reinterpret_cast<char*>(&blue), sizeof(std::uint8_t));
				input.read(reinterpret_cast<char*>(&green), sizeof(std::uint8_t));
				input.read(reinterpret_cast<char*>(&red), sizeof(std::uint8_t));

				// Create a Pixel object and set it in the BitmapImage
				BitmapPixel pixel(red, green, blue);
				image.set_pixel(x, y, pixel);
			}
		}

		// Close the file stream
		input.close();

		return image;
	}

	static void write_bitmap(const std::filesystem::path& image_path, const BitmapImage& image) {
		std::ofstream output(image_path, std::ios::binary);

		// Check if the file is open
		if (!output.is_open()) {
			throw std::invalid_argument("Unable to open file for writing: " + image_path.string());
		}


		// Write BitmapFileHeader (necessary)
		BITMAPFILEHEADER bfh;
		bfh.bfType = 0x4D42; // 'BM' for Bitmap
		bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
			image.get_width() * image.get_height() * 3; // Assuming 24 bits per pixel
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		output.write(reinterpret_cast<const char*>(&bfh), sizeof(BITMAPFILEHEADER));

		// Write BitmapInfoHeader (necessary)
		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = image.get_width();
		bih.biHeight = image.get_height();
		bih.biPlanes = 1;
		bih.biBitCount = 24; // 24 bits per pixel
		bih.biCompression = 0; // No compression
		bih.biSizeImage = 0; // Can be set to 0 for uncompressed images
		bih.biXPelsPerMeter = 0; // Not important for BMP files
		bih.biYPelsPerMeter = 0; // Not important for BMP files
		bih.biClrUsed = 0; // Not important for BMP files
		bih.biClrImportant = 0; // Not important for BMP files

		output.write(reinterpret_cast<const char*>(&bih), sizeof(BITMAPINFOHEADER));

		// Write pixel data
		for (int y = 0; y < image.get_height(); y++) {
			for (int x = 0; x < image.get_width(); x++) {
				const BitmapPixel& pixel = image.get_pixel(x, y);
				std::uint8_t red_channel = pixel.get_red_channel();
				std::uint8_t green_channel = pixel.get_green_channel();
				std::uint8_t blue_channel = pixel.get_blue_channel();

				// Write individual color channels
				output.write(reinterpret_cast<const char*>(&blue_channel), sizeof(std::uint8_t));
				output.write(reinterpret_cast<const char*>(&green_channel), sizeof(std::uint8_t));
				output.write(reinterpret_cast<const char*>(&red_channel), sizeof(std::uint8_t));
				

			}
		}

		// Close the file stream
		output.close();
	}

};
