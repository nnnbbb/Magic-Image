#include "capture-screen.h"

void SavePng(const char *filename, int width, int height, unsigned char *data) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);
    png_set_IHDR(
      png,
      info,
      width,
      height,
      8,
      PNG_COLOR_TYPE_RGB,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // 上下翻转图像数据并写入 PNG
    for (int y = height - 1; y >= 0; y--) {
        png_write_row(png, data + (y * width * 3));
    }

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

std::string CaptureScreen(int x, int y, int w, int h) {
    // 获取屏幕的设备上下文
    HDC hdcScreen = GetDC(NULL);
    int width = GetDeviceCaps(hdcScreen, HORZRES);
    int height = GetDeviceCaps(hdcScreen, VERTRES);

    // 创建一个与屏幕相同大小的位图
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
    SelectObject(hdcMem, hBitmap);

    // 从屏幕复制图像到位图
    BitBlt(hdcMem, 0, 0, w, h, hdcScreen, x, y, SRCCOPY);

    // 获取位图数据
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    char *data = (char *)malloc(bmp.bmWidthBytes * bmp.bmHeight);
    GetBitmapBits(hBitmap, bmp.bmWidthBytes * bmp.bmHeight, data);

    // 将数据转换为 RGB 格式
    unsigned char *rgb = (unsigned char *)malloc(w * h * 3);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int pixel = y * bmp.bmWidthBytes + x * 4;  // 32位位图
            int index = (h - 1 - y) * w * 3 + x * 3;   // PNG 需要倒转行
            rgb[index + 0] = data[pixel + 2];          // R
            rgb[index + 1] = data[pixel + 1];          // G
            rgb[index + 2] = data[pixel + 0];          // B
        }
    }

    // 保存为 PNG 文件
    std::string filename = "screenshot.png";
    SavePng(filename.c_str(), w, h, rgb);

    // 清理资源
    free(data);
    free(rgb);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    std::filesystem::path path = std::filesystem::current_path();
    return path.string() + "\\" + filename;
}