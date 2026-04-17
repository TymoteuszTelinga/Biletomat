#include <gtest/gtest.h>
#include "CashRegister.h"
#include "Biletomat.h"

class TestClient : public ::testing::Test
{
protected:

    void SetUp() override
    {
        Tickets[0] = { 8, 170, "Ulgowy" };
        Tickets[1] = { 3, 270, "Normalny" };

        svr.Get("/tickets", [&](const httplib::Request& req, httplib::Response& res)
            {
                nlohmann::json ticketsData = Tickets;
                res.set_content(ticketsData.dump(), "application/json");
            });

        svr.Post("/book", [&](const httplib::Request& req, httplib::Response& res) 
            {
                ClientRequest request = nlohmann::json::parse(req.body);
                auto it = Tickets.find(request.TicketID);
                uint32_t returnID = it != Tickets.end() ? request.TicketID : -1;
                nlohmann::json response = { {"id", returnID} };
                res.set_content(response.dump(), "application/json");
            });

        worker = std::thread([this]() {svr.listen("localhost", 8080);});
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    void TearDown() override
    {
        svr.stop();
        if (worker.joinable())
            worker.join();
    }

    httplib::Server svr;
    std::thread worker;
    std::unordered_map<uint32_t, TicketData> Tickets;
};


TEST(CashRegisterTest, CorectChange)
{
    CashRegister regiser;
    uint32_t coins[6] = { 0,1,5,2,0,0 };
    regiser.SetState(coins);

    uint32_t input[6] = { 1,0,0,0,0,0 };
    bool isSucesful = regiser.GetChange(150, input);

    EXPECT_TRUE(isSucesful);
}

TEST(CashRegisterTest, NotInafChange)
{
    CashRegister regiser;
    uint32_t coins[6] = { 0,10,0,0,0,0 };
    regiser.SetState(coins);

    uint32_t input[6] = { 1,0,0,0,0,0 };
    bool isSucesful = regiser.GetChange(220, input);

    EXPECT_FALSE(isSucesful);
}

TEST(ClientTest, NoConectionToServer)
{
    BiletomatCli client;
    auto result = client.GetAvailableTickets();
    bool bBookingResult = client.BookTicket(0);
    
    EXPECT_TRUE(result.empty());
    EXPECT_FALSE(bBookingResult);
}

TEST_F(TestClient, ConectionToServer)
{
    BiletomatCli client;
    auto result = client.GetAvailableTickets();

    ASSERT_FALSE(result.empty());
    TicketData testTicket = result.at(0);
    EXPECT_EQ(testTicket.Count, 8);
    EXPECT_EQ(testTicket.Name, "Ulgowy");
    EXPECT_EQ(testTicket.Cost, 170);
}

TEST_F(TestClient, BookingTicket)
{
    BiletomatCli client;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool bBooking0 = client.BookTicket(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool bBooking234 = client.BookTicket(234);

    EXPECT_TRUE(bBooking0);
    EXPECT_FALSE(bBooking234);
}