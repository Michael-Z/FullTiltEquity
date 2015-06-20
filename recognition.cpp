
#include "main.h"
#include "utils.h"
#include "recognition.h"

extern boost::mutex mutex;
extern bool done;

screenshot::screenshot(HWND window) : hwnd(window)
{
	try
	{
		init();
		capture();
	}
	catch (const std::exception & ex)
	{
		std::ofstream error_log("error_log.txt", std::ios::out);
		error_log << ex.what() << std::endl;
		release();
	}
	catch (...)
	{
		release();
	}
}

screenshot::~screenshot()
{
	release();
}

void screenshot::init()
{
	screen_dc = NULL;
	bitmap = NULL;

	if (!hwnd)
		throw std::runtime_error("Window not opened");
}

void screenshot::release()
{
	if (screen_dc) SelectObject(screen_dc, bitmap_buf);
	if (bitmap) DeleteObject(bitmap);
	if (screen_dc) DeleteDC(screen_dc);
}

void screenshot::capture()
{
	hdc = GetWindowDC(hwnd);

	RECT wnd_rect;
	GetClientRect(hwnd, &wnd_rect);
	width = wnd_rect.right - wnd_rect.left;
	height = wnd_rect.bottom - wnd_rect.top;

	screen_dc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	bitmap_buf = SelectObject(screen_dc, bitmap);

	bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmap_info.bmiHeader.biWidth = width;
	bitmap_info.bmiHeader.biHeight = -height;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 24;
	bitmap_info.bmiHeader.biCompression = BI_RGB;

	const int color_size = 3;
	const int alignment = 4;
	int step = (int) ceil(width * color_size / (double) alignment) * alignment;

	data.reset( new uchar[step * height] );

	if (data)
		screen.reset( new cv::Mat(height, width, CV_8UC3, data.get(), step) );
}

cv::Mat screenshot::image()
{
	BitBlt( screen_dc, 0, 0, width, height, hdc, 0, 0, SRCCOPY );
	GetDIBits( screen_dc, bitmap, 0, height, data.get(), &bitmap_info, DIB_RGB_COLORS );
	return *screen;
}

cv::Mat screenshot::grayscale()
{
	cv::Mat gray;
	cv::cvtColor(this->image(), gray, CV_BGR2GRAY);
	return gray;
}

bool screenshot::ready() const
{
	return screen != 0;
}



resources::resources()
{
	boost::mutex::scoped_lock lock(mutex);

	seats = load("seat");
	center = load("center");
	cards = load_map("card");
	actions = load_map("action");
	misc = load_map("misc");
	digits = load_map("digit");
}

path resources::get_root()
{
	return path(current_path().generic_string() + "//resources");
}

map<string, cv::Mat> resources::load_map(const string category)
{
	map<string, cv::Mat> res;
	directory_iterator it(get_root());

	while (it != directory_iterator())
	{
		const path & file = *it;

		if (is_regular_file(file) && file.extension().generic_string() == ".bmp")
		{
			const string filename = file.filename().generic_string();
			
			if (filename.find(category) != string::npos)
			{
				const string name = filename.substr(0, filename.length() - 4);
				const string signame = get_signame(name);
				const cv::Mat & buf = cv::imread(file.generic_string());
				res.insert(make_pair(signame, buf));
			}
		}

		it++;
	}

	return res;
}

vector<cv::Mat> resources::load(const string category)
{
	vector<cv::Mat> res;
	directory_iterator it(get_root());

	while (it != directory_iterator())
	{
		const path & file = *it;

		if (is_regular_file(file) && file.extension().generic_string() == ".bmp")
		{
			const string filename = file.filename().generic_string();

			if (filename.empty())
				continue;
			
			if (filename.find(category) != string::npos)
			{
				const cv::Mat & buf = cv::imread(file.generic_string());
				res.push_back(buf);
			}
		}

		it++;
	}

	return res;
}

