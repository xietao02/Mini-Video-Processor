/*******************************************************************************
 * Copyright (c) 2022 xietao02@outlook.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 ********************************************************************************/

#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <iostream>
#include <io.h>
#include <fstream>
#include <vector>
#include <Windows.h>
#include <unistd.h>
#include "xgetopt.h"
#include <direct.h>
#include <stdio.h>

using namespace cv;
using namespace std;

int init(vector <string>& category, bool RENAME_FILES_FLAG, string* WORKING_DIR, int* START_NUM, int* FPS, int* NEW_VIDEO_FRAMES);
int loadConfig(vector <string>& category, int* FPS, int* NEW_VIDEO_FRAMES);

void getFiles(string path, vector <string>& files);
void reNameFiles(bool flag, string WORKING_DIR);

int VideoProcessor(int filesNum, int FPS, int START_NUM, string WORKING_DIR, int NEW_VIDEO_FRAMES, vector <string>& category);
void setTrack(VideoCapture& capture, Mat& frame, int keyValue, int skip);
bool processVideo(VideoCapture& capture, Mat& frame, int delay, string WORKING_DIR, int NEW_VIDEO_FRAMES, vector <string>& category);
void onMouse(int event, int x, int y, int flags, void* ustc);

void setcursorpos(unsigned x, unsigned y);
void hidecursor();
void displaycursor();
