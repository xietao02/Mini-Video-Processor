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

#include "header.h"

int main(int argc, char* argv[]){
    system("cls");
    bool RENAME_FILES_FLAG = false;
    string WORKING_DIR = "";
    int START_NUM = 1;
    int FPS = 30;
    int NEW_VIDEO_FRAMES = 70;
    int TOTAL_VIDEOS = 0;
	
    int op;
    bool op_flag = true;
    while ((op = xgetopt(argc, argv, "r")) != -1) {
        switch (op) {
        case 'r': {
            RENAME_FILES_FLAG = true;
            break;
        }
        default: {
            op_flag = false;
            break;
        }
        }
    }
    if (!op_flag) {
        cout << "Usage: " << endl;
        cout << "-r   rename video file" << endl;
        return -1;
    }
	
	vector<string> category;
    int status;
	status = init(category, RENAME_FILES_FLAG, &WORKING_DIR, &START_NUM, &FPS, &NEW_VIDEO_FRAMES);
    if (status == -1) return -1;
    hidecursor();
    reNameFiles(RENAME_FILES_FLAG, WORKING_DIR);
    vector <string> files;
    getFiles(WORKING_DIR, files);
    VideoProcessor(files.size(), FPS, START_NUM, WORKING_DIR, NEW_VIDEO_FRAMES, category);
    displaycursor();
    return 0;
}
