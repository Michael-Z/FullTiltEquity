
#include "poker.h"
#include "recognition.h"
#include "utils.h"

#include "pokerstovelib.h"


state::state() : street(NO_STREET), dealer_pos(0), pocket(make_hand()), active_players(0) {}

street_t state::get_street() const { return street; }

double state::get_equity() const { return equity; }

uint state::get_dealer_pos() const { return dealer_pos; }

cards<2> state::get_pocket() const { return pocket; }

uint state::get_active_players_count() const { return active_players; }

void state::set_street(street_t street) { this->street = street; }

void state::set_equity(double equity) { this->equity = equity; }

void state::set_dealer_pos(uint pos) { dealer_pos = pos; }

void state::set_pocket(const cards<2> & pocket) { this->pocket = pocket; }

void state::set_active_players(uint active_players) { this->active_players = active_players; }

void state::update(table & table)
{
	street = table.get_street();
	dealer_pos = table.get_dealer_pos();
	active_players = table.active_players_count();

	if (table.my_player_exists())
	{
		pocket = table.get_my_player().get_hand();
		equity = table.calculate_equity();
	}
	else
	{
		pocket = make_hand();
		equity = 0.f;
	}
}



event::event() {};

game::game()
{
	log.str(string());
}

void game::init(const table & table)
{
	save();


}

void game::process(const table & table, const state & state)
{

}

void game::save()
{
	if (!log.str().empty())
	{
		ofstream out("games.txt", ios::app);
		//out << log << endl;
		log.str(string());
	}
}



benchmark::benchmark(bool sound_on) : sound_on(sound_on)
{
	if (sound_on)
		play_sound("beep");

	start = boost::chrono::high_resolution_clock::now();
}

benchmark::~benchmark()
{
	boost::chrono::milliseconds delta = 
		boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::high_resolution_clock::now() - start);
	
	ofstream out("benchmark.txt", ios::app);
	out << "Cycle in: " << delta.count() << " millisec." << endl;
	out.close();

	if (sound_on)
		play_sound("beep_end");
}



card::card() : rank(NO_RANK), 
	suit(NO_SUIT) {}

card::card(rank_t rank, suit_t suit) :
	rank(rank), suit(suit) {}

card::card(int rank, suit_t suit) : rank((rank_t)rank), suit(suit) {}

card::card(rank_t rank) :
	rank(rank), suit(NO_SUIT) {}

card::card(suit_t suit) :
	rank(NO_RANK), suit(suit) {}

card::card(const string & codename)
{
	if (codename.length() != 2)
	{
		this->rank = NO_RANK;
		this->suit = NO_SUIT;

		return;
	}

	const char rank = codename[0];
	const char suit = codename[1];

	switch (rank)
	{
		case '2': this->rank = TWO;		break;
		case '3': this->rank = THREE;	break;
		case '4': this->rank = FOUR;	break;
		case '5': this->rank = FIVE;	break;
		case '6': this->rank = SIX;		break;
		case '7': this->rank = SEVEN;	break;
		case '8': this->rank = EIGHT;	break;
		case '9': this->rank = NINE;	break;
		case 'T': this->rank = TEN;		break;
		case 'J': this->rank = JACK;	break;
		case 'Q': this->rank = QUEEN;	break;
		case 'K': this->rank = KING;	break;
		case 'A': this->rank = ACE;		break;
		default:  this->rank = NO_RANK;	break;
	}

	switch (suit)
	{
		case 'c': this->suit = CLUBS;	break;
		case 'd': this->suit = DIAMONDS;break;
		case 'h': this->suit = HEARTS;	break;
		case 's': this->suit = SPADES;	break;
		default:  this->suit = NO_SUIT; break;
	}
}

rank_t card::get_rank() const { return rank; }

suit_t card::get_suit() const { return suit; }

