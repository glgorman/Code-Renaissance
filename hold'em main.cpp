

#include "holdem.h"

/////////////////////////////////////////////////////////////
//
// Analyis Software for the game of Texas Hold'em
//
// TERMS OF USE:
// 
// Copyright 1995-2023 Gerold Lee Gorman
// Made Available under the GNU/MIT license
// Free to use for any open sourc application according
// to GPL, or LGPL if any portion is included as part 
// of a compilier package, or other open source work 
// commonly refered to as an SDK, and where that other
// work is a substantial work likely to attract a wide
// interest with respect ot similar subject matter.
// 
// Also available under the ususal MIT
// license, if used in a closed source, redistributed
// commerical work.
//
// Other Restrictions: NO REDISTRIBUTION IN ANY PART
// OR IN WHOLE, in either source or binary form as a
// part of any commerical printed work, i.e., such as,
// but not limited to any published work on Poker Strategy,
// "Casino Secrets, etc", i.e.,  in hard copy form.
// Any other use should provide an approriate attribution,
// according the the medium in which this work or a
// deriavatie work based upon this work is used such as:
//
// "Lenes by Panavision; Color By Deluxe; Catering by XYZ Deli;
// Poker strategy tables compiled with Mister G's Poker Software,
// Copyright Gerold Lee Gorman, used by Permission"
// 
// Use at your own risk.  Not responsible for any damages, losses,
// whether pecuniary, incidental, or other, as a consequence
// of the use of this software, or any data produced by this 
// software or any other work derived from. 
// 
// YOU HAVE BEEN WARNED!
//
/////////////////////////////////////////////////////////////

deck the_deck;
// ---------------------------------------------------------------------------

int
  min = 3,
  option = 3;
  
char response;
int blind, dealt, number_of_players, winner, pot;
long badbeats, games, highhands, oldTickTime, quota;

struct {
	bool activateDA,betting,goaway,
	debug,highhand,hitit,params,quit,reset,
	resume,run,status,splitpot,PrefExists,
	PlayExists,NumExists; } flag;
   
struct { 
	bool all, each, high, prob, bad; } wr;
	
char menuText[80];
deck new_deck, old_deck;
card the_board[5];

int ordering[13][13];
long appeared[13][13], won[13][13];
long strongest[straightflush+1];

strategies playstyle[11];

statistics stats;
ranking rankings [171];

// ------------------------------------------------------------------------ 
   
void startup (void);
void init_cash (void);
void init_rank_table (void);
void init_tallies (void);
void reset_strategies (void);
void play (long count);
//void syscheck (void);
void text_parameters (void);

// ------------------------------------------------------------------------ 

main()
{
	startup();
	init_cash();
	init_rank_table();
	init_tallies();
	playstyle[0] = tight;
	reset_strategies();
	blind = 0;
	quota = 100;

	number_of_players = 10;
	flag.run = true;
	wr.each = true;

	while (flag.quit == false)
	{	
		if (flag.run == true)
			play(quota);
		flag.resume = false;
	}
	return 0;
}

// ------------------------------------------------------------------------ 


int random()
// linear congruential algorithm from Schneier p 347 
{
	const a = 7141, b = 54773, m = 259200;
	static long seed;
	seed = (a * seed + b) % m;
	return seed & 0xFFFF; 	
}

bool reply (void)
{
	char response[80];
	scanf("%s",&response);
	select (response[0])
	{
		case 'y': case 'Y':
			return true;
			break;
			
   		case 'n': case 'N': 
    		return false;
    		break;
    	
    	default:
   			printf("y/n expected\n");
   			return false;
 	}
}


// ------------------------------------------------------------------------ 

deck::deck ()
{
	int this_card;
	for_each_card_in_the_deck
	{
		pack[this_card].suit = (suits) ((this_card - 1)/13);
		pack[this_card].name = (card_name) ((this_card - 1)%13 + 1);
	};
	dealt = 0;
}

void deck::cut (int depth)
{
	int this_card;
	card oldpack[53];
	
	for_each_card_in_the_deck
		oldpack[this_card] = pack[this_card];
			
	for_each_card_in_the_deck
		pack[this_card] = oldpack[(this_card + depth)%52 + 1];
}

void deck::interlace ()
{
	int this_card;
	card olddeck [53];
	
	for_each_card_in_the_deck
		olddeck[this_card] = pack[this_card];
		
	for_each_card_in_the_deck
		select (this_card%2)
		{
			case 1: pack[this_card] = olddeck[this_card/2 + 1];
				break;
				
			case 0: pack[this_card] = olddeck[this_card/2 + 26];
				break;
		}
}

void deck::shuffle (int times)
{
	int time;
//	pack olddeck;
	for (time=1; time<=times; time++)
	{
		cut(23 + random() %11);
		interlace();
	}
	cut(13 + games%11);
	dealt = 0;
}

void deck::show_the_deck1()
{
	int this_card;
	
	for_each_card_in_the_deck
	{
		select (pack[this_card].name) {
		
		case 1: printf("A");
	     	break;
	     
		case 10: case 11: case 12: case 13: 
			printf("X");
			break;
			
		default: 
	     	printf("%d",pack[this_card].name);
	     	break; }
	     	
	    if (this_card%13 == 0) printf("\n");
	}
}

void write_card_description(bool capitalize, card the_card)
{
	select (the_card.name)
	{
		case ace: printf("Ace of ");
			break;
			
		case jack: printf("Jack of ");
			break;
			
		case queen: printf("Queen of ");
			break;
			
		case king: printf("King of ");
			break;
		
		default: printf("%d of ",the_card.name);
			break;
	}
	select (the_card.suit)
	{	
		case club: printf("Clubs");
			break;
			
		case diamond: printf("Diamonds");
			break;
			
		case heart: printf("Hearts");
			break;
			
		case spade: printf("Spades");
			break; }
}

void deck::show_the_deck2()
{
	int this_card;
	printf("\n");
	for_each_card_in_the_deck
	{
		write_card_description(true,pack[this_card]);
						
		if ((this_card%13)%5 == 0)
			printf("\n");
		else
			printf(",");
			
		if (this_card%13 == 0)
			printf("\n");
	}
}

void hand::show_a_hand ()
{
	int this_card;

	printf("Player %d has the ",owner->seat_number);
	for_each_card_in_the_dealt_hand
	{
	    write_card_description(true,cards[this_card]);
	    
	    if (this_card == 0)
			printf(" and the ");
	    else
			printf(".\n");
   }
}

void show_the_board ()
{
	int this_card;
	
	printf("\n");
	printf("The board has the ");
	for_each_card_on_the_board
	{
	    write_card_description(true,the_board[this_card]);
				
	    select  (this_card) 
	    {
			case 0: case 2: 
				printf(", ");
				break;
				
			case 1: 
				printf(",\n");
				break;
				
			case 3: 
		     	 printf(" and the ");
		     	break;
		     	
			case 4: 
				printf(".\n");
				break; }
	}
	printf("\n");
}

