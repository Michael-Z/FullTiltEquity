
#pragma once

#include "stdafx.h"
#include "poker.h"

class screenshot
{
public:
	screenshot(HWND window);
	~screenshot();

	cv::Mat image();
	cv::Mat grayscale();
	bool ready() const;
private:
	void capture();
	void init();
	void release();
private:
	HWND hwnd;
	HDC hdc;
	HDC screen_dc;
	HBITMAP bitmap;
	BITMAPINFO bitmap_info;

	int width;
	int height;

	boost::shared_ptr<uchar> data;
	boost::shared_ptr<cv::Mat> screen;

	HGDIOBJ bitmap_buf;
};

struct resources
{
	vector<cv::Mat> seats;
	vector<cv::Mat> center;
	map<string, cv::Mat> cards;
	map<string, cv::Mat> actions;
	map<string, cv::Mat> misc;
	map<string, cv::Mat> digits;

	resources();
	static path get_root();
private:
	map<string, cv::Mat> load_map(const string category);
	vector<cv::Mat> load(const string category);
	string get_signame(const string filename);
};

bool frame_valid(const cv::Mat & frame);
bool images_match_memory(const cv::Mat & lval, const cv::Mat & rval);
bool images_match(const cv::Mat & first, const cv::Mat & second);

uint count_matches(const cv::Mat & image, const vector<cv::Mat> & signatures);
uint count_matches(const cv::Mat & image, const vector<cv::Rect> & rects, const vector<cv::Mat> & signatures);
bool is_inside(const cv::Mat source, const vector<cv::Mat> & signatures);
bool is_inside(const cv::Mat source, const cv::Mat & signature);
vector<int> x_coords_inside(const cv::Mat source, const cv::Mat & signature);
vector<cv::Mat> get_image_rois(const cv::Mat & image, const vector<cv::Rect> & rects);
uint find_first_digit_x(const cv::Mat & frame, const cv::Rect & rect, const resources & res); 

uint count_table_size(HWND window, const resources & res);
cards<5> recog_table_cards(const cv::Mat & frame, const resources & res);
uint recog_dealer_position(const cv::Mat & frame, const resources & res, uint table_size);
vector<player> recog_players(const cv::Mat & frame, const resources & res, uint table_size);
player recog_player(const cv::Mat & frame, const cv::Mat & roi, const resources & res, uint table_size, uint player_num);
double recog_pot(const cv::Mat & frame, const resources & res);
cards<2> recog_player_hand(const cv::Mat & frame, const resources & res, uint table_size);
double recog_digits(const cv::Mat & frame, const cv::Rect & rect, const resources & res);
uint get_play_position(uint table_size);
bool recog_is_player_active(const cv::Mat & frame, const resources & res, uint table_size, uint player_index);
bool recog_is_banner_opened(const cv::Mat & frame, const resources & res, uint table_size);

vector<cv::Rect> seats_rects(uint table_size);
vector<cv::Rect> central_cards_rects();
vector<cv::Rect> my_hand_cards_rects(uint table_size);
vector<cv::Rect> card_backs_rects(uint table_size);
vector<cv::Rect> dealer_rects(uint table_size);
vector<cv::Rect> digit_rects(uint table_size);

/*debug*/ void save_open_cards_rois(const cv::Mat & table);