string card::get_codename() const
{
	string codename;
	char rank, suit;

	switch (this->rank)
	{
		case TWO:	rank = '2';	break;
		case THREE: rank = '3';	break;
		case FOUR:	rank = '4';	break;
		case FIVE:	rank = '5';	break;
		case SIX:	rank = '6';	break;
		case SEVEN: rank = '7';	break;
		case EIGHT: rank = '8';	break;
		case NINE:	rank = '9';	break;
		case TEN:	rank = 'T';	break;
		case JACK:	rank = 'J';	break;
		case QUEEN: rank = 'Q';	break;
		case KING:	rank = 'K';	break;
		case ACE:	rank = 'A';	break;
		default:	rank = ' '; break;
	}

	switch (this->suit)
	{
		case CLUBS:		suit = 'c'; break;
		case DIAMONDS:	suit = 'd'; break;
		case HEARTS:	suit = 'h'; break;
		case SPADES:	suit = 's'; break;
		default:		suit = ' '; break;
	}

	codename.push_back(rank);
	codename.push_back(suit);

	return codename;
}

bool card::operator==(const card & rvalue) const
{
	return (this->rank == rvalue.rank &&
		this->suit == rvalue.suit);
}

bool card::operator!=(const card & rvalue) const
{
	return (this->rank != rvalue.rank ||
		this->suit != rvalue.suit);
}

bool card::operator<(const card & rvalue) const
{
	if (this->rank != rvalue.rank)
		return this->rank < rvalue.rank;
	else
		return this->suit < rvalue.suit;
}

bool card::operator>(const card & rvalue) const
{
	if (this->rank != rvalue.rank)
		return this->rank > rvalue.rank;
	else
		return this->suit > rvalue.suit;
}

card card::operator=(const card & src)
{
	this->rank = src.rank;
	this->suit = src.suit;
	return *this;
}

ostream& operator<<(ostream & stream, const card & c)
{
	switch (c.rank)
	{
		case TWO: { stream << "[2"; break; }
		case THREE: { stream << "[3"; break; }
		case FOUR: { stream << "[4"; break; }
		case FIVE: { stream << "[5"; break; }
		case SIX: { stream << "[6"; break; }
		case SEVEN: { stream << "[7"; break; }
		case EIGHT: { stream << "[8"; break; }
		case NINE: { stream << "[9"; break; }
		case TEN: { stream << "[10"; break; }
		case JACK: { stream << "[J"; break; }
		case QUEEN: { stream << "[Q"; break; }
		case KING: { stream << "[K"; break; }
		case ACE: { stream << "[A"; break; }
	}

	switch (c.suit)
	{
		case SPADES: { stream << "s] "; break; }
		case CLUBS: { stream << "c] "; break; }
		case HEARTS: { stream << "h] "; break; }
		case DIAMONDS: { stream << "d] "; break; }
	}

	return stream;
}

bool card::valid() const
{
	return this->rank != NO_RANK
		&& this->suit != NO_SUIT;
}

bool card::higher(const card & another) const
{
	assert(this->valid() && another.valid());

	return this->rank > another.rank;
}

bool card::lower(const card & another) const
{
	assert(this->valid() && another.valid());

	return this->rank < another.rank;
}

bool card::equal_rank(const card & another) const
{
	assert(this->valid() && another.valid());

	return this->rank == another.rank;
}

bool card::equal_suit(const card & another) const
{
	assert(this->valid() && another.valid());

	return this->suit == another.suit;
}

card card::empty() { return card(); }

card card::wrong() { return card(WRONG_RANK, WRONG_SUIT); }



template <size_t size>
cards<size>::cards()
{
	for (uint i = 0; i < size(); i++)
		this->at(i) = card::empty();
}

template <size_t size>
void cards<size>::append(const cards<2> & pocket, const cards<5> & shared)
{
	for (uint i = 0; i < 2; i++)
		this->at(i) = pocket.at(i);

	for (uint i = 0; i < 5; i++)
		this->at(i + 2) = shared.at(i);
}