void dealer::deal_to_the_players (player *players)
{
	int seat_number;
	int this_card;
	
	for_each_card_in_the_dealt_hand 
	for(seat_number=1; seat_number<=number_of_players; seat_number++)
	{
		dealt++;
		players [seat_number].my_hand[this_card] = the_deck.pack[dealt];
	}
}

void deck::deal_to_the_board ()
{
	int this_card;
	for_each_card_on_the_board
	{
    	dealt++;
    	the_board[this_card] = pack[dealt];
	}
}


void hand::pack_no_pair ()
{
	int this_card;
	unsigned char the_name;
	bool found_the_card;
	
	found = 0;
	found_the_card = false;
	if (name_count [ace] == 1)
	{
	    for_each_card_in_the_dealt_hand
			if (cards [this_card].name is_an_ace)
			{
				use_to_make_the_best_hand(cards[this_card]);
				found_the_card = true;
			}
		
		for_each_card_on_the_board	
		if ((found_the_card == false)&&(the_board[this_card].name is_an_ace))
	    	{
	       		use_to_make_the_best_hand(the_board[this_card]);
				found_the_card = true;
	     	}
	}
	
	for	(the_name=king; ((the_name>=deuce)&&(found!=5)); the_name--)
	{
		found_the_card = false;
		if (name_count [the_name] == 1)
    	{
     		for_each_card_in_the_dealt_hand
      			if (cards[this_card].name == the_name)
       			{
       				use_to_make_the_best_hand(cards[this_card]);
					found_the_card = true;
       			}
       			
			for_each_card_on_the_board
			if ((found_the_card == false)
				&&(the_board[the_name].name == the_name))
				{
					use_to_make_the_best_hand(the_board[this_card]);
					found_the_card = true;
	      		}
   		}
	}
}


// This next void sorts the cards in a players hand when the player
// has any pairs, sets or a fullhouse.  The suits are tracked as a matter
 // of record, although this information is not really needed in most cases


void hand::pack_groups ()
{	
	int has_this_many_to_find, found, this_card;
	unsigned char the_name;
	found = 0;
	
	if (stats.trips == true)
		has_this_many_to_find = 3;
   	else
    	has_this_many_to_find = 2;
    	
   	while ((has_this_many_to_find!=0)&&(found!=5))
   	{
    	if (name_count[ace] == has_this_many_to_find)
		{
			for_each_card_in_the_dealt_hand
				if (cards[this_card].name is_an_ace)
				use_to_make_the_best_hand(cards[this_card]);
				
			for_each_card_on_the_board
				if (the_board[this_card].name is_an_ace)
				use_to_make_the_best_hand(the_board[this_card]);
		}
					
		for_each_card_name_descending
		{
			if ((name_count[the_name] == has_this_many_to_find)
				&&(found < 5))
			{
				for_each_card_in_the_dealt_hand
					if ((cards[this_card].name == the_name)
						&&(found < 5))
					use_to_make_the_best_hand(cards[this_card]);
					
				for_each_card_on_the_board
					if ((the_board[this_card].name == the_name)&&(found<5))
					use_to_make_the_best_hand(the_board[this_card]);
			}
			if (found == 5) break;
	 	}
	 	has_this_many_to_find--;
     }    
}

void hand::use_to_make_a_flush (card the_card, int found)
{
	best [found] = the_card;
	found++;
}

void hand::pack_flush ()
{
	unsigned char this_card, found;
	unsigned char the_name;

/* This segment expropriates the variable
name_count and uses it to recount only the cards
in the hand that have the same suit as fsuit */

	for(the_name=ace;the_name<=king;the_name++)
		name_count[the_name] = 0;
	
	for_each_card_in_the_dealt_hand
		if (cards[this_card].suit == fsuit)
    		name_count[cards [this_card].name]++;
    		
	for_each_card_on_the_board if (the_board[this_card].suit == fsuit)
		name_count[the_board[this_card].name]++;
    	

/*
Now the cards that comprise the flush have been loaded into name_count.
I have copied the straight detector inline here to determine if is a
straightflush.  This works here because name_count now only carries info
regarding the cards in the suit that we already have a flush from.*/

	found = 0;
	if (name_count[ace] == 1)
		use_to_make_a_flush(ace,found);
    	
	for_each_card_name_descending
    {
		if (name_count[the_name] == 1)
			use_to_make_a_flush(the_name,found);
		else
			found = 0;
		if (found == 5)
			stats.straightflush = true; }

	if (stats.straightflush == true)
		strength  = straightflush;
  	else

//  Else it is not a straight flush and the flush
//	ordering routine should proceed 

	{	found = 0;
		if (name_count[ace] == 1)
      		use_to_make_a_flush(ace,found);
     	
   	 while (found<5)
		for (the_name=king; ((the_name>=deuce)&&(found<5)); the_name--)
			if (name_count[the_name] == 1)
				use_to_make_a_flush(the_name,found);
	}
}

// 	The next void is specific to the rating of four of a kind. 

void hand::pack_four_of_a_kind ()
{
	bool found_kicker;
	unsigned char found, the_name, this_card, fourof, kicker;

	for(this_card=ace;this_card<=king;this_card++)
		if (name_count[this_card] == 4)
			fourof = this_card;
			
	found = 0;
			
	for_each_card_in_the_dealt_hand 
		if (cards [this_card].name == fourof)
		use_to_make_the_best_hand(cards[this_card]);
				
	for_each_card_on_the_board
		if (the_board[this_card].name == fourof)
		use_to_make_the_best_hand(the_board[this_card]);

// Identify Kicker 

	if ((name_count[ace] > 0)&&(fourof != ace))
		kicker = ace;
	else
	for_each_card_name_descending
		if ((name_count[the_name] > 0)&&(fourof != the_name))
		{
     		kicker = the_name;
     		break;
     	}

// Pack kicker into best hand. 

	for_each_card_in_the_dealt_hand
		if (cards [this_card].name == kicker)
		{
			use_to_make_the_best_hand(cards[this_card]);
			found_kicker = true;
			break; }
	
	if (found_kicker == false)	
	for_each_card_on_the_board
		if (the_board[this_card].name == kicker)
		{
			use_to_make_the_best_hand(the_board[this_card]);
			found_kicker = true;
			break; }	
}

// ------------------------------------------------------------------------ 

