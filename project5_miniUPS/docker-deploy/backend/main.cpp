#include "main.hpp"

#include <pqxx/pqxx>

#include "database.hpp"
#include "handleamazon.hpp"
#include "handleinit.hpp"
#include "handleworld.hpp"
using namespace pqxx;
connection * C;
int worldFD;
mutex mutexForWorld;
mutex mutexForAmazon;
mutex mutexForTruck;
mutex mutexForPackage;
mutex mutexForUser;
mutex mutexForDelivery;
int amazonFD;

const char * worldHostName = "vcm-39849.vm.duke.edu";
const char * worldPort = "12345";
const char * amazonHostName = "vcm-39849.vm.duke.edu";
// const char* amazonHostName = "152.3.77.215";
const char * amazonPort = "9999";
// const char* amazonPort="9999";

std::set<int> waitingForAckSet;
std::set<int> receivedSeqSet;

int seqnumGlobal = 0;

void handleReceive() {
  fd_set readfds;
  int nfds = max(worldFD, amazonFD) + 1;
  cout << "enter handle_thread" << endl;
  int i = 0;
  while (1) {
    i++;
    cout << "select : " << i << endl;
    FD_ZERO(&readfds);
    FD_SET(worldFD, &readfds);
    FD_SET(amazonFD, &readfds);
    cout << "enter handle_thread while(1)" << endl;
    int ret = select(nfds, &readfds, NULL, NULL, NULL);
    if (ret > 0) {
      if (FD_ISSET(worldFD, &readfds)) {
        cout << "recv world_fd:" << worldFD << endl;
        UResponses worldResponse;
        mutexForWorld.lock();
        {
          unique_ptr<google::protobuf::io::FileInputStream> in(
              new google::protobuf::io::FileInputStream(worldFD));

          if (recvMesgFrom<UResponses>(worldResponse, in.get()) == false) {
            cout << "Error: cannot receive UResponses from world.";
            mutexForWorld.unlock();
            continue;
          }
        }
        mutexForWorld.unlock();
        std::thread world(handleWorldMsg, worldResponse);
        if (world.joinable()) {
          world.detach();
        }
      }
      if (FD_ISSET(amazonFD, &readfds)) {
        cout << "recv amazon:" << amazonFD << endl;
        AUCommands AmazonMsg;

        unique_ptr<google::protobuf::io::FileInputStream> in(
            new google::protobuf::io::FileInputStream(amazonFD));
        mutexForAmazon.lock();
        if (recvMesgFrom<AUCommands>(AmazonMsg, in.get()) == false) {
          cout << "Error: cannot receive AU_commands from Amazon.";
          mutexForAmazon.unlock();
          continue;
        }
        mutexForAmazon.unlock();
        std::thread amazon(handleAmazonMsg, AmazonMsg);
        if (amazon.joinable()) {
          amazon.detach();
        }
      }
    }
  }
}
int main(int argc, char * argv[]) {
  //======STEP1:INITIAL THE DATABASE======
  try {
    C = new connection(
        "dbname=postgres user=postgres password=postgres host=db port=5432");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else {
      cout << "could not open database" << endl;
    }
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
  }

  //drop database???

  clearTables();
  //oid clearTables(work& W);
  //======STEP2:CONNECT TO WORLD======

  worldFD = createClient(worldHostName, worldPort);
  cout << "Successfully connected to world on port 12345" << endl;
  //=======STEP3:INITIALIZE WORLD AND RECEIVE CONNECTED
  initWorld();
  cout << "now is out of the initWorld function" << endl;
  connectedWorld();
  //=======STEP4:CONNETCT WITH AMAZON

  amazonFD = createClient(amazonHostName, amazonPort);
  cout << "Successfully connected to amazon on port: " << amazonPort << endl;
  initAmazon();

  //========STEP5:SELECT LISTEN AND RECEIVE AMAZON OR WORLD===============
  handleReceive();
}
