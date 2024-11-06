#include "capture-screen.h"

std::string CaptureScreen(int x, int y, int w, int h) {
    // 获取屏幕的设备上下文
    HDC hdc = GetDC(NULL);

    // 创建一个与屏幕相同大小的位图
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(hdcMem, bitmap);

    // 从屏幕复制图像到位图
    BitBlt(hdcMem, 0, 0, w, h, hdc, x, y, SRCCOPY);

    // 获取位图数据
    BITMAP bmp;
    GetObject(bitmap, sizeof(BITMAP), &bmp);
    char *data = (char *)malloc((u64)bmp.bmWidth * (u64)bmp.bmHeight * 4);  // 32位位图数据
    NOT_NULL(data);
    GetBitmapBits(bitmap, bmp.bmWidthBytes * bmp.bmHeight, data);
    std::vector<unsigned char> image((u64)w * (u64)h * 4);

    for (size_t row = 0; row < h; row++) {
        for (size_t col = 0; col < w; col++) {

            size_t pixel = row * bmp.bmWidth * 4 + col * 4;  // 32位位图偏移量
            size_t index = row * w * 4 + col * 4;

            image[index + 0] = data[pixel + 2];  // R
            image[index + 1] = data[pixel + 1];  // G
            image[index + 2] = data[pixel + 0];  // B
            image[index + 3] = 255;              // A
        }
    }

    std::string filename = "screenshot.png";
    std::vector<unsigned char> png;
    unsigned error = lodepng::encode(png, image, w, h);

    if (error) {
        std::cout << "PNG encoding error " << error << ": " << lodepng_error_text(error) << std::endl;
    }

    lodepng::save_file(png, filename);

    free(data);
    DeleteObject(bitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);

    // 返回文件路径
    std::filesystem::path path = std::filesystem::current_path();
    return path.string() + "\\" + filename;
}
