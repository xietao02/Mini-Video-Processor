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

CONSOLE_CURSOR_INFO CursorInfo;
bool leftButtonDownFlag = false;
Point originalPoint;
Point processPoint;

int init(vector <string>& category, bool RENAME_FILES_FLAG, string* WORKING_DIR, int* START_NUM, int* FPS, int* NEW_VIDEO_FRAMES) {
	if (loadConfig(category, FPS, NEW_VIDEO_FRAMES) == -1)return -1;
	
	char* buffer;
	if ((buffer = _getcwd(NULL, 0)) == NULL) {
		perror("An error occurred while getting the working path! ");
		return -1;
	}
	else {
		string tempDir = buffer, temp;
		istringstream itemp(tempDir);
		*WORKING_DIR = "";
		while (getline(itemp, temp, '\\')) {
			*WORKING_DIR += (temp + R"(\\)");
		}
		free(buffer);
	}
	cout << "Welcome to Video Processor!" << endl;
	cout << "info: 第一次使用需重命名文件!" << endl << endl;

	cout << "<----------- configuration ----------->" << endl;
	cout << "$ Working Directory: " << *WORKING_DIR << endl;
	if (RENAME_FILES_FLAG)cout << "$ RENAME FILES: true" << endl;
	else cout << "$ Rename files?: false  (use \"vp -r\" to rename files)" << endl;
	cout << "$ FPS: " << *FPS << endl;
	cout << "$ Total frames of new video: " << *NEW_VIDEO_FRAMES << endl;
	cout << "$ Category: " << endl;
	for (int i = 0; i < category.size(); i++) {
		cout << "- " << category[i] << endl;
	}
	cout << "<------------------------------------->" << endl;
	
	int num;
	bool flag = false;
	cout << "Please select start number: ";
	cin >> num;
	do {
		if (num > 0) {
			*START_NUM = num;
			flag = true;
		}
		else cin >> num;
	} while (!flag);
	return 0;
}

int loadConfig(vector <string>& category, int* FPS, int* NEW_VIDEO_FRAMES) {
	char exePath[MAX_PATH];
	GetModuleFileName(NULL, (LPSTR)exePath, sizeof(exePath));
	string tempDir = exePath, temp;
	istringstream itemp(tempDir);
	string CONFIG_PATH = "";
	string exe = "vp.exe";
	while (getline(itemp, temp, '\\')) {
		if (temp != exe)CONFIG_PATH += (temp + R"(\\)");
	}
	CONFIG_PATH += "config.txt";
	if (_access(CONFIG_PATH.c_str(), 0) == -1) {
		cout << "warning: The config file does not exist! " << endl;
		ofstream fout;
		fout.open(CONFIG_PATH, ios::out);
		if (!fout.is_open()) {
			cout << "error: Failed to create config file! " << endl;
		}
		else{
			cout << "info: config file has been created in \"" << CONFIG_PATH << "\"." << endl;
			cout << "info: please edit the config file and run the program again." << endl;
			fout << "// 如果不小心破坏了配置文件格式，可删除配置文件后运行 vp.exe 恢复！" << endl;
			fout << "" << endl;
			fout << "// 视频播放器刷新率" << endl;
			fout << "$ FPS: 30" << endl;
			fout << "" << endl;
			fout << "// 剪辑视频的总帧数" << endl;
			fout << "$ Total frames of new video: 70" << endl;
			fout << "" << endl;
			fout << "// 视频的所有类别" << endl;
			fout << "$ Categories:" << endl;
			fout << "- cat_running" << endl;
			fout << "- cat_walking" << endl;
			fout << "- cat_eating" << endl;
			fout << "- cat_jumping" << endl;
			fout << "" << endl;
			fout << "" << endl;
			fout << "// Copyright (c) 2022 xietao02@outlook.com" << endl;
		}
		return -1;
	}
	else {
		ifstream fin(CONFIG_PATH);
		string line, match, kind;
		size_t pos1, pos2, length;
		int line_num = 0;
		while (getline(fin, line, '\n')) {
			pos1 = line.find("//");
			if (pos1 < 20) continue;
			pos1 = line.find('$');
			if (pos1 > 20) continue;
			else {
				pos2 = line.find(':');
				length = pos2 - pos1;
				match = line.substr(pos1 + 2, length - 2);
				if (match == "FPS") {
					match = line.substr(pos2 + 1);
					*FPS = stoi(match);
				}
				else if (match == "Total frames of new video") {
					match = line.substr(pos2 + 1);
					*NEW_VIDEO_FRAMES = stoi(match);
				}
				else if (match == "Categories") {
					while (getline(fin, line, '\n')) {
						if (pos1 = line.find('-') > 20) continue;
						kind = line.substr(2);
						category.push_back(kind);
					}
				}
			}
		}
		fin.close();
		cout << "Load config file successfully!" << endl;
	}
	return 0;
}


void getFiles(string path, vector <string>& files) {
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
	perror("error: ");
}

