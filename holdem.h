
#include <stdio.h>
//#include <console.h>
#include <ctype.h>
#include <string.h>

#define match(x,procedure_name) case x: procedure_name(); break;
	
//#define floop1(loop_var) for(loop_var=ace;loop_var<=king;loop_var++)
	
#define for_each_card_in_the_deck\
	for(this_card=1; this_card<=52; this_card++)
	
#define for_each_card_on_the_board\
	for(this_card=0; this_card<=4; this_card++)
	
#define for_each_card_name_ascending floop1(the_name)
	
#define for_each_card_name_descending\
	for(the_name=king; the_name>=ace; the_name--)
	
#define for_each_card_in_the_dealt_hand\
	for(this_card=0; this_card<=1; this_card++)
	
#define for_every_combination_of_two_card_names \
	floop1(the_name1) floop1(the_name2)

#define the_player_was_dealt_a_hand (value[player]!=folded)
#define is_an_ace == ace
#define is_a_deuce == deuce
#define is_a_king == king
#define is_not_an_ace != ace
#define select switch

#define both_of_this_players_cards_play \
	((cardplays[player][0]) && (cardplays[player][1])
	
#define both_of_the_winners_cards_play \
	((cardplays[player][0]) && (cardplays[player][1]))
	
#define winners_first_card winner->cards[0]
#define winners_other_card winner->cards[1]

//----------------------------------------------------------------------------

typedef int card_name;
typedef int hand_value;

typedef enum { null, ace, deuce, three, four, five, six, seven, eight,
	nine, ten, jack, queen, king, rocket, joker } names;
	 	
typedef enum { club, diamond, heart, spade, blank } suits;
typedef enum { unknown, folded, nopair, onepair, twopair,
	trips, straight, flush, fullhouse, fourofakind, straightflush }
	hand_values;

typedef enum { draw, flop, turn, river } rounds;
typedef enum { logical1, logical2, rock, tight, aggressive,
		 kamikaze, calling, ontilt } strategies;


class card
{
public:
	int suit;
	int name;
	card () { };
	card (card_name a) { name = a; }
};

class deck
{
public:
	card pack[53];
	deck ();
	void cut (int depth);
	void interlace ();
	void shuffle (int times);
	void show_the_deck1();
	void show_the_deck2();
	void deal_to_the_board ();
	void deal_to_the_players (int players);
};


class player;

class hand
{
public:
	player *owner;
	card cards[2];
	bool card_plays[2], tied;
	int  name_count [13];
	int fsuit, suit_count [4];
	int strength;
	int found;
	card best[5];

	void use_to_make_the_best_hand(card what_card)
	{
		best [found] = what_card;
		found++;
	}
	card &operator [] (int a)
	{
		return cards[a];
	}
	void pack_no_pair ();
	void pack_groups ();
	void use_to_make_a_flush (card the_card, int found);
	void pack_flush ();
	void pack_four_of_a_kind ();
	double return_strength ();
	void show_a_hand ();
};


class player
{
public:

	char name[32];
	int seat_number;		 
	strategies playstyle;
	long cash;
	int table, seat, position;
	hand my_hand;
};


class dealer: public player
{
public:
	void deal_to_the_players (player *players);
	void evaluate_the_strength_of_each_hand();

};

class table
{
	deck the_deck;
	card board [5];
	player players [10];

	long pot, drop;
	int big_blind;
};
	
		
class statistics
{
public:

    int first, second;
    char pairs;
    bool trips,
    straight,
    flush,
    fullhouse,
    fourofakind,
    straightflush;
};
   
class ranking
{
public:
    bool suited;
    card_name name1, name2;
    double strength;
};	
