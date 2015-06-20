
#pragma once

// win32
#include <windows.h>
#include <wingdi.h>
#include "commctrl.h"

// c
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// std
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <fstream>
#include <exception>
#include <sstream>

//boost
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/bind.hpp>
#include <boost/crc.hpp>
#include <boost/tokenizer.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/range/algorithm/random_shuffle.hpp>
#include <boost/algorithm/string.hpp>

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace boost::filesystem;

typedef unsigned int uint;