string resources::get_signame(const string filename)
{
	boost::char_separator<char> sep("-");
	boost::tokenizer<boost::char_separator<char>> token(filename, sep);
	string signame;
	bool first = true;

	for (boost::tokenizer<boost::char_separator<char>>::iterator it 
		= token.begin(); it != token.end(); ++it)
	{
		if (!first)
			signame += *it;

		first = false;
	}

	return signame;
}



bool frame_valid(const cv::Mat & frame)
{
	return frame.cols == 794
		&& frame.rows == 547
		&& frame.dims == 2
		&& frame.channels() == 1;
}

bool images_match_memory(const cv::Mat & lval, const cv::Mat & rval)
{
	return memcmp(lval.data, rval.data, rval.rows * rval.cols * rval.channels()) == 0;
}

bool images_match(const cv::Mat & first, const cv::Mat & second)
{
	if (first.cols != second.cols || 
		first.rows != second.rows || 
		first.dims != second.dims) 
	{
		return false;
	}

	for (int x = 0; x < first.rows; x++)
		for (int y = 0; y < first.cols; y++)
			if ( first.at<byte>(x, y) != second.at<byte>(x, y) )
				return false;

	return true;
}



uint count_matches(const cv::Mat & image, const vector<cv::Mat> & signatures)
{
	uint matches = 0;

	assert(signatures.size());

	for (int x = 0; x < image.cols; x++)
	{
		for (int y = 0; y < image.rows; y++)
		{
			cv::Rect rect(x, y, signatures[0].cols, signatures[0].rows);

			if ( (x + rect.width) > image.cols || 
				(y + rect.height) > image.rows )
				continue;

			cv::Mat roi(image, rect);

			for (size_t i = 0; i < signatures.size(); i++)
			{
				if (images_match(signatures[i], roi))
					matches++;
			}
		}
	}

	return matches;
}

uint count_matches(const cv::Mat & image, const vector<cv::Rect> & rects, const vector<cv::Mat> & signatures)
{
	uint matches = 0;

	assert(rects.size() && signatures.size());

	for (size_t idx = 0; idx < rects.size(); idx++)
	{
		cv::Mat roi(image, rects[idx]);
		matches += count_matches(roi, signatures);
	}

	return matches;
}

bool is_inside(const cv::Mat source, const vector<cv::Mat> & signatures)
{
	assert(signatures.size());

	for (int x = 0; x < source.cols; x++)
	{
		for (int y = 0; y < source.rows; y++)
		{
			cv::Rect rect(x, y, signatures[0].cols, signatures[0].rows);

			if ( (x + rect.width) > source.cols || 
				(y + rect.height) > source.rows )
				continue;

			cv::Mat roi(source, rect);

			for (size_t i = 0; i < signatures.size(); i++)
			{
				if (images_match(signatures[i], roi))
					return true;
			}
		}
	}

	return false;
}

bool is_inside(const cv::Mat source, const cv::Mat & signature)
{
	for (int x = 0; x < source.cols; x++)
	{
		for (int y = 0; y < source.rows; y++)
		{
			cv::Rect rect(x, y, signature.cols, signature.rows);

			if ( (x + rect.width) > source.cols || 
				(y + rect.height) > source.rows )
				continue;

			cv::Mat search_roi(source, rect);

			if (images_match(signature, search_roi))
				return true;
		}
	}

	return false;
}

vector<int> x_coords_inside(const cv::Mat source, const cv::Mat & signature)
{
	vector<int> coords;

	for (int x = 0; x < source.cols; x++)
	{
		for (int y = 0; y < source.rows; y++)
		{
			cv::Rect rect(x, y, signature.cols, signature.rows);

			if ( (x + rect.width) > source.cols || 
				(y + rect.height) > source.rows )
				continue;

			cv::Mat search_roi(source, rect);

			if (images_match(signature, search_roi))
			{
				coords.push_back(x);
			}
		}
	}

	return coords;
}

vector<cv::Mat> get_image_rois(const cv::Mat & image, const vector<cv::Rect> & rects)
{
	vector<cv::Mat> rois;

	for (size_t idx = 0; idx < rects.size(); idx++)
	{
		rois.push_back( cv::Mat(image, rects[idx]) );
	}

	return rois;
}

