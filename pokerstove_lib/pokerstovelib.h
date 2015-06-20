
#include <iostream>
#include <vector>
#include <string>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/chrono.hpp>
#include <boost/format.hpp>

#include "PokerHandEvaluator.h"
#include "HoldemHandEvaluator.h"
#include "SimpleDeck.hpp"

using namespace std;
namespace po = boost::program_options;

namespace pokerstove
{
	class pokerstovelib
	{
	public:

		static double calc_equity(const string & hand, const string & shared_cards, int players, int cycles);
	};
}