
#pragma once

#include "stdafx.h"
#include "recognition.h"
#include "poker.h"

BOOL CALLBACK enum_windows(HWND hwnd, LPARAM param);
vector<string> find_all_windows();
vector<HWND> find_target_windows();
pair<double, double> recog_blinds(HWND source);
string generate_unique_name(uint name_len, string ext = "");
void play_sound(const string sound);
void save_card_images_DEBUG(const cv::Mat & frame);
void save_players_rois(const cv::Mat & frame, uint table_size);
void save_frame(const cv::Mat & frame);
void display_hud(HWND window, string text, int x, int y);