#define START_BYTE 0xFE
#define TIMEOUT

// NOTE: Beware of clashing variable names with your own code

// This number I anyhow choose one
#define ACK_ID 0xFD // TODO: Ehhhh maybe change the id to follow the msgid you sent out is better?
#define ACK_LEN 1
#define ACK_DATA 0xFD
#define ACK_TIMEOUT 500

// Receive buffer
int16_t incoming_data = 0; 
uint8_t read_flag = 0;
char read_buffer[11]; // Takes [id,len,data,crc] no FE FE
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr; // Counts size of incoming_data without FE FE
bool ack = true;
uint8_t ack_packet[] = {ACK_DATA};
// TODO: Need add a queue to store all msgs to be retransmitted

// Send
char some_data[] = "HELLO";
char some_other_data[] = "BYE BYE";
uint32_t curr_time;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(9600);
  curr_time = millis();
  ack_time = millis();

  Serial.print("START~");
}

void loop() {
  // put your main code here, to run repeatedly:
//  SEND
  if(millis() - curr_time > 500)
  {
    forwardToRadio(111, 5, some_data);
    forwardToRadio(222, 7, some_other_data);
    curr_time = millis();

    ack = false;
  }

//  Retransmit
// If no ack received, and TIMEOUT
  if(!ack && (millis() - ack_time > ACK_TIMEOUT))
  {
    // TODO: Retransmit~
    ack_time = millis();
  }

// RECEIVE
  if (Serial2.available())
  {
    //read
    while(incoming_data > -1)
    {
      incoming_data = Serial2.read();
      //Serial.print(incoming_data,HEX);
      if (incoming_data == -1)
      {
        incoming_data = 0;
        break;
      }
      if (incoming_data == START_BYTE && !read_flag)
      {
        read_flag = 1;
      }
      else if (incoming_data == START_BYTE && read_flag == 1)
      {
        read_flag++;
        read_flag = 2;
      }
      else if (read_flag == 2)
      {
        if (read_ctr == 0)
        {
          read_id = incoming_data;
          read_buffer[read_ctr] = incoming_data;
          read_ctr++;
        }
        else if (read_ctr == 1)
        {
          read_size = incoming_data;
          read_buffer[read_ctr] = incoming_data;
          read_ctr++;
        }
        else if (read_ctr >= 2)
        {
          read_buffer[read_ctr] = incoming_data;
          if (read_ctr == (2 + read_size))
          {
            if(isValidCrc(read_buffer, read_ctr))
            {
              // Full packet received

              if(isAck())
              {
                // TODO: Pop first msg from retransmit queue
                ack = false;
              }
              else
              {
                // Decode packet
  
                // Print out received msg
                Serial.print("id: ");
                Serial.print(read_id);
                Serial.print(" size: ");
                Serial.print(read_size);
                Serial.print(" data: ");
                for(int i = 2; i < read_ctr; i++)
                {
                  Serial.print(read_buffer[i]);
                  Serial.print(" ");
                }
                Serial.println("");
  
                sendAck();
              }
            }
            read_flag = 0;
            read_ctr = 0;
          }
          else {
            read_buffer[read_ctr] = incoming_data;
            read_ctr++;
            //Serial.print("|");
          }
        }
      }
    }
  }
}

void forwardToRadio(int id, int len, uint8_t data[]) {
  uint8_t temp[13] = {0};

  temp[0] = 0xFE;
  temp[1] = 0xFE;
  temp[2] = id;
  temp[3] = len;
  for (int i = 4, j = 0; j < len; i++, j++) {
    temp[i] = data[j];
  }
  // Take checksum of [id,len,data] in case [id] and [len] have errors too
  uint8_t crc = checksum(temp+2, len + 2);
  temp[len+4] = crc;
  
  for(int i = 0;i<len+5;i++) {
    Serial2.write(temp[i]);
    //Serial.print(temp[i],HEX);
    //Serial.print(" ");
  }
  Serial2.flush();
    //Serial.println();
  //Serial.print("SEND");
}

// Takes the crc of [len] bytes in [data] and compares to last byte
// data[] is [id,len,data] without FE FE
bool isValidCrc (uint8_t data[], int len) {
    uint8_t crc = checksum(data, len);
    if (crc == data[len]) {
      return true;
    }
  return false;
}

uint8_t checksum(uint8_t data[], uint8_t len) {
  uint8_t crc = 0;
  uint8_t i = 0;
  while(len != 0) {
    crc = crc ^ data[i]; 
    i++;
    len--;
  }
  return crc;
}

bool isAck()
{
  if (read_id != ACK_ID)          return false;
  if (read_size != ACK_LEN)       return false;
  if (read_buffer[2] != ACK_DATA) return false;  
  return true;
}

void sendAck()
{
  forwardToRadio(ACK_ID, ACK_LEN, ack_packet);
}

