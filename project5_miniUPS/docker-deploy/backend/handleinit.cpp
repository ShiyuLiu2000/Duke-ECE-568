#include"main.hpp"
#include"handleinit.hpp"
int worldID;
void initWorld(){
//   message UInitTruck{
//   required int32 id = 1;
//   required int32 x=2;
//   required int32 y=3;
// }
cout<<"now is in the initWorld function"<<endl;
UConnect Uconnect_msg;
UInitTruck* trucks;
int trucksNum=300;
int truck_id;
int x;
int y;
for(int i = 0; i < trucksNum; ++i) {
   trucks = Uconnect_msg.add_trucks();
   truck_id=i;
   x=i;
   y=i;

   trucks->set_id(truck_id);
    trucks->set_x(x);//???????????怎么定义初始位置
    trucks->set_y(y);
}
  string status="idle";
  
      string updateTruckSql = "INSERT INTO \"UPS_truck\" ";
    updateTruckSql += "(truck_id, current_x, current_y, status, ";
    updateTruckSql += "ready_to_pickup_time, arrive_warehouse_time, delivery_start_time, delivered_time) ";
    updateTruckSql += "VALUES (";
    updateTruckSql += to_string(truck_id) + ", ";
    updateTruckSql += to_string(x) + ", ";
    updateTruckSql += to_string(y) + ", ";
    updateTruckSql += "'" + status + "', ";
    updateTruckSql+= "NULL, NULL, NULL, NULL);";

  
  mutexForTruck.lock();
  executeCUD(updateTruckSql);
  mutexForTruck.unlock();

  Uconnect_msg.set_isamazon(false);
  //Uconnect_msg.set_worldid(worldID);

  mutexForWorld.lock();
    {
      cout<<"now is to create stream"<<endl;
    unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(worldFD));
     cout<<"now is created stream"<<endl;
    if (sendMesgTo<UConnect>( Uconnect_msg, out.get()) == false) {
        cerr<< "Error: cannot send UConnect to world." << endl;
        cout<<"now is sended connect world message:::error!!!!!!"<<endl;
    }
     cout<<"now is sended connect world message"<<endl;
    }
   mutexForWorld.unlock();
   return;
}
void connectedWorld(){
    UConnected Uconnected_msg;
   mutexForWorld.lock();
    {
    unique_ptr<google::protobuf::io::FileInputStream> in(new google::protobuf::io::FileInputStream(worldFD));
    
    if (recvMesgFrom<UConnected>(Uconnected_msg, in.get()) == false) {
        cerr<< "Error: cannot receive UConnected from world.";
        mutexForWorld.unlock();
        return;
    }
    }

    mutexForWorld.unlock();

    //check Uconnect
    string result = Uconnected_msg.result();
    if(result != "connected!"){
        cerr<< "Error: UPS cannot connect to world."<<endl;
    }
   worldID = Uconnected_msg.worldid();
    cout << "connected with worldID: " << worldID << endl;
}



void initAmazon(){

UAInitConnect UAInitConnect_msg;
 
UAInitConnect_msg.set_worldid(worldID);
   mutexForAmazon.lock();
   {
 unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(amazonFD));
    if (sendMesgTo<UAInitConnect>( UAInitConnect_msg, out.get()) == false) {
        cerr<< "Error: cannot send UConnect to amazon." << endl;
    }
   }
 mutexForAmazon.unlock();
AUConfirmConnect AUConfirmConnect_msg;
 
   mutexForAmazon.lock();
    {
unique_ptr<google::protobuf::io::FileInputStream> in(new google::protobuf::io::FileInputStream(amazonFD));
  if (recvMesgFrom<AUConfirmConnect>(AUConfirmConnect_msg, in.get()) == false) {
        cerr<< "Error: cannot receive UConnected from amazon.";
        mutexForAmazon.unlock();
        return;
    }
 }
mutexForAmazon.unlock();
bool connected=AUConfirmConnect_msg.connected();
if(connected==true){
  cout<<"amazon successfully connected to world!"<<endl;
}else{
  cerr<<"Error:amazon cannot connected to world!"<<endl;
}
}
