#include <WString.h>
#include "nunchuck_funcs.h"
#include "WProgram.h"
#include <Wire.h>
#include <math.h>
#include <EEPROM.h>

//variaveis genericas
int loop_cnt=0;
int ledPin = 13;
char c;
int i=0,inicio=0;
boolean first_exec=true,debug_msg=true,debug_k=false;
double T=0;

//variaveis do nunchuck
int zbut,cbut;

//variaveis do aceleroemtro
int offsetX=132,offsetY=134,offsetZ=190,offsetW=319;  //offset values are obtained holding the chuck on a fix zero position
static double scaleX=-50,scaleY=50,scaleZ=50,scaleW=0.512;  //point axxis upward then downward, subtracting the values, and divide by 2
double accx,accy,accz;
byte countx,county,countz;
double pitch=0;

//variaveis do gyroscopio
double gyro=0,w=0,teta=0,filter_teta=0;

//variaveis do PID
double dt=1;
double error=0, last_error=0;
double last_time=0,time=0;
double setpoint=0;
double integral=0,derivative=0,output=0;

//constantes
#define MAXTRESHOLD 230
#define MINTRESHOLD 35
#define HPF 0.98
#define LPF (1-HPF)
#define SAMPLE 10



/* No Setup, sao inicializados as portas, entrada ou saida,
 configurado a velocidade da porta serial e o protocolo de comunicacao
 com o controle do wii, nunchuck.
 */

void setup()
{
  //Inicializa a porta Serial
  Serial.begin(57600);
  //Inicializa o controle do wii, nunchuck
  nunchuck_setpowerpins();
  nunchuck_init();
  analogReference(DEFAULT);
  Serial.print("WiiChuck pronto\n");
  Serial.flush();

  pinMode(ledPin,OUTPUT);

}


/* O codigo dentro do loop eh executado continuamente, aqui onde serao feitas as leituras dos sensores
 o processamento deste sinal, e a execucao do controle. O tempo entre cada loop eh de aproximademente
 0.05 segundos, ou seja 200Hz de frequencia.
 */
void loop()
{
  /*Esse comando gera um delay de 2s na primeira execucao do programa para calibracao das variaveis*/
  if(first_exec==true && inicio>200){
    zera();
    set_offset();
    zera(); 
    first_exec=false;
  }
  inicio++;


  /*Nesta parte do codigo, os valores das contantes podem ser alterados por software*/
  if(Serial.available()) c=Serial.read();
  switch(c){   
  case 'j':
    zera();
    set_offset();
    break;
  }

  c='l';

  while( millis() - last_time < 4 ){
    dt=4;
  }
  last_time = millis();

  /* Executa a leitura do nunchuck (acelerometro e botoes) e a
   leitura do gyrscopio.    
   */
  nunchuck_get_data();
  countx  = nunchuck_accelx();
  county  = nunchuck_accely();
  countz  = nunchuck_accelz(); 
  zbut = nunchuck_zbutton();
  cbut = nunchuck_cbutton();
  gyro = analogRead(0);
  //nunchuck_print_data();


  /* Transforma os valores lidos pelo controle em aceleracao (g)
   pitch eh o angulo em graus calculado pela aceleracao, ou seja, inclinacao
   do acelerometro. 
   */
  accx = ((double)((countx-offsetX)/scaleX));
  accy = ((double)((county-offsetY)/scaleY));
  accz = ((double)((countz-offsetZ)/scaleZ+1));
  pitch = (double)(atan2(accy,sqrt(sq(accz)+sq(accx)))*180/3.14);

  /* Transforma os valores lidos pela porta ADC em velocidade angular */
  w    = ((double)(gyro-offsetW)/scaleW);

  teta = teta + w*dt/1000;

  /* Calculo do angulo teta estimado, aqui eh usado a tecnica descrita na secao de filtros,
   onde HPF eh a constante do filtro passa alta, LPF a cosntante do filtro passa baixa,
   w eh a velocidade angular, dt eh o tempo entre cada loop.
   */
  filter_teta = HPF*(filter_teta + w*dt/1000 ) + LPF*(pitch);
  


  //Aqui novos offsets sao setados, ou seja, a nova posicao zero do robo 
  if(cbut==1){
    set_offset();
  }

  /* Os erros podem ser zeradas para evitar que o controlador se torne instavel  */
  if(zbut==1) {
    zera();
  } 
  else{
    digitalWrite(ledPin,LOW);
  }



  /* Informacoes sobre o tempo, angulo, erro, saida, sao enviadas para visualizacao no computador*/

  switch(i){
  case 1:
    Serial.print(dt); 
    Serial.write(":");
    break; 
  case 2:
    Serial.print(w); 
    Serial.write(":"); 
    break;
  case 3:
    Serial.print(teta); 
    Serial.write(":"); 
    break;
  case 4:
    Serial.print(pitch); 
    Serial.write(":"); 
    break;   
  case 5:
    Serial.print(filter_teta);
    Serial.write("$");
    Serial.println();
    i=0;
  }
  i++;


}


void zera(){
  digitalWrite(ledPin,HIGH);
  error=0;
  filter_teta=0;
  output=0;
  last_error=0;
  teta=0;
  w0=0;
  Serial.println("Variaveis zeradas");
  delay(500); 
}

void set_offset(){
  delay(500);
  nunchuck_get_data();
  offsetX=countx;
  offsetY=county;
  offsetZ=countz;
  offsetW=analogRead(0);
  Serial.println("Novos offsets setados");
}
