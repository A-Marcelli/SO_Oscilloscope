#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define SPEED 19200
#define PORT "/dev/ttyACM0"
#define STOP 10
#define VREF 2.86

int main(int argc, char** argv) {
  if (argc<2) {
    printf("usage: %s output_file", argv[0]);
    return 0;
  }

  char* output_file       = argv[1];
  char* filename          = PORT;
  int baudrate            = SPEED;
  uint8_t adc_num         = 0;
  uint16_t frequency_in   = 0;
  uint8_t frequency_out   = 0;
  uint8_t mode            = 0;
  uint8_t trigger         = 0;
  uint8_t while_var       = 0;
  uint32_t len            = 0;
  uint32_t max_conv       = 0;
  FILE *fd_out;
  uint8_t *buffer, *buffer_var;
  float *buffer_out;
  
  
  printf( "opening serial device [%s] ... ", filename);
  int fd=serial_open(filename);
  if (fd<=0) {
    printf ("Error\n");
    return 0;
  } else {
    printf ("Success\n");
  }

  printf( "setting baudrate [%d] ... ", baudrate);
  int attribs=serial_set_interface_attribs(fd, baudrate, 0);
  if (attribs) {
    printf("Error\n");
    return 0;
  }

  serial_set_blocking(fd, 1);

  printf( "opening output file [%s] ... ", output_file);
  fd_out=fopen(output_file,"w");
  if (fd_out<=0) {
    printf ("Error\n");
    return 0;
  } else {
    printf ("Success\n");
  }

  //const int bsize=10;
  //char buf[bsize];
  while_var = 0;
  while(! while_var){
    printf("Quanti canali ADC utilizzare?\nSelezionare un numero da 1 a 8:\n");
    scanf("%hhu", &adc_num);
    while ( getchar() != '\n' );
    if(adc_num >= 1 && adc_num <= 8){
      printf("Hai selezionato %hhx canali\n\n", adc_num);
      ssize_t res_adc = write(fd, &adc_num, 1);             //invia adc_num
      if(res_adc != 1){
        printf("ERRORE SU INVIO NUMERO CANALI ADC\n");
        return -1;
      }
      while_var = 1;
    }
  }

  while_var = 0;
  while(! while_var){
    printf("Ogni quanti ms deve essere effuttuato il sampling?\nSelezionare un valore tra 1 e 255:\n");
    scanf("%hu", &frequency_in);
    while ( getchar() != '\n' );
    if(frequency_in >= 1 && frequency_in <=255){
      frequency_out = (uint8_t ) (frequency_in & 0xff);
      printf("Hai selezionato 0x%hhx ms\n\n", frequency_out);
      ssize_t res_freq = write(fd, &frequency_out, 1);          //invia frequency
      if(res_freq != 1){
        printf("ERRORE SU INVIO FREQUENCY\n");
        return -2;
      }
      while_var = 1;
    }
  }

  max_conv = (uint32_t) (STOP * 1000) / frequency_out;
  len = max_conv * 2 * adc_num;
  buffer = (uint8_t *) malloc(sizeof(uint8_t) * len);
  buffer_out = (float *) malloc(sizeof(float) * (len/2));
  buffer_var = buffer;

  while_var = 0;
  while(! while_var){
    printf("Che modalità si vuole utilizzare?\n1 -> continous sampling\n2 -> buffered mode\n");
    scanf("%hhu", &mode);
    while ( getchar() != '\n' );
    if(mode == 1 || mode == 2){
      printf("Hai selezionato modalità numero %hhx\n\n", mode);
      ssize_t res_mode = write(fd, &mode, 1);               //invia mode
      if(res_mode != 1){
        printf("ERRORE SU INVIO MODE\n");
        return -3;
      }
      while_var = 1;
    }
  }

  while_var = 0;
  while(! while_var){
    printf("Selezionare \"1\" per far partire le conversioni\n");
    scanf("%hhu", &trigger);
    while ( getchar() != '\n' );
    if(trigger == 1){
      printf("Hai iniziato le conversioni!\n\n");
      ssize_t res_trig = write(fd, &trigger, 1);               //invia mode
      if(res_trig != 1){
        printf("ERRORE SU INVIO MODE\n");
        return -3;
      }
      while_var = 1;
    }
  }
  
  if(mode == 1){
    //sfrutto il fatto che invia 2*adc_num byte per volta prima di aspettare l'interrupt successivo
    int n_read = 0;
    for(int i=0;i<(len/(2*adc_num));i++){
      
      n_read += read(fd, buffer_var, 2*adc_num);   //devo printare subito i valori che arrivano, dopo averli ricostruiti
      buffer_var += 2*adc_num;
      //printf("%d", frequency_out*i); //prova
      fprintf(fd_out,"%d", frequency_out*i);
      for(int j=0;j<adc_num;j++){
        fprintf(fd_out," %f", ((( (uint16_t) buffer[(2*j)+1+2*i*adc_num]) <<8) | buffer[2*j+2*i*adc_num]) * VREF / 1024);
        //printf(" %f", ((( (uint16_t) buffer[(2*j)+1+2*i*adc_num]) <<8) | buffer[2*j+2*i*adc_num]) * VREF / 1024);
      }
      //printf("\n");   //prova
      fprintf(fd_out,"\n");
    }
    printf("\n\%d\n", len);  //prova
    printf("\n\%d\n", n_read); //prova

  }
  
  if(mode == 2){  //buffered mode

    int n_read = 0;
    for(int i=0;i<((uint32_t) len/64 );i++){       //read legge solo 64 bytes per volta
      
      n_read += read(fd, buffer_var, 64);
      buffer_var += 64;

    }

    n_read += read(fd,buffer_var,(len%64));         //restante parte del buffer


    //printf("\n\%d\n", len);  //prova
    //printf("\n\%d\n", len/64);  //prova
    //printf("\n\%d\n", n_read); //prova


    for(int i = 0; i<(len/2); i++){
        buffer_out[i] = ((( (uint16_t) buffer[(2*i)+1]) <<8) | buffer[2*i]) * VREF / 1024;   //unisco i byte high e low dell'adc e trasformo in Volt
    }

    //free(buffer);   //controlla come si libera un array malloc

    for (int i=1; i<(len/2); i++) {             //IL PRIMO VALORE CONTIENE UN RISULTATO CASUALE, è DA SCARTARE
      //fprintf(fd_out,"%d %hhx%hhx\n", frequency_out*(i/2), buffer[i+1], buffer[i]);
      //printf("%d %02hhx%02hhx\n", frequency_out*(i/2), buffer[i+1], buffer[i]);
      printf("%d %f\n", frequency_out*(i%(len/(2*adc_num))), buffer_out[i]);
    }
  }

  return 0;

  //while (1) {
    //int n_read=read(fd, buf, bsize);
    //for (int i=0; i<n_read; ++i) {
    //  printf("%c", buf[i]);
    //}
  //}
}
