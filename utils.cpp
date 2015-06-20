
#include "utils.h"

namespace
{
	const string window_name = "Hold'em";
	vector<string> wnds;
}

BOOL CALLBACK enum_windows(HWND hwnd, LPARAM param)
{
	char buf[255];

	if (IsWindowVisible(hwnd))
	{
		GetWindowTextA(hwnd, buf, 254);
		wnds.push_back(buf);
	}

	return TRUE;
}

vector<string> find_all_windows()
{
	wnds.clear();
	EnumWindows(enum_windows, NULL);

	return wnds;
}

vector<HWND> find_target_windows()
{
	vector<HWND> windows;
	vector<string> window_list = find_all_windows();

	for (vector<string>::const_iterator it = window_list.begin();
		it != window_list.end(); ++it)
	{
		if (it->empty())
			continue;

		if (it->find(window_name) != string::npos)
		{
			if (HWND handle = FindWindowA(NULL, it->c_str()))
				windows.push_back(handle);
		}
	}

	return windows;
}

pair<double, double> recog_blinds(HWND source)
{
	pair<double, double> blinds = make_pair(0, 0);

	if (!IsWindow(source))
		return blinds;

	char buf[255];
	GetWindowTextA(source, buf, 254);

	const string title(buf);
	string temp;
		
	for (string::const_reverse_iterator crit = title.crbegin();
		crit != title.crend(); ++crit)
	{
		const char ch = *crit;

		if (ch != '-')
			temp += ch;
		else break;
	}

	string str;

	for (string::const_reverse_iterator crit = temp.crbegin();
		crit != temp.crend(); ++crit)
	{
		const char ch = *crit;
		
		if (isdigit(ch) || ch == '.')
		{
			str += ch;
		}
		else if (ch == '/')
		{
			blinds.first = boost::lexical_cast<double>(str);
			str.clear();
		}
	}

	blinds.second = boost::lexical_cast<double>(str);

	return blinds;
}

string generate_unique_name(uint name_len, string ext)
{
	boost::uuids::random_generator generator;
	boost::uuids::uuid id = generator();
	string temp = boost::lexical_cast<string>(id);

	assert(name_len > 0);

	if (name_len > temp.length())
		name_len = temp.length();

	if (!ext.empty())
		ext = "." + ext;

	return temp.substr(0, name_len) + ext;
}

void play_sound(const string sound)
{
	if (sound.length())
	{
		path root = resources::get_root();
		string filename = root.generic_string() + "//" + sound + ".wav";
		
		if (exists(path(filename)))
			PlaySound(filename.c_str(), NULL, SND_FILENAME);
	}
}

void save_card_images_DEBUG(const cv::Mat & frame)
{
	boost::array<cv::Rect, 5> rects;
	rects[0] = cv::Rect(274, 239, 15, 29);
	rects[1] = cv::Rect(327, 239, 15, 29);
	rects[2] = cv::Rect(380, 239, 15, 29);
	rects[3] = cv::Rect(433, 239, 15, 29);
	rects[4] = cv::Rect(486, 239, 15, 29);

	for (int i = 0; i < 5; i++)
	{
		cv::Mat roi(frame, rects[i]);
		cv::imwrite(generate_unique_name(6) + "_" + boost::lexical_cast<string>(i + 1) + ".bmp", roi);
	}
}

void save_players_rois(const cv::Mat & frame, uint table_size)
{
	vector<cv::Rect> rects = seats_rects(table_size);

	for (uint i = 0; i < rects.size(); i++)
	{
		cv::Mat buf(frame, rects[i]);
		cv::imwrite(generate_unique_name(6, "bmp"), buf);
	}
}

void save_frame(const cv::Mat & frame)
{
	const string name = generate_unique_name(6, "bmp");
	cv::imwrite(name, frame);
}

void display_hud(HWND window, string text, int x, int y)
{
	HDC dc = GetDC(window);
	TextOut(dc, x, y, text.c_str(), text.length());
}