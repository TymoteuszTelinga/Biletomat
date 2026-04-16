
#include "CashRegister.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

CashRegister::CashRegister()
{
    for (uint32_t i = 0; i < 6; i++)
    {
        m_register[i] = 0;
    }
}

void CashRegister::LoadState(const std::filesystem::path& path)
{
    std::ifstream file(path);
    nlohmann::json data;
    if (file.is_open())
    {
        file >> data;
        for (size_t i = 0; i < 6; i++)
        {
            m_register[i] = data.at(i).get<uint32_t>();
        }
    }
}

bool CashRegister::GetChange(uint32_t change, uint32_t inputCoins[6])
{
    if (change == 0)
    {
        //add coins to register
        for (size_t i = 0; i < 6; i++)
        {
            m_register[i] += inputCoins[i];
        }
        return true;
    }

    uint32_t tempRegister[6];
    uint32_t changeCoins[6];
    for (size_t i = 0; i < 6; i++)
    {
        tempRegister[i] = m_register[i] + inputCoins[i];
        changeCoins[i] = 0;
    }

    //gready aprouch
    for (uint32_t i = 0; i < 6; i++)
    {
        if (tempRegister[i] > 0)
        {
            uint32_t multiplayer = CoinMultiplayer(static_cast<Nominal>(i));
            uint32_t coinCount = 0;
            while ((coinCount + 1) * multiplayer <= change && coinCount <= tempRegister[i])
            {
                coinCount++;
            }
            changeCoins[i] = coinCount;
            change -= coinCount * multiplayer;
        }

        if (change == 0)
        {
            break;
        }
    }

    if (change != 0)
    {
        return false;
    }

    for (uint32_t i = 0; i < 6; i++)
    {
        m_register[i] = tempRegister[i] - changeCoins[i];
        //std::cout << "coin" << i << " count" << m_register[i]<<std::endl;
    }

    return true;
}

void CashRegister::SetState(uint32_t state[6])
{
    for (size_t i = 0; i < 6; i++)
    {
        m_register[i] = state[i];
    }
}

uint32_t CashRegister::CoinMultiplayer(const Nominal nom)
{
    switch (nom)
    {
    case Nominal::zl5: return 500;
    case Nominal::zl2: return 200;
    case Nominal::zl1: return 100;
    case Nominal::gr50: return 50;
    case Nominal::gr20: return 20;
    case Nominal::gr10: return 10;
    }
}
