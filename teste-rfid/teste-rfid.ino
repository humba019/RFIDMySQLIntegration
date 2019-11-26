//MYSQL CONNECTION ATTRIBUTES
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
byte mac_addr[] = {0xDE,0xAD,0xBE,0xEF,0xFE,0xED};

byte server_addr[] = {192,168,1,102};  // IP of the MySQL *server* here
char user[] = "arduino";              // MySQL user login username
char password[] = "1234";        // MySQL user login password

EthernetClient client;
MySQL_Connection conn((Client *)&client);
char *SELECT_TAG = "SELECT inicio FROM accesswave.tag WHERE idtag= '%d' ";
char query[128];

//RFID CONNECTION ATTRIBUTES
#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#define SS_PIN 53
#define RST_PIN 5
MFRC522::MIFARE_Key key;


MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
String tag;

void setup() {
  
   Serial.begin(115200);

   connBD();
   connRFID();
   SPI.begin();      //Inicia  SPI bus
   mfrc522.PCD_Init();   //Inicia MFRC522
 
  //Prepara chave - padrao de fabrica = FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
   
}
void loop() {

/*
    while(Serial.available()) {
      
      selectByTag();
      
    }
*/

    modo_gravacao();
  
  
  /*
    if ( mfrc522.PICC_IsNewCardPresent())
        {
            if ( mfrc522.PICC_ReadCardSerial())
            {
               Serial.print("Tag UID:");
               
               char conteudo[] = "";
               char cont[] = "";
               for (byte i = 0; i < mfrc522.uid.size; i++) {
                     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                     Serial.print(mfrc522.uid.uidByte[i], HEX);
                     conteudo[i] = mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ";
                     cont[1] = mfrc522.uid.uidByte[i], HEX;
                       
                } 
                       
                      Serial.print(cont[1]);       
                //selectByTag(conteudo);  D5 D2 F3 E3
                mfrc522.PICC_HaltA();
            }
            
    }
    */

        
}

void connBD(){
  
      Ethernet.begin(mac_addr);
      Serial.println("Connecting...");

        if (conn.connect(server_addr, 3306, user, password)) {
          delay(1000);
          Serial.println("Sucess!");
        }
        else{
          Serial.println("Connection failed.");
          conn.close();
        }      
    
}

void connRFID(){
  
      SPI.begin();       
      mfrc522.PCD_Init(); 
      Serial.println("RFID reading UID");
      
}

void selectByTag(){
    tag = Serial.readString();
    //char tagChar[8] = "";
    //tag.toCharArray(tagChar, 8);
    Serial.println(tag.toInt());
    sprintf(query, SELECT_TAG, tag.toInt());
    row_values *row = NULL;
    String head_count = "";
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Execute the query
    cur_mem->execute(query);
    // Fetch the columns (required) but we don't use them.
    column_names *columns = cur_mem->get_columns();
  
    // Read the row (we are only expecting the one)
    do {
      row = cur_mem->get_next_row();
      if (row != NULL) {
        for(int i = 0; i <= 18; i++){
        head_count += row->values[i];
        }
      }
    } while (row != NULL);
    // Deleting the cursor also frees up memory used
    delete cur_mem;

    // Show the result
    if(head_count != 0){
      Serial.println("SUCESSO");
      Serial.println("----------------------");
      Serial.println(head_count);
      Serial.println("----------------------");
    }else{
      Serial.println("ATENÇÃO");
      Serial.println("------------------");
      Serial.println("Essa tag não exite");
      Serial.println("------------------");
    }

}


void modo_gravacao()
{
  //Aguarda cartao
  while ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(100);
  }
  if ( ! mfrc522.PICC_ReadCardSerial())    return;
 
  //Mostra UID na serial
  Serial.print(F("UID do Cartao: "));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  //Mostra o tipo do cartao
  Serial.println(F("nTipo do PICC: "));
  byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
 
  byte buffer[34];
  byte block;
  byte status, len;
 
  Serial.setTimeout(20000L) ;
  Serial.println(F("Digite o sobrenome,em seguida o caractere #"));
  Serial.println("Digite o sobreno");
  Serial.println("me + #");
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ;
  for (byte i = len; i < 30; i++) buffer[i] = ' ';
 
  block = 1;
  
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //Grava no bloco 1
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  block = 2;
  //Serial.println(F("Autenticacao usando chave A..."));
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
 
  //Grava no bloco 2
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println(F("Dados gravados com sucesso!"));
   
    Serial.println("Gravacao OK!");
  }
}
