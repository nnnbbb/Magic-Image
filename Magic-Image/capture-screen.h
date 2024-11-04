#include <windows.h>
#include <png.h>
#include <iostream>
#include <fstream>
#include <filesystem>

std::string CaptureScreen(int x, int y, int w, int h);

void SavePng(const char *filename, int width, int height, unsigned char *data);
