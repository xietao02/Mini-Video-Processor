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

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <vector>
#include <unistd.h>
#include "xgetopt.h"
#include <json/json.h>
#include <fstream>
#include <direct.h>

using namespace std;
using namespace cv;

struct VideoInfo {
    string name;
    bool valid;
};

class miniVideoProcessor {
    bool RENAME_FILES_FLAG = false;
    bool RENAME_CUSTOMIZED = false;
    string RENAME;
    string WORKING_DIR_R;
    string WORKING_DIR;
    string CONFIG_PATH_R;
    string VIDEO_LIST_DIR_R;
    int START_NUM = 1;
    int FPS;
    int DELAY;
    int SKIP_FRAMES;
    int NEW_VIDEO_FRAMES;
    int TOTAL_VIDEOS;
    int LAST_NUM = 0;
    vector<string> CATEGORY;
    vector<VideoInfo> VIDEO_LIST;


    void readCMDOption(int argc, char* argv[]);
    bool loadConfig();
    void generateConfig(string);
    bool readConfig();
    bool init();
    void reNameFiles();
    void getFiles(string, vector <string>&);
    bool generateVideoList(vector<string>);
    bool generateVideoList();
    bool checkVideoList(vector<string>);
    void printConfig();
    bool MVP();
    void setTrack(VideoCapture&, Mat&, int);
    bool processVideo(VideoCapture&, Mat&);
    Rect StandardizeBOX(VideoCapture&, Point, Point);
    bool saveLastNum();

public:
    miniVideoProcessor(int argc, char* argv[]);
    ~miniVideoProcessor();
};

void onMouse(int, int, int, int, void*);
