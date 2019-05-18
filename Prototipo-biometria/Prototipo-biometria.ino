// INCLUSÃO DAS BIBLIOTECAS
#include <Adafruit_Fingerprint.h> //biblioteca sensor biométrico
#include <SoftwareSerial.h> //biblioteca padrão monitor serial
//#include <ESP8266WiFi.h> //biblioteca para conexão wifi

#include <Adafruit_SleepyDog.h> 
#include <Adafruit_CC3000.h> 
#include <SPI.h> 
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_CC3000.h"


#define WLAN_SSID       "Campus Boulevard"
#define WLAN_PASS       ""
#define WLAN_SECURITY   WLAN_SEC_WPA2

//MQTT
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "thiago_alves"
#define AIO_KEY         "mt@br2014"


const char FINGERPRINT_FEED[] PROGMEM = AIO_USERNAME "/feeds/sensorbiometrico";
Adafruit_MQTT_Publish fingerprint = Adafruit_MQTT_Publish(&mqtt, FINGERPRINT_FEED);


// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(5, 4);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// DECLARAÇÃO DAS VARIÁVEIS E FUNCOES
uint8_t numID = 1;
bool gravar = false;
int fingerID = 0;

uint8_t modoGravacaoID(uint8_t IDgravar);

void setup() {

  Serial.begin(9600);
  finger.begin(57600);

  /*WiFi.begin("Campus Boulevard", "");
  
  Serial.println();
  Serial.println("Preparando o ambiente de trabalho, tome um café enquanto isso...");
  Serial.println("Conectando-se a rede...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Conectado! Endereço IP: ");
  Serial.println(WiFi.localIP());*/

  
  if (finger.verifyPassword()) { //RETORNA VERDADEIRO OU FALSO SE O SENSOR FOI ENCONTRADO OU NÃO
    Serial.println("Sensor biometrico encontrado!");
  } else {
    Serial.println("Sensor biometrico não encontrado! Verifique a conexão e reinicie o sistema");
    while (true) {
      delay(1);
    }
  }


  Serial.println("Fim do Setup!");
}

void loop() {
  
    MQTT_connect ();

  /*if ( botao.pressed() ){ //SE O BOTÃO FOR APERTADO ENTRA NO MODO DE GRAVAÇÃO DO ADMINISTRADOR
    gravar = true;
    }

    if(gravar){ // E SE GRAVAR ENTÃO IRÁ EXECUTAR
    modoGravacaoID(0); //A GRAVAÇÃO DA DIGITAL DO ADMIN NA POSIÇÃO DE MEMORIA ZERO (0)
    gravar = false; // LOGO EM SEGUIDA SAÍ DO MODO DE GRAVAÇÃO DE DIGITAL DO ADMIN
    }*/

  getFingerprintIDez(); //FUNÇÃO QUE VERIFICA SE EXISTE UMA DIGITAL NO SENSOR

}

