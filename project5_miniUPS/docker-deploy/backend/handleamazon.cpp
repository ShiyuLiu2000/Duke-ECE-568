#include "handleamazon.hpp"

bool hasReceivedMessage(int seq_num)
{
    if (receivedSeqSet.find(seq_num) != receivedSeqSet.end())
    {
        return true; //
    }
    receivedSeqSet.insert(seq_num);
    return false;
}

void handleAUNeedATruck(AUNeedATruck needTruckMsg)
{
    //================1. CHECK IS RECEIVED =============
    if (hasReceivedMessage(needTruckMsg.seqnum()))
    {
        return;
    }
    cout << "didn't received" << endl;
    //================2. SEN ACK TO AMAZON ==========
    sendAmazonAck(needTruckMsg.seqnum());
    cout << "**************************************" << endl;
    cout << "Enter handleAUNeedATruck function" << endl;
    cout << "**************************************" << endl;
    //================3. GET DATAS IN MESSAGE ==========
    Pack packageToPickup = needTruckMsg.pack();
    int wh_id = packageToPickup.wh_id();
    string trackingid = packageToPickup.trackingid();
    long packageid = packageToPickup.packageid();

    // if(packageToPickup.has_upsaccount()){
    // upsaccount = packageToPickup.upsaccount();
    // upsaccountSTR=to_string(upsaccount);
    // }else{
    // upsaccount=-1;
    // upsaccountSTR="";
    // }
    int amazonaccount = packageToPickup.amazonaccount();
    int dest_x = packageToPickup.dest_x();
    int dest_y = packageToPickup.dest_y();
    
    // cout << "AUNeedATruck msg: trackingid=" << trackingid << ", wh_id = " << wh_id  << ", amazon account =" << amazonaccount << endl;
    string description = "";
    for (int j = 0; j < packageToPickup.things_size(); j++)
    {
        Product product = packageToPickup.things(j);
        description.append(product.description() + " * " + to_string(product.count()) + ", ");
    }
    cout << "make description!!!" << endl;
    // string s = "SELECT id FROM auth_user WHERE username = '" + user_name + "';";
    //   result r = run_query(s);
    //   if(r.empty()) {
    //     cout << "username doesn't exist." << endl;
    //     return;
    //   }
    stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::stringstream sqlUpdate;
    //===================4. GET TRUCK TO PICKUP AND UPDATE \"UPS_truck\" TABLE============
    int truck_id = getTruckTopickup();
    cout << "get truck!!!" << endl;

    // string sql_selectPackage = "SELECT tracking_number FROM \"UPS_package\" WHERE tracking_number = " + trackingid + ";";
    // result res = executeRead(sql_selectPackage);
    // if (!res.empty())
    // {
    //     cout << "ship_id has already existed." << endl;
    //     return;
    // }
    // int user_id = r[0][0].as<int>();

    // ========================5. INSERT PACKAGE ====================
    stringstream sql;
    if (packageToPickup.has_upsaccount())
    {
         int upsaccount = packageToPickup.upsaccount();
    string upsaccountSTR = to_string(upsaccount);

        sql << "INSERT INTO \"UPS_package\" (whid, whats_inside,tracking_number, user_id, amazon_id, dest_addr_x, dest_addr_y, status, truck_id,init_addr_x,init_addr_y,package_id_from_amazon) VALUES (";
        sql << wh_id << ", ";
        sql << "'" << description << "', ";
        sql << "'" << trackingid << "', ";
        sql << upsaccount << ", ";
        sql << amazonaccount << ", ";
        sql << dest_x << ", ";
        sql << dest_y << ", ";
        sql << "'at warehouse', ";
        sql << truck_id << ", ";
        sql << dest_x << ", ";
        sql << dest_y << ", ";
        sql << packageid;
        sql << ");";
        cout << "AUNeedATruck msg: upsaccount= "<< upsaccount<<",trackingid=" << trackingid << ", wh_id = " << wh_id << ", amazon account =" << amazonaccount << endl;
    }
    else
    {

        sql << "INSERT INTO \"UPS_package\" (whid, whats_inside,tracking_number, amazon_id, dest_addr_x, dest_addr_y, status, truck_id,init_addr_x,init_addr_y,package_id_from_amazon) VALUES (";
        sql << wh_id << ", ";
        sql << "'" << description << "', ";
        sql << "'" << trackingid << "', ";
        sql << amazonaccount << ", ";
        sql << dest_x << ", ";
        sql << dest_y << ", ";
        sql << "'at warehouse', ";
        sql << truck_id << ", ";
        sql << dest_x << ", ";
        sql << dest_y << ", ";
        sql << packageid;
        sql << ");";
        cout << "AUNeedATruck msg: trackingid=" << trackingid << ", wh_id = " << wh_id<< ", amazon account =" << amazonaccount << endl;
    }
    mutexForPackage.lock();
    executeCUD(sql.str());
    mutexForPackage.unlock();
    //=========================DELIVERY TABLE===================
    // std::string pickup_location = "x = " + std::to_string(current_x) + ", y= " + std::to_string(current_y);

    std::string deliveryInsertSql = "INSERT INTO \"UPS_delivery\" (truck_id, package_id, go_warehouse_time) VALUES (" +
                                    std::to_string(truck_id) + ", '" +
                                    trackingid + "', '" +
                                    ss.str()  +
                                    "');";

    mutexForDelivery.lock();
    executeCUD(deliveryInsertSql);
    mutexForDelivery.unlock();

    //=====================  send UGoPickup to world========
    UCommands UGoPickup_msg;

    UGoPickup *pickUps_msg = UGoPickup_msg.add_pickups();
    pickUps_msg->set_truckid(truck_id);
    pickUps_msg->set_whid(wh_id);
    pickUps_msg->set_seqnum(seqnumGlobal++);
    cout << "UGoPickup to world, truck_id = " << truck_id << ", wh_id = " << wh_id << ", seq_num = " << seqnumGlobal << endl;

    cout << "In parseAU_pick_truck function. send UGoPickup to world   " << endl;

    unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(worldFD));

    do
    {
        mutexForWorld.lock();
        {
            if (sendMesgTo<UCommands>(UGoPickup_msg, out.get()) == false)
            {
                waitingForAckSet.insert(seqnumGlobal);
                cout << "Error: cannot send UCommand to world." << endl;
            }
            else
            {
                cout << "send UGoPickUp!" << endl;
            }
        }
        mutexForWorld.unlock();
        sleep(7);
    } while (!(waitingForAckSet.find(seqnumGlobal) == waitingForAckSet.end()));
}

