#pragma once

#include <cstdint>
#include <filesystem>

enum Nominal
{
	zl5 = 0,
	zl2 = 1,
	zl1 = 2,
	gr50 = 3,
	gr20 = 4,
	gr10 = 5
};

class CashRegister
{
public:
	CashRegister();
	~CashRegister() {};

	void LoadState(const std::filesystem::path& path);
	bool GetChange(uint32_t returnAmount, uint32_t inputCoins[6]);
	void SetState(uint32_t state[6]);

	static uint32_t CoinMultiplayer(const Nominal nom);

private:
	uint32_t m_register[6];
};