void reNameFiles(bool flag, string WORKING_DIR) {
	if (flag) {
		string basedir = WORKING_DIR;
		vector <string> files;
		getFiles(basedir, files);
		for (int i = 0; i < files.size(); i++) {
			cout << files[i] << " - ";
			string number = to_string(i + 1);
			string oldName = basedir + files[i];
			string newName = basedir + number + ".mp4";
			if (-1 == rename(oldName.c_str(), newName.c_str())) {
				cout << "rename: failed" << endl;
			}
			else cout << "rename: done" << endl;
		}
		cout << "视频文件重命名结束.." << endl << endl;
		system("pause");
	}
}


int VideoProcessor(int filesNum, int FPS, int START_NUM, string WORKING_DIR, int NEW_VIDEO_FRAMES, vector <string>& category) {
	bool quit = false;
	VideoCapture capture;
	Mat frame;
	for (int i = START_NUM; i <= filesNum && !quit; i++) {
		int delay = 1000 / FPS;
		int totalFrames, curFrame;
		string name = to_string(i);
		frame = capture.open(WORKING_DIR + name + ".mp4");
		if (!capture.isOpened()) {
			system("cls");
			printf("can not open file...\n");
			return -1;
		}
		totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
		namedWindow("Video Player", CV_WINDOW_NORMAL);
		auto frame_height = capture.get(CAP_PROP_FRAME_HEIGHT);
		auto frame_width = capture.get(CAP_PROP_FRAME_WIDTH);
		setMouseCallback("Video Player", onMouse);
		int keyValue;
		bool finish = false, next = true;
		while (capture.read(frame) && !finish) { //读取 ++curFrame
			curFrame = capture.get(CAP_PROP_POS_FRAMES);
			setcursorpos(0, 0);
			cout << "当前处理视频序号: " << i << endl;
			cout << "当前帧: " << curFrame << "/" << totalFrames << "      " << endl;
			imshow("Video Player", frame);
			keyValue = waitKeyEx(delay);
			setTrack(capture, frame, keyValue, 30);
			if (keyValue == 113) {
				finish = true;
				quit = true;
			}
			else if (keyValue == 106) {
				finish = true;
				system("cls");
				setcursorpos(0, 0);
				cout << "info: 已跳过该视频！" << endl;
				waitKey(500);
			}
			else if (keyValue == 32) {
				while (true) {
					keyValue = waitKeyEx(0);
					if (keyValue == 32)break;
					else if (keyValue == 113) {
						finish = true;
						quit = true;
						break;
					}
					else if (keyValue == 13) {
						finish = processVideo(capture, frame, delay, WORKING_DIR, NEW_VIDEO_FRAMES, category);
						break;
					}
					else setTrack(capture, frame, keyValue, 30);
				}
			}
			if (capture.get(CAP_PROP_POS_FRAMES) == totalFrames) {
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
	cout << "文件全部处理完毕!";
	return 0;
}

void setTrack(VideoCapture& capture, Mat& frame, int keyValue, int skip) {
	bool flag = false;
	int curFrame = capture.get(CAP_PROP_POS_FRAMES), totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
	if (keyValue == 2424832) {
		flag = true;
		if (curFrame < skip) curFrame = 0;
		else curFrame -= skip;
	}
	else if (keyValue == 2555904) {
		flag = true;
		if (curFrame + skip >= totalFrames)curFrame = totalFrames - 1;
		else curFrame += skip;
	}
	if (flag) {
		capture.set(CAP_PROP_POS_FRAMES, curFrame);
		capture.read(frame);
		imshow("Video Player", frame);
		setcursorpos(0, 1);
		cout << "当前帧: " << curFrame + 1 << "/" << totalFrames << "      " << endl;
	}
}

bool processVideo(VideoCapture& capture, Mat& frame, int delay, string WORKING_DIR, int NEW_VIDEO_FRAMES, vector <string>& category) {
	int beginFrame = capture.get(CAP_PROP_POS_FRAMES), endFrame, totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
	int curFrame = beginFrame, frames = NEW_VIDEO_FRAMES, gap = 1, newFrame = 0;
	if (totalFrames - beginFrame < NEW_VIDEO_FRAMES - 1) {
		frames = totalFrames - beginFrame + 1;
		setcursorpos(0, 3);
		cout << "warning: 素材不足" << NEW_VIDEO_FRAMES << "帧！当前总帧数：" << frames << "，预览播放中.." << endl << "[抽帧间隔：1]" << endl;
		endFrame = totalFrames;
	}
	else {
		setcursorpos(0, 3);
		cout << "info: 素材共有" << NEW_VIDEO_FRAMES << "帧，预览播放中.." << endl << "[抽帧间隔：1]" << endl;
		endFrame = beginFrame + NEW_VIDEO_FRAMES - 1;
	}
	capture.set(CAP_PROP_POS_FRAMES, curFrame - 1);
	auto frame_height = capture.get(CAP_PROP_FRAME_HEIGHT);
	auto frame_width = capture.get(CAP_PROP_FRAME_WIDTH);
	originalPoint = Point(0, 0);
	processPoint = Point(frame_width, frame_height);
	Rect box;
	bool writeFlag = false;
	VideoWriter outputVideo;
	while (true) {
		capture.read(frame);
		curFrame = capture.get(CAP_PROP_POS_FRAMES);
		if ((curFrame - beginFrame + 1) % gap == 0) {
			newFrame++;
			if (originalPoint != processPoint) {
				rectangle(frame, originalPoint, processPoint, Scalar(255, 0, 0), 2);
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
			}
			imshow("Video Player", frame);
			int keyValue = waitKeyEx(delay);
			if (!writeFlag) {
				setcursorpos(0, 1);
				cout << "当前全局帧: " << curFrame << "/" << totalFrames << "      " << endl;
				cout << "素材帧： " << newFrame << "/" << frames << "      " << endl;
			}
			if (keyValue == 13 && !writeFlag) {
				capture.set(CAP_PROP_POS_FRAMES, beginFrame - 1);
				capture.read(frame);
				curFrame = capture.get(CAP_PROP_POS_FRAMES);
				newFrame = 1;
				writeFlag = true;
				string basedir = WORKING_DIR;
				system("cls");
				cout << "Category: " << endl;
				int list = 0;
				for (list = 0; list < category.size(); list++) {
					cout << list + 1 <<". " << category[list] << endl;
				}
				cout << "Please select a category for this video: ";
				int select = -1;
				bool judge = false;
				do {
					if (judge) {
						cout << "number should be in 1 to " << list << ": ";
					}
					cin >> select;
					judge = true;
				} while (select < 0 || select > list);
				string outputdir = basedir + category[select - 1] + "\\";
				CreateDirectoryA(outputdir.c_str(), NULL);
				vector <string> output_files;
				getFiles(outputdir, output_files);
				string outputName = outputdir + to_string(output_files.size()) + "_" + category[select - 1] + ".mp4";
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
				cout << "info: 已跳过该视频！" << endl;
				waitKey(500);
				return true;
			}
			else if (keyValue == 2490368 && !writeFlag) {
				gap++;
				if ((totalFrames - beginFrame + 1) / gap < NEW_VIDEO_FRAMES) {
					frames = (totalFrames - beginFrame + 1) / gap;
					setcursorpos(0, 3);
					cout << "warning: 素材不足" << NEW_VIDEO_FRAMES << "帧！当前总帧数：" << frames << "，预览播放中.." << endl << "[抽帧间隔：" << gap << "]            " << endl;
					endFrame = totalFrames;
				}
				else {
					setcursorpos(0, 3);
					cout << "info: 素材共有" << NEW_VIDEO_FRAMES << "帧，预览播放中.." << endl << "[抽帧间隔：" << gap << "]                        " << endl;
					endFrame = beginFrame + NEW_VIDEO_FRAMES * gap - 1;
				}
			}
			else if (keyValue == 2621440 && !writeFlag) {
				gap--;
				if (gap < 1) gap = 1;
				if ((totalFrames - beginFrame + 1) / gap < NEW_VIDEO_FRAMES) {
					frames = (totalFrames - beginFrame + 1) / gap;
					setcursorpos(0, 3);
					cout << "warning: 素材不足" << NEW_VIDEO_FRAMES << "帧！当前总帧数：" << frames << "，预览播放中.." << endl << "[抽帧间隔：" << gap << "]            " << endl;
					endFrame = totalFrames;
				}
				else {
					setcursorpos(0, 3);
					cout << "info: 素材共有" << NEW_VIDEO_FRAMES << "帧，预览播放中.." << endl << "[抽帧间隔：" << gap << "]                        " << endl;
					endFrame = beginFrame + NEW_VIDEO_FRAMES * gap - 1;
				}
			}
			if (writeFlag) {
				setcursorpos(0, 0);
				cout << "info: 保存裁剪视频中.." << endl << "进度：" << newFrame << "/" << frames << "      " << endl << endl << endl;;
				Mat crop(frame, box);
				outputVideo << crop;
			}
		}
		if (curFrame == endFrame) {
			if (writeFlag) {
				system("cls");
				setcursorpos(0, 0);
				cout << "info: 保存裁剪视频完成！" << endl;
				system("cls");
				waitKey(300);
				return false;
			}
			else {
				newFrame = 0;
				capture.set(CAP_PROP_POS_FRAMES, beginFrame - 1);
			}
		}
	}
}

void onMouse(int event, int x, int y, int flags, void* ustc) {
	if (event == CV_EVENT_LBUTTONDOWN) {
		leftButtonDownFlag = true; //标志位
		originalPoint = Point(x, y);  //设置左键按下点的矩形起点
		processPoint = originalPoint;
	}
	if (event == CV_EVENT_MOUSEMOVE && leftButtonDownFlag) {
		processPoint = Point(x, y);
	}
	if (event == CV_EVENT_LBUTTONUP) {
		leftButtonDownFlag = false;
	}
}


void setcursorpos(unsigned x, unsigned y) {
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hidecursor() {
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CursorInfo);
	CursorInfo.bVisible = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CursorInfo);
}

void displaycursor() {
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CursorInfo);
}
