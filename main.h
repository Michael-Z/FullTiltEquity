
#pragma once

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "winmm.lib")

#include "stdafx.h"
#include "poker.h"
#include "utils.h"
#include "recognition.h"

using namespace std;
using namespace boost::filesystem;

#define TIMER_ID 101

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show);
HWND init_window(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show);
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, UINT w_param, LONG l_param);
void init(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show);
void handle_thread_exit();

void thread_loop(HWND handle);
bool frame_changed(HWND handle, cv::Mat & frame);
void handle_frame(HWND handle, uint table_size, const resources & res, 
	state & state, boost::shared_ptr<game> & game, table & old_table);
