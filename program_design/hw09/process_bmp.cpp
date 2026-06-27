#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

#pragma pack(push, 1)
typedef struct {
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t data_offset;
} BMPFileHeader;

typedef struct {
    uint32_t biSize; 
    int32_t biWidth; 
    int32_t biHeight; 
    uint16_t biPlanes; 
    uint16_t biBitcount;
    uint32_t biCompression; 
    uint32_t biSizeImage; 
    int32_t biXPelsPerMeter; 
    int32_t biYPelsPerMeter; 
    uint32_t biClrUsed; 
    uint32_t biClrImportant; 
} BMPInfoHeader;
#pragma pack(pop)

int main() {
    ifstream fin("image.bmp", ios::binary);
    ofstream fout("output.bmp", ios::binary);
    if (!fin.is_open()) {
        cerr << "Error" << endl;
        return 1;
    }
    BMPFileHeader fileheader;
    BMPInfoHeader infoheader;
    fin.read(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));
    fin.read(reinterpret_cast<char*>(&infoheader), sizeof(infoheader));
    int w = infoheader.biWidth;
    int h = infoheader.biHeight;
    int row_size = w * 3;
    vector<vector<unsigned char>> image(h, vector<unsigned char>(row_size));
    for (int i = 0; i < h; i++) {
        fin.read(reinterpret_cast<char*>(image[i].data()), row_size);
    }
    double center_x = w / 2.0;
    double center_y = h / 2.0;
    double r = min(w, h) / 2.0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            bool keep = 0;
            if (h == 100) {
                double dist_sq = pow(x - center_x, 2) + pow(y - center_y, 2);
                keep = (dist_sq <= r * r);
            } else if (h == 200) {
                double dist_sq = pow(x - center_x, 2) + pow(y - center_y, 2);
                keep = (dist_sq > r * r);
            } else if (h == 300) {
                keep = (x >= center_x && y >= center_y);
            } else if (h == 400) {
                keep = (x >= w / 4.0 && x <= 3.0 * w / 4.0 &&
                        y >= h / 4.0 && y <= 3.0 * h / 4.0);
            } else if (h == 500) {
                double dist = abs(x - y) / sqrt(2.0);
                keep = (dist <= 25.0);
            }
            if (!keep) {
                int base_idx = x * 3;
                image[y][base_idx] = 0;
                image[y][base_idx + 1] = 0;
                image[y][base_idx + 2] = 0;
            }
        }
    }
    fout.write(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));
    fout.write(reinterpret_cast<char*>(&infoheader), sizeof(infoheader));
    for (int i = 0; i < h; i++) {
        fout.write(reinterpret_cast<char*>(image[i].data()), row_size);
    }
    fin.close();
    fout.close();
    return 0;
}