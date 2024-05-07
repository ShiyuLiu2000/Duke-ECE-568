#include"database.hpp"

void executeCUD(string sql) {
  //  transaction<serializable,read_write> W(*C); 
  work W(*C);
  W.exec(sql);

  W.commit();
  cout<<sql<<endl;
}
result executeRead(string sql) {
  work W(*C);
  result R(W.exec(sql));
  W.commit();
  return R;
}

int getTruckTopickup() {
  int truck_id;
   while(1){


    string sql = "SELECT truck_id FROM \"UPS_truck\" WHERE status IN ('idle', 'delivering','arrive at warehouse');";
    result res = executeRead(sql);
    if(!res.empty()) {
        truck_id = res[0][0].as<int>();
  
        break;
    }
   }
      stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
 
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::stringstream sqlUpdate;
    sqlUpdate << "UPDATE \"UPS_truck\" SET ";
    string status="go to warehouse";
    sqlUpdate << "status = '" << status << "', ";
sqlUpdate << "ready_to_pickup_time = '" << ss.str() << "' ";
    sqlUpdate << "WHERE truck_id = " << truck_id << ";";
     mutexForTruck.lock();
    executeCUD( sqlUpdate.str());
    mutexForTruck.unlock();
  
  return truck_id;
}
void clearTables() {

    pqxx::work W(*C);
    // SQL to only select specific table names
    string sql = "SELECT tablename FROM pg_tables WHERE schemaname='public' AND tablename IN ('UPS_package', 'UPS_truck', 'UPS_delivery');";
    pqxx::result R = W.exec(sql);

    // Iterate over selected table names and delete their contents
    for (const auto& row : R) {
        string table_name = row["tablename"].as<string>();
        string delete_sql = "DELETE FROM \"" + table_name + "\";";
        W.exec(delete_sql);
        cout << "Cleared table: " << table_name << endl;
    }
    W.commit();  // Commit transaction


}