template <size_t size>
bool cards<size>::has(const cards & subset) const
{
	for (uint i = 0; i < subset.size(); i++)
	{
		if (find(this->begin(), this->end(), subset[i]) == this->end())
			return false;
	}

	return true;
}

template <size_t size>
bool cards<size>::has(uint rank) const
{
	return this->count( (rank_t)rank ) > 0;
}

template <size_t size>
bool cards<size>::has(rank_t rank) const
{
	return this->count(rank) > 0;
}

template <size_t size>
bool cards<size>::has(rank_t rank, suit_t suit) const
{
	return find(this->begin(), this->end(), card(rank, suit)) != this->end();
}

template <size_t size>
bool cards<size>::has(const card & card) const
{
	return find(this->begin(), this->end(), card) != this->end();
}

template <size_t size>
bool cards<size>::has(uint rank, suit_t suit) const
{
	return find(this->begin(), this->end(), card((rank_t) rank, suit)) != this->end();
}

template <size_t size>
uint cards<size>::count(rank_t rank) const
{
	uint count = 0;

	for (uint i = 0; i < size(); i++)			
	{
		if (this->at(i).rank == rank)
			count++;
	}

	return count;
}

template <size_t size>
uint cards<size>::count(suit_t suit) const
{
	uint count = 0;

	for (uint i = 0; i < size(); i++)			
	{
		if (this->at(i).suit == suit)
			count++;
	}

	return count;
}

template <size_t size>
rank_t cards<size>::max_rank() const
{
	rank_t rank = MIN_RANK;

	for (uint i = 0; i < size(); i++)			
	{
		const rank_t current = this->at(i).rank;

		if (current > rank)
			rank = current;
	}

	return rank;
}

template <size_t size>
rank_t cards<size>::max_rank(suit_t suit) const
{
	rank_t rank = MIN_RANK;

	for (uint i = 0; i < size(); i++)			
	{
		const suit_t s = this->at(i).suit;

		if (s != suit)
			continue;

		const rank_t current = this->at(i).rank;

		if (current > rank)
			rank = current;
	}

	return rank;
}

template <size_t size>
rank_t cards<size>::max_rank(rank_t min_range, int max_range, suit_t suit) const
{
	rank_t rank = MIN_RANK;

	for (uint i = 0; i < size(); i++)			
	{
		const suit_t s = this->at(i).suit;

		if (s != suit)
			continue;

		const rank_t current = this->at(i).rank;

		if (current < min_range || current > max_range)
			continue;

		if (current > rank)
			rank = current;
	}

	return rank;
}

template <size_t size>
rank_t cards<size>::min_rank() const
{
	rank_t rank = MAX_RANK;

	for (uint i = 0; i < size(); i++)
	{
		const rank_t current = this->at(i).rank;

		if (current < rank)
			rank = current;
	}

	return rank;
}

template <size_t size>
bool cards<size>::operator==(const cards<2> & another) const
{
	return this->at(first) == another.at(first)
		&& this->at(second) == another.at(second);
}

template <size_t size>
bool cards<size>::operator==(const cards<5> & another) const
{
	for (uint i = 0; i < size(); i++)
	{
		if (find(another.begin(), another.end(), this->at(i)) == another.end())
			return false;
	}

	return true;
}

ostream& operator<<(ostream & stream, const cards<5> & cards)
{
	for (int i = 0; i < 5; i++)
		stream << cards[i];

	stream << endl;
	return stream;
}

ostream& operator<<(ostream & stream, const cards<2> & cards)
{
	for (int i = 0; i < 2; i++)
		stream << cards[i];

	stream << endl;
	return stream;
}

ostream& operator<<(ostream & stream, const cards<23> & cards)
{
	for (int i = 0; i < 23; i++)
	{
		if (cards[i].valid())
			stream << cards[i];
	}

	stream << endl;
	return stream;
}

