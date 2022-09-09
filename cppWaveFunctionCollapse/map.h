#pragma once
#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <tuple>
#include <chrono>
#include <thread>


enum uChars {
	// priority (V)ertical->(N)orth->(S)outh->(H)orizontal->(E)ast->(W)est
	EMPTY = 32,	//  
	LOW = 176,	// ░
	MID = 177,	// ▒
	DARK = 178,	// ▓
	V1 = 179,	// │
	V1W1 = 180,	// ┤
	V1W2 = 181,	// ╡
	V2W1 = 182,	// ╢
	S2W1 = 183,	// ╖
	S1W2 = 184,	// ╕
	V2W2 = 185,	// ╣
	V2 = 186,	// ║
	S2W2 = 187,	// ╗
	N2W2 = 188,	// ╝
	N2W1 = 189,	// ╜
	N1W2 = 190,	// ╛
	S1W1 = 191,	// ┐
	N1E1 = 192,	// └
	N1H1 = 193,	// ┴
	S1H1 = 194,	// ┬
	V1E1 = 195,	// ├
	H1 = 196,	// ─
	V1H1 = 197,	// ┼
	V1E2 = 198,	// ╞
	V2E1 = 199,	// ╟
	N2E2 = 200,	// ╚
	S2E2 = 201,	// ╔
	N2H2 = 202,	// ╩
	S2H2 = 203,	// ╦
	V2E2 = 204,	// ╠
	H2 = 205,	// ═
	V2H2 = 206,	// ╬
	N1H2 = 207,	// ╧
	N2H1 = 208,	// ╨
	S1H2 = 209,	// ╤
	S2H1 = 210,	// ╥
	N2E1 = 211,	// ╙
	N1E2 = 212,	// ╘
	S1E2 = 213,	// ╒
	S2E1 = 214,	// ╓
	V2H1 = 215,	// ╫
	V1H2 = 216,	// ╪
	N1W1 = 217,	// ┘
	S1E1 = 218,	// ┌
};

class Kernal {
public:
	uChars self{ DARK };
	int entropyCount();
	void assignRandomly();
	bool isCollapsed();
	bool updateEntropy(std::vector<uChars>);
	std::vector<uChars> entropy{EMPTY, N2H2, V2E2, S2H2, V2W2, V1, V1W1, V1W2, V2W1, S2W1, S1W2, V2, S2W2, N2W2, N2W1, N1W2, S1W1, N1E1, N1H1, S1H1, V1E1, H1, V1H1, V1E2, V2E1, N2E2, S2E2, H2, V2H2, N1H2, N2H1, S1H2, S2H1, N2E1, N1E2, S1E2, S2E1, V2H1, V1H2, N1W1, S1E1, };
	int w;
	int h;
	bool collapsed{ false };

private:

};

class Map {
public:
	Map(int, int);
	void draw();
	std::vector<uChars> getConstraints(uChars, int);
	void collapse(bool, int);

private:
	std::chrono::milliseconds dura{50}; // time to sleep for animation
	Kernal** board{ nullptr };
	int remainingKernals{-1};
	int height{ 0 };
	int width{ 0 };
	std::map<uChars, std::vector<std::vector<uChars>>> constraints;
	int maxEntropy;
	bool animate{ false };

	void chooseRandomFromAvailable(int, int);
	void findLowestEntropyCount(int&, int&);
	void spreadEntropy(int, int);
	void pupulateConstraints();
};