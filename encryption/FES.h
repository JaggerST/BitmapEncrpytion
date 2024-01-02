#pragma once
#include "encryption/Key.h"
#include "image/bitmap_image.h"
#include <iostream>

class FES {

public:

	using key_type = Key::key_type;
	using row_type = std::array<BitmapImage::BitmapPixel, 16>;
	using block_type = std::array<std::array<row_type, 3>, 3>;

	FES() {}

	static block_type convert_key_to_block(key_type key) { // should be correct
		block_type result;
		row_type row = convert_key_to_row(key);
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				result[i][j] = row;
			}
		}
		return result;
	}

	static row_type convert_key_to_row(key_type key) { // we can assume that rows are always complete
		row_type result;
		int blue_counter = 2;
		int green_counter = 1;
		int red_counter = 0;
		for (int i = 0; i < 16; i++) {
			BitmapImage::BitmapPixel pixel(key[red_counter], key[green_counter], key[blue_counter]);  
			result[i] = pixel;
			blue_counter += 3;
			green_counter += 3;
			red_counter += 3;
		}
		return result;
	}

	static row_type combine_rows(row_type row1, row_type row2) {
		row_type result;
		for (int i = 0; i < 16; i++) {
			BitmapImage::BitmapPixel pixel1 = row1[i];
			BitmapImage::BitmapPixel pixel2 = row2[i];
			BitmapImage::BitmapPixel new_pixel(pixel1.get_red_channel() ^ pixel2.get_red_channel(),
				pixel1.get_green_channel() ^ pixel2.get_green_channel(), pixel1.get_blue_channel() ^ pixel2.get_blue_channel());
			result[i] = new_pixel;
		}
		return result;
	}

	static block_type encrypt_block(block_type current_block, block_type prev_block) {  // not finsihed (especially edge cases )
		block_type result;
		int encrypted_rows_counter = 0;
		for (int i = 0; i < current_block.size(); i++) { // Iteration durch die Reihen		   // (jeder Block besteht im Nomralfall aus 3x3 Reihen)
			for (int j = 0; j < current_block[i].size(); j++) { // Iteration durch die Reihen 
				row_type combined_row = combine_rows(current_block[i][j], prev_block[i][j]);
				// combine combined_row with every other already encrypted row
				int x = 0;
				int y = 0;
				for (int z = 0; z < encrypted_rows_counter; z++) {
					combined_row = combine_rows(combined_row, result[y][x]);
					if ((x + 1) % 3 == 0) {
						x = 0;
						y++;
					}
					else {
						x++;
					}
				}
				result[i][j] = combined_row;
				encrypted_rows_counter++;
			}
		}
		return result;
	}

	static BitmapImage encrypt(BitmapImage image, key_type key) {
		BitmapImage encrypted_image(image.get_width(), image.get_height());
		block_type key_block = convert_key_to_block(key);
		block_type prev_block = key_block;

		// iterate through the image block-wise
		for (int i = 0; i < image.get_height(); i += 3) {
			for (int j = 0; j < image.get_width(); j += 48) {
				// create current block and fill it with values
				block_type current_block;
				if (j <= image.get_width() - 48 && i <= image.get_height() - 3) {  // current_block is neither rightmost nor leftmost 
					for (int y = 0; y < 3; y++) {
						for (int x = 0; x < 3; x++) {
							for (int z = 0; z < 16; z++) {
								current_block[y][x][z] = image.get_pixel(j + z + 16 * x, i + y);
							}
						}

					}
					// now our current_block has the correct values
					// now we want to encrypt the block 
					block_type encrypted_block = encrypt_block(current_block, prev_block);

					// now fill the image with the encrypted pixels 
					for (int y = 0; y < 3; y++) {
						for (int x = 0; x < 3; x++) {
							for (int z = 0; z < 16; z++) {
								encrypted_image.set_pixel(j + z + 16 * x, i + y, encrypted_block[y][x][z]);
							}
						}
					}

				}
				else if (i <= image.get_height() - 3) {  // current block is rightmost  // FALL 3
					// determine area of the block that is needed for the image 
					int actual_block_width = (image.get_width() % 48) / 16;  // represents the number of rows we need for the width (either 1, 2 or 3)   // !!!! question: is it posible that the width of the image is not divisible by 16? !!!!
					for (int y = 0; y < 3; y++) {
						for (int x = 0; x < actual_block_width; x++) {
							for (int z = 0; z < 16; z++) {
								current_block[x][y][z] = image.get_pixel(j + z + 16 * x, i + y);  // canged x and y -> transposed (siehe Fall 3 der edge cases) 
							}
						}
					}
					// now our current_block has the correct values
					// now we want to encrypt the block 
					block_type encrypted_block = encrypt_block(current_block, prev_block);

					// now fill the image with the encrypted pixels 
					for (int y = 0; y < 3; y++) {
						for (int x = 0; x < actual_block_width; x++) {
							for (int z = 0; z < 16; z++) {
								encrypted_image.set_pixel(j + z + 16 * x, i + y, encrypted_block[x][y][z]);  // canged x and y -> transposed (siehe Fall 3 der edge cases) 
							}
						}
					}
				}
				else if (j <= image.get_width() - 48) { // FALL 2       // !!! Problem beim testen: Wir haben kein Bild, dessen H�he nicht durch 3 teilbar ist !!!
					int actual_block_height = image.get_height() % 3;
					for (int y = 0; y < actual_block_height; y++) {  // !!!! nehmen wir so die fehlenden Reihen als 0 an? !!!!
						for (int x = 0; x < 3; x++) {
							for (int z = 0; z < 16; z++) {
								current_block[y][x][z] = image.get_pixel(j + z + 16 * x, i + y);
							}
						}
					}
					// now our current_block has the correct values
					// now we want to encrypt the block 
					block_type encrypted_block = encrypt_block(current_block, prev_block);

					// now fill the image with the encrypted pixels 
					for (int y = 0; y < actual_block_height; y++) {  // !!!! nehmen wir so die fehlenden Reihen als 0 an? !!!!
						for (int x = 0; x < 3; x++) {
							for (int z = 0; z < 16; z++) {
								encrypted_image.set_pixel(j + z + 16 * x, i + y, encrypted_block[y][x][z]);
							}
						}
					}

				}
				else { // Fall 4 und 5  // can only happen with the top right block
					int actual_block_width = (image.get_width() % 48) / 16;
					int actual_block_height = image.get_height() % 3;

					if (actual_block_width == 1) { // FALL 5
						for (int y = 0; y < actual_block_height; y++) {
							for (int x = 0; x < actual_block_width; x++) {
								for (int z = 0; z < 16; z++) {
									current_block[x][y][z] = image.get_pixel(j + z + 16 * x, i + y);  // canged x and y -> transposed (siehe Fall 3 der edge cases) 
								}
							}
						}
						// now our current_block has the correct values
						// now we want to encrypt the block 
						block_type encrypted_block = encrypt_block(current_block, prev_block);

						// now fill the image with the encrypted pixels 
						for (int y = 0; y < actual_block_height; y++) {
							for (int x = 0; x < actual_block_width; x++) {
								for (int z = 0; z < 16; z++) {
									encrypted_image.set_pixel(j + z + 16 * x, i + y, encrypted_block[x][y][z]);  // canged x and y -> transposed (siehe Fall 3 der edge cases) 
								}
							}
						}
					}
					else { // FALL 4
						for (int y = 0; y < actual_block_height; y++) {  // !!!! nehmen wir so die fehlenden Reihen als 0 an? !!!!
							for (int x = 0; x < actual_block_width; x++) {
								for (int z = 0; z < 16; z++) {
									current_block[y][x][z] = image.get_pixel(j + z + 16 * x, i + y);
								}
							}
						}
						// now our current_block has the correct values
						// now we want to encrypt the block 
						block_type encrypted_block = encrypt_block(current_block, prev_block);

						// now fill the image with the encrypted pixels 
						for (int y = 0; y < actual_block_height; y++) {  // !!!! nehmen wir so die fehlenden Reihen als 0 an? !!!!
							for (int x = 0; x < actual_block_width; x++) {
								for (int z = 0; z < 16; z++) {
									encrypted_image.set_pixel(j + z + 16 * x, i + y, encrypted_block[y][x][z]);
								}
							}
						}
					}
				}

				// prepare next iteration 
				if (j + 48 >= image.get_width()) {
					prev_block = key_block;
				}
				else {
					prev_block = current_block;
				}
			}
		}
		return encrypted_image;
	}

};