template <size_t size>
cards<5> cards<size>::get_shared()
{
	cards<5> shared;
	uint idx = 0;

	for (uint i = 0; i < size() && idx < 5; i++)
	{
		card & cur = this->at(i);

		if ( cur.valid() )
		{
			shared.at(idx) = cur;
			idx++;
			cur = card::empty();
		}
	}

	return shared;
}

template <size_t size>
cards<2> cards<size>::get_hand()
{
	cards<2> hand;
	uint idx = 0;

	for (uint i = 0; i < size() && idx < 2; i++)
	{
		card & cur = this->at(i);

		if ( cur.valid() )
		{
			hand.at(idx) = cur;
			idx++;
			cur = card::empty();
		}
	}

	return hand;
}

template <size_t size>
void cards<size>::erase(const cards<2> & hand)
{
	for (uint i = 0; i < size(); i++)
	{
		card & cur = this->at(i);

		if (cur == hand.at(first) ||
			cur == hand.at(second))
		{
			cur = card::empty();
		}
	}
}

template <size_t size>
void cards<size>::erase(const cards<5> & shared)
{
	for (uint i = 0; i < size(); i++)
	{
		card & cur = this->at(i);

		for (uint j = 0; j < 5; j++)
		{
			if (cur == shared[j])
				cur = card::empty();
		}
	}
}

template <size_t size>
void cards<size>::pull_from(cards<23> & deck)
{
	for (uint i = 0; i < size(); i++)
	{
		card & cur = this->at(i);

		if (cur == card::empty())
			cur = deck.pull_card();
	}
}

template <size_t size>
void cards<size>::pull_from(cards<52> & deck)
{
	for (uint i = 0; i < size(); i++)
	{
		card & cur = this->at(i);

		if (cur == card::empty())
			cur = deck.pull_card();
	}
}

template <size_t size>
card cards<size>::pull_card()
{
	for (uint i = 0; i < size(); i++)
	{
		card cur = this->at(i);

		if (cur != card::empty())
		{
			this->at(i) = card::empty();
			return cur;
		}
	}

	return card::empty();
}

template <size_t size>
string cards<size>::get_codename()
{
	string codename;

	for (uint i = 0; i < size(); i++)
		codename += this->at(i).get_codename();

	boost::algorithm::trim(codename);
	return codename;
}



bool valid(const cards<2> & hand)
{
	return hand[0].valid() && hand[1].valid();
}



player::player() : stack(0), action(NO_ACTION), hand(make_hand()), 
	dealer(false), me(false), active(false) {}

player::player(double stack, action_t action, cards<2> hand, bool dealer, bool me): stack(stack),
	action(action), hand(hand), dealer(dealer), me(me), active(false) {};

void player::set_stack(double stack) { this->stack = stack; }

void player::set_action(action_t action) { this->action = action; }

void player::set_action(const string & action)
{
	if (action == "check")				this->action = CHECK;
	else if (action == "fold")			this->action = FOLD;
	else if (action == "call")			this->action = CALL;
	else if (action == "raise")			this->action = RAISE;
	else if (action == "bet")			this->action = BET;
	else if (action == "small blind")	this->action = SMALL_BLIND;
	else if (action == "big blind")		this->action = BIG_BLIND;
	else if (action == "posts")			this->action = POSTS;
	else if (action == "winner")		this->action = WINNER;
	else if (action == "join table")	this->action = JOIN_TABLE;
	else if (action == "sitting out")	this->action = SITTING_OUT;
	else this->action = NO_ACTION;
}

void player::set_hand(const cards<2> & hand) { this->hand = hand; }

void player::set_dealer(bool is_dealer) { this->dealer = is_dealer; }

void player::set_me(bool is_me) { this->me = is_me; }

void player::set_active(bool active) { this->active = active; }

double player::get_stack() const { return stack; }

action_t player::get_action() const { return action; }

cards<2> player::get_hand() const { return hand; }

