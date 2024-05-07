#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <pqxx/pqxx>

#include "main.hpp"

pqxx::result executeRead(string sql);
void executeCUD(string sql);
int getTruckTopickup();
void clearTables();
#endif