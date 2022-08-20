#include "map.h"
#include <map>
#include <iterator>
#include <random>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <chrono>
#include <thread>
#include <windows.h>

/// <summary>
/// sets cursor at x,y for printing
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

/// <summary>
/// Stolen from the internet, just clears the console.
/// I use it to 'animate'
/// </summary>
void ClearScreen()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR)' ',
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}

#pragma region Kernal

using std::string;
using std::pair;
using std::vector;

int random(int min, int max) {
	if (min == max)
	{
		return min;
	}

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dist(min, max);
	return dist(gen);

}

/// <summary>
/// Finds the uChars that are the same between two vectors.
/// Order doesn't matter.
/// </summary>
/// <param name="v1"> a vector </param>
/// <param name="v2"> other vector</param>
/// <returns> vector of uChars contained in both </returns>
vector<uChars> intersection(vector<uChars> v1,
	vector<uChars> v2) {
	vector<uChars> v3;

	std::sort(v1.begin(), v1.end());
	std::sort(v2.begin(), v2.end());

	std::set_intersection(v1.begin(), v1.end(),
		v2.begin(), v2.end(),
		back_inserter(v3));
	return v3;
}

int Kernal::entropyCount() {
	return (int)entropy.size();
}

/// <summary>
/// assigns self randomly from remaining valid uChars (entropy)
/// </summary>
void Kernal::assignRandomly() {
	self = entropy.at(random((int)0, (int)entropy.size() - 1));
	entropy = { self };
	collapsed = true;
}

bool Kernal::isCollapsed() {
	return collapsed;
}

/// <summary>
/// updates the entropy based on the neighbor's accepting tiles passed in
/// </summary>
/// <param name="options"> the possible tiles for second tile in relation to first tile </param>
/// <returns> true if any entropy changed </returns>
bool Kernal::updateEntropy(vector<uChars> options) {
	int origSize = entropy.size();

	if (origSize == 0)
	{
		entropy = intersection(entropy, {self});
	}
	else
	{
		entropy = intersection(entropy, options);
	}

	return origSize != entropy.size();;
}

#pragma endregion

#pragma region Map

/// <summary>
/// board set-up
/// </summary>
/// <param name="h"> height </param>
/// <param name="w"> width </param>
Map::Map(int h, int w) {
	pupulateConstraints();
	height = h;
	width = w;
	board = new Kernal * [h];
	for (int j = 0; j < h; j++)
	{
		board[j] = new Kernal[w];
		for (int i = 0; i < w; i++)
		{
			board[j][i].h = j;
			board[j][i].w = i;
		}
	}
	remainingKernals = width * height;
}

void Map::draw() {
	for (int i = 0; i < height; i ++)
	{
		for (int j = 0; j < width; j++)
		{
			printf("%c", board[i][j].self);
		}
		printf("\n");
	}
}

void Map::test()
{
	ClearScreen();
	tester(0);
}

void Map::tester(int i) {
	gotoxy(i, i);
	std::cout << i;
	if (i<100)
	{
		tester(i+1);
	}
}

std::vector<uChars> Map::getConstraints(uChars uc, int dir)
{
	std::map<uChars, std::vector<std::vector<uChars>>>::iterator it = constraints.find(uc);
	
	return it->second[dir];
}

void Map::chooseRandomFromAvailable(int h, int w) {
	board[h][w].assignRandomly();
	spreadEntropy(h, w);
	remainingKernals -= 1;
	if (animate)
	{
		gotoxy(w, h);
		printf("%c", board[h][w].self);
	}
}

/// <summary>
/// loops through the board and gets the lowest and second lowest entropy counts
/// </summary>
/// <param name="lowest"> very lowest </param>
/// <param name="low"> second lowest </param>
void Map::findLowestEntropyCount(int & lowest, int & low) {
	lowest = maxEntropy; // large number
	low = maxEntropy; // large number

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (board[i][j].collapsed || board[i][j].entropyCount() < 1 || board[i][j].entropyCount() == maxEntropy) {
				continue;
			}
			if (board[i][j].entropyCount() == 1) {
				low = lowest;
				lowest = 1;
			}
			if (lowest > board[i][j].entropyCount()) {
				low = lowest;
				lowest = board[i][j].entropyCount();
			}
		}
	}
}

