
#pragma once

#include "stdafx.h"
#include <intrin.h>
#include <stdint.h>

struct resources;

const int cylces = 10000;
const size_t first = 0;
const size_t second = 1;
const size_t any = 0;

enum suit_t
{
	CLUBS, 
	DIAMONDS,
	HEARTS, 
	SPADES,
	NO_SUIT,
	WRONG_SUIT
};

enum rank_t
{
	TWO, 
	THREE, 
	FOUR, 
	FIVE, 
	SIX, 
	SEVEN, 
	EIGHT, 
	NINE, 
	TEN, 
	JACK, 
	QUEEN, 
	KING, 
	ACE,
	NO_RANK,
	MIN_RANK = TWO,
	MAX_RANK = ACE,
	WRONG_RANK = -1
};

enum hand_t
{
	HIGH_CARD,
	ONE_PAIR,
	TWO_PAIR,
	THREE_OF_KIND,
	STRAIGHT,
	FLUSH,
	FULL_HOUSE,
	FOUR_OF_KIND,
	STRAIGHT_FLUSH,
	ROYAL_FLUSH,
	MIN_HAND = HIGH_CARD,
	MAX_HAND = ROYAL_FLUSH,
	NO_HAND	= -1
};

enum action_t
{
	CHECK,
	FOLD,
	CALL,
	RAISE,
	BET,
	SMALL_BLIND,
	BIG_BLIND,
	POSTS,
	WINNER,
	JOIN_TABLE,
	SITTING_OUT,
	NO_ACTION
};

enum result_t
{
	GAME_WON,
	GAME_LOST,
	GAME_TIED
};

enum street_t
{
	NO_STREET,
	PREFLOP,
	FLOP,
	TURN,
	RIVER
};

enum event_t
{
	INIT

};

class card
{
public:
	rank_t rank;
	suit_t suit;

	card();
	card(rank_t rank, suit_t suit);
	card(int rank, suit_t suit);
	explicit card(rank_t rank);
	explicit card(suit_t suit);
	explicit card(const string & codename);

	rank_t get_rank() const;
	suit_t get_suit() const;
	string get_codename() const;

	bool operator==(const card & rvalue) const;
	bool operator!=(const card & rvalue) const;
	bool operator<(const card & rvalue) const;
	bool operator>(const card & rvalue) const;
	card operator=(const card & src);
	friend ostream& operator<<(ostream & stream, const card & c);

	bool valid() const;
	bool higher(const card & another) const;
	bool lower(const card & another) const;
	bool equal_rank(const card & another) const;
	bool equal_suit(const card & another) const;

	static card empty();
	static card wrong();
};

template <size_t size>
class cards : public boost::array<card, size>
{
public:
	cards();
	void append(const cards<2> & pocket, const cards<5> & shared);
	bool has(const cards & subset) const;
	bool has(uint rank) const;
	bool has(rank_t rank) const;
	bool has(rank_t rank, suit_t suit) const;
	bool has(const card & card) const;
	bool has(uint rank, suit_t suit) const;
	uint count(rank_t rank) const;
	uint count(suit_t suit) const;
	rank_t max_rank() const;
	rank_t max_rank(suit_t suit) const;
	rank_t max_rank(rank_t min_range, int max_range, suit_t suit) const;
	rank_t min_rank() const;
	bool operator==(const cards<2> & another) const;
	bool operator==(const cards<5> & another) const;
	friend ostream& operator<<(ostream & stream, const cards<5> & cards);
	friend ostream& operator<<(ostream & stream, const cards<2> & cards);
	friend ostream& operator<<(ostream & stream, const cards<23> & cards);
	cards<5> get_shared();
	cards<2> get_hand();
	void erase(const cards<2> & hand);
	void erase(const cards<5> & shared);
	void pull_from(cards<23> & deck);
	void pull_from(cards<52> & deck);
	card pull_card();
	string get_codename();
};

bool valid(const cards<2> & hand);

class table;

class state
{
	street_t street;
	double equity;
	uint dealer_pos;
	cards<2> pocket;
	uint active_players;
public:
	state();

	street_t get_street() const;
	double get_equity() const;
	uint get_dealer_pos() const;
	cards<2> get_pocket() const;
	uint get_active_players_count() const;

	void set_street(street_t street);
	void set_equity(double equity);
	void set_dealer_pos(uint pos);
	void set_pocket(const cards<2> & pocket);
	void set_active_players(uint active_players);

	void update(table & table);
};

struct player
{
	double stack;
	action_t action;
	cards<2> hand;
	bool dealer;
	bool me;
	bool active;
public:
	player();
	player(double stack, action_t action, 
		cards<2> hand, bool dealer, bool me);

	void set_stack(double stack);
	void set_action(action_t action);
	void set_action(const string & action);
	void set_hand(const cards<2> & hand);
	void set_dealer(bool is_dealer);
	void set_me(bool is_me);
	void set_active(bool active);
	
	double get_stack() const;
	action_t get_action() const;
	cards<2> get_hand() const;

	bool is_dealer() const;
	bool is_active() const;
	bool is_me() const;
	bool hand_exist() const;

	bool operator==(const player rvalue) const;
	player operator=(const player src);
};

class table
{
	uint size;
	pair<double, double> blinds;
	double pot;
	vector<player> players;
	cards<5> shared;
	street_t street;
	bool banner;
public:
	table();
	table(HWND handle, const cv::Mat & frame, const resources & res, uint table_size, street_t old);

	uint get_size() const;
	pair<double, double> get_blinds() const;
	double get_pot() const;
	vector<player> get_players() const;
	street_t get_street() const;
	uint players_count() const;
	uint get_dealer_pos() const;
	uint active_players_count() const;
	cards<5> get_cards() const;
	bool my_player_exists() const;
	player get_my_player() const;
	double calculate_equity();
	bool need_calculate(const state & state) const;
	bool initialized() const;
	bool banner_opened() const;
	bool operator!=(const table & another) const;
	bool operator==(const table & another) const;
	table operator=(const table & another);

	void display_hud(HWND window, const resources & res, 
		uint table_size, double equity) const;
private:
	void assign_street(street_t old);
	street_t recognize_street() const;
	string street_str() const;
};

class event
{
	int x;
	
	event();
};

class game
{
	vector<event> events;
	ostringstream log;
public:
	game();

	void init(const table & table);
	void process(const table & table, const state & state);
private:
	void save();
};

struct benchmark
{
	boost::chrono::high_resolution_clock::time_point start;
	bool sound_on;

	benchmark(bool sound_on = false);
	~benchmark();
};

cards<2> make_hand();