bool player::is_dealer() const { return dealer; }

bool player::is_active() const { return active; }

bool player::is_me() const { return me; }

bool player::hand_exist() const
{
	return hand[first].valid()
		&& hand[second].valid();
}

bool player::operator==(const player rvalue) const
{
	return this->stack == rvalue.stack
		&& this->action == rvalue.action
		&& this->hand == rvalue.hand;
}

player player::operator=(const player src)
{
	this->stack = src.stack;
	this->action = src.action;
	this->hand = src.hand;
	return *this;
}



table::table() : size(0), blinds(make_pair(0.f, 0.f)), pot(0), players(vector<player>()), 
	shared(cards<5>()), street(NO_STREET), banner(false) {};

table::table(HWND handle, const cv::Mat & frame, const resources & res, 
	uint table_size, street_t old) : size(table_size)
{
	blinds = recog_blinds(handle);
	pot = recog_pot(frame, res);
	players = recog_players(frame, res, table_size);
	shared = recog_table_cards(frame, res);
	banner = recog_is_banner_opened(frame, res, table_size);

	assign_street(old);
}

void table::assign_street(street_t old)
{
	street_t temp = recognize_street();

	if (temp != NO_STREET)
		street = temp;
}

string table::street_str() const
{
	if (street == NO_STREET)
		return "No street";
	else if (street == PREFLOP)
		return "Preflop";
	else if (street == FLOP)
		return "Flop";
	else if (street == TURN)
		return "Turn";
	else if (street == RIVER)
		return "River";
	else
		return "Error";
}

street_t table::recognize_street() const
{
	const card empty = card::empty();

	if (shared[0] == empty &&
		shared[1] == empty &&
		shared[2] == empty &&
		shared[3] == empty &&
		shared[4] == empty)
	{
		return PREFLOP;
	}
	else if (shared[0] != empty &&
		shared[1] != empty &&
		shared[2] != empty &&
		shared[3] == empty &&
		shared[4] == empty)
	{
		return FLOP;
	}
	else if (shared[0] != empty &&
		shared[1] != empty &&
		shared[2] != empty &&
		shared[3] != empty &&
		shared[4] == empty)
	{
		return TURN;
	}
	else if (shared[0] != empty &&
		shared[1] != empty &&
		shared[2] != empty &&
		shared[3] != empty &&
		shared[4] != empty)
	{
		return RIVER;
	}
	else
	{
		return NO_STREET;
	}
}

uint table::get_size() const { return size; }

pair<double, double> table::get_blinds() const { return blinds; }

double table::get_pot() const { return pot; }

vector<player> table::get_players() const { return players; }

street_t table::get_street() const { return street; }

uint table::players_count() const { return players.size(); }

uint table::get_dealer_pos() const
{
	uint pos = 0;

	for (uint i = 0; i < players.size(); i++)
	{
		if (players[i].is_dealer())
		{
			pos = i + 1;
			break;
		}
	}

	return pos;
}

uint table::active_players_count() const
{
	uint count = 0;

	for (vector<player>::const_iterator it = players.begin();
		it != players.end(); ++it)
	{
		if (it->is_active())
			count++;
	}

	return count;
}

cards<5> table::get_cards() const { return shared; }

bool table::my_player_exists() const
{
	for (vector<player>::const_iterator it = players.begin();
		it != players.end(); ++it)
	{
		if (it->me)
			return true;
	}

	return false;
}

player table::get_my_player() const
{
	for (vector<player>::const_iterator it = players.cbegin();
		it != players.cend(); ++it)
	{
		if (it->me)
			return *it;
	}

	return player();
}

