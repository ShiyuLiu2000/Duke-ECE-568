#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <pqxx/pqxx>
#include <sstream>
#include <string>

#include "tinyxml2.h"
using namespace pqxx;
using namespace std;
using namespace tinyxml2;
using std::string;

extern std::mutex db_mutex;  // mutex to ensure thread-safe DB access
std::time_t timestampToEpoch(const std::string & timestampStr);
connection * connectDatabase();
void dropTables(connection * C);
void createTables(connection * C);
string format_decimal(const double value);
void insertAccount(connection * C, const string & accountId, double balance);
void insertSymbol(connection * C,
                  const string & accountId,
                  const string & symbolId,
                  double shareCount);
int insertOrder(pqxx::work & W,
                 const string & accountId,
                 const string & symbol,
                 double quantity,
                 double limitPrice);
void insertTransaction(connection * C,
                       const string & symbol,
                       double quantity,
                       double pricePerUnit,
                       const string & transactionType,
                       const string & buyerAccountId,
                       const string & sellerAccountId);
string handle_creation(const string & xml, connection * C);
//string query(pqxx::work & W, const string & orderId);
string cancelOrder(pqxx::work & W, const string & orderId);
string handle_transactions(const string & xml, connection * C);

// here start handle the order matchings
void match_orders(pqxx::work & W, const string & symbol);
long execute_trade(pqxx::work & W,
                   int buy_order_id,
                   int sell_order_id,
                   double execution_amount,
                   double execution_price);
void update_order_status(pqxx::work & W,
                         int order_id,
                         double execution_amount,
                         double execution_price,
                         long transaction_id);
string queryAll(pqxx::work & W, const string & orderId);
string handle_client_request(const string & xml, connection * C);