uint find_first_digit_x(const cv::Mat & frame, const cv::Rect & rect, const resources & res)
{
	vector<int> x_coords;

	for (int y = 0; y < rect.height; y++)
	{
		for (int x = 0; x < rect.width; x++)
		{
			cv::Vec3b col = frame.at<cv::Vec3b>(cv::Point(x + rect.x, y + rect.y));

			if (col[0] == 255 && col[1] == 255 && col[2] == 255)
			{
				const cv::Mat one = res.digits.at("1");
				const cv::Mat roi(frame, cv::Rect(x + rect.x - 1, y + rect.y, one.cols, one.rows));

				if (images_match(roi, one))
					x_coords.push_back(x + rect.x - 2);
				else
					x_coords.push_back(x + rect.x - 1);

				break;
			}
		}
	}

	sort(x_coords.begin(), x_coords.end());

	if (x_coords.size() == 0)
		return 0;

	return x_coords.at(0);
}



uint count_table_size(HWND window, const resources & res)
{
	screenshot capture(window);

	if (!capture.ready())
		return 0;

	cv::Mat table = capture.image();
	
	return count_matches(table, res.seats);
}

cards<5> recog_table_cards(const cv::Mat & frame, const resources & res)
{
	vector<cv::Mat> now = get_image_rois(frame, central_cards_rects());
	cards<5> cards;
	
	for (int i = 0; i < 5; i++)
	{
		if (images_match(now[i], res.center[i]))
		{
			cards.at(i) = card::empty();
			continue;
		}

		bool found = false;

		for (map<string, cv::Mat>::const_iterator it = res.cards.begin();
			it != res.cards.end(); ++it)
		{
			if (images_match(it->second, now[i]))
			{
				cards.at(i) = card(it->first);
				found = true;
				break;
			}
		}

		if (!found)
			cards.at(i) = card::wrong();
	}

	return cards;
}

uint recog_dealer_position(const cv::Mat & frame, const resources & res, uint table_size)
{
	uint pos = 0;

	vector<cv::Mat> rois = get_image_rois(frame, dealer_rects(table_size));
	const cv::Mat & dealer = res.misc.at("dealer");

	for (uint i = 0; i < table_size; i++)
	{
		if (images_match(dealer, rois[i]))
		{
			pos = i + 1;
			break;
		}
	}

	return pos;
}

vector<player> recog_players(const cv::Mat & frame, const resources & res, uint table_size)
{
	vector<player> players;
	vector<cv::Mat> players_rois = get_image_rois(frame, seats_rects(table_size));

	for (uint i = 0; i < table_size; i++)
	{
		players.push_back( recog_player(frame, players_rois[i], res, table_size, i) );
	}

	cards<2> hand = recog_player_hand(frame, res, table_size);
	
	if (valid(hand))
	{
		uint idx = get_play_position(table_size);
		players[idx].set_hand(hand);
		players[idx].set_me(true);
		players[idx].set_active(true);
	}

	uint pos = recog_dealer_position(frame, res, table_size);
	assert(pos > 0);
	players[pos-1].set_dealer(true);

	return players;
}

player recog_player(const cv::Mat & frame, const cv::Mat & roi, const resources & res, uint table_size, uint player_num)
{
	player player;

	for (map<string, cv::Mat>::const_iterator it = res.actions.begin();
		it != res.actions.end(); ++it)
	{
		if (is_inside(roi, it->second))
		{
			player.set_action(it->first);
			break;
		}
	}

	player.set_active( recog_is_player_active(frame, res, table_size, player_num) );

	vector<cv::Rect> rects = digit_rects(table_size);
	
	if (player_num == 2 && table_size == 9)
		player.set_stack( recog_digits(frame, rects[player_num], res) );
	else
		player.set_stack(0);

	return player;
}

double recog_pot(const cv::Mat & frame, const resources & res)
{
	double pot = 0;
	
	return pot;
}

