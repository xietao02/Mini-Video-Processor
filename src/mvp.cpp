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

#include "mvp.h"
#include "util.h"

bool leftButtonDownFlag;
Point originalPoint;
Point processPoint;

miniVideoProcessor::miniVideoProcessor(int argc, char* argv[]) {
    readCMDOption(argc, argv);
    if (init()) {
        hidecursor();
        if (static_cast<int>(VIDEO_LIST.size()) != 0)MVP();
    }
}

miniVideoProcessor::~miniVideoProcessor() {
    displaycursor();
}

void miniVideoProcessor::readCMDOption(int argc, char* argv[]) {
    int op;
    bool op_flag = true;
    while ((op = xgetopt(argc, argv, "r::")) != -1) {
        switch (op) {
        case 'r': {
            RENAME_FILES_FLAG = true;
            if (xoptarg != NULL) {
                RENAME_CUSTOMIZED = true;
                RENAME = xoptarg;
            }
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
        cout << "-r   rename video files" << endl;
    }
}

bool miniVideoProcessor::loadConfig() {
    CONFIG_PATH_R = WORKING_DIR_R + R"(mvp_config.json)";
    string CONFIG_PATH = WORKING_DIR + "mvp_config.json";
    if (_access(CONFIG_PATH_R.c_str(), 0) == -1) {
        // generate config file
        generateConfig(CONFIG_PATH);
        return false;
    }
    else if (readConfig())return true;// read config file
    else return false;
}

void miniVideoProcessor::generateConfig(string CONFIG_PATH) {
    cout << "> info: The config file does not exist! " << endl;
    ofstream fout;
    fout.open(CONFIG_PATH_R, ios::out);
    if (!fout.is_open()) {
        cout << "> error: Failed to create config file! " << endl;
    }
    else {
        fout << "{\n";
        fout << "  \"FPS\": 30, // Video player refresh rate\n";
        fout << "  \"Frames skipped per fast-forward\": 30,\n";
        fout << "  \"new video frames\": 70,\n\n";
        fout << "  // Up to nine category options\n";
        fout << "  \"Categories\": [\n";
        fout << "    \"cat_running\",\n";
        fout << "    \"cat_walking\",\n";
        fout << "    \"cat_eating\",\n";
        fout << "    \"cat_jumping\"\n";
        fout << "  ]\n}\n";
        fout.close();
        cout << "> info: The regenerated config file can be found in the directory:" << endl;
        cout << "> " << CONFIG_PATH << endl;
        cout << "> Please fill in the configuration file and re-run." << endl;
    }
}

bool miniVideoProcessor::readConfig() {
    Json::Value config;
    Json::Reader reader(Json::Features::strictMode());
    ifstream ifs;
    ifs.open(CONFIG_PATH_R, ios::binary);
    bool keyExist = true;
    if (!reader.parse(ifs, config)) {
        cout << "> error: Failed to parse config file!" << endl;
        return false;
    }
    else {
        if (!config.isMember("FPS")) {
            cout << "> error: The \"FPS\" key does not exist in the config file!" << endl;
            keyExist = false;
        }
        if (!config.isMember("Frames skipped per fast-forward")) {
            keyExist = false;
            cout << "> error: The \"Frames skipped per fast-forward\" key does not exist in the config file!" << endl;
        }
        if (!config.isMember("new video frames")) {
            keyExist = false;
            cout << "> error: The \"new video frames\" key does not exist in the config file!" << endl;
        }
        if (!config.isMember("Categories")) {
            keyExist = false;
            cout << "> error: The \"Categories\" key does not exist in the config file!" << endl;
        }
        else if (config["Categories"].empty()) {
            cout << "> error: The \"Categories\" key is empty in the config file!" << endl;
            keyExist = false;
        }
        if (!keyExist) return false;
        else {
            FPS = config["FPS"].asInt();
            SKIP_FRAMES = config["Frames skipped per fast-forward"].asInt();
            NEW_VIDEO_FRAMES = config["new video frames"].asInt();
            Json::Value arrayObj = config["Categories"];
            for (unsigned int i = 0; i < arrayObj.size() && i < 9; i++) {
                CATEGORY.push_back(arrayObj[i].asString());
            }
            return true;
        }
    }
}

bool miniVideoProcessor::init() {
    char* buffer;
    if ((buffer = _getcwd(NULL, 0)) == NULL) {
        perror("> errer: Failed to get working directory!");
        return false;
    }
    else {
        string tempDir = buffer, temp;
        istringstream itemp(tempDir);
        WORKING_DIR_R = "";
        while (getline(itemp, temp, '\\')) {
            WORKING_DIR_R += (temp + R"(\\)");
            WORKING_DIR += (temp + "\\");
        }
        free(buffer);
    }

    // load config file
    if (!loadConfig())return false;
    reNameFiles();

    VIDEO_LIST_DIR_R = WORKING_DIR_R + "video_list.json";
    vector<string> vlist_disk;
    getFiles(WORKING_DIR_R, vlist_disk);
    if (_access(VIDEO_LIST_DIR_R.c_str(), 0) == -1) {
        // generate video_list.json
        if (!generateVideoList(vlist_disk))return false;
    }
    else {
        // check video_list.json
        if (!checkVideoList(vlist_disk))return false;
    }
    printConfig();
    return true;
}

void miniVideoProcessor::reNameFiles() {
    if (RENAME_FILES_FLAG) {
        vector<string> files;
        getFiles(WORKING_DIR, files);
        for (int i = 0; i < files.size(); i++) {
            cout << "> info: " << files[i] << " - ";
            string number = to_string(i + 1);
            string oldName = WORKING_DIR + files[i], newName;
            if (RENAME_CUSTOMIZED) {
                newName = WORKING_DIR + RENAME + "_" + number + ".mp4";
            }
            else newName = WORKING_DIR + number + ".mp4";
            if (-1 == rename(oldName.c_str(), newName.c_str())) {
                cout << "Rename failed" << endl;
            }
            else
                cout << "Rename completed" << endl;
        }
        cout << "> info: All files have been renamed..\n\n";
        system("pause");
    }
}

void miniVideoProcessor::getFiles(string path, vector <string>& files) {
    long long hFile = 0;
    struct _finddata_t fileinfo;
    string pathp;
    int i = 0;
    if ((hFile = _findfirst(pathp.assign(path).append("*.mp4").c_str(), &fileinfo)) != -1) {
        do {
            if (fileinfo.attrib & _A_SUBDIR) continue;
            else {
                string filestr = fileinfo.name;
                files.push_back(filestr);
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

bool miniVideoProcessor::generateVideoList(vector<string> vlist_disk) {
    // This function is used to generate and initialize the video_list.json file
    ofstream fout;
    fout.open(VIDEO_LIST_DIR_R, ios::out);
    if (!fout.is_open()) {
        cout << "> error: Failed to create video list file! " << endl;
        return false;
    }
    else {
        fout << "{\n";
        fout << "  // This file is automatically generated by MVP,\n";
        fout << "  // which can prevent duplicate processing of a video\n";
        fout << "  // due to changes in the working directory.\n";
        fout << "  // DO NOT MODIFY THIS FILE MANUALLY!\n";
        fout << "  \"Last processed video serial number\": 0,\n";
        fout << "  \"VideoList\": [\n";
        for (auto iter = vlist_disk.begin(); iter != vlist_disk.end(); iter++) {
            VideoInfo temp;
            temp.name = *iter;
            temp.valid = true;
            VIDEO_LIST.push_back(temp);
            fout << "    \"" + *iter + "\"";
            if (iter + 1 != vlist_disk.end()) {
                fout << ",";
            }
            fout << "\n";
        }
        fout << "  ]\n}\n";
        fout.close();
        return true;
    }
}

bool miniVideoProcessor::generateVideoList() {
    // This function is used to truncate the video_list.json file
    // when it exists and generate a new file
    ofstream fout;
    fout.open(VIDEO_LIST_DIR_R, ios::out | ios::trunc);
    if (!fout.is_open()) {
        cout << "> error: Failed to open video list file! " << endl;
        return false;
    }
    else {
        fout << "{\n";
        fout << "  // This file is automatically generated by MVP,\n";
        fout << "  // which can prevent duplicate processing of a video\n";
        fout << "  // due to changes in the working directory.\n";
        fout << "  // PLEASE DO NOT MODIFY THIS FILE MANUALLY!\n";
        fout << "  \"Last processed video serial number\": " << LAST_NUM << ", \n";
        fout << "  \"VideoList\": [\n";
        for (auto iter = VIDEO_LIST.begin(); iter != VIDEO_LIST.end(); iter++) {
            fout << "    \"" + iter->name + "\"";
            if (iter + 1 != VIDEO_LIST.end()) {
                fout << ",";
            }
            fout << "\n";
        }
        fout << "  ]\n}\n";
        fout.close();
        return true;
    }
}

bool miniVideoProcessor::checkVideoList(vector<string> vlist_disk) {
    Json::Value vlist_json;
    Json::Reader reader(Json::Features::strictMode());
    ifstream ifs;
    ifs.open(VIDEO_LIST_DIR_R, ios::binary);
    if (!reader.parse(ifs, vlist_json)) {
        cout << "> error: Failed to parse video list file!" << endl;
        ifs.close();
        return false;
    }
    else {
        ifs.close();
        // check if the video is still in the working directory
        cout << "> info: updating video list, it might take a few seconds..." << endl;
        LAST_NUM = vlist_json["Last processed video serial number"].asInt();
        Json::Value arrayObj = vlist_json["VideoList"];
        // load videos which is in the video_list.json
        for (unsigned int i = 0; i < arrayObj.size(); i++) {
            VideoInfo temp;
            temp.name = arrayObj[i].asString();
            temp.valid = false;
            VIDEO_LIST.push_back(temp);
        }
        // update VIDEO_LIST
        // add new videos
        for (auto vlist_iter_disk = vlist_disk.begin(); vlist_iter_disk != vlist_disk.end(); vlist_iter_disk++) {
            auto vlist_iter_obj = find_if(VIDEO_LIST.begin(), VIDEO_LIST.end(), [&](VideoInfo& vi) {return vi.name == *vlist_iter_disk; });
            if (vlist_iter_obj != VIDEO_LIST.end()) {
                vlist_iter_obj->valid = true;
            }
            else {
                VideoInfo temp;
                temp.name = *vlist_iter_disk;
                temp.valid = true;
                VIDEO_LIST.push_back(temp);
                cout << "> VideoList changed: " << temp.name << " has been added to the list!" << endl;
            }
        }
        // remove deleted videos
        for (auto iter = VIDEO_LIST.begin(); iter != VIDEO_LIST.end();) {
            if (!iter->valid) {
                auto serialNum = distance(VIDEO_LIST.begin(), iter);
                cout << "> VideoList changed: " << iter->name << " has been removed!" << endl;
                if (static_cast<int>(serialNum) < LAST_NUM - 1) {
                    LAST_NUM--;
                }
                iter = VIDEO_LIST.erase(iter);
            }
            else iter++;
        }
        // update video_list.json
        generateVideoList();
        cout << "> info: Update completed." << endl << endl;
        return true;
    }
}

void miniVideoProcessor::printConfig() {
    cout << R"( __  __ _       _ _     _ _     _            ____)" << endl;
    cout << R"(|  \/  (_)_ __ (_\ \   / /_) __| | ___  ___ |  _ \ _ __ ___   ___ ___  ___ ___  ___  _ __ )" << endl;
    cout << R"(| |\/| | | '_ \| |\ \ / /| |/ _` |/ _ \/ _ \| |_) | '__/ _ \ / __/ _ \/ __/ __|/ _ \| '__|)" << endl;
    cout << R"(| |  | | | | | | | \ V / | | (_| |  __/ (_) |  __/| | | (_) | (_|  __/\__ \__ \ (_) | | )" << endl;
    cout << R"(|_|  |_|_|_| |_|_|  \_/  |_|\__,_|\___|\___/|_|   |_|  \___/ \___\___||___/___/\___/|_| )" << endl << endl;
    cout << "-------------------------------------- configuration --------------------------------------" << endl;
    cout << "$ Working Directory: " << WORKING_DIR << endl;
    if (RENAME_FILES_FLAG) {
        cout << "$ RENAME FILES: true" << endl;
        if (RENAME_CUSTOMIZED) cout << "$ PREFIX: " << RENAME << endl;
    }
    else cout << "$ Rename files?: false  (use \"mvp -r prefix\" to rename files, prefix is optional.)" << endl;
    cout << "$ FPS: " << FPS << endl;
    cout << "$ Total frames of new video: " << NEW_VIDEO_FRAMES << endl;
    cout << "$ Category: " << endl;
    for (int i = 0; i < CATEGORY.size(); i++) {
        cout << "  - " << CATEGORY[i] << endl;
    }
    cout << "$ Total number of videos in the working directory: " << VIDEO_LIST.size() << endl;
    if (LAST_NUM != 0)cout << "$ The last processed video serial number is: " << LAST_NUM << endl;
    cout << "-------------------------------------------------------------------------------------------" << endl << endl;
    if (static_cast<int>(VIDEO_LIST.size()) == 0) {
        cout << "> info: No video found in the working directory!" << endl;
    }
    else {
        int num;
        cout << "> Please enter the start video number: ";
        string wrongInput;
        while (true) {
            cin >> num;
            while (cin.fail()) {
                cin.clear();
                cin >> wrongInput;
                cout << "> error: \"" << wrongInput << "\"is not a number, re-enter: ";
                cin >> num;
            }
            if (num < 1 || num > VIDEO_LIST.size()) {
                cout << "> error: the number should be between 1 and " << VIDEO_LIST.size() << ", re-enter: ";
            }
            else {
                START_NUM = num;
                break;
            }
        }
    }
}

bool miniVideoProcessor::MVP() {
    bool quit = false;
    VideoCapture capture;
    Mat frame;
    for (LAST_NUM = START_NUM; LAST_NUM <= VIDEO_LIST.size() && !quit; LAST_NUM++) {
        system("cls");
        DELAY = 1000 / FPS;
        int totalFrames, curFrame;
        string name = VIDEO_LIST[LAST_NUM - 1].name;
        frame = capture.open(WORKING_DIR_R + name);
        if (!capture.isOpened()) {
            cout << "> error: can not open video file..." << endl;
            return false;
        }
        totalFrames = static_cast<int>(capture.get(CAP_PROP_FRAME_COUNT));
        namedWindow("Video Player", CV_WINDOW_NORMAL);
        setMouseCallback("Video Player", onMouse);
        int keyValue;
        bool finish = false, next = true;
        while (capture.read(frame) && !finish) { //read ++curFrame!
            curFrame = static_cast<int>(capture.get(CAP_PROP_POS_FRAMES));
            setcursorpos(0, 0);
            cout << "Current video serial number: " << LAST_NUM << endl;
            cout << "Current frame: " << curFrame << "/" << totalFrames << "      " << endl;
            imshow("Video Player", frame);
            keyValue = waitKeyEx(DELAY);
            setTrack(capture, frame, keyValue);
            if (keyValue == 'q') {
                finish = true;
                quit = true;
            }
            else if (keyValue == 'j') {
                finish = true;
                system("cls");
                setcursorpos(0, 0);
                cout << "> info: Skipped this video£¡" << endl;
                waitKey(300);
            }
            else if (keyValue == ' ') {
                while (true) {
                    keyValue = waitKeyEx(0);
                    if (keyValue == 32)break;
                    else if (keyValue == 'q') {
                        finish = true;
                        quit = true;
                        break;
                    }
                    else if (keyValue == 13) {
                        finish = processVideo(capture, frame);
                        break;
                    }
                    else setTrack(capture, frame, keyValue);
                }
            }
            if (static_cast<int>(capture.get(CAP_PROP_POS_FRAMES)) == totalFrames) {
                waitKey(100);
                capture.set(CAP_PROP_POS_FRAMES, 0);
            }
            if (next) {
                next = false;
                system("cls");
            }
        }
        capture.release();
    }
    system("cls");
    if (saveLastNum()) {
        cout << "> info: Last processed video serial number(" << LAST_NUM - 1 << ") has been saved successfully." << endl;
    }
    else {
        cout << "> error: Failed to save last processed video serial number(" << LAST_NUM - 1 << ")..." << endl;
    }
    cout << "> MVP: Exit successfully!\n\n";
    return true;
}

void miniVideoProcessor::setTrack(VideoCapture& capture, Mat& frame, int keyValue) {
    bool flag = false;
    int curFrame = static_cast<int>(capture.get(CAP_PROP_POS_FRAMES)),
        totalFrames = static_cast<int>(capture.get(CAP_PROP_FRAME_COUNT));
    if (keyValue == 2424832) {
        flag = true;
        if (curFrame < SKIP_FRAMES) curFrame = 0;
        else curFrame -= SKIP_FRAMES + 1;
    }
    else if (keyValue == 2555904) {
        flag = true;
        if (curFrame + SKIP_FRAMES >= totalFrames)curFrame = totalFrames - 1;
        else curFrame += SKIP_FRAMES - 1;
    }
    if (flag) {
        capture.set(CAP_PROP_POS_FRAMES, curFrame);
        capture.read(frame);
        imshow("Video Player", frame);
        setcursorpos(0, 1);
        cout << "Current frame: " << curFrame + 1 << "/" << totalFrames << "      " << endl;
    }
}

bool miniVideoProcessor::processVideo(VideoCapture& capture, Mat& frame) {
    int beginFrame = static_cast<int>(capture.get(CAP_PROP_POS_FRAMES)), endFrame,
        totalFrames = static_cast<int>(capture.get(CAP_PROP_FRAME_COUNT));
    int curFrame = beginFrame, frames = NEW_VIDEO_FRAMES, gap = 1, newFrame = 0;
    if (totalFrames - beginFrame < NEW_VIDEO_FRAMES - 1) {
        frames = totalFrames - beginFrame + 1;
        setcursorpos(0, 3);
        cout << "[Frame extraction interval£º1]" << endl;
        cout << "> warning: Frame of new video is " << frames << ", which is below " << NEW_VIDEO_FRAMES << "!" << endl;
        endFrame = totalFrames;
    }
    else {
        setcursorpos(0, 3);
        cout << "[Frame extraction interval£º1]" << endl;
        cout << "> info: Frame of new video is " << NEW_VIDEO_FRAMES << endl;
        endFrame = beginFrame + NEW_VIDEO_FRAMES - 1;
    }
    capture.set(CAP_PROP_POS_FRAMES, curFrame - 1);
    originalPoint = Point(0, 0);
    processPoint = Point(static_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH)), static_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT)));
    Rect box;
    bool writeFlag = false;
    VideoWriter outputVideo;
    int keyValue;
    while (true) {
        capture.read(frame);
        curFrame = static_cast<int>(capture.get(CAP_PROP_POS_FRAMES));
        if ((curFrame - beginFrame + 1) % gap == 0) {
            newFrame++;
            if (originalPoint != processPoint) {
                rectangle(frame, originalPoint, processPoint, Scalar(255, 0, 0), 2);
                box = StandardizeBOX(capture, originalPoint, processPoint);
            }
            imshow("Video Player", frame);
            if (!writeFlag)keyValue = waitKeyEx(DELAY);
            else keyValue = waitKeyEx(1);
            if (!writeFlag) {
                setcursorpos(0, 1);
                cout << "Current frame: " << curFrame << "/" << totalFrames << "      " << endl;
                cout << "Current frame in new video£º " << newFrame << "/" << frames << "      " << endl;
            }
            if (keyValue == 13 && !writeFlag) {
                capture.set(CAP_PROP_POS_FRAMES, beginFrame - 1);
                capture.read(frame);
                curFrame = static_cast<int>(capture.get(CAP_PROP_POS_FRAMES));
                newFrame = 1;
                writeFlag = true;
                system("cls");
                cout << "> Category: " << endl;
                int list = 0;
                for (list = 0; list < CATEGORY.size(); list++) {
                    cout << list + 1 << ". " << CATEGORY[list] << endl;
                }
                cout << "> info: Please press the corresponding number key to select the category.('q' to cancel)";
                int select;
                while (true) {
                    keyValue = waitKeyEx(0);
                    if (keyValue >= 49 && keyValue <= 48 + list) {
                        select = keyValue - 48;
                        break;
                    }
                    else if (keyValue == 'q') {
                        return false;
                    }
                    else {
                        setcursorpos(0, 6);
                        char c = keyValue;
                        cout << "> info: '";
                        if (keyValue == 13) cout << "Enter";
                        else if (keyValue == 27) cout << "ESC";
                        else if (keyValue == 9) cout << "Tab";
                        else cout << c;
                        cout << "' is not a valid number key. Please try again.('q' to cancel)" << endl;
                    }
                }
                string outputdir = WORKING_DIR + static_cast<string>(CATEGORY[select - 1]) + "\\";
                CreateDirectoryA(outputdir.c_str(), NULL);
                vector <string> output_files;
                getFiles(outputdir, output_files);
                string outputName = outputdir + to_string(output_files.size() + 1) + "_" + static_cast<string>(CATEGORY[select - 1]) + ".mp4";
                outputVideo.open(outputName, VideoWriter::fourcc('D', 'I', 'V', 'X'), 30, Size(box.width, box.height), true);
                system("cls");
            }
            else if (keyValue == 27 && !writeFlag) {
                system("cls");
                capture.set(CAP_PROP_POS_FRAMES, beginFrame - 1);
                return false;
            }
            else if (keyValue == 106 && !writeFlag) {
                system("cls");
                setcursorpos(0, 0);
                cout << "> info: Skipped this video£¡" << endl;
                waitKey(500);
                return true;
            }
            else if (keyValue == 2490368 && !writeFlag) {
                gap++;
                if ((totalFrames - beginFrame + 1) / gap < NEW_VIDEO_FRAMES) {
                    frames = (totalFrames - beginFrame + 1) / gap;
                    setcursorpos(0, 3);
                    cout << "[Frame extraction interval£º" << gap << "]                    " << endl;
                    cout << "> warning: Frame of new video is " << frames << ", which is below " << NEW_VIDEO_FRAMES << "!" << endl;
                    endFrame = totalFrames;
                }
                else {
                    setcursorpos(0, 3);
                    cout << "[Frame extraction interval£º" << gap << "]                    " << endl;
                    cout << "> info: Frame of new video is " << NEW_VIDEO_FRAMES << endl;
                    endFrame = beginFrame + NEW_VIDEO_FRAMES * gap - 1;
                }
            }
            else if (keyValue == 2621440 && !writeFlag) {
                gap--;
                if (gap < 1) gap = 1;
                if ((totalFrames - beginFrame + 1) / gap < NEW_VIDEO_FRAMES) {
                    frames = (totalFrames - beginFrame + 1) / gap;
                    setcursorpos(0, 3);
                    cout << "[Frame extraction interval£º" << gap << "]                    " << endl;
                    cout << "> warning: Frame of new video is " << frames << ", which is below " << NEW_VIDEO_FRAMES << "!" << endl;
                    endFrame = totalFrames;
                }
                else {
                    setcursorpos(0, 3);
                    cout << "[Frame extraction interval£º" << gap << "]                    " << endl;
                    cout << "> info: Frame of new video is " << NEW_VIDEO_FRAMES << endl;
                    endFrame = beginFrame + NEW_VIDEO_FRAMES * gap - 1;
                }
            }
            if (writeFlag) {
                setcursorpos(0, 0);
                cout << "> info: Saving video.." << endl << "progress £º" << newFrame << "/" << frames << "      " << endl << endl << endl;;
                Mat crop(frame, box);
                outputVideo << crop;
            }
        }
        if (curFrame == endFrame) {
            if (writeFlag) {
                system("cls");
                setcursorpos(0, 0);
                cout << "> info: Video saved successfully£¡" << endl;
                waitKey(600);
                system("cls");
                return false;
            }
            else {
                newFrame = 0;
                capture.set(CAP_PROP_POS_FRAMES, beginFrame - 1);
            }
        }
    }
}

Rect miniVideoProcessor::StandardizeBOX(VideoCapture& capture, Point originalPoint, Point processPoint) {
    Rect box;
    auto frame_height = static_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT));
    auto frame_width = static_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH));
    int x1 = originalPoint.x, x2 = processPoint.x;
    if (x1 < 0)x1 = 0;
    if (x2 < 0)x2 = 0;
    if (x1 > frame_width)x1 = frame_width;
    if (x2 > frame_width)x2 = frame_width;
    if (x1 > x2)swap(x1, x2);
    int y1 = originalPoint.y, y2 = processPoint.y;
    if (y1 < 0)y1 = 0;
    if (y2 < 0)y2 = 0;
    if (y1 > frame_height)y1 = frame_height;
    if (y2 > frame_height)y2 = frame_height;
    if (y1 > y2)swap(y1, y2);
    box.x = x1;
    box.y = y1;
    box.width = x2 - x1;
    box.height = y2 - y1;
    return box;
}

