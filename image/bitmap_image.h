#pragma once
#include <vector>
#include <exception>
#include "image/pixel.h"

class BitmapImage {

public:

    using BitmapPixel = Pixel<std::uint8_t>;
    using index_type = uint16_t;

    BitmapImage(int w, int h) {
        if (h > 0 && h <= 8192 && w > 0 && w <= 8192) {
            height = h;
            width = w;
            // Initialisiere das Vektor-Array mit der angegebenen Größe
            imageArray.resize(height, std::vector<BitmapPixel>(width));
            BitmapPixel default_value(0, 0, 0);
            for (int i = 0; i < imageArray.size(); i++) {
                for (int j = 0; j < imageArray[i].size(); j++) {
                    imageArray[i][j] = default_value;
                }
            }
        }
        else {
            throw std::exception("invalid height or width");
        }
    }

    index_type get_height() const {
        return height;
    }

    index_type get_width() const {
        return width;
    }

    void set_pixel(index_type x, index_type y, BitmapPixel p) {
        if (x < 0 || x > 8192 || x >= imageArray[0].size() || y < 0 || y >= imageArray.size() || y > 8192) { //added indexOutOfBounds check //recheck condition!!
            throw std::exception("invalid x or y coordinate");
        }
        imageArray[y][x] = p;
    }

    BitmapPixel get_pixel(index_type x, index_type y) const {
        if (x < 0 || x > 8192 || x >= imageArray[0].size() || y < 0 || y >= imageArray.size() || y > 8192) { //added indexOutOfBounds check //recheck condition!!
            throw std::exception("invalid x or y coordinate"); ////
        }
        return imageArray[y][x];
    }

    BitmapImage transpose() const {
        BitmapImage transposedImage(height, width);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                transposedImage.set_pixel(j, i, get_pixel(i, j));
            }
        }
        return transposedImage;
    }



private:
    // Vektor-Array, um die Pixel zu speichern
    std::vector<std::vector<BitmapPixel>> imageArray;
    index_type height = 0;
    index_type width = 0;
};