uint8_t modoGravacaoID(uint8_t IDgravar) { //FUNÇÃO QUE REALIZA A GRAVAÇÃO DE UMA NOVA DIGITAL.

  int p = -1;
  Serial.print("Esperando uma leitura válida para gravar #"); Serial.println(IDgravar);
  delay(2000);


  while (p != FINGERPRINT_OK) {

    p = finger.getImage(); // PEGOU A IMAGEM OBTIDA!

    switch (p) {

      case FINGERPRINT_OK: // CASO A IMAGEM CAPTURADA É UMA DIGITAL ENTÃO EXECUTE.
        Serial.println("Leitura concluída");
        break; //PULE PARA O PRÓXIMO BLOCO...

      case FINGERPRINT_NOFINGER: //O PROGRAMA DIZ QUE TEM ALGO ERRADO E RECUSA.
        Serial.println(".");
        delay(200);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:  //OUTRA RESPOSTA DE ERRO
        Serial.println("Erro comunicação");
        break;
      case FINGERPRINT_IMAGEFAIL: //A PARTIR DAQUI DEU MERDA
        Serial.println("Erro de leitura");
        break;
      default:
        Serial.println("Erro desconhecido"); //FUDEU!!!
        break;
    }
  }

  // SE A GRAVAÇÃO FOI FEITA COM SUCESSSO ENTÃO VAI CONTINUA A EXECUÇÃO DESSE PRÓXIMO BLOCO ABAIXO.

  p = finger.image2Tz(1); //AQUI VERIFICA E CONVERTE A IMAGEM DA DIGITAL E O RESTO VCS JÁ SABEM...

  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Leitura convertida");
      break; //PULA PARA O PRÓXIMA BLOCO.

    case FINGERPRINT_IMAGEMESS:
      Serial.println("Leitura suja");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro de comunicação");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Não foi possível encontrar propriedade da digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Não foi possível encontrar propriedade da digital");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
  }

  //TUDO CERTO? ENTÃO VAMOS NOVAMENTE PARA GARANTIR QUE É A DIGITAL CORRETA

  Serial.println("Remova o dedo");
  delay(2000);
  p = 0;


  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(IDgravar);
  p = -1;


  Serial.println("Coloque o Mesmo dedo novamente");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Leitura concluída");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        delay(200);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Erro de comunicação");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Erro de Leitura");
        break;
      default:
        Serial.println("Erro desconhecido");
        break;
    }
  }

  //VAMOS A SEGUNDA ETAPA QUE É VALIDAR A IMAGEM GUARDADA E COMPARAR COM A DIGITAL COLADA NO SENSOR.

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Leitura convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Leitura suja");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro de comunicação");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Não foi possível encontrar as propriedades da digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Não foi possível encontrar as propriedades da digital");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
  }

  //VAMOS COMPARAR O MODELO CRIADO COM O ORIGINAL QUE ESTÁ NO SENSOR AGORA.
  Serial.print("Criando modelo para #");  Serial.println(IDgravar);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("As digitais batem!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro de comunicação");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("As digitais não batem");
    return p;
  } else {
    Serial.println("Erro desconhecido");
    return p;
  }

  //SE DEU TUDO CERTO ENTÃO ESTÁ NA HORA DE GUARDAR ELA NA MEMORIA. O PRÓXIMO BLOCO TRATA DISSO.

  Serial.print("ID "); Serial.println(IDgravar);
  p = finger.storeModel(IDgravar);
  if (p == FINGERPRINT_OK) {
    Serial.println("Armazenado!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro de comunicação");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Não foi possível gravar neste local da memória");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Erro durante escrita na memória flash");
    return p;
  } else {
    Serial.println("Erro desconhecido");
    return p;
  }
}


//A PARTIR DAQUI O SENSOR TRABALHA DE VERDADE PARA CAPTAR DIGITAIS NO SENSOR.

int getFingerprintIDez() { //TEM UMA IMAGEM NO SENSOR?

  uint8_t p = finger.getImage(); //RETORNA COM VERDADEIRO OU FASO.

  if (p != FINGERPRINT_OK)  return -1; //ENTÃO PEGUE A IMAGEM!!

  p = finger.image2Tz(); //AVALIE ESSA IMAGEM!
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch(); //JÁ VI ESTE PADRÃO?
  if (p != FINGERPRINT_OK)  return -1;

  //Encontrou uma digita em uma posição de memoria!!!
  if (finger.fingerID == 0) { // se estiver na posição zero (0) então execute
    Serial.print("Modo Administrador!"); //o modo admin

    numID++; //adiciona uma nova ID
    modoGravacaoID(numID); //e grava na memoria do sensor
    return 0;

  } else { //NÃO É UM ADMIN!!! ENTÃO EXECUTE

    Serial.print("ID encontrado #"); Serial.print(finger.fingerID);   //INFORMA QUAL id FOI ENCONTRADO.
    Serial.print(" com confiança de "); Serial.println(finger.confidence);   // NO MONITOR SERIAL.
    delay(500);   //ESPERA UNS MILISEGUNDOS ANTES DE EXECUTAR A LINHA DE BAIXO.
    return finger.fingerID;
  }
}