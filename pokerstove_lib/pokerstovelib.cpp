
#include "pokerstovelib.h"

namespace pokerstove
{
	double pokerstovelib::calc_equity(const string & hand, const string & shared_cards, int players, int cycles)
	{
		CardSet my_hand(hand);
		CardSet shared;
		vector<CardSet> hands;
		vector<PokerHandEvaluation> evals;
		vector<EquityResult> equity;
		HoldemHandEvaluator device;

		for (int i = 0; i < players; i++)
			equity.push_back( EquityResult() );

		SimpleDeck deck;
		deck.shuffle();

		const CardSet shared_const = CardSet(shared_cards);
		int shared_count = 0;

		if (!shared_cards.empty())
		{
			shared.insert(shared_const);
			shared_count = shared.size();
		}
	
		const int need = (players * 2) + 5;

		for (int i = 0; i < cycles; i++)
		{
			hands.clear();
			evals.clear();
			shared.clear();

			if (deck.size() < need)
				deck.shuffle();
		
			hands.push_back(my_hand);
			deck.remove(my_hand);

			if (shared_count > 0)
			{
				shared.insert(shared_const);
				deck.remove(shared);
				shared.insert(deck.deal(5 - shared_count));
			}
			else
			{
				shared = deck.deal(5);
			}

			for (int i = 0; i < players - 1; i++)
				hands.push_back(deck.deal(2));

			for (size_t i = 0; i < players; i++)
				evals.push_back( device.evaluate(hands[i], shared) );

			device.evaluateShowdown(hands, shared, evals, equity);
		}

		return static_cast<double>(equity[0].winShares + equity[0].tieShares) / cycles * 100;
	}
}