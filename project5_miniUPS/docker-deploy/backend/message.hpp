// message.hpp
#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include"main.hpp"
#include <google/protobuf/io/zero_copy_stream_impl.h>

template<typename T>
bool sendMesgTo(const T & message,
		google::protobuf::io::FileOutputStream *out) {
  { //extra scope: make output go away before out->Flush()
    // We create a new coded stream for each message. Don't worry, this is fast.
    google::protobuf::io::CodedOutputStream output(out);
    // Write the size.
    const int size = message.ByteSize();
    output.WriteVarint32(size);
    uint8_t* buffer = output.GetDirectBufferForNBytesAndAdvance(size);
    if (buffer != NULL) {
      // Optimization: The message fits in one buffer, so use the faster
      // direct-to-array serialization path.
      message.SerializeWithCachedSizesToArray(buffer);
    } else {
      // Slightly-slower path when the message is multiple buffers.
      message.SerializeWithCachedSizes(&output);
      if (output.HadError()) {
	return false;
      }
    }
  }
  out->Flush();
  return true;
}

template<typename T>
bool recvMesgFrom(T & message, google::protobuf::io::FileInputStream * in ){ 
    google::protobuf::io::CodedInputStream input(in);
    uint32_t size;
    if (!input.ReadVarint32(&size)) {
      cout<<"1------wrong"<<endl;
        return false; 
    }
      cout<<"1------right"<<endl;
    // Tell the stream not to read beyond that size. 
    google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);
    // Parse the message.
    if (!message.MergeFromCodedStream(&input)) {
        return false; 
          cout<<"2------wrong"<<endl;
    }
      cout<<"2------right"<<endl;
    if (!input.ConsumedEntireMessage()) { 
        cout<<"3------wrong"<<endl;
        return false;
    }
      cout<<"3------right"<<endl;
    // Release the limit. 
    input.PopLimit(limit); 
    return true;
}



void sendAmazonAck(long seq_num);
void sendWorldAck(long seq_num);
void sendArrChangeToAmazon(int x,int y,string tracking_number );
#endif // MESSAGE_HPP
