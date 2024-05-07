#include "handleworld.hpp"

bool hasReceivedMessageW(int seq_num)
{
  if (receivedSeqSet.find(seq_num) != receivedSeqSet.end())
  {
    return true; //
  }
  receivedSeqSet.insert(seq_num);
  return false;
}
void handleUDeliveryMade(UDeliveryMade deliverdMsg)
{
  //================1. CHECK IS RECEIVED =============
  if (hasReceivedMessageW(deliverdMsg.seqnum()))
  {
    return;
  }
  //================2. SEN ACK TO AMAZON ==========
  sendWorldAck(deliverdMsg.seqnum());
  cout << "**********************************" << endl;
  cout << "Enter handleUDeliveryMade function" << endl;
  cout << "**********************************" << endl;
  //================3. GET DATAS IN MESSAGE ==========
  // message UDeliveryMade{
  //   required int32 truckid = 1;
  //   required int64 packageid = 2;
  //   required int64 seqnum = 3;
  // }
  int seqnum = deliverdMsg.seqnum();
  int truck_id = deliverdMsg.truckid();
  long packageid = deliverdMsg.packageid();
  //=================4. SELECT DATABASE PACKAGE
  string selectPackageDest = "SELECT dest_addr_x, dest_addr_y, tracking_number FROM \"UPS_package\" WHERE  package_id_from_amazon = " + to_string(packageid) + ";";
  result packageDest = executeRead(selectPackageDest);
   string tracking_id;

  //===========5. UPDATE \"UPS_truck\" LACATION
  if (!packageDest.empty())
  {
    int dest_addr_x = packageDest[0][0].as<int>();
    int dest_addr_y = packageDest[0][1].as<int>();
  tracking_id=packageDest[0][2].as<string>();
     stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string updateTruckStatusSql = 
        "UPDATE \"UPS_truck\" SET current_x = " + std::to_string(dest_addr_x) + 
        ", current_y = " + std::to_string(dest_addr_y) + 
        ", delivered_time = '" + ss.str() + "' " +
        "WHERE truck_id = " + std::to_string(truck_id) + ";";
    mutexForTruck.lock();
    executeCUD(updateTruckStatusSql);
    mutexForTruck.unlock();
 //================7. DELIVERY TABLE==============

    std::stringstream sqlUpdateDelivery;
    sqlUpdateDelivery << "UPDATE \"UPS_delivery\" SET ";

    sqlUpdateDelivery << "delivered_time = '" << ss.str() << "' ";
    sqlUpdateDelivery << "WHERE package_id = '" << tracking_id << "';";
    mutexForDelivery.lock();
    executeCUD(sqlUpdateDelivery.str());
    mutexForDelivery.unlock();

  }

  //=============6. UPDATE \"UPS_package\" STATUS============
  string updatePackageStatusSql = "UPDATE \"UPS_package\" SET status = 'delivered' WHERE  package_id_from_amazon = " + to_string(packageid) + ";";
  mutexForPackage.lock();
  executeCUD(updatePackageStatusSql);
  mutexForPackage.unlock();
 
  //==================8. SEND UADelivered TO AMAZON===========

  // message UADelivered{
  //   required string trackingid = 1;
  //   required int32 truckid = 2;
  //   required int64 seqnum = 3;
  // }
  UACommands UADelivered_meg;
  UADelivered *deliverd_mag = UADelivered_meg.add_delivered();
  deliverd_mag->set_trackingid(to_string(tracking_id));
  deliverd_mag->set_truckid(truck_id);

  deliverd_mag->set_truckid(truck_id);
  deliverd_mag->set_seqnum(seqnumGlobal++);

  unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(amazonFD));
  
    do{
  mutexForAmazon.lock();
  
  if (sendMesgTo<UACommands>(UADelivered_meg, out.get()) == false)
  {
    cout << "Error: cannot send UCommands UATruckArrived_msg to Amazon." << endl;
  }
    cout << "send UCommands UATruckArrived_msg to Amazon." << endl;
  mutexForAmazon.unlock();
  cout << "__________________________________" << endl;
   sleep(7);
    } while (!(waitingForAckSet.find(seqnumGlobal) == waitingForAckSet.end()));
}