void table::display_hud(HWND window, const resources & res, uint table_size, double equity) const
{
	HDC dc = GetDC(window);
	string hud;
	// Blinds
	ostringstream ss;
	ss.setf(ios::fixed);
	ss << " Blinds: " << setprecision(2);
	ss << blinds.first << "/" << blinds.second << " ";
	hud = ss.str();
	TextOut(dc, 330, 60, hud.c_str(), hud.length());
	// Table size
	hud = " Table size: " + boost::lexical_cast<string>(table_size) + " ";
	TextOut(dc, 330, 80, hud.c_str(), hud.length());
	// Opened cards
	hud = " Cards: ";
	for (int i = 0; i < 5; i++)
	{
		if (shared.at(i).valid())
			hud += shared[i].get_codename() + " ";
	}
	TextOut(dc, 330, 100, hud.c_str(), hud.length());
	// Dealer position
	for (uint i = 0; i < players.size(); i++)
	{
		if (players[i].is_dealer())
		{
			hud = " Dealer position: " + boost::lexical_cast<string>(i + 1) + " ";
			TextOut(dc, 330, 40, hud.c_str(), hud.length());
		}
	}
	// Player's hand
	hud = " Hand: ";
	if (my_player_exists())
	{
		const player & me = get_my_player();
		hud = " Hand: " + me.hand[first].get_codename()
			+ " " + me.hand[second].get_codename() + " ";
	}
	TextOut(dc, 330, 140, hud.c_str(), hud.length());
	// Stack
	const player & me = get_my_player();
	hud = " Stack: " + boost::lexical_cast<string>(me.get_stack()) + " ";
	TextOut(dc, 330, 120, hud.c_str(), hud.length());
	// Equity
	ss.str(string());
	ss << fixed << setprecision(1);
	ss << " Equity: " << equity << " % ";
	hud = ss.str();
	TextOut(dc, 330, 160, hud.c_str(), hud.length());
	// Street
	hud = " Street: " + street_str() + " ";
	TextOut(dc, 330, 20, hud.c_str(), hud.length());
	// Active players
	hud = " Active: " + boost::lexical_cast<string>(active_players_count()) + " ";
	TextOut(dc, 330, 180, hud.c_str(), hud.length());
	// 3 player
	ss.str(string());
	if (table_size == 9)
	{
		ss << setprecision(1) << " 3 player stack: " << boost::lexical_cast<string>(players[2].get_stack()) << " ";
		hud = ss.str();
		TextOut(dc, 330, 180, hud.c_str(), hud.length());
	}
}



double table::calculate_equity()
{
	using namespace pokerstove;

	cards<2> pocket = get_my_player().get_hand();

	double equity = pokerstovelib::calc_equity(pocket.get_codename(), 
		shared.get_codename(), active_players_count(), cylces);
	//double equity = 0;
	return equity;
}

bool table::need_calculate(const state & state) const
{
	return this->street != state.get_street()
		|| this->get_dealer_pos() != state.get_dealer_pos()
		|| this->get_my_player().get_hand() != state.get_pocket()
		|| this->active_players_count() != state.get_active_players_count();
}

bool table::initialized() const
{
	return size == 0 && pot == 0 && street == NO_STREET && shared == cards<5>()
		&& players == vector<player>() && blinds.first == 0 && blinds.second == 0;
}

bool table::banner_opened() const
{
	return banner;
}

bool table::operator!=(const table & another) const
{
	return !(*this == another);
}

bool table::operator==(const table & another) const
{
	return this->size == another.get_size()
		&& this->pot == another.get_pot()
		&& this->street == another.get_street()
		&& this->shared == another.get_cards()
		&& this->players == another.get_players()
		&& this->blinds == another.get_blinds();
}

table table::operator=(const table & another)
{
	this->size = another.get_size();
	this->pot = another.get_pot();
	this->street = another.get_street();
	this->shared = another.get_cards();
	this->players = another.get_players();
	this->blinds = another.get_blinds();
	return *this;
}

cards<2> make_hand()
{
	cards<2> hand;
	hand[0] = card(NO_RANK, NO_SUIT);
	hand[1] = card(NO_RANK, NO_SUIT);
	return hand;
}