/// <summary>
/// Main purpose of program!
/// goes turn by turn, adding a linking uChar to the board
/// </summary>
/// <param name="animate"> will animate if true </param>
/// <param name="animateSpeed"> speed of animation in milliseconds </param>
void Map::collapse(bool animated, int animateSpeed) {
	animate = animated;
	if (animate)
	{
		std::chrono::seconds flashWarningTime{ 3 };
		dura = (std::chrono::milliseconds)animateSpeed;
		ClearScreen();
	}
	
	// loop until all Kernals are collapsed
	do
	{
		// create list of lowest matching
		int lowestEntropy{-1};
		int secondLowestEntropy{-1};
		findLowestEntropyCount(lowestEntropy, secondLowestEntropy);

		// fill vector with kernals that have the lowest entropy
		vector<Kernal*> lowest;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if (!board[i][j].collapsed && board[i][j].entropy.size() == lowestEntropy) {
					lowest.insert(lowest.begin(), &(board[i][j]));
				}
				else if (!board[i][j].collapsed && board[i][j].entropy.size() == lowestEntropy) {
					lowest.insert(lowest.begin(), &(board[i][j]));
				}
			}
		}

		// randomly select from lowest and assign randomly from entropy
		Kernal* temp = lowest.at(random(0, (int)lowest.size() - 1));
		chooseRandomFromAvailable((*temp).h, (*temp).w);
		

		// update entropy
		spreadEntropy((*temp).h, (*temp).w);


		if (animate)
		{
			// pause to create animation
			std::this_thread::sleep_for(dura);

			//ClearScreen();
			//draw();
		}


	} while (remainingKernals != 0);

	// only draw once if no animation
	if (!animate)
	{
		draw();
	}
	else 
	{
		gotoxy(width, height);
	}
}

/// <summary>
/// Once a Kernal has been modified (collapsed or entropy is changed)
/// continue spreading the entropy possibilities to keep a valid map
/// </summary>
/// <param name="h"> height of board </param>
/// <param name="w"> width of board </param>
void Map::spreadEntropy(int h, int w) {
	// check each direction and try to update entropy
	// if it does update, continue to spread

	// North
	if (h > 0)
	{
		if (!board[h - 1][w].collapsed)
		{
			vector<uChars> possibilities = {};

			for (uChars uc : board[h][w].entropy) {
				vector<uChars> temp = getConstraints(uc, 0);
				for (uChars ucTemp : temp)
				{
					if (std::find(possibilities.begin(), possibilities.end(), ucTemp) == possibilities.end()) {
						possibilities.insert(possibilities.begin(), ucTemp);
					}
				}
			}

			// recurse if we changed the entropy in that direction
			if (board[h - 1][w].updateEntropy(possibilities)) {
				spreadEntropy(h - 1, w);
			}
		}
	}
	// East
	if (w < width - 1)
	{
		if (!board[h][w + 1].collapsed)
		{
			vector<uChars> possibilities = {};

			for (uChars uc : board[h][w].entropy) {
				vector<uChars> temp = getConstraints(uc, 1);
				for (uChars ucTemp : temp)
				{
					if (std::find(possibilities.begin(), possibilities.end(), ucTemp) == possibilities.end()) {
						possibilities.insert(possibilities.begin(), ucTemp);
					}
				}
			}

			// recurse if we changed the entropy in that direction
			if (board[h][w + 1].updateEntropy(possibilities)) {
				spreadEntropy(h, w + 1);
			}
		}
	}
	// South
	if (h < height - 1)
	{
		if (!board[h + 1][w].collapsed)
		{
			vector<uChars> possibilities = {};

			for (uChars uc : board[h][w].entropy) {
				vector<uChars> temp = getConstraints(uc, 2);
				for (uChars ucTemp : temp)
				{
					if (std::find(possibilities.begin(), possibilities.end(), ucTemp) == possibilities.end()) {
						possibilities.insert(possibilities.begin(), ucTemp);
					}
				}
			}

			// recurse if we changed the entropy in that direction
			if (board[h + 1][w].updateEntropy(possibilities)) {
				spreadEntropy(h + 1, w);
			}
		}
	}
	// West
	if (w > 0)
	{
		if (!board[h][w - 1].collapsed)
		{
			vector<uChars> possibilities = {};

			for (uChars uc : board[h][w].entropy) {
				vector<uChars> temp = getConstraints(uc, 3);
				for (uChars ucTemp : temp)
				{
					if (std::find(possibilities.begin(), possibilities.end(), ucTemp) == possibilities.end()) {
						possibilities.insert(possibilities.begin(), ucTemp);
					}
				}
			}

			// recurse if we changed the entropy in that direction
			if (board[h][w - 1].updateEntropy(possibilities)) {
				spreadEntropy(h, w - 1);
			}
		}
	}
}