void dealer::evaluate_the_strength_of_each_hand()
{
	unsigned char found, player, this_card;
	unsigned char the_name, this_suit;

	my_hand.strength = unknown;
	winner = 0;
	
	for(player=1; player<=number_of_players; player++) if (my_hand.strength!=folded)
	{
	     my_hand.strength = unknown;
	     stats.second = null;
	     stats.pairs = 0;
	     stats.trips = false;
	     stats.straight = false;
	     stats.flush = false;
	     stats.fullhouse = false;
	     stats.fourofakind = false;
	     stats.straightflush = false;
	     
		for(the_name=ace;the_name<=king;the_name++)
			my_hand.name_count[the_name] = 0;				
		
		for(this_suit=club; this_suit<=spade; this_suit++)
			my_hand.suit_count[this_suit] = 0;
			
		for_each_card_in_the_dealt_hand
		{
			my_hand.name_count[my_hand.cards [this_card].name]++;
			my_hand.suit_count[my_hand.cards [this_card].suit]++;
		}
		for_each_card_on_the_board
		{
	       my_hand.name_count [the_board[this_card].name]++;
	       my_hand.suit_count [the_board[this_card].suit]++;
		}
	
// 	Now it should be possible to determine hand strength by
//	using the name_count and suit_count_of data for (this player.  Use 
//	name_count to arbitrate pairs, trips, full houses or quads. 

			for(the_name=ace;the_name<=king;the_name++)
		 
			select (my_hand.name_count[the_name]) {
			
			case 2: 
				stats.pairs++;
				break;		
			case 3: 
	       		stats.trips = true;
	       		break;      		
			case 4: 
	       		stats.fourofakind = true;
	       		break;      	
	       	default:
	       		break; }
	
	// Handle nopair, one pair and two pair. 
	
		select (stats.pairs) {
		
		case 0: 
	       my_hand.strength = nopair;
	       break;	       
		case 1: 
	       my_hand.strength = onepair;
	       break;	       
		case 2: case 3: 
	       my_hand.strength = twopair;
	       break; }
	
	//	Check for trips.  This can be later upgraded to
//		fullhouse if (the player also has an extra pair. 
	
		if (stats.trips == true)
			my_hand.strength = trips;
	
	// Check for (a straight. 
	
		if (my_hand.name_count[ace] > 0)
		{
			my_hand.best[0].name = ace;
			my_hand.best[0].suit = blank;
			found = 1;
		}
		else
			my_hand.found = 0;
			
		for_each_card_name_descending
		{
			if (my_hand.name_count[the_name] > 0)
			{
				my_hand.best[found].name = the_name;
				my_hand.best[found].suit = blank;
				found++;
			}
			else
	      	 my_hand.found = 0;
	      	 
	 		if (my_hand.found == 5)
	 		{
	 			stats.straight = true;
	 			break;
	 		}
		}
		if (stats.straight == true) my_hand.strength = straight;
	
	// Check for a flush. 
	
		for(this_suit=club; this_suit<=spade; this_suit++)
		if (my_hand.suit_count[this_suit] > 4)
		{
			stats.flush = true;
			my_hand.strength = flush;
			my_hand.fsuit = this_suit;
		}
	
	// Assert any full house. 
	
		if (stats.trips == true)
		{
			if (my_hand.name_count[ace] == 3) stats.first = ace;			
			if (my_hand.name_count[ace] == 2) stats.second = ace;
				
			for_each_card_name_descending 
			{
				select (my_hand.name_count[the_name]) {
				
					case 3: 
						if (stats.first == null)
							stats.first = the_name;
						else
							if (the_name is_not_an_ace)
							{
								stats.second = the_name;
								my_hand.strength = fullhouse;
							}
						break;
						
					case 2: 
						if (((the_name is_not_an_ace)||
							(stats.first is_not_an_ace))
							&&(stats.second == null))
							{
								stats.second = the_name;
								my_hand.strength = fullhouse;
							}
							break;
					default:
						break;	}
			}
	}

// Assert any four of a kind. 

	if (stats.fourofakind == true) my_hand.strength = fourofakind;

// Now pack best declaration into best for this player. 

	select (my_hand.strength) {
		
		case nopair: 
			my_hand.pack_no_pair();
			break;
		       		      
		case onepair: case twopair: case trips: case fullhouse: 
			my_hand.pack_groups ();
			break;
		       		       
		case flush: 
			my_hand.pack_flush();
			break;
		       
		case fourofakind: 
			my_hand.pack_four_of_a_kind();
			break; 
			 
		default:
			break; }
	}
}

// ------------------------------------------------------------------------ 

double hand::return_strength ()
{   
	unsigned char index0, index1;
	index0 = cards[0].name;
	index1 = cards[1].name;
	if (index0 > index1)
	{
		index0 = cards[1].name;
		index1 = cards[0].name;
	}
	if (cards[0].suit == cards[1].suit)
	{
		if (appeared[index0][index1] != 0 )
			return won[index0][index1] / appeared[index0][index1];
	}
 	else if (appeared[index1][index0] != 0 )
		return won[index1][index0] / appeared[index1][index0];

	return nopair;
}

void init_tallies() 
{
	card_name the_name1, the_name2;
	hand_value strength;

	games = 0;
	for (strength = nopair; strength<= straightflush; strength++)
		strongest[strength] = 0;

	for(the_name1=ace;the_name1<=king;the_name1++)
	for(the_name2=ace;the_name2<=king;the_name2++)
	{
		appeared[the_name1][the_name2] = 0;
		won[the_name1][the_name2] = 0;
	}
}


// 	This function gets called after each hand in order
//	to tabulate the various strength tables for each
//	hand.

void tally_hand_performance()
{
	int p_index;
	card_name index1, index0;
	
	hand *winner;
	index0 = winners_first_card.name;
	index1 = winners_other_card.name;
	
  	if (index1>index0)
	{
  		index0 = winners_other_card.name;
		index1 = winners_first_card.name;
	}
	else
	{
		index0 = winners_first_card.name;
		index1 = winners_other_card.name;	
	}
	if (winners_first_card.suit == winners_other_card.suit)
   		won[index0][index1]++;
	else
		won[index1][index0]++;
		
	for(p_index=1; p_index<=number_of_players; p_index++) if the_player_was_dealt_a_hand
	{	
		if (index0 > index1)
		{
			index0 = the_hand[1].name;
			index1 = the_hand[0].name;
		}
		else
		{
			index0 = the_hand[0].name;
			index1 = the_hand[1].name;
		}	
		if (the_hand[0].suit == the_hand[1].suit)
			appeared[index0][index1]++;
		else
			appeared[index1][index0]++;
	}	
	strongest[my_hand.strength[0]]++;
}

void init_cash()
{
	int player;
	
	cash[0] = 0;
	for(player=1; player<=number_of_players; player++) cash[player] = 1000;
}

void payoff()
{
	if (wr.each == true)
	{
		printf("Player %d wins %d\n",winner, pot);
	};
	cash[winner] += pot;
	pot = 0;
}

