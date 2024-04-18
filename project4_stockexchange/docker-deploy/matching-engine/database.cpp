#include "database.hpp"

int order_id = 0;
std::mutex db_mutex;

std::time_t timestampToEpoch(const std::string & timestampStr) {
  std::tm tm = {};
  std::istringstream ss(timestampStr);
  ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  tm.tm_isdst = 0;
  return std::mktime(&tm);
}

connection * connectDatabase() {
  connection * conn;
  try {
    conn = new connection(
        "host=db port=5432 dbname=enginedb user=postgres password=postgres");
    //"host=localhost port=5432 dbname=enginedb user=postgres password=postgres");
    if (conn->is_open()) {
      std::cout << "Opened database successfully: " << conn->dbname() << std::endl;
    }
    else {
      std::cout << "Can't open database" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  return conn;
}

void dropTables(connection * C) {
  stringstream sql;
  sql << "DROP TABLE IF EXISTS SYMBOL CASCADE;";
  sql << "DROP TABLE IF EXISTS ORDERS CASCADE;";
  sql << "DROP TABLE IF EXISTS TRANSACTIONS CASCADE;";
  sql << "DROP TABLE IF EXISTS ACCOUNT CASCADE;";

  work W(*C);
  W.exec(sql.str());
  W.commit();
}

void createTables(connection * C) {
  stringstream sql;

  // ACCOUNT table
  sql << "CREATE TABLE ACCOUNT ("
      << "ACCOUNT_ID CHAR(20) PRIMARY KEY NOT NULL, "
      << "BALANCE DECIMAL(15, 2) NOT NULL CHECK (BALANCE >= 0));";

  // TRANSACTIONS table
  sql << "CREATE TABLE TRANSACTIONS ("
      << "TRANSACTION_ID BIGSERIAL PRIMARY KEY, "
      << "SYMBOL VARCHAR(20) NOT NULL, "
      << "QUANTITY DECIMAL NOT NULL, "
      << "PRICE_PER_UNIT DECIMAL(10, 2) NOT NULL, "
      << "TRANSACTION_TYPE VARCHAR(10) NOT NULL CHECK (TRANSACTION_TYPE IN "
         "('EXECUTED', 'CANCELED')), "
      << "BUYER_ACCOUNT_ID CHAR(20), "
      << "SELLER_ACCOUNT_ID CHAR(20), "
      << "EXECUTION_TIME TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, "
      << "FOREIGN KEY (BUYER_ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID) ON "
         "DELETE CASCADE, "
      << "FOREIGN KEY (SELLER_ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID) ON "
         "DELETE CASCADE);";

  // ORDERS table
  sql << "CREATE TABLE ORDERS ("
      << "UNIQUE_ID BIGSERIAL PRIMARY KEY, "
      << "ORDER_ID BIGINT NOT NULL, "
      << "ACCOUNT_ID CHAR(20) NOT NULL, "
      << "SYMBOL TEXT NOT NULL, "
      << "ORDER_QUANTITY DECIMAL NOT NULL, "
      << "LIMIT_PRICE DECIMAL(10, 2) NOT NULL, "
      << "ORDER_STATUS VARCHAR(15) NOT NULL CHECK (ORDER_STATUS IN ('OPEN', 'EXECUTED',"
         "'CLOSED')), "
      << "TRANSACTION_ID BIGINT, "
      << "CREATED_AT TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, "
      << "FOREIGN KEY (ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID) ON DELETE "
         "CASCADE, "
      << "FOREIGN KEY (TRANSACTION_ID) REFERENCES TRANSACTIONS(TRANSACTION_ID) ON DELETE "
         "SET NULL);";

  // SYMBOL table
  sql << "CREATE TABLE SYMBOL ("
      << "SYMBOL_ID VARCHAR(20) NOT NULL, "
      << "ACCOUNT_ID CHAR(20) NOT NULL, "
      << "SHARE_COUNT DECIMAL NOT NULL CHECK (SHARE_COUNT >= 0), "
      << "PRIMARY KEY (SYMBOL_ID, ACCOUNT_ID), "
      << "FOREIGN KEY (ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID) ON DELETE "
         "CASCADE);";

  work W(*C);
  W.exec(sql.str());
  W.commit();
}

// trim and format floating point numbers for SQL insertion
string format_decimal(const double value) {
  std::ostringstream oss;
  oss << std::fixed << value;
  std::string str = oss.str();
  str.erase(str.find_last_not_of('0') + 1, std::string::npos);
  if (str.back() == '.') {
    str.pop_back();
  }
  return str;
}

void insertAccount(connection * C, const string & accountId, double balance) {
  // protect database access in a multi-threaded context
  std::lock_guard<std::mutex> guard(db_mutex);
  work W(*C);
  std::string query = "INSERT INTO ACCOUNT (ACCOUNT_ID, BALANCE) VALUES (" +
                      W.quote(accountId) + ", " + W.quote(format_decimal(balance)) + ");";
  W.exec(query);
  W.commit();
}

void insertSymbol(connection * C,
                  const string & accountId,
                  const string & symbolId,
                  double shareCount) {
  std::lock_guard<std::mutex> guard(db_mutex);
  work W(*C);
  std::string query = "INSERT INTO SYMBOL (SYMBOL_ID, ACCOUNT_ID, SHARE_COUNT) VALUES (" +
                      W.quote(symbolId) + ", " + W.quote(accountId) + ", " +
                      W.quote(format_decimal(shareCount)) +
                      ") ON CONFLICT (SYMBOL_ID, ACCOUNT_ID) DO UPDATE SET SHARE_COUNT = "
                      "EXCLUDED.SHARE_COUNT;";
  W.exec(query);
  W.commit();
}

int insertOrder(pqxx::work & W,
                const string & accountId,
                const string & symbol,
                double quantity,
                double limitPrice) {
  std::lock_guard<std::mutex> guard(db_mutex);
  // check if account exists
  result accountCheck =
      W.exec("SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID = " + W.quote(accountId));
  if (accountCheck.empty()) {
    return 0;
  }
  double accountBalance = accountCheck[0][0].as<double>();
  // preparing for symbol checks
  result symbolCheck;
  double sharesOwned = 0;
  // for buy orders, check if balance is sufficient
  if (quantity >= 0) {
    double totalCost = quantity * limitPrice;
    if (accountBalance < totalCost) {
      return 1;
    }
  }
  // for sell orders, check if symbol is owned and shares are sufficient
  else {
    symbolCheck =
        W.exec("SELECT SHARE_COUNT FROM SYMBOL WHERE ACCOUNT_ID = " + W.quote(accountId) +
               " AND SYMBOL_ID = " + W.quote(symbol));
    if (symbolCheck.empty()) {
      return 2;
    }
    sharesOwned = symbolCheck[0][0].as<double>();
    if (std::abs(quantity) > sharesOwned) {
      return 3;
    }
  }

  std::string query =
      "INSERT INTO ORDERS (ORDER_ID, ACCOUNT_ID, SYMBOL, "
      "ORDER_QUANTITY, LIMIT_PRICE, ORDER_STATUS, CREATED_AT) VALUES ( -1, " +
      W.quote(accountId) + ", " + W.quote(symbol) + ", " +
      W.quote(format_decimal(quantity)) + ", " + W.quote(format_decimal(limitPrice)) +
      ", 'OPEN', NOW()) "
      "RETURNING UNIQUE_ID;";
  // get the newest unique id
  pqxx::result get_unique_id = W.exec(query);
  int unique_id = get_unique_id[0][0].as<int>();
  // now update the OrderID
  std::string updateOrderID =
      "UPDATE ORDERS SET ORDER_ID = " + std::to_string(unique_id) +
      " WHERE UNIQUE_ID = " + std::to_string(unique_id) + ";";
  W.exec(updateOrderID);
  return 4;
}

// void insertTransaction(connection * C,
//                        const string & symbol,
//                        double quantity,
//                        double pricePerUnit,
//                        const string & transactionType,
//                        const string & buyerAccountId,
//                        const string & sellerAccountId) {
//   std::lock_guard<std::mutex> guard(db_mutex);
//   work W(*C);
//   std::string query = "INSERT INTO TRANSACTIONS (SYMBOL, QUANTITY, "
//                       "PRICE_PER_UNIT, TRANSACTION_TYPE, BUYER_ACCOUNT_ID, "
//                       "SELLER_ACCOUNT_ID, EXECUTION_TIME) VALUES (" +
//                       W.quote(symbol) + ", " + W.quote(format_decimal(quantity)) + ", " +
//                       W.quote(format_decimal(pricePerUnit)) + ", " +
//                       W.quote(transactionType) + ", " + W.quote(buyerAccountId) + ", " +
//                       W.quote(sellerAccountId) + ", NOW());";
//   W.exec(query);
//   W.commit();
// }

// parse <create> XML data, interact with database, and generate XML response indicating if success or failure
string handle_creation(const string & xml, connection * C) {
  //cout << "xml: " << xml << endl;
  XMLDocument doc;
  XMLError parseResult = doc.Parse(xml.c_str());

  if (parseResult != XML_SUCCESS) {
    cout << "Error parsing XML: " << doc.ErrorStr() << endl;
    return "<?xml version=\"1.0\"?>\n<results>\n    <error>Invalid XML format passed "
           "in</error>\n</results>\n";
  }
  XMLElement * elmtRoot = doc.FirstChildElement("create");
  // cout << "elmtRoot: " << elmtRoot << endl;
  if (elmtRoot == nullptr) {
    return "<?xml version=\"1.0\"?>\n<results>\n    <error>Missing create "
           "tag</error>\n</results>\n";
  }
  string response = "<?xml version=\"1.0\"?>\n<results>\n";
  for (XMLElement * child = elmtRoot->FirstChildElement(); child != nullptr;
       child = child->NextSiblingElement()) {
    string elementName = child->Name();
    if (elementName == "account") {
      string accountId = child->Attribute("id");
      double balance = child->DoubleAttribute("balance");
      try {
        insertAccount(C, accountId, balance);
        response += "    <created id=\"" + accountId + "\"/>\n";
      }
      catch (const std::exception & e) {
        response += "    <error id=\"" + accountId + "\">" + e.what() + "</error>\n";
      }
    }
    else if (elementName == "symbol") {
      string symbolId = child->Attribute("sym");
      for (XMLElement * accountChild = child->FirstChildElement("account");
           accountChild != nullptr;
           accountChild = accountChild->NextSiblingElement("account")) {
        string accountId = accountChild->Attribute("id");
        double shareCount = std::stod(accountChild->GetText());
        try {
          insertSymbol(C, accountId, symbolId, shareCount);
          response +=
              "    <created sym=\"" + symbolId + "\" id=\"" + accountId + "\"/>\n";
        }
        catch (const std::exception & e) {
          response += "    <error sym=\"" + symbolId + "\" id=\"" + accountId + "\">" +
                      e.what() + "</error>\n";
        }
      }
    }
    else {
      response += "    <error>Unknown element: " + elementName + "</error>\n";
    }
  }
  response += "</results>\n";
  return response;
}

// string query(pqxx::work & W, const string & orderId) {
//   std::lock_guard<std::mutex> guard(db_mutex);
//   string response = "<status id=\"" + orderId + "\">\n";
//   // query open orders from ORDERS table
//   try {
//     result R =
//         W.exec("SELECT ORDER_QUANTITY FROM ORDERS WHERE ORDER_ID = " + W.quote(orderId) +
//                " AND ORDER_STATUS = 'OPEN'");
//     if (!R.empty()) {
//       for (const auto & row : R) {
//         double shares = row["ORDER_QUANTITY"].as<double>();
//         response += "<open shares=\"" + format_decimal(shares) + "\"/>\n";
//       }
//     }
//     // query executed transactions from TRANSACTIONS table
//     result R2 = W.exec("SELECT QUANTITY, PRICE_PER_UNIT, EXTRACT(EPOCH FROM "
//                        "EXECUTION_TIME) AS TIME "
//                        "FROM TRANSACTIONS WHERE (BUYER_ACCOUNT_ID = " +
//                        W.quote(orderId) + " OR SELLER_ACCOUNT_ID = " + W.quote(orderId) +
//                        ") AND TRANSACTION_TYPE = 'EXECUTED'");
//     for (const auto & row : R2) {
//       double shares = row["QUANTITY"].as<double>();
//       double price = row["PRICE_PER_UNIT"].as<double>();
//       string time = std::to_string(row["TIME"].as<long long>());
//       response += "<executed shares=\"" + format_decimal(shares) + "\" price=\"" +
//                   format_decimal(price) + "\" time=\"" + time + "\"/>\n";
//     }
//     // query canceled transactions from TRANSACTIONS table
//     result R3 = W.exec("SELECT QUANTITY, EXTRACT(EPOCH FROM EXECUTION_TIME) AS TIME "
//                        "FROM TRANSACTIONS WHERE (BUYER_ACCOUNT_ID = " +
//                        W.quote(orderId) + " OR SELLER_ACCOUNT_ID = " + W.quote(orderId) +
//                        ") AND TRANSACTION_TYPE = 'CANCELED'");
//     for (const auto & row : R3) {
//       double shares = row["QUANTITY"].as<double>();
//       string time = std::to_string(row["TIME"].as<long long>());
//       response +=
//           "<canceled shares=\"" + format_decimal(shares) + "\" time=\"" + time + "\"/>\n";
//     }
//   }
//   catch (const std::exception & e) {
//     return "<error id=\"" + to_string(orderId) +
//            "\">Failed to query status: " + string(e.what()) + "</error>\n";
//   }
//   response += "</status>\n";
//   return response;
// }

string cancelOrder(pqxx::work & W, const string & orderId) {
  std::lock_guard<std::mutex> guard(db_mutex);
  string response = "<canceled id=\"" + orderId + "\">\n";
  try {
    // check if the order exists and is open
    result R = W.exec("SELECT ACCOUNT_ID, SYMBOL, ORDER_QUANTITY, LIMIT_PRICE, "
                      "ORDER_STATUS FROM ORDERS WHERE ORDER_ID = " +
                      W.quote(orderId));
    if (R.empty()) {
      return "    <error id=\"" + to_string(orderId) + "\">Order not found</error>\n";
    }
    else {
      auto row = R[0];
      string accountId = row["ACCOUNT_ID"].as<string>();
      string symbol = row["SYMBOL"].as<string>();
      double quantity = row["ORDER_QUANTITY"].as<double>();
      double limitPrice = row["LIMIT_PRICE"].as<double>();
      string orderStatus = row["ORDER_STATUS"].as<string>();
      if (orderStatus != "OPEN") {
        return "    <error id=\"" + to_string(orderId) + "\">Order is not open</error>\n";
        ;
      }
      // cancel the order, update order status to 'CLOSED'
      W.exec0("UPDATE ORDERS SET ORDER_STATUS = 'CLOSED' WHERE ORDER_ID = " +
              W.quote(orderId));
      // refund process
      if (orderStatus == "OPEN") {
        double refundAmount = quantity * limitPrice;
        W.exec0("UPDATE ACCOUNT SET BALANCE = BALANCE + " +
                W.quote(format_decimal(refundAmount)) +
                " WHERE ACCOUNT_ID = " + W.quote(accountId));
      }
    }
    response += "</canceled>\n";
    return response;
  }
  catch (const std::exception & e) {
    return "    <error id=\"" + to_string(orderId) +
           "\">Failed to cancel order: " + string(e.what()) + "</error>\n";
  }
}

string handle_transactions(const string & xml, connection * C) {
  XMLDocument doc;
  doc.Parse(xml.c_str());
  XMLElement * elmtRoot = doc.FirstChildElement("transactions");
  if (elmtRoot == nullptr) {
    return "<?xml version=\"1.0\"?>\n<results>\n    <error>Missing transactions "
           "tag</error>\n</results>\n";
  }
  string response = "<?xml version=\"1.0\"?>\n<results>\n";
  string accountId = elmtRoot->Attribute("id");
  // cout << "accountId: " << accountId << endl;

  // validate account ID
  work W(*C);
  result R =
      W.exec("SELECT COUNT(*) FROM ACCOUNT WHERE ACCOUNT_ID = " + W.quote(accountId));
  if (R[0][0].as<int>() == 0) {
    W.abort();
    return "<?xml version=\"1.0\"?>\n<results>\n    <error id=\"" + accountId +
           "\">Account does not exist</error>\n</results>\n";
  }
  try {
    // cout << "enter1" << endl;
    for (XMLElement * child = elmtRoot->FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      // cout << "enter2" << endl;
      string elementName = child->Name();
      if (elementName == "order") {
        // cout << "enter3" << endl;
        string symbol = child->Attribute("sym");
        double amount = child->DoubleAttribute("amount");
        double limit = child->DoubleAttribute("limit");
        int insert_result = insertOrder(W, accountId, symbol, amount, limit);
        if (insert_result == 4) {
          order_id += 1;
          response += "    <opened sym=\"" + symbol + "\" amount=\"" + to_string(amount) +
                      "\" limit=\"" + to_string(limit) + "\" id=\"" +
                      to_string(order_id) + "\"/>\n";
          //cout << "created successfully" << endl;
          match_orders(W, symbol);
          //cout << "reached here" << endl;
        }
        else {
          if (insert_result == 0) {
            response += "    <error>Account does not exist</error>\n";
          }
          if (insert_result == 1) {
            response += "    <error>Balance is not sufficient</error>\n";
          }
          if (insert_result == 2) {
            response += "    <error>Symbol is not owned by account</error>\n";
          }
          if (insert_result == 3) {
            response += "    <error>Insufficient shares for this sell</error>\n";
          }
        }
      }
      else if (elementName == "cancel") {
        // cout << "enter4" << endl;
        string orderId = child->Attribute("id");
        response += cancelOrder(W, orderId);
      }
      else if (elementName == "query") {
        // cout << "enter query" << endl;
        string orderId = child->Attribute("id");
        result checkOrder =
            W.exec("SELECT COUNT(*) FROM ORDERS WHERE ORDER_ID = " + W.quote(orderId) +
                   " AND ACCOUNT_ID = " + W.quote(accountId) + ";");
        if (checkOrder[0][0].as<int>() == 0) {
          response += "    <error>Order ID (" + orderId +
                      ") does not exist for account ID (" + accountId + ") </error>\n";
        }
        else {
          response += queryAll(W, orderId);
        }
        // cout << response << endl;
      }
      else {
        // cout << "enter6" << endl;
        response += "    <error>Unknown operation: " + elementName + "</error>\n";
      }
    }
    W.commit();
  }
  catch (const std::exception & e) {
    // cout << "enter7" << endl;
    W.abort();
    return "    <error>" + string(e.what()) + "</error>\n";
  }
  response += "</results>\n";
  return response;
}

// here I write a function to match orders
void match_orders(pqxx::work & W, const string & symbol) {
  // fetch all open buy orders and sort them by price and time
  pqxx::result buy_orders =
      W.exec("SELECT UNIQUE_ID, ORDER_ID, ACCOUNT_ID, SYMBOL, ORDER_QUANTITY, "
             "LIMIT_PRICE, CREATED_AT "
             "FROM ORDERS WHERE SYMBOL = " +
             W.quote(symbol) +
             " AND ORDER_STATUS = 'OPEN' "
             "AND ORDER_QUANTITY > 0 ORDER BY LIMIT_PRICE DESC, CREATED_AT ASC;");

  // for all the open buys
  for (pqxx::result::const_iterator buy = buy_orders.begin(); buy != buy_orders.end();
       ++buy) {
    int buy_unique_id = buy["UNIQUE_ID"].as<int>();
    int buy_order_id = buy["ORDER_ID"].as<int>();
    double buy_price = buy["LIMIT_PRICE"].as<double>();
    // double buy_amount = buy["ORDER_QUANTITY"].as<double>();
    // cout << "buy_amount: " << buy_amount << endl;

    std::string buy_time_string = buy["CREATED_AT"].as<std::string>();
    std::time_t buy_time = timestampToEpoch(buy_time_string);

    // fetch all open sell orders and sort them by price and time
    pqxx::result sell_orders =
        W.exec("SELECT UNIQUE_ID, ORDER_ID, ACCOUNT_ID, SYMBOL, ORDER_QUANTITY, "
               "LIMIT_PRICE, CREATED_AT "
               "FROM ORDERS WHERE SYMBOL = " +
               W.quote(symbol) +
               " AND ORDER_STATUS = 'OPEN'"
               " AND LIMIT_PRICE <= " +
               W.quote(buy_price) +
               " AND ORDER_QUANTITY < 0 ORDER BY LIMIT_PRICE ASC, CREATED_AT ASC;");

    // for all the open sells
    for (pqxx::result::const_iterator sell = sell_orders.begin();
         sell != sell_orders.end();
         ++sell) {
      pqxx::result new_buy =
          W.exec("SELECT ORDER_QUANTITY FROM ORDERS WHERE UNIQUE_ID = " +
                 W.quote(buy_unique_id) + ";");
      double buy_amount = new_buy[0]["ORDER_QUANTITY"].as<double>();
      // cout << "buy_amount: " << buy_amount << endl;
      int sell_unique_id = sell["UNIQUE_ID"].as<int>();
      int sell_order_id = sell["ORDER_ID"].as<int>();
      // use the positve quantity
      double sell_amount = std::abs(sell["ORDER_QUANTITY"].as<double>());
      double sell_price = sell["LIMIT_PRICE"].as<double>();
      std::string sell_time_string = sell["CREATED_AT"].as<std::string>();
      std::time_t sell_time = timestampToEpoch(buy_time_string);

      // cout << "sell_amount: " << sell_amount << endl;

      // for the first matched pair
      if (buy_price >= sell_price) {
        // cout << "find a matched pair: " << endl;
        // cout << "buy_price: " << buy_price << endl;
        // cout << "buy_amount: " << buy_amount << endl;
        // cout << "sell_price: " << sell_price << endl;
        // cout << "sell_amount: " << sell_amount << endl;

        // sell at the buyer's price
        double execution_price =
            (buy_unique_id > sell_unique_id) ? buy_price : sell_price;
        // cout << "buy_time: " << buy_time << endl;
        // cout << "sell_time: " << sell_time << endl;
        // cout << "buy_price: " << buy_price << endl;
        // cout << "sell_price: " << sell_price << endl;
        // cout << "execution_price: " << execution_price << endl;
        // the amount should be the minimum
        double execution_amount = std::min(buy_amount, sell_amount);
        // cout << "execution_amount: " << execution_amount << endl;

        long transaction_id = execute_trade(
            W, buy_unique_id, sell_unique_id, execution_amount, execution_price);

        // update
        update_order_status(
            W, buy_unique_id, execution_amount, execution_price, transaction_id);
        update_order_status(
            W, sell_unique_id, execution_amount, execution_price, transaction_id);

        // if buy order is completed, break to next buy order
        if (buy_amount == execution_amount) {
          break;
        }
      }
      // continue for the next seller
    }
    // continue for the next buyer
  }
  //W.commit();
}

long execute_trade(pqxx::work & W,
                   int buy_unique_id,
                   int sell_unique_id,
                   double execution_amount,
                   double execution_price) {
  std::lock_guard<std::mutex> guard(db_mutex);
  // cout << "buy_unique_id: " << buy_unique_id << endl;
  // cout << "sell_unique_id: " << sell_unique_id << endl;
  // cout << "execution_amount: " << execution_amount << endl;
  // cout << "execution_price: " << execution_price << endl;

  // add the current transaction
  pqxx::result get_trans_id =
      W.exec("INSERT INTO TRANSACTIONS (BUYER_ACCOUNT_ID, SELLER_ACCOUNT_ID, SYMBOL, "
             "QUANTITY, PRICE_PER_UNIT, TRANSACTION_TYPE, EXECUTION_TIME) "
             "VALUES ("
             "(SELECT ACCOUNT_ID FROM ORDERS WHERE UNIQUE_ID = " +
             W.quote(buy_unique_id) +
             "), "
             "(SELECT ACCOUNT_ID FROM ORDERS WHERE UNIQUE_ID = " +
             W.quote(sell_unique_id) +
             "), "
             "(SELECT SYMBOL FROM ORDERS WHERE UNIQUE_ID = " +
             W.quote(buy_unique_id) + "), " + W.quote(execution_amount) + ", " +
             W.quote(execution_price) + ", " +
             "'EXECUTED', "
             "NOW()"
             ") RETURNING TRANSACTION_ID;");
  // return the transaction id
  long transaction_id = get_trans_id[0][0].as<long>();
  return transaction_id;
}

void update_order_status(pqxx::work & W,
                         int unique_id,
                         double execution_amount,
                         double execution_price,
                         long transaction_id) {
  // cout << "unique_id: " << unique_id << endl;
  // cout << "execution_amount: " << execution_amount << endl;
  // cout << "execution_price: " << execution_price << endl;

  std::lock_guard<std::mutex> guard(db_mutex);

  // use query to find the row
  pqxx::result update_order =
      W.exec("SELECT ORDER_ID, ORDER_QUANTITY, ACCOUNT_ID, SYMBOL, LIMIT_PRICE FROM "
             "ORDERS WHERE UNIQUE_ID = " +
             W.quote(unique_id) + ";");
  int order_id = update_order[0]["ORDER_ID"].as<int>();
  double order_amount = update_order[0]["ORDER_QUANTITY"].as<double>();
  std::string account_id = update_order[0]["ACCOUNT_ID"].as<std::string>();
  std::string symbol = update_order[0]["SYMBOL"].as<std::string>();
  double limit_price = update_order[0]["LIMIT_PRICE"].as<double>();

  int isSell = 0;
  if (order_amount < 0) {
    // make sure the amount is postive for both buy and sell
    order_amount = std::abs(order_amount);
    isSell = 1;
  }
  // cout << "order_amount: " << order_amount << endl;
  // cout << "execution_amount: " << execution_amount << endl;

  // calculate the remain
  double remain_amount = order_amount - execution_amount;
  // cout << "remain_amount: " << remain_amount << endl;

  // if the order is fully executed
  if (remain_amount == 0) {
    W.exec("UPDATE ORDERS SET ORDER_STATUS = 'EXECUTED', LIMIT_PRICE = " +
           W.quote(execution_price) + ", TRANSACTION_ID = " + W.quote(transaction_id) +
           " WHERE UNIQUE_ID = " + W.quote(unique_id) + ";");
  }
  // if the order is partially executed
  else if (remain_amount > 0) {
    // make sure that the udpate amount and new amount are all negative now
    if (isSell) {
      remain_amount = 0 - remain_amount;
      execution_amount = 0 - execution_amount;
    }
    W.exec("UPDATE ORDERS SET ORDER_STATUS = 'EXECUTED', ORDER_QUANTITY = " +
           W.quote(execution_amount) + ", LIMIT_PRICE = " + W.quote(execution_price) +
           ", TRANSACTION_ID = " + W.quote(transaction_id) +
           " WHERE UNIQUE_ID = " + W.quote(unique_id) + ";");
    W.exec("INSERT INTO ORDERS (ORDER_ID, ACCOUNT_ID, SYMBOL, ORDER_QUANTITY, "
           "LIMIT_PRICE, ORDER_STATUS) "
           "VALUES (" +
           W.quote(order_id) + ", " + W.quote(account_id) + ", " + W.quote(symbol) +
           ", " + W.quote(remain_amount) + ", " + W.quote(limit_price) +
           ", "
           "'OPEN'"
           ");");
  }
  else {
    cerr << "The update order quantity is smaller than 0. " << endl;
  }
}

string queryAll(pqxx::work & W, const string & orderId) {
  // cout << "enter queryALL" << endl;
  std::lock_guard<std::mutex> guard(db_mutex);
  string response = "    <status id=\"" + orderId + "\">\n";

  // get all related orders from ORDERS table, including splits orders
  try {
    pqxx::result all_orders = W.exec(
        "SELECT UNIQUE_ID, ORDER_QUANTITY, LIMIT_PRICE, ORDER_STATUS, TRANSACTION_ID "
        "FROM ORDERS "
        "WHERE ORDER_ID = " +
        W.quote(orderId) +
        " "
        "ORDER BY CREATED_AT ASC;");

    for (pqxx::result::const_iterator order = all_orders.begin();
         order != all_orders.end();
         ++order) {
      double quantity = std::abs(order["ORDER_QUANTITY"].as<double>());
      double price = order["LIMIT_PRICE"].as<double>();
      string status = order["ORDER_STATUS"].as<string>();

      // handle open status
      if (status == "OPEN") {
        response += "      <open shares=\"" + format_decimal(quantity) + "\"/>\n";
      }
      else if (status == "EXECUTED") {
        long transaction_id = order["TRANSACTION_ID"].as<long>();
        pqxx::result get_transaction_time =
            W.exec("SELECT EXECUTION_TIME "
                   "FROM TRANSACTIONS "
                   "WHERE TRANSACTIONS.TRANSACTION_ID = " +
                   W.quote(transaction_id) + ";");
        // long transaction_time = get_transaction_time[0][0].as<long>();
        // std::string timestampStr = std::to_string(transaction_time);
        std::string execution_time =
            get_transaction_time[0]["EXECUTION_TIME"].as<std::string>();
        std::time_t epochTime = timestampToEpoch(execution_time);
        string time = std::to_string(epochTime);
        response += "      <executed shares=\"" + format_decimal(quantity) +
                    "\" price=\"" + format_decimal(price) + "\" time=\"" + time +
                    "\"/>\n";
      }
      else if (status == "CANCELED") {
        response += "      <canceled shares=\"" + format_decimal(quantity) + "\"/>\n";
      }
    }
  }
  catch (const pqxx::sql_error & e) {
    return "    <error id=\"" + orderId + "\">Failed to query status: " + e.what() +
           "</error>\n";
  }

  response += "    </status>\n";
  return response;
}

string handle_client_request(const string & input, connection * C) {
  std::istringstream stream(input);
  std::string line;
  std::string xml;
  std::getline(stream, line);
  std::getline(stream, line);
  while (std::getline(stream, line)) {
    xml += line + "\n";
  }
  XMLDocument doc;
  doc.Parse(xml.c_str());
  if (doc.Error()) {
    return "<?xml version=\"1.0\"?>\n<results>\n    <error>Invalid XML "
           "format</error>\n</results>\n";
  }
  XMLElement * createRoot = doc.FirstChildElement("create");
  if (createRoot != nullptr) {
    return handle_creation(xml, C);
  }
  XMLElement * transactionsRoot = doc.FirstChildElement("transactions");
  if (transactionsRoot != nullptr) {
    return handle_transactions(xml, C);
  }
  return "<?xml version=\"1.0\"?>\n<results>\n    <error>Invalid XML input, only accept "
         "creation or "
         "transactions</error>\n</results>\n";
}

// int main() {
//   connection * C = connectDatabase();
//   dropTables(C);
//   createTables(C);
//   const string xmlData =
//       R"(
//       <create>
//         <account id="123456" balance="100"/>
//         <symbol sym="APY">
//           <account id="123456">1000</account>
//         </symbol>
//       </create>)";
//       string response = handle_creation(xmlData, C);
//   cout << "XML Response:\n" << response << endl;
//   const string xmlTransactions = R"(
//     <transactions id="123456">
//         <order sym="APY" amount="10" price="1500"/>
//         <query id="1"/>
//         <cancel id="1"/>
//     </transactions>
//     )";
//   string transactionResponse = handle_transactions(xmlTransactions, C);
//   cout << "Transaction XML Response:\n" << transactionResponse << endl;

//     const string xmlTransactions1 = R"(
//     <transactions id="123456">
//         <order sym="X" amount="300" price="125"/>
//     </transactions>
//     )";
//   string transactionResponse1 = handle_transactions(xmlTransactions1, C);
//     cout << "Transaction XML Response:\n" << transactionResponse1 << endl;
//       const string xmlTransactions2 = R"(
//     <transactions id="123456">
//         <order sym="X" amount="-100" price="130"/>
//     </transactions>
//     )";
//   string transactionResponse2 = handle_transactions(xmlTransactions2, C);
//       cout << "Transaction XML Response:\n" << transactionResponse2 << endl;
//       const string xmlTransactions3 = R"(
//     <transactions id="123456">
//         <order sym="X" amount="200" price="127"/>
//     </transactions>
//     )";
//   string transactionResponse3 = handle_transactions(xmlTransactions3, C);
//       cout << "Transaction XML Response:\n" << transactionResponse3 << endl;
//   const string xmlTransactions4 = R"(
//   <transactions id="123456">
//       <order sym="X" amount="-500" price="128"/>
//   </transactions>
//   )";
//   string transactionResponse4 = handle_transactions(xmlTransactions4, C);
//   cout << "Transaction XML Response:\n" << transactionResponse4 << endl;
//   const string xmlTransactions5 = R"(
//   <transactions id="123456">
//       <order sym="X" amount="-200" price="140"/>
//   </transactions>
//   )";
//   string transactionResponse5 = handle_transactions(xmlTransactions5, C);
//   cout << "Transaction XML Response:\n" << transactionResponse5 << endl;
//   const string xmlTransactions6 = R"(
//   <transactions id="123456">
//       <order sym="X" amount="400" price="125"/>
//       <query id="2"/>
//       <query id="4"/>
//   </transactions>
//   )";
//   string transactionResponse6 = handle_transactions(xmlTransactions6, C);
//   cout << "Transaction XML Response:\n" << transactionResponse6 << endl;
//   const string xmlTransactions7 = R"(
//   <transactions id="123456">
//       <order sym="X" amount="-400" price="124"/>
//       <query id="2"/>
//       <query id="4"/>
//       <query id="8"/>
//   </transactions>
//   )";
//   string transactionResponse7 = handle_transactions(xmlTransactions7, C);
//   cout << "Transaction XML Response:\n" << transactionResponse7 << endl;

//   // now test for the order matches
//   // work W1(*C);
//   // insertOrder(W1, "123456", "X", 300, 125);
//   // insertOrder(W1, "123456", "X", -100, 130);
//   // insertOrder(W1, "123456", "X", 200, 127);
//   // insertOrder(W1, "123456", "X", -500, 128);
//   // insertOrder(W1, "123456", "X", -200, 140);
//   // insertOrder(W1, "123456", "X", 400, 125);
//   // match_orders(W1, "X");

//   // work W2(*C);
//   // string before8_2 = queryAll(W2, "2");
//   // string before8_4 = queryAll(W2, "4");
//   // cout << "This is before the 8th order: \n" << before8_2 << "\n" << before8_4 << endl;

//   // insertOrder(W2, "123456", "X", -400, 124);
//   // match_orders(W2, "X");

//   // work W3(*C);
//   // string after8_2 = queryAll(W3, "2");
//   // string after8_4 = queryAll(W3, "4");
//   // string after8_8 = queryAll(W3, "8");
//   // cout << "This is after the 8th order: \n"
//   //      << after8_2 << "\n"
//   //      << after8_4 << "\n"
//   //      << after8_8 << endl;
//   // W3.commit();

//   C->disconnect();
//   delete C;
//   return 0;
// }