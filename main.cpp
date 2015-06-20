
#include "main.h"
#include "utils.h"
#include "poker.h"
#include "recognition.h"

boost::mutex mutex;
uint threads_count = 0;
bool done = false;

namespace
{
	boost::thread_group threads;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, 
	LPSTR cmd_line, int cmd_show)
{
	init(instance, prev_instance, cmd_line, cmd_show);

	vector<HWND> target_windows = find_target_windows();

	for (vector<HWND>::const_iterator it = target_windows.begin();
		it != target_windows.end(); ++it)
	{
		threads.create_thread( boost::bind(thread_loop, *it) );
		threads_count++;
	}
	
	MSG msg;

	while (GetMessage(&msg, NULL, NULL, NULL))
	{
		DispatchMessage(&msg);
	}

	threads.interrupt_all();
	threads.join_all();

	return EXIT_SUCCESS;
}

HWND init_window(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = "pokerclient";
	wc.hInstance = instance;
	wc.lpfnWndProc = wnd_proc;
	RegisterClass(&wc);
	return CreateWindowEx(0, "pokerclient", "pokerclient", NULL, 0, 0, 0, 0, NULL, NULL, instance, 0);
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, UINT w_param, LONG l_param)
{
	switch (message)
	{
		case WM_TIMER:
		{
			if (w_param == TIMER_ID && threads_count == 0)
			{
				KillTimer(hwnd, TIMER_ID);
				DestroyWindow(hwnd);
			}
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY:
		case WM_ENDSESSION:
		{
			PostQuitMessage(0);
			break;
		}
	}
	return DefWindowProc(hwnd, message, w_param, l_param);
}

void init(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
	HWND handle = init_window(instance, prev_instance, cmd_line, cmd_show);

	SetTimer(handle, TIMER_ID, 100, NULL);
}

void handle_thread_exit()
{
	if (threads_count)
		threads_count--;
}

void thread_loop(HWND handle)
{
	try
	{
		boost::this_thread::at_thread_exit( boost::bind(handle_thread_exit) );

		resources res;
		cv::Mat frame;

		const uint table_size = count_table_size(handle, res);

		if (!table_size)
			return;

		assert(table_size == 6 || table_size == 9);

		state state;
		boost::shared_ptr<game> game_ptr = boost::make_shared<game>();

		table table;

		while (true)
		{
			if (!IsWindow(handle))
				return;

			if (frame_changed(handle, frame))
				handle_frame(handle, table_size, res, state, game_ptr, table);

			boost::this_thread::interruption_point();
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		}
	}
	catch (boost::thread_interrupted) { return; }
	catch (cv::Exception ex) { MessageBox(NULL, ex.what(), "Error", NULL); return; }
	catch (bad_alloc ex) { MessageBox(NULL, ex.what(), "Error", NULL); return; }
	catch (exception ex) { MessageBox(NULL, ex.what(), "Error", NULL); return; }
	catch (...) { return; }
}

bool frame_changed(HWND handle, cv::Mat & frame)
{
	screenshot capture(handle);

	if (!capture.ready())
		return false;

	cv::Mat new_frame = capture.grayscale();

	if (new_frame.empty() || !frame_valid(new_frame))
		return false;

	if (!frame.empty())
	{
		if (!images_match_memory(new_frame, frame))
		{
			new_frame.copyTo(frame);
			new_frame.release();
			return true;
		}
	}
	else
	{
		new_frame.copyTo(frame);
		new_frame.release();
	}

	return false;
}

void handle_frame(HWND handle, uint table_size, const resources & res, 
	state & state, boost::shared_ptr<game> & game, table & old_table)
{
	screenshot capture(handle);

	if (!capture.ready())
		return;

	cv::Mat frame = capture.image();

	table table(handle, frame, res, table_size, state.get_street());

	/*if (table.banner_opened())
	{
		play_sound("beep");
		return;
	}*/

	if (table.need_calculate(state))
	{
		state.update(table);
	}

	if (!old_table.initialized())
	{
		game->init(table);
	}
	else if (old_table != table)
	{
		game->process(table, state);

		old_table = table;
	}

	table.display_hud(handle, res, table_size, state.get_equity());
}