void call (int player)
{
	cash[player] -= min;
	pot += pot;
}

void init_rank_table ()
{
	int count;	
	for (count = 0; count<=170; count++)
	{
		rankings [count].suited = false;
		rankings [count].name1 = null;
		rankings [count].name2 = null;
		rankings [count].strength = 0;
	}
}

void insert (double strength, int the_name1,
	int the_name2, bool suited)
{
	int count, scan;
	
	enum { scanning, moving, finished } status;
	scan = 1;
	status = scanning;
	
	while ((status != finished) && (scan > 169))
	{
		if (strength >= rankings [scan].strength)
		{
			count = 169;
			while ((rankings [count].name1 == null)&&(count > scan))
				count--;
			while (count >= scan)
			{
				rankings [count+1] = rankings [count];
				count--;
			}
			rankings [scan].strength = strength;
			rankings [scan].name1 = the_name1;
			rankings [scan].name2 = the_name2;
			rankings [scan].suited = suited;
			status = finished;
	    }
		scan ++;
	}
}

//	This void scans the tables of hands won by and hands that
//	appeared and calculates the win ratio for (each hand) it
//	calls a routine; perform an insert sort into the rankings table. 

void adjust_rank_table_data()
{
	card_name the_name, the_name1, the_name2;
	long w1, a1;
	double strength;

// Pairs

	for(the_name=ace;the_name<=king;the_name++)
	{
		w1 = won[the_name][the_name];
		a1 = appeared[the_name][the_name];
		if (a1 > 0)
		{
			strength = w1 / a1;
			insert(strength,the_name,the_name,false);
		}
	}

// Suited Cards

	for(the_name1=ace;the_name1<=king;the_name1++)
	for(the_name2=ace;the_name2<=king;the_name2++)
	{
		w1 = won[the_name1][the_name2];
		a1 = appeared[the_name1][the_name2];
			
		if ((a1 > 0)&&(the_name1 != the_name2))
		{
			strength = w1 / a1;
			insert(strength,the_name1,the_name2,true);
		}
	}

//	Unsuited cards

	for(the_name1=ace;the_name1<=king;the_name1++)
	for(the_name2=ace;the_name2<=king;the_name2++)
	{
		w1 = won[the_name1][the_name2];
		a1 = appeared[the_name1][the_name2];
		
		if ((a1 > 0)&&(the_name1 != the_name2))
		{
			strength = w1 / a1;
			insert(strength,the_name2, the_name1, false);
		}
	}
}

void sort_the_rank_table()
{
	int count;
	count = 1;
	
	while ((rankings[count].strength > 0)&&(count < 170))
	{
		if (rankings [count].suited == true)
			ordering[rankings[count].name1][rankings[count].name2] = count;
		else
			ordering[rankings[count].name2][rankings[count].name1] = count;
			count++;
	}
}

void update()
{
	adjust_rank_table_data();
	sort_the_rank_table();
}

int order (hand hands)
{
	card_name index1, index0;
	
	if (games%1024==0)
		update();
	if (hands[0].name < hands[1].name)
	{
		index1 = hands[0].name;
		index0 = hands[1].name;
	}
	else
	{
		index0 = hands[0].name;
		index1 = hands[1].name;
	}
	if (hands[0].suit == hands[1].suit)
		return ordering[index1][index0];
	else
		return ordering[index0][index1];
}

void rock_bettor_opens (int player)
{
	if (order(the_hand) < 20)
	{
		cash[player] -= min;
		pot += min;
	}
	else
		my_hand.strength[player] = folded;
}

void tight_bettor_opens (int player)
{
	if (order(the_hand) < 40)
	{
		cash[player] -= min;
		pot += min;
	}
	else
		my_hand.strength[player] = folded;
}

void aggressive_bettor_opens (int player)
{
	if (strength(the_hand) > min / (pot + min))
	{
		cash[player] -= min;
		pot += min;
	}
	else
		my_hand.strength[player] = folded;
}

void action_bettor_opens (int player)
{
	card_name low_card, high_card;
	
	if (the_hand[0].name < the_hand[1].name)
	{
		low_card = the_hand[0].name;
		high_card = the_hand[1].name;
	}
	else
	{
		low_card = the_hand[1].name;
		high_card = the_hand[0].name;
	}
	if (low_card is_an_ace)
	{
		low_card = high_card;
		high_card = ace;
	}
	if (((high_card is_an_ace)||(low_card > ten))||
	((the_hand[0].suit == the_hand[1].suit)
	&&(low_card > seven))||(((high_card - low_card == 1))
	&&(low_card > seven)))
	
	call(player);
	
	else
		my_hand.strength[player] = folded;	
}

void ontilt_bettor_opens (int player)
{
	call(player);
}

void make_opening_wager (int player)
{
	int raises;
	raises = 0;
	select ((number_of_players+player-blind)%number_of_players)
	{
		case 0:
		{
			cash[player] -= min;
			cash[0] += min;
			break;
		}
		case 1:
		{
			cash[player] -= option;
			pot += option;
			break;
		}
		case 2:
		{
			cash[player] -= min;
			pot += 3;
			break;
		}
		
		default:
			select (playstyle[player])
			{
				case rock:
					rock_bettor_opens(player);
					break;
					
				case tight:
					tight_bettor_opens(player);
					break;
					
				case aggressive:
					aggressive_bettor_opens(player);
					break;
					
				case kamikaze:
					aggressive_bettor_opens(player);
					break;
					
				case calling:
					action_bettor_opens(player);
					break;
					
				case ontilt:
					ontilt_bettor_opens(player);
					break;
					
				default:
					break; }
			}
}

void holdem_opening()
{
	int position, player;
	for(player=1; player<=number_of_players; player++) my_hand.strength[player] = unknown;
	
	if (flag.betting == true) 
	for (position = 1; position <= number_of_players; position++)
	{
		player = (number_of_players + position - blind) % number_of_players + 1;
		make_opening_wager(player);
	}
}

void this_player_has_a_bigger_hand(int player)
{
	int i;
	
	if (tied[0] == true)
	for (i = 1; i<= number_of_players; i++)
		tied[i] = false;
	winner = player;
	my_hand.strength[0] = my_hand.strength[player];
	for (i = 0; i<= 4; i++)
		best[0][i] = best[player][i];
}

void check_for_tie_with_current_winner(int player)
{
	int this_card;
	bool superior, still_tied;
	
	superior = false;
	still_tied = false;
	
	for (this_card=0; this_card<5; this_card++)
	{
		if (best[player][this_card].name == ace)
		{
			if (best[0][this_card].name is_not_an_ace)
			{
				superior = true;
				break;
			}	
		}		
		else
		if (best[0][this_card].name is_an_ace)
			// Previous hand holds up 
			break;
			
		if (best[player][this_card].name > best[0][this_card].name)
		{
			superior = true;
			break;
		}
		
		if (best[player][this_card].name == best[0][this_card].name)
			still_tied = true;
		else
			break;
	}
	if (superior == true)
		this_player_has_a_bigger_hand(player);
		
	if ((still_tied == true)&&(this_card == 5))
	{
		tied[0] = true;
		tied[winner] = true;
		tied[player] = true;
	}
}