cards<2> recog_player_hand(const cv::Mat & frame, const resources & res, uint table_size)
{
	cards<2> hand = make_hand();
	vector<card> hand_buf;

	vector<cv::Mat> rois = get_image_rois(frame, my_hand_cards_rects(table_size));

	for (vector<cv::Mat>::const_iterator it = rois.begin(); it != rois.end(); ++it)
	{
		const cv::Mat & cur = *it;

		for (map<string, cv::Mat>::const_iterator cit = res.cards.begin();
			cit != res.cards.end(); ++cit)
		{
			if (images_match(cit->second, cur))
			{
				hand_buf.push_back( card(cit->first) );
				break;
			}
		}
	}

	if (hand_buf.size() == 2)
	{
		hand[first] = hand_buf[first];
		hand[second] = hand_buf[second];
	}

	return hand;
}

double recog_digits(const cv::Mat & frame, const cv::Rect & rect, const resources & res)
{
	double digits = 0;

	uint x = find_first_digit_x(frame, rect, res);

	string digits_str;
	uint counter = 0;

	while (x < x + rect.width)
	{
		bool dead_end = false;
		uint i = 0;

		for (map<string, cv::Mat>::const_iterator it =
			res.digits.cbegin(); it != res.digits.cend(); ++it)
		{
			const cv::Mat & digit = it->second;
			const cv::Mat roi(frame, cv::Rect(x, rect.y, digit.cols, digit.rows));
			i++;

			if (dead_end)
			{
				x--;
				counter++;
			}

			if (counter > 6)
				goto exit;

			if (i + 1 == res.digits.size())
				dead_end = true;

			if (images_match(digit, roi))
			{
				digits_str += it->first;
				x += digit.cols;
				dead_end = false;
				break;
			}
		}
	}

	exit:
	try
	{
		digits = boost::lexical_cast<double>(digits_str);
		digits *= 1000;
	}
	catch (const boost::bad_lexical_cast & e) {}

	return digits;
}

uint get_play_position(uint table_size)
{
	if (table_size == 6)
		return 2;
	else if (table_size == 9)
		return 4;
	return 0;
}

bool recog_is_player_active(const cv::Mat & frame, const resources & res, uint table_size, uint player_index)
{
	vector<cv::Mat> rois = get_image_rois(frame, card_backs_rects(table_size));

	return images_match(rois[player_index], res.misc.at("back"));
}

bool recog_is_banner_opened(const cv::Mat & frame, const resources & res, uint table_size)
{
	cv::Rect rect;

	if (table_size == 6)
	{
		rect = cv::Rect(266, 97, 13, 13);
	}
	else if (table_size == 9)
	{
		rect = cv::Rect(266, 97, 13, 13);
	}

	cv::Mat roi(frame, rect);

	return images_match(roi, res.misc.at("banner_cross"));
}



vector<cv::Rect> seats_rects(uint table_size)
{
	vector<cv::Rect> rects;
	const int w = 150;
	const int h = 60;

	if (table_size == 6)
	{
		rects.push_back(cv::Rect(492, 80, w, h));
		rects.push_back(cv::Rect(644, 240, w, h));
		rects.push_back(cv::Rect(466, 437, w, h));
		rects.push_back(cv::Rect(189, 436, w, h));
		rects.push_back(cv::Rect(14, 241, w, h));
		rects.push_back(cv::Rect(164, 80, w, h));
	}
	else if (table_size == 9)
	{
		rects.push_back(cv::Rect(492, 80, w, h));
		rects.push_back(cv::Rect(644, 152, w, h));
		rects.push_back(cv::Rect(644, 292, w, h));
		rects.push_back(cv::Rect(540, 415, w, h));
		rects.push_back(cv::Rect(329, 431, w, h));
		rects.push_back(cv::Rect(111, 416, w, h));
		rects.push_back(cv::Rect(13, 291, w, h));
		rects.push_back(cv::Rect(13, 152, w, h));
		rects.push_back(cv::Rect(164, 80, w, h));
	}

	return rects;
}