void handleAUTruckCanGo(AUTruckCanGo loadedMsg)
{
    if (hasReceivedMessage(loadedMsg.seqnum()))
    {
        return;
    }
    //================2. SEN ACK TO AMAZON ==========
    sendAmazonAck(loadedMsg.seqnum());
    cout << "**************************************" << endl;
    cout << "Enter handleAUTruckCanGo function" << endl;
    cout << "**************************************" << endl;
    //     message AUTruckCanGo{
    //   required int32 truckid = 1;
    //   required int64 seqnum = 2;
    // }
    //================3. GET DATAS IN MESSAGE ==========
    int truck_id = loadedMsg.truckid();

    //=================4. UPDATE \"UPS_package\" STATUS===========
    string updatePackageStatusSql = "UPDATE \"UPS_package\" SET status = 'delivering' WHERE status = 'loading' AND truck_id = " + to_string(truck_id) + ";";
    mutexForPackage.lock();
    executeCUD(updatePackageStatusSql);
    mutexForPackage.unlock();
    //================5. SELECT DATABASE PACKAGES ==========
    //     message UDeliveryLocation{
    //   required int64 packageid = 1;
    //   required int32 x = 2;
    //   required int32 y = 3;
    // }

    // message UGoDeliver{
    //   required int32 truckid = 1;
    //   repeated UDeliveryLocation packages = 2;
    //   required int64 seqnum = 3;
    // }

    string selectPackageSql = "SELECT package_id_from_amazon, dest_addr_x, dest_addr_y,init_addr_x, init_addr_y, tracking_number FROM \"UPS_package\" WHERE status = 'delivering' AND truck_id = " + to_string(truck_id) + ";";
    result packageData = executeRead(selectPackageSql);

    //==================
    string status = "delivering";
    //==================6. UPDATE DATABASE TRUCK==============
    std::ostringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    // 格式化时间并添加到字符串流
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    std::string updateTruckSql = "UPDATE \"UPS_truck\" SET status = '" + status + "', delivery_start_time = '" + ss.str() + "' WHERE truck_id = " + std::to_string(truck_id) + ";";
    mutexForTruck.lock();
    executeCUD(updateTruckSql);
    mutexForTruck.unlock();
    //=======================DELIVERY TABLE=====================
    string selectPackageSql2 = "SELECT tracking_number FROM \"UPS_package\" WHERE truck_id =  " + to_string(truck_id) + ";";

    result res = executeRead(selectPackageSql2);
    string tracking_id;

    if (!res.empty())
    {
        tracking_id = res[0][0].as<string>();
    }
    std::stringstream sqlUpdateDelivery;
    sqlUpdateDelivery << "UPDATE \"UPS_delivery\" SET ";

    sqlUpdateDelivery << "delivery_start_time = '" << ss.str() << "' ";
    sqlUpdateDelivery << "WHERE package_id = '" << tracking_id << "';";
    mutexForDelivery.lock();
    executeCUD(sqlUpdateDelivery.str());
    mutexForDelivery.unlock();

    //=================. SEND UGoDeliver TO WORLD===============
    // message UGoDeliver{
    //   required int32 truckid = 1;
    //   repeated UDeliveryLocation packages = 2;
    //   required int64 seqnum = 3;
    // }
    // message UDeliveryLocation{
    //   required int64 packageid = 1;
    //   required int32 x = 2;
    //   required int32 y = 3;
    // }
    UCommands UGoDeliver_msg;

    UGoDeliver *goDeliver_msg = UGoDeliver_msg.add_deliveries();
    // UGoDeliver goDeliver_msg;
    goDeliver_msg->set_truckid(truck_id);
    goDeliver_msg->set_seqnum(seqnumGlobal++);
    if (!packageData.empty())
    {

        for (size_t i = 0; i < packageData.size(); ++i)
        {
            UDeliveryLocation *location = goDeliver_msg->add_packages();
            long packageid = packageData[i][0].as<long>();
            int x = packageData[i][1].as<int>();
            int y = packageData[i][2].as<int>();
            int init_x = packageData[i][3].as<int>();
            int init_y = packageData[i][4].as<int>();
            string tracking_number = packageData[i][5].as<string>();
            location->set_packageid(packageid);
            location->set_x(x);
            location->set_y(y);
            if (x != init_x || y != init_y)
            {
                sendArrChangeToAmazon(x, y, tracking_number);
            }
        }
    }
    unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(worldFD));

    do
    {
        mutexForWorld.lock();
        {
            if (sendMesgTo<UCommands>(UGoDeliver_msg, out.get()) == false)
            {
                waitingForAckSet.insert(seqnumGlobal);
                cout << "Error: cannot send UCommand to world." << endl;
            }
            else
            {
                cout << "send UGoDeliver!" << endl;
            }
        }
        mutexForWorld.unlock();

        sleep(7);
    } while (!(waitingForAckSet.find(seqnumGlobal) == waitingForAckSet.end()));
}

void handleAUerror(Err errorMsg)
{
}
void handleAmazondArc(long seq_num)
{
    if (waitingForAckSet.find(seq_num) == waitingForAckSet.end())
    {
        cerr << "this seq_num did not waiting for ack, seq_num: " << seq_num << endl;
    }
    //////////加锁！！！！！！！！！！！！！
    waitingForAckSet.erase(seq_num);
}
void handleAmazonMsg(AUCommands AmazonMsg)
{
    if (AmazonMsg.acks_size() != 0)
    {
        for (int i = 0; i < AmazonMsg.acks_size(); ++i)
        {
            long seq_num = AmazonMsg.acks(i);
            cout << "received ack from amazon:" << seq_num << endl;
            handleAmazondArc(seq_num);
        }
    }
    if (AmazonMsg.need_size() != 0)
    {
        for (int i = 0; i < AmazonMsg.need_size(); ++i)
        {
            AUNeedATruck needTruckMsg = AmazonMsg.need(i);
            handleAUNeedATruck(needTruckMsg);
        }
    }
    if (AmazonMsg.go_size() != 0)
    {
        for (int i = 0; i < AmazonMsg.go_size(); ++i)
        {
            AUTruckCanGo loadedMsg = AmazonMsg.go(i);
            handleAUTruckCanGo(loadedMsg);
        }
    }

    if (AmazonMsg.errors_size() != 0)
    {
        for (int i = 0; i < AmazonMsg.errors_size(); ++i)
        {
            Err errorMsg = AmazonMsg.errors(i);
            handleAUerror(errorMsg);
        }
    }
}