void show_down()
{
	int player;
	
	my_hand.strength[0] = unknown;
	tied[0] = false;
	flag.splitpot = false;
	
	for(player=1; player<=number_of_players; player++)
	{
		tied[player] = false;
		if (my_hand.strength[player] > folded)
		{
			if (my_hand.strength[player] > my_hand.strength[0])
				this_player_has_a_bigger_hand(player);			
			else
				if (my_hand.strength[player] == my_hand.strength[0])
					check_for_tie_with_current_winner(player);		
		}
		flag.splitpot = tied[0];
	}
}

void write_name (card_name the_name)
{
	select (the_name) {
	
	case ace:
		printf("Ace   ");
		break;
		
	case jack:
		printf("Jack  ");
		break;
		
	case queen:
		printf("Queen ");
		break;
		
	case king:
		printf("King  ");
		break;
		
	default:
		printf("%-6d",the_name);
		break; }
}

void write_names (card_name the_name1, card_name the_name2)
{
	if (the_name2 == ace)
	{
		write_name(the_name2);
		write_name(the_name1);
	}
	else
	{	if ((the_name1 > the_name2)||(the_name1 is_an_ace))
		{
			write_name(the_name1);
			write_name(the_name2);
		}
		else
		{
			write_name(the_name2);
			write_name(the_name1);
		}
	}
}

card_name incr (card_name the_name)
{
	card_name incr1;
	
	if (the_name is_a_king)
		incr1 = ace;
	else
		incr1 = the_name + 1;
	if (the_name is_a_deuce)
		incr1 = null;
		
	return incr1;
}

card_name decr (card_name the_name)
{
	card_name decr1;
	
	if (the_name is_an_ace)
		decr1 = king;
	else
		decr1 = the_name - 1;
	if (the_name is_a_deuce)
		decr1 = null;

	return decr1;
}

void write_rank_table_entry (
		card_name name1, card_name name2,
		long w1, long a1)
{
	double win_ratio;
	win_ratio = 100 * w1 / a1;
	write_names (name1, name2);
	printf("%7ld %7ld",w1,a1);
	printf("%7.2lf %%\n", win_ratio);
}

void tabulate_pairs()
{
	card_name the_name1;
	long a1, w1;
	
	the_name1 = ace;
	
	while (the_name1 != null) {
		w1 = won[the_name1][the_name1];
		a1 = appeared[the_name1][the_name1];
		if (a1 > 0)
			write_rank_table_entry(the_name1,the_name1,w1,a1);	
		the_name1 = decr(the_name1); }
}

#define valid_entry ((a1 > 0)&&(the_name1 != the_name2))

void tabulate_non_pairs(bool suited)
{
	card_name the_name1, the_name2;
	long a1, w1;

	the_name1 = ace;	
	while (the_name1 != null) {
		the_name2 = decr(the_name1);
		while (the_name2 != null) {
			w1 = won[the_name2][the_name1];
			a1 = appeared[the_name2][the_name1];		
			if valid_entry switch (suited) {
				case false:
					write_rank_table_entry(the_name1,the_name2,w1,a1);
					break;
				case true:
					write_rank_table_entry(the_name2,the_name1,w1,a1);
					break; }			
			the_name2 = decr(the_name2);
			}	
		the_name1 = decr(the_name1); }
}

void tabulate_dealt_hand_statistics()
{	
	printf("\n");
	printf("%d Games\n",games);
	printf("\n");
	printf("Pairs\n");
	printf("\n");
	tabulate_pairs();				
	printf("\n");
	printf("Suited cards:\n");
	printf("\n");
	tabulate_non_pairs(true);				
	printf("\n");
	printf("Unsuited cards:\n");
	printf("\n");
	tabulate_non_pairs(false);
	printf("\n");
}

void tabulate_stength_of_winning_hands()
{
	double percent_won_by;
	 
	const char *poker_text[] =	{	"Unknown:","folded:","High card:",
	"One pair:","Two pair:", "3 of a kind:","Straight:",
	"Flush:", "Full House:","Four of a Kind:","Straight flush:" };
	
	hand_value my_hand.strength;
	printf("Completed %ld games with %d players.\n", games, number_of_players);
	printf("\n");
	printf("Percentage of hands won by:\n");
	printf("\n");
	if (games == 0) games++;	
	for (my_hand.strength = nopair; my_hand.strength <= straightflush; my_hand.strength++)
	{
		if (my_hand.strength == folded) continue;	
		printf("%-18s",poker_text[my_hand.strength]);
		
		percent_won_by = 100 * (double)strongest[my_hand.strength] / games;	
		printf("%6.2lf\n",percent_won_by);	
	}
	printf("\n");
}

void top_performing_hands (int draws)
{
	int count;
	double strength_of_hand;
	
	if (draws == 20)
		printf("Current Top 20\n");
	else
		printf("Current Rankings\n");
		
	printf("\n");
	count = 0;
	if (rankings[1].strength > 0)
		while ((rankings[count].strength!=0)&&(count!=draws)) {
			count ++;
			write_names (rankings[count].name1, rankings[count].name2);
			if (rankings[count].suited == true)
				printf("Suited   ");
			else
				printf("         ");
				strength_of_hand = number_of_players * rankings[count].strength;
				printf("%lf5.3\n",strength_of_hand); }
		printf("\n");
}

void display_rank_table (int how_many)
{
	init_rank_table();
	adjust_rank_table_data();
	sort_the_rank_table();	
}

void display_statitics (int how_many)
{
	init_rank_table();
	adjust_rank_table_data();
	sort_the_rank_table();
	top_performing_hands(how_many);
}


void announce_the_results_of_this_game ()
{
	int player;
	
	static char *poker_text[] = 
		{ "unknown.","folded.","no pair.","a pair.","two pair.",
		"three of a kind.","a straight!","a flush!","a full house!",
		"four of a kind!","a straight flush!" };
		
	for(player=1; player<=number_of_players; player++)
	{	
//		if (Button() == true) interrupt = true;
		
		if ((player == winner) && (flag.splitpot == false))
			printf("Player %d wins with ", player);
		else
			printf("Player %d has ", player);
		
		printf("%s",poker_text[my_hand.strength[player]]);			
		printf("\n");
	}
}