bool miniVideoProcessor::saveLastNum() {
    Json::Value vlist_json;
    Json::Reader reader(Json::Features::strictMode());
    ifstream ifs;
    ifs.open(VIDEO_LIST_DIR_R, ios::binary);
    if (!reader.parse(ifs, vlist_json)) {
        cout << "> error: Failed to parse video list file!" << endl;
        ifs.close();
        return false;
    }
    else {
        ifs.close();
        vlist_json["Last processed video serial number"] = LAST_NUM - 1;
        ofstream fout;
        fout.open(VIDEO_LIST_DIR_R, ios::out | ios::trunc);
        if (!fout.is_open()) {
            cout << "> error: Failed to open video list file! " << endl;
            return false;
        }
        else {
            string json_str = "{\n";
            json_str += "  // This file is automatically generated by MVP,\n";
            json_str += "	// which can prevent duplicate processing of a video\n";
            json_str += "	// due to changes in the working directory.\n";
            json_str += "	// DO NOT MODIFY THIS FILE MANUALLY!";
            string temp = vlist_json.toStyledString();
            temp = temp.substr(1);
            json_str += temp;
            fout << json_str;
            fout.close();
            return true;
        }
    }
}

void onMouse(int event, int x, int y, int flags, void* ustc) {
    if (event == CV_EVENT_LBUTTONDOWN) {
        leftButtonDownFlag = true;
        originalPoint = Point(x, y);
        processPoint = originalPoint;
    }
    if (event == CV_EVENT_MOUSEMOVE && leftButtonDownFlag) {
        processPoint = Point(x, y);
    }
    if (event == CV_EVENT_LBUTTONUP) {
        leftButtonDownFlag = false;
    }
}
