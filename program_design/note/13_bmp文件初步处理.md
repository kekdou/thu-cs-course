## BMP 文件处理初步

### BMP 基本信息

```C++
#pragma pack(push, 1)     // 确保结构体按字节数对齐
// 文件头
// 标记文件类型、大小和数据偏移量等
struct BMPFileHeader {    
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t data_offset;
};
// 信息头
// 定义文件的维度、深度等
struct BMPInfoHeader {
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
};
#pragma pack(pop)
```

此外，bmp处理还需要注意 4字节对齐(padding) 问题

这是历史遗留的硬件性能优化问题   
> BMP 格式由微软在早期 Windows 时代定义，当时 CPU 架构在处理内存时，如果数据的起始地址是 4 的倍数（即 32 位），处理速度是最快的  
> 因为 cpu 并不是一个字节一个字节读取内存，而是按块（通常是 4 字节或 8 字节）读取  
> 如果一行的开头没有对齐到 4 字节边界，CPU 可能需要跨越两个指令周期来读取同一个像素的数据，这在早期的低性能计算机上会导致明显的显示延迟
> 因此，BMP 标准规定：图像的每一行像素数据所占用的字节数，必须能够被 4 整除

即假如一张图片的宽度是4像素，每个像素的占3字节(R, G, B 各一字节) 则每行数据为 12 字节，是 4 的倍数，无需 padding  
而如果不是 4 的倍数，系统就会在行末补上缺少的字节，padding 的部分不会包含任何颜色信息，仅是为了占位  

所以在读取写入时注意，否则容易导致图片倾斜  

```C++
// 读取
int padding = (4 - (width * 3) % 4) % 4;
for (int i = 0; i < height; i++) {
    file.read(&pixels[i * width * 3], width * 3);
     // 跳过无用字节
    file.seekg(padding, std::ios::cur); 
}
// 写入
uint8_t zero = 0;
for (int i = 0; i < height; i++) {
    file.write(&pixels[i * width * 3], width * 3); 
    for (int p = 0; p < padding; p++) {
        file.write((char*)&zero, 1);
    }
}
```

以上是针对 24位(RGB)图片，对于 32位(RGBA)图片则不需要考虑，因为一个像素占用 4 字节，永远是 4 的倍数  

---

总结就是，bmp 文件开头是 文件头和信息头，接下来是图像每一行的像素信息，储存 R, G, B色彩  

### 读取和处理

```C++
ifstream fin("image.bmp", ios::binary);
// 记录文件头和信息头
BMPFileHeader fileheader;
BMPInfoHeader infoheader;
fin.read(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));
fin.read(reinterpret_cast<char*>(&infoheader), sizeof(infoheader));
// 读出宽和高，以及每一行的大小
int w = infoheader.biWidth;
int h = infoheader.biHeight;
int row_size = w * 3;
// 读取图片信息
vector<vector<unsigned char>> image(h, vector<unsigned char>(row_size));
for (int i = 0; i < h; i++) {
    fin.read(reinterpret_cast<char*>(image[i].data()), row_size);
}
```

写入基本相同

```C++
ofstream fout("output.bmp", ios::binary);
fout.write(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));
fout.write(reinterpret_cast<char*>(&infoheader), sizeof(infoheader));
for (int i = 0; i < h; i++) {
    fout.write(reinterpret_cast<char*>(image[i].data()), row_size);
}
```