void report_tied_players()
{
	int player, ties;
	printf("This round appears to be a tie between players ");
	ties = 0;
	for(player=1; player<=number_of_players; player++)
		if (tied[player] == true) ties++;
			
	player = 0;
		
	while (ties>1)
	{
		player ++;
		if (tied[player] == true)
		{
			ties --;
			printf("%d",player);
			if (ties > 1)
				printf(", ");
		}
	}				
	printf(" and ");
	
	while (player<number_of_players)
	{
		player ++;
		if (tied[player])
			printf("%d.\n",player);
	}		
	printf("\n");
}

void report_the_hands ()
{
	int player;
		
//	interrupt = false;
	printf("Game number %ld.  ", games);
		
	printf("Player %d has the button.\n\n",blind);
	
	for(player=1; player<=number_of_players; player++) show_a_hand (player);

#ifdef MAC
	if (Button() == true)
		interrupt = true;
#endif
	
	show_the_board();
	announce_the_results_of_this_game();
	printf("\n");
	
	if (flag.splitpot == true) report_tied_players();
	
	if (flag.hitit == true)
		printf("Badbeat reported!\n");
	else if (flag.highhand == true)
		printf("High hand reported!\n");
}

void reset_strategies ()
{
	int player;
	for(player=1; player<=number_of_players; player++) playstyle[player] = playstyle[0];
}

void badbeat ()
{
	int j, player, this_card;
	bool bigenough;
	flag.highhand = false;
	bigenough = false;
	flag.hitit = false;
	if (my_hand.strength[winner] > flush)
	{
		for(player=1; player<=number_of_players; player++)
		for_each_card_in_the_dealt_hand
		{
			cardplays[player][this_card] = false;
			for (j = 0; j<=1; j++)
				if ((hands [this_card].name == best[player][j].name)
				&& (the_hand[this_card].suit == best[player][j].suit))
					cardplays[player][this_card] = true;
		}
		if both_of_the_winners_cards_play for(player=1; player<=number_of_players; player++)
		{
			bigenough = false;
			if (both_of_this_players_cards_play && (my_hand.strength[player] > flush)))
				
			select (my_hand.strength[player]) {
								
				case fullhouse:
					if ((best[player][0].name is_an_ace)
						&& (best[player][4].name > ten))
					{
						if (player == winner)
							flag.highhand = true;
						else
							bigenough = true;
					}
					break;
					
				case fourofakind: case straightflush:
					{ if (player == winner)
							flag.highhand = true;
						else
							bigenough = true; }
				default:
					break; }
		}
		if ((bigenough == true) && (my_hand.strength[winner] > fullhouse))
			flag.hitit = true; }
}

void players_cash_report ()
{
	int player;
	for(player=1; player<=number_of_players; player++)
		printf("Player %d has $%ld\n", player, cash[player]);
	printf("The house has raked $%ld\n", cash[0]);
	printf("\n");
}

#ifdef MAC
void handle_rank_table_item ()
{
	display_statistics (169);
}
#endif

void text_parameters ()
{
	printf("How many games?");
	scanf("%ld",&quota);
	if (quota > 1)
	{
		fflush(NULL);
		printf("\nReport results for each hand?");
		wr.each = reply();
		printf("\nReport results for highhands?");
		wr.high = reply();
	}
	else
	{
		wr.each = true;
		wr.high = true;
	}
	printf("\nInclude the betting structure?");
	flag.betting = reply();
	printf("\n");
}

#ifdef MAC
void set_gameflow (void)
{
#define prepare_check_box(number, assignment)\
		case number:param = assignment; break;

	static DialogPtr PrefRef;
	short itemHit, itemNum, itemType, param;
	WindowPtr behind;
	Ptr windowstorage, filterProc;
	Handle itemHndl;
	Rect box;
	filterProc = nil;
	windowstorage = nil;
	if (flag.PrefExists == false)
	{
		behind = (WindowPtr)(-1);
		PrefRef = GetNewDialog(128, windowstorage, behind);
		flag.PrefExists = true;
	}
	for (itemHit=4; itemHit<=7; itemHit++) 
	{
		switch ( itemHit ) {
		
		prepare_check_box(4,wr.each)
		prepare_check_box(5,wr.high)
		prepare_check_box(6,wr.bad)
		prepare_check_box(7,flag.betting) }
		
		GetDialogItem(PrefRef, itemHit, &itemType, &itemHndl, &box);
		SetControlValue((ControlHandle)(itemHndl), param);
		SetDialogItem(PrefRef, itemHit, itemType, itemHndl, &box);
	}
	ShowWindow(PrefRef);
	DrawDialog(PrefRef);
	while (itemHit>3)
	{
		ModalDialog(nil, &itemHit);
		GetDialogItem(PrefRef, itemHit, &itemType, &itemHndl, &box);
		param = GetControlValue((ControlHandle)(itemHndl));
		param = 1 - param;
		SetControlValue((ControlHandle)(itemHndl), param);
		
		switch ( itemHit ) {
		
		case 1:
			flag.run = true;
			break;
		case 2:
			flag.run = false;
			break;				
		case 3:
			flag.quit = true;
			break;				
		case 4:
			wr.each = (bool)(param);
			break;				
		case 5:
			wr.high = (bool)(param);
			break;					
		case 6:
			wr.bad = (bool)(param);
			break;				
		case 7:
			flag.betting = (bool)(param);
			break;				
		default:
			break; }
								
		SetDialogItem(PrefRef,itemHit,itemType,itemHndl,&box);
	}
	CloseDialog(PrefRef);
	flag.PrefExists = false;
}
#endif

#ifdef MAC
void strategy_dialog(int player)
{
	static DialogPtr PlayRef;
	
	short itemHit, itemNum, itemType, param;
	WindowPtr behind;
	Ptr dstorage, filterProc;
	Handle itemHndl;
	Rect *box;
	
	filterProc = nil;
	dstorage = nil;
	if (flag.PlayExists == false)
	{
		behind = (WindowPtr)(-1);
		PlayRef = GetNewDialog(130, dstorage, behind);
		flag.PlayExists = true;
	}
	ShowWindow(PlayRef);
	itemHit = playstyle[player];
	while ((itemHit != 9) && (itemHit != 10)) {
		if (itemHit < 8)
		for (itemNum = 2;itemNum <= 7; itemNum++)
		{
			GetDialogItem(PlayRef, itemNum, &itemType, &itemHndl, box);
			param = GetControlValue((ControlHandle)(itemHndl));
			if (itemHit == itemNum)
			{
				param = 1;
				playstyle[player] = (strategies)(itemNum);
			}
			else
				param = 0;
			SetControlValue((ControlHandle)(itemHndl), param);
			SetDialogItem(PlayRef, itemNum, itemType, itemHndl, box);
		}
		ModalDialog(nil, &itemHit);
	}
	CloseDialog(PlayRef);
	flag.PlayExists = false;
}
#endif


