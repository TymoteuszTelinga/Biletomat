#include <gtest/gtest.h>
#include <memory>

#include "CashRegister.h"
#include "Biletomat.h"
#include "StructDefiicions.h"
#include "Server.h"

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

class TestServer : public ::testing::Test
{
protected:

    void SetUp() override 
    {
        std::filesystem::path dataDir = TEST_DATA_DIR;
        dataDir += "/tickets.txt";
        //std::cout << "dataDir:" << dataDir<<std::endl;
        svr = std::make_unique<TicketServer>();
        svr->LoadData(dataDir);
        worker = std::thread([this]() { svr->Run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    void TearDown() override
    {
        svr->Stop();
        if (worker.joinable())
            worker.join();
    }

    std::unique_ptr<TicketServer> svr;
    std::thread worker;
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

TEST_F(TestServer, GetResponse)
{
    httplib::Client cli("localhost", 8080);
    auto res = cli.Get("/tickets");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->get_header_value("Content-Type"), "application/json");
}

TEST_F(TestServer, BookingSucesful)
{
    httplib::Client cli("localhost", 8080);
    ClientRequest request;
    request.ClientID = "testMachine";
    request.TicketID = 0;
    nlohmann::json requestJson = request;

    auto res = cli.Post("/book", requestJson.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    auto body = nlohmann::json::parse(res->body);
    EXPECT_EQ(body.at("id").get<uint32_t>(), 0);
}

TEST_F(TestServer, BookingFaild)
{
    httplib::Client cli("localhost", 8080);
    ClientRequest request;
    request.ClientID = "testMachine";
    request.TicketID = 1035;
    nlohmann::json requestJson = request;

    auto res = cli.Post("/book", requestJson.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    auto body = nlohmann::json::parse(res->body);
    EXPECT_EQ(body.at("id").get<uint32_t>(), uint32_t(-1));
}

TEST_F(TestServer, TicketBuying)
{
    httplib::Client cli("localhost", 8080);
    ClientRequest request;
    request.ClientID = "TicketBuying";
    request.TicketID = 0;
    nlohmann::json requestJson = request;

    auto resBook = cli.Post("/book", requestJson.dump(), "application/json");
    ASSERT_NE(resBook, nullptr);


    BookingData bookingData;
    bookingData.ClientID = request.ClientID;
    bookingData.TicketID = request.TicketID;
    bookingData.Name = "Test";
    bookingData.Surname = "Test";
    nlohmann::json bookingJson = bookingData;
    auto resConfirm = cli.Post("/confirm", bookingJson.dump(), "application/json");
    ASSERT_NE(resConfirm, nullptr);
    EXPECT_EQ(resConfirm->status, 200);
    ServerResponse response = nlohmann::json::parse(resConfirm->body);
    EXPECT_EQ(response.Status, 0);
}

TEST_F(TestServer, BookingNotEgsist)
{
    httplib::Client cli("localhost", 8080);

    BookingData bookingData;
    bookingData.ClientID = "BookingNotEgsist";
    bookingData.TicketID = 0;
    bookingData.Name = "Test";
    bookingData.Surname = "Test";
    nlohmann::json bookingJson = bookingData;
    auto resConfirm = cli.Post("/confirm", bookingJson.dump(), "application/json");
    ASSERT_NE(resConfirm, nullptr);
    EXPECT_EQ(resConfirm->status, 200);
    ServerResponse response = nlohmann::json::parse(resConfirm->body);
    EXPECT_EQ(response.Status, 1);
}

TEST_F(TestServer, ConfirmationMisMatch)
{
    httplib::Client cli("localhost", 8080);
    ClientRequest request;
    request.ClientID = "ConfirmationMisMatch";
    request.TicketID = 0;
    nlohmann::json requestJson = request;

    auto resBook = cli.Post("/book", requestJson.dump(), "application/json");
    ASSERT_NE(resBook, nullptr);


    BookingData bookingData;
    bookingData.ClientID = request.ClientID;
    bookingData.TicketID = 2;
    bookingData.Name = "Test";
    bookingData.Surname = "Test";
    nlohmann::json bookingJson = bookingData;
    auto resConfirm = cli.Post("/confirm", bookingJson.dump(), "application/json");
    ASSERT_NE(resConfirm, nullptr);
    EXPECT_EQ(resConfirm->status, 200);
    ServerResponse response = nlohmann::json::parse(resConfirm->body);
    EXPECT_EQ(response.Status, 2);
}