vector<cv::Rect> central_cards_rects()
{
	vector<cv::Rect> rects;
	const int w = 15;
	const int h = 29;

	rects.push_back( cv::Rect(274, 239, w, h) );
	rects.push_back( cv::Rect(327, 239, w, h) );
	rects.push_back( cv::Rect(380, 239, w, h) );
	rects.push_back( cv::Rect(433, 239, w, h) );
	rects.push_back( cv::Rect(486, 239, w, h) );

	return rects;
}

vector<cv::Rect> my_hand_cards_rects(uint table_size)
{
	vector<cv::Rect> rects;
	const int w = 15;
	const int h = 29;

	if (table_size == 6)
	{
		rects.push_back( cv::Rect(491, 377, w, h) );
		rects.push_back( cv::Rect(540, 377, w, h) );
	}
	else if (table_size == 9)
	{
		rects.push_back( cv::Rect(356, 370, w, h) );
		rects.push_back( cv::Rect(405, 370, w, h) );
	}
	
	return rects;
}

vector<cv::Rect> card_backs_rects(uint table_size)
{
	vector<cv::Rect> rects;
	const int w = 6;
	const int h = 6;

	if (table_size == 6)
	{
		rects.push_back( cv::Rect(553, 47, w, h) );
		rects.push_back( cv::Rect(706, 211, w, h) );
		rects.push_back( cv::Rect(527, 406, w, h) );
		rects.push_back( cv::Rect(251, 406, w, h) );
		rects.push_back( cv::Rect(74, 211, w, h) );
		rects.push_back( cv::Rect(224, 47, w, h) );
	}
	else if (table_size == 9)
	{
		rects.push_back( cv::Rect(554, 48, w, h) );
		rects.push_back( cv::Rect(707, 122, w, h) );
		rects.push_back( cv::Rect(707, 261, w, h) );
		rects.push_back( cv::Rect(603, 385, w, h) ); 
		rects.push_back( cv::Rect(391, 399, w, h) );
		rects.push_back( cv::Rect(173, 385, w, h) );
		rects.push_back( cv::Rect(75, 261, w, h) );
		rects.push_back( cv::Rect(75, 122, w, h) );
		rects.push_back( cv::Rect(225, 48, w, h) );
	}
	
	return rects;
}

vector<cv::Rect> dealer_rects(uint table_size)
{
	vector<cv::Rect> rects;
	const int w = 11;
	const int h = 9;

	if (table_size == 6)
	{
		rects.push_back( cv::Rect(612, 165, w, h) );
		rects.push_back( cv::Rect(621, 319, w, h) );
		rects.push_back( cv::Rect(463, 372, w, h) );
		rects.push_back( cv::Rect(188, 379, w, h) );
		rects.push_back( cv::Rect(163, 217, w, h) );
		rects.push_back( cv::Rect(313, 163, w, h) );
	}
	else if (table_size == 9)
	{
		rects.push_back( cv::Rect(612, 163, w, h) );
		rects.push_back( cv::Rect(669, 223, w, h) );
		rects.push_back( cv::Rect(629, 342, w, h) );
		rects.push_back( cv::Rect(521, 404, w, h) ); 
		rects.push_back( cv::Rect(326, 403, w, h) );
		rects.push_back( cv::Rect(246, 395, w, h) );
		rects.push_back( cv::Rect(148, 274, w, h) );
		rects.push_back( cv::Rect(177, 182, w, h) );
		rects.push_back( cv::Rect(310, 164, w, h) );
	}
	
	return rects;
}

vector<cv::Rect> digit_rects(uint table_size)
{
	vector<cv::Rect> rects;
	const int w = 60;
	const int h = 11;

	if (table_size == 6)
	{
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
	}
	else if (table_size == 9)
	{
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(680, 319, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
		rects.push_back( cv::Rect(0, 0, w, h) );
	}

	return rects;
}

void save_open_cards_rois(const cv::Mat & table)
{
	vector<cv::Mat> rois = get_image_rois(table, central_cards_rects());

	int i = 1;
	for (auto & roi : rois)
	{
		string name = "center-" + boost::lexical_cast<string>(i) + ".bmp";
		cv::imwrite(name, roi);

		i++;
	}

	done = true;
}