#ifdef MAC
void set_strategy()
{
	int player;
	
	player = 0;
	strategy_dialog(0);
}
#endif

void set_games_dialog()
{
//	static DialogPtr NumRef;
	
//	short itemHit, itemType;
//	long Num;
//	WindowPtr behind; 
//	Ptr dstorage, filterProc;
//	Handle itemHndl;
//	Rect box;
	
	char *the_c_string = "Name no one man";
//	Str255 editStr;
	number_of_players = 0;
	if (flag.debug == true)
		while ((0 < number_of_players) && (number_of_players < 11)) {
			printf("How many players?");
			scanf("%d",&number_of_players);
			printf("\n"); }

#ifdef MAC
	else
	{
		filterProc = ((void *) 0);
		dstorage = ((void *) 0);
		if (flag.NumExists == false)
		{
			behind = (WindowPtr)(-1);
			NumRef = GetNewDialog(131, dstorage, behind);
			flag.NumExists = true;
		}
		ShowWindow(NumRef);
		DrawDialog(NumRef);
		itemHit = 0;
		while (itemHit < 3) {
			ModalDialog((void *) 0, &itemHit);
			if ((itemHit == 1) || (itemHit == 2))
			{
				GetDialogItem(NumRef, itemHit, &itemType, &itemHndl, &box);
				GetDialogItemText(itemHndl, editStr);
				strcpy(the_c_string,PtoCstr(editStr));
				sscanf(the_c_string,"%ld",&Num);
				
			switch ( itemHit ) {
				case 1:
					number_of_players = Num;
					break;
				case 2:
					quota = Num;
				};
			}; }
		CloseDialog(NumRef);
		flag.NumExists = false;
	}
#endif
}

void reset()
{
	badbeats = 0;
	highhands = 0;
	games = 0;
}

void set_quit()
{
	flag.quit = true;
	flag.resume = false;
	flag.run = false;
}

void unsupported()
{
	printf("This feature is not yet supported.\n");
}


/*
void copyright_notice()
{
	int baseresid = 128;
	int horizontalpixel = 45;
	int verticalpixel = 20;
	
	char mode;
	EventRecord event;
	
	WindowPtr helloWindow;
		
	helloWindow = GetNewWindow(baseresid, (void *) 0, (WindowPtr)(-1));
	ShowWindow(helloWindow);
	
	while (Button() == false)
	{
		SetPort(helloWindow);
		MoveTo(horizontalpixel, verticalpixel);
		DrawString("\pCopyright 1995 Gerold Lee Gorman");
		syscheck();
	}	
	HideWindow(helloWindow);
}
*/

#ifdef MAC
void startup()
{
	flag.quit = false;
	flag.reset = false;
	flag.PrefExists = false;
	flag.PlayExists = false;
	flag.debug = false;
	badbeats = 0;
	highhands = 0;
	games = 0;
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
	
	make_menu_bar();
// 	do(copyright_notice)	
	
			
	console_options.pause_atexit = 0;
	console_options.nrows = 48;
	console_options.ncols = 80;
	console_options.title = "\pFull Power Texas Hold 'Em";
	
	c1 = fopenc();
	cshow(c1);
	printf("Texas Hold' Em Tutorial Software\n");
	printf("Copyright 1995 Gerold Lee Gorman\n");
}
#else

void startup()
{
	flag.quit = false;
	flag.reset = false;
	flag.PrefExists = false;
	flag.PlayExists = false;
	flag.debug = true;
	badbeats = 0;
	highhands = 0;
	games = 10;
}
#endif


#ifdef MAC
void play (long game_quota)
{
	bool interrupt;
	long current_games;
	static long oldTickTime;
	
	oldTickTime = TickCount();
	current_games = 1;
	while ((flag.quit == false)&&(current_games<=game_quota))
	{
		current_games++;
		games++;
		pot = 0;
		blind = blind % number_of_players + 1;
		shuffle(3);
		deal_to_the_players(number_of_players);
		deal_to_the_board();
		holdem_opening();
		evaluate_the_strength_of_each_hand();
		show_down();
		payoff();
		
		// badbeat();
		tally_hand_performance();
		
		if (flag.highhand == true) highhands ++;
		if (flag.hitit == true) badbeats ++;
		
		if (games % 10000 == 0)
		{
			printf("\n");
			printf("%ld High Hands/ %ld Games\n",highhands, games);
			printf("%ld Bad Beats/  %ld Games\n",badbeats, games);
		}
		if ((flag.highhand && wr.high) || (flag.hitit && wr.bad) || wr.each)
			report_the_hands(interrupt);
				
		if ((TickCount() - oldTickTime > 5)||(wr.each == true)||interrupt)
		{
			oldTickTime = TickCount();
			Sleep (0);
		} 
	}
	if (current_games >= game_quota) flag.run = false;
}
#else

void play (long game_quota)
{
//	bool interrupt;
	long current_games;
	static long oldTickTime;
	
//	oldTickTime = GetTickCount();
	current_games = 1;
	while ((flag.quit == false)&&(current_games<=game_quota))
	{
		current_games++;
		games++;
		pot = 0;
		blind = blind % number_of_players + 1;
		the_deck.shuffle(3);
		the_deck.deal_to_the_players(number_of_players);
		the_deck.deal_to_the_board();
		holdem_opening();
		evaluate_the_strength_of_each_hand();
		show_down();
		payoff();
		
		// badbeat();
		tally_hand_performance();
		
		if (flag.highhand == true) highhands ++;
		if (flag.hitit == true) badbeats ++;
		
		if (games % 10000 == 0)
		{
			printf("\n");
			printf("%ld High Hands/ %ld Games\n",highhands, games);
			printf("%ld Bad Beats/  %ld Games\n",badbeats, games);
		}
		if ((flag.highhand && wr.high) || (flag.hitit && wr.bad) || wr.each)
			report_the_hands();

#if 0
		if ((TickCount() - oldTickTime > 5)||(wr.each == true)||interrupt)
		{
			oldTickTime = TickCount();
			Sleep (0);
		}
#endif
	}
	if (current_games >= game_quota) flag.run = false;
}
#endif

//
#ifdef MAC
void resize_window (WindowPtr thisWindow, EventRecord event)
{
	long NewSize;
	Rect limitRect, oldViewRect, wrect;
	RgnHandle locUpdateRgn;
	bool theResult;
	
	SetRect(&limitRect, kMinDocSize, kMinDocSize, kMaxDocSize, kMaxDocSize);
	NewSize = GrowWindow(thisWindow, event.where, &limitRect);
	if (NewSize != 0)
	{
		locUpdateRgn = NewRgn();
		SizeWindow(thisWindow, LoWord(NewSize), HiWord(NewSize), TRUE);
		ValidRect(&oldViewRect);
		InvalRgn(locUpdateRgn);
		DisposeRgn(locUpdateRgn);
		// GetTextRect(wrect);
		// SetTextRect(wrect);
	}
}
 
void make_window (int ID, WindowPtr refCon)
{
	Ptr wStorage;
	WindowPtr behind;
	behind = nil;
	wStorage = nil;
	refCon = GetNewWindow(ID, wStorage, (WindowPtr)(-1));
}
#endif

#ifdef MAC
void make_menu_bar ()
{
	Handle menuBar;
	long ID = 128;
	menuBar = GetNewMBar(ID);
	SetMenuBar(menuBar);
	DisposeHandle(menuBar);
	AddResMenu(GetMHandle(mApple), 'DRVR');
	DrawMenuBar();
}
#endif

#ifdef MAC
void handle_apple_menu (int itemNumber)
{
	Str255 DAName;
	long Result;

	switch (itemNumber) {
	
	iAbout:
		printf("Full Power Texas Hold 'Em\n");
		printf("Copyright 1995 G L Gorman\n");
		break;
		
	default:		
		GetMenuItemText(GetMenuHandle(mApple), itemNumber, DAName);
		Result = OpenDeskAcc(DAName);
		flag.activateDA = true;
		break; }
}
#endif

#ifdef MAC
void handle_file_menu (int ItemNumber)
{
	switch (ItemNumber) {
	
	match(1,unsupported) // New
	match(2,unsupported) // Open
	match(3,unsupported) // Page Setup		
	match(7,unsupported) // Print
	match(9,set_quit) 	 // Quit
	
	default:
		break; }
}
#endif

#ifdef MAC
handle_edit_menu (int ItemNumber)
{	
	switch ( ItemNumber ) {
	
	match(1, unsupported) // Undo		
	match(3, unsupported) // Cut	
	match(4, unsupported) // Copy	
	match(5, unsupported) // Paste				
	match(7, unsupported) // Select All
				
	default:
		break; }
}
#endif

#ifdef MAC
void handle_options_menu(int itemNumber)
{
	switch (itemNumber) {
	
	case 1: // Cards 
		unsupported();
		break;
		
	case 2: // Players
		unsupported();
		break;
		
	match(3, players_cash_report)
		
	case 4: // Strategy
		set_strategy();
		break;
		
	match(5, set_games_dialog)
				
	default:
		break; }
}
#endif

#ifdef MAC
void handle_analysis_menu (int itemNumber)
{
	switch ( itemNumber ) {
			
	case 1: // Replace The Deck
		HiliteMenu(0);
		make_deck();
		break;
		
	case 2: // Extra Shuffle
		HiliteMenu(0);
		shuffle(3);
		break;
	
	case 4: // Hands that have Won
		HiliteMenu(0);
		tabulate_stength_of_winning_hands();
		break;
		
	case 5: // Cards that have won
		HiliteMenu(0);
		display_stastics (20);
		break;
		
	case 7: // Show Cards in Deck
		HiliteMenu(0);
		show_the_deck1();
		break;
		
	case 8: // Show Cards in Deck 
		HiliteMenu(0);
		show_the_deck2();
		break;
	
	default:
		break; }
}
#endif

#ifdef MAC
void handle_game_flow_menu (int ItemNumber)
{
	switch (ItemNumber) {
	
	case 1:  // Reset
		unsupported();
		break;

	case 2:	// Parameterize
		set_gameflow();
		break;
		
	case 3: // Run Simulation
		printf("Now simulating %ld games.\n",quota);
		flag.run = true;
		flag.resume = true;
		break;
				
	case 5: // rankings Table
		HiliteMenu(0);
		handle_rank_table_item();
		break;
		
	case 6: // Statistics 
		HiliteMenu(0);
		tabulate_dealt_hand_statistics();
		break;
		
	case 7: // Critique 
		HiliteMenu(0);
		unsupported();
		break; };
}
#endif

#ifdef MAC
void handle_display_menu (int ItemNumber)
{
	switch ( ItemNumber ) {
	
	match(1,unsupported) // Text
	match(2,unsupported) // Drawing		
	match(4,unsupported) // Special
	
	default:
		break; }
}
#endif

#ifdef MAC
void menu_task (long menuResult)
{
	char mApple = 128;
	char iAbout = 1;
	Str255 MenuText;
	MenuHandle theMenu;
	int menuID, itemNumber, Result;
	char DAName[255];
	WindowPtr Window;
	
	menuID = HiWord(menuResult);
	itemNumber = LoWord(menuResult);
	if (menuID == mApple)
		handle_apple_menu(itemNumber);
	else
	{
		theMenu = GetMenuHandle(menuID);
		GetMenuItemText(theMenu, itemNumber, MenuText);
		switch (menuID) {
		
		case 129:
			handle_file_menu(itemNumber);
			break;
	
		case 130:
			handle_edit_menu(itemNumber);
			break;

		case 131: 
			handle_options_menu(itemNumber);
			break;
		
		case 132:
			handle_analysis_menu(itemNumber);
			break;
			
		case 133: 
			handle_game_flow_menu(itemNumber);
			break;
			
		case 134:
			handle_display_menu(itemNumber);
			break; }
	}
	HiliteMenu(0);
}
#endif

// ------------------------------------------------------------------------ 

#ifdef MAC
void handle_content_click (WindowPtr thisWindow, EventRecord event)
{
	SetCursor(&qd.arrow);
}
#endif

#ifdef MAC
void handle_mouse_down (EventRecord event)
{
	int part;
	WindowPtr thisWindow;
	RgnHandle permitted_region;
	
	flag.activateDA = false;
	part = FindWindow(event.where, &thisWindow);
	
	switch (part) {
	
		case inMenuBar:
			menu_task(MenuSelect(event.where));
			SystemTask();
			break;
			
		case inSysWindow:
			SystemClick(&event, thisWindow);
			break;
			
		case inContent:
			if (thisWindow != FrontWindow())
				SelectWindow(thisWindow);
			else
				handle_content_click (thisWindow, event);
			break;
			
		inDrag:
			permitted_region = GetGrayRgn();
			DragWindow(thisWindow, event.where, &(**permitted_region).rgnBBox);
			break;
			
		inGoAway:
			if (TrackGoAway(thisWindow, event.where) == true)
			flag.goaway = true;
			break;
			
		inGrow:
			resize_window(thisWindow, event);
			break;
			
		default:
			break; }
			
	if (flag.activateDA == true)
		SystemClick(&event, thisWindow);
	flag.activateDA = false;
}
#endif

#ifdef MAC
void syscheck()
{
	flag.goaway = false;
	flag.resume = false;
	flag.status = GetNextEvent(-1, &event);
	if (Button() == true)
		handle_mouse_down(event);
}
#endif