/// <summary>
/// hard code of all the possible uChars of each uChar in all 4 direction
/// </summary>
void Map::pupulateConstraints()
{
	constraints.insert(pair<uChars,vector<vector<uChars>>>(EMPTY, { //   
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, }, // north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, }, // east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, }, // south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));// west)
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N2H2, { // ╩
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,}, // north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, }, // east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, }, // south
		{N2H2, S2H2, V2E2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));// west)
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2E2, { // ╠
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,}, // north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, }, // east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,}, // south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } })); // west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S2H2, { // ╦
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, }, // north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, }, // east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,}, // south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } })); // west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2W2, { // ╣
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,}, // north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, }, // east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,}, // south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } })); // west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1, { // │
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{EMPTY, V2W2, V1W2, V2W1, V1W1, V1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1W1, { // ┤
		{V1W1, V1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{V1, V2E2, EMPTY, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{V1W1, V1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1W2, { // ╡
		{V1W2, V1W1, V1, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2,  S1E1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{V1W2, V1W1, V1, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2W1, { // ╢
		{V2W1, V2E2, S2H2, V2W2, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{EMPTY, V1, V2E2, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{V2W1, N2H2, V2E2, V2W2, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S2W1, { // ╖
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S1W2, { // ╕
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{V2E2, S2H2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2, { // ║
		{S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{N2H2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{EMPTY, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S2W2, { // ╗
		{EMPTY, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{N2H2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{N2H2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N2W2, { // ╝
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N2W1, { // ╜
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N1W2, { // ╛
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S1W1, { // ┐
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N1E1, { // └
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1,V1E2,  S1H2, S1E2, V1H2, S1E1, },		// north
		{V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N1H1, { // ┴
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S1H1, { // ┬
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1E1, { // ├
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(H1, { // ─
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1H1, { // ┼
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1E2, { // ╞
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{N2H2, S2H2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{EMPTY, V2W2, V1, V1W2, V1W1, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2E1, { // ╟
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2,V2H2,  S2H1, S2E1, V2H1,},		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{EMPTY, V2W2, V1, V1W2, V1W1, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N2E2, { // ╚
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S2E2, { // ╔
		{EMPTY, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(H2, { // ═
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2H2, { // ╬
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N1H2, { // ╧
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N2H1, { // ╨
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S2H1, S1E2, S1E1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S1H2, { // ╤
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, N1H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S2H1, { // ╥
		{EMPTY, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N2E1, { // ╙
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{EMPTY, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N1E2, { // ╘
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S1E2, { // ╒
		{EMPTY, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, N1H2, S1H2, V1H2, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S2E1, { // ╓
		{EMPTY, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V2H1, { // ╫
		{V2E2, S2H2, V2W2, V2W1, S2W1, V2, S2W2, V2E1, S2E2, V2H2, S2H1, S2E1, V2H1,},		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1, N1W1, },		// east
		{N2H2, V2E2, V2W2, V2W1, V2, N2W2, N2W1, V2E1, N2E2, V2H2, N2H1, N2E1, V2H1,},		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(V1H2, { // ╪
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{N2H2, S2H2, V2W2, V1W2, S1W2, S2W2, N2W2, N1W2, H2, V2H2, S1H2, V1H2, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{N2H2, V2E2, S2H2, V1E2, N2E2, S2E2, H2, V2H2, S1H2, N1E2, S1E2, V1H2, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(N1W1, { // ┘
		{V1, V1W1, V1W2, S1W2, S1W1, S1H1, V1E1, V1H1, V1E2, S1H2, S1E2, V1H2, S1E1, },		// north
		{EMPTY, V2E2, V1, V2, N1E1, V1E1, V1E2, V2E1, N2E2, S2E2, N2E1, N1E2, S1E2, S2E1, S1E1, },		// east
		{EMPTY, S2H2, S2W1, S1W2, S2W2, S1W1, S1H1, H1, S2E2, H2, S1H2, S2H1, S1E2, S2E1, S1E1, },		// south
		{N1E1, N1H1, S1H1, V1E1, H1, V1H1, V2E1, N2H1, S2H1, N2E1, S2E1, V2H1, S1E1, } }));	// west
	constraints.insert(pair<uChars,vector<vector<uChars>>>(S1E1, { // ┌
		{EMPTY, N2H2, N2W2, N2W1, N1W2, N1E1, N1H1, H1, N2E2, H2, N1H2, N2H1, N2E1, N1E2, N1W1, },		// north
		{V1W1, V2W1, S2W1, N2W1, S1W1, N1H1, S1H1, H1, V1H1, N2H1, S2H1, V2H1,N1W1, },		// east
		{V1, V1W1, V1W2, N1W2, N1E1, N1H1, V1E1, V1H1, V1E2, N1H2, N1E2, V1H2, N1W1, },		// south
		{EMPTY, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1W1, } }));	// west





	maxEntropy = constraints.size();
}

#pragma endregion