void handleFinishAll(UFinished finishedMsg)
{
   //================1. CHECK IS RECEIVED =============
  if (hasReceivedMessageW(finishedMsg.seqnum()))
  {
    return;
  }
    //================2. SEN ACK TO AMAZON ==========
  sendWorldAck(finishedMsg.seqnum());
  cout << "**********************************" << endl;
  cout << "Enter  handleFinishAllfunction" << endl;
  cout << "**********************************" << endl;
  //================3. GET DATAS IN MESSAGE ==========
 int truck_id = finishedMsg.truckid();
  int current_x = finishedMsg.x();
  int current_y = finishedMsg.y();
  string status = finishedMsg.status();
  status = "idle";
 //==================4. UPDATE DATABASE TRUCK==============
  string updateTruckSql = "UPDATE \"UPS_truck\" SET current_x = " + to_string(current_x) + ", current_Y = " + to_string(current_y) + ", status = '" + status + "' WHERE truck_id = " + to_string(truck_id) + "; ";
  mutexForTruck.lock();
  executeCUD(updateTruckSql);
  mutexForTruck.unlock();

}
void handleArrivedWH(UFinished finishedMsg)
{

  //================1. CHECK IS RECEIVED =============
  if (hasReceivedMessageW(finishedMsg.seqnum()))
  {
    return;
  }
  //================2. SEN ACK TO AMAZON ==========
  sendWorldAck(finishedMsg.seqnum());
  cout << "**********************************" << endl;
  cout << "Enter handleArrivedWH function" << endl;
  cout << "**********************************" << endl;
  //================3. GET DATAS IN MESSAGE ==========

  int truck_id = finishedMsg.truckid();
  int current_x = finishedMsg.x();
  int current_y = finishedMsg.y();
  string status = finishedMsg.status();

   stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
     ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
 status = "arrive at warehouse";
  //==================4. UPDATE DATABASE TRUCK==============
  std::string updateTruckSql = "UPDATE \"UPS_truck\" SET current_x = " + std::to_string(current_x) +
                                 ", current_y = " + std::to_string(current_y) +
                                 ", status = '" + status + 
                                 "', arrive_warehouse_time = '" + ss.str() +
                                 "' WHERE truck_id = " + std::to_string(truck_id) + ";";

  mutexForTruck.lock();
  executeCUD(updateTruckSql);
  mutexForTruck.unlock();
  //==============5. UPDATE DATABASE PACKAGE===========
  string updatePackageSql = "UPDATE \"UPS_package\" SET status = 'loading' WHERE truck_id = " + to_string(truck_id) + "; ";
  mutexForPackage.lock();
  executeCUD(updatePackageSql);
  mutexForPackage.unlock();

  //=============6. 查数据库---根据truckID和status=loading查package表========
  string selectPackageSql = "SELECT tracking_number, whid FROM \"UPS_package\" WHERE status = 'loading' AND  truck_id =  " + to_string(truck_id) + ";";

  result res = executeRead(selectPackageSql);
  string tracking_id;
  int whid;
  if (!res.empty())
  {
    tracking_id = res[0][0].as<string>();
    whid = res[0][1].as<int>();
  }
//=============================== DELIVERY TABLE====================
  //=====================
    // class Delivery(models.Model):
    // delivery_id = models.AutoField(primary_key=True)
    // truck = models.ForeignKey(Truck, on_delete=models.CASCADE)
    // package = models.ForeignKey(Package, on_delete=models.CASCADE)
    // pickup_location = models.CharField(max_length=255)
    // go_warehouse_time = models.DateTimeField(null=True, blank=True)
    // arrive_warehouse_time = models.DateTimeField(null=True, blank=True)
    // delivery_start_time = models.DateTimeField(null=True, blank=True)
    // delivered_time = models.DateTimeField(null=True, blank=True)
 
     std::string pickup_location = "x = " + std::to_string(current_x) + ", y= " + std::to_string(current_y);
std::stringstream sqlUpdateDelivery;
    sqlUpdateDelivery << "UPDATE \"UPS_delivery\" SET ";
    sqlUpdateDelivery << "pickup_location = '" <<  pickup_location << "', ";
    sqlUpdateDelivery << "arrive_warehouse_time = '" << ss.str() << "' ";
    sqlUpdateDelivery << "WHERE package_id = '" << tracking_id << "';";
    mutexForDelivery.lock();
    executeCUD( sqlUpdateDelivery.str());
    mutexForDelivery.unlock();
  

  //============================= SEND UATruckArrived TO AMAZON=======
  //     message UATruckArrived{
  //   required int32 truckid = 1;
  //   required string trackingid = 2;
  //   required int32 wh_id = 3;
  //   required int64 seqnum = 4;
  // }
  UACommands UATruckArrived_msg;
  UATruckArrived *arrived_msg = UATruckArrived_msg.add_arrived();
  arrived_msg->set_trackingid(tracking_id);
  arrived_msg->set_truckid(truck_id);
  arrived_msg->set_wh_id(whid);
  arrived_msg->set_seqnum(seqnumGlobal++);
  std::cout << "send UATruckArrived_msg to amazon, pkg_id=" << tracking_id << ", truck_id = " << truck_id << ",whid="<<whid<<endl;

  unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(amazonFD));
  do{
  mutexForAmazon.lock();

  if (sendMesgTo<UACommands>(UATruckArrived_msg, out.get()) == false)
  {
    cout << "Error: cannot send UCommands UATruckArrived_msg to Amazon." << endl;
  }
  mutexForAmazon.unlock();
  cout << "__________________________________" << endl;
  sleep(7);
    } while (!(waitingForAckSet.find(seqnumGlobal) == waitingForAckSet.end()));
}

void handleWorldArc(long seq_num)
{
  if (waitingForAckSet.find(seq_num) == waitingForAckSet.end())
  {
    cerr << "this seq_num did not waiting for ack, seq_num: " << seq_num << endl;
  }
  //////////加锁！！！！！！！！！！！！！
  waitingForAckSet.erase(seq_num);
}

void handleWorldMsg(UResponses worldResponse)
{
  if (worldResponse.acks_size() != 0)
  {
    for (int i = 0; i < worldResponse.acks_size(); ++i)
    {
      long seq_num = worldResponse.acks(i);
       cout<< "received ack from world:" << seq_num<<endl;
      handleWorldArc(seq_num);
    }
  }

  if (worldResponse.delivered_size() != 0)
  {
    for (int i = 0; i < worldResponse.delivered_size(); ++i)
    {
      UDeliveryMade deliverdMsg = worldResponse.delivered(i);
      handleUDeliveryMade(deliverdMsg);
    }
  }
  if (worldResponse.completions_size() != 0)
  {
    for (int i = 0; i < worldResponse.completions_size(); i++)
    {
      UFinished finishedMsg = worldResponse.completions(i);

      // 如果Uresponse.completions[i].status==IDLE
      // 则handleFinishAll
      if (finishedMsg.status() == "IDLE")
      {
        handleFinishAll(finishedMsg);
      }
      // 如果Uresponse.completions[i].status==arrivedWarehouse,
      // 则 handleArrivedWH
      else
      {
        handleArrivedWH(finishedMsg);
      }
    }
  }
}