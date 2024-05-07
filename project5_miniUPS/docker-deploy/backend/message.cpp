#include "message.hpp"
//this is adpated from code that a Google engineer posted online 

// template<typename T>
// void sendToAmazon(T & message){
//     mutexForWorld.lock();
//     {
//     unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(worldFD));
//     if (sendMesgTo<UConnect>( Uconnect_msg, out.get()) == false) {
//         cerr<< "Error: cannot send UConnect to world." << endl;
//     }
//     }
// //    mutexForWorld.unlock();
// }
void sendAmazonAck(long seq_num){
    UACommands ackMsg;
    ackMsg.add_acks(seq_num);
  unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(amazonFD));
    mutexForAmazon.lock();
    if(sendMesgTo< UACommands>(ackMsg, out.get()) == false) {
        cout<< "Error: cannot send UAcommands to amazon." << endl;
    }
    cout<< "send ack to amazon:" << seq_num<<endl;
   mutexForAmazon.unlock();





}
void sendWorldAck(long seq_num){
    UCommands ackMsg;
    ackMsg.add_acks(seq_num);
  unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(worldFD));
    mutexForWorld.lock();
    if(sendMesgTo< UCommands>(ackMsg, out.get()) == false) {
        cout<< "Error: cannot send UAcommands to world." << endl;
    }
      cout<< "send ack to world:" << seq_num<<endl;
   mutexForWorld.unlock();


}

void sendArrChangeToAmazon(int x,int y,string tracking_number ){
 cout << "**********************************" << endl;
  cout << "Enter sendArrChangeToAmazon function!!!!!!!!!!!!!!!!!!" << endl;
  cout << "**********************************" << endl;
// message UAChangeAddr{
//   required string trackingid = 1;
//   required int32 dest_x = 2;
//   required int32 dest_y = 3;
//   required int64 seqnum = 4;
// }
UACommands UAChangeAddr_msg;
  UAChangeAddr *changeArr_msg = UAChangeAddr_msg.add_changeaddr();
  changeArr_msg->set_trackingid(tracking_number);
  changeArr_msg->set_dest_x (x);
  changeArr_msg->set_dest_y (y);
  changeArr_msg->set_seqnum(seqnumGlobal++);
  std::cout << "send  UAChangeAddr_msg to amazon, pkg_id=" << tracking_number << endl;

  unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(amazonFD));
  do{
  mutexForAmazon.lock();

  if (sendMesgTo<UACommands>(UAChangeAddr_msg, out.get()) == false)
  {
    cout << "Error: cannot send UCommands UAChangeAddr_msg to Amazon." << endl;
  }
  mutexForAmazon.unlock();
  cout << "__________________________________" << endl;
  sleep(7);
    } while (!(waitingForAckSet.find(seqnumGlobal) == waitingForAckSet.end()));
}


