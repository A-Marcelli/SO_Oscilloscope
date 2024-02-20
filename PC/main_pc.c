#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define SPEED 19200
#define PORT "/dev/ttyACM0"
#define STOP 10

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
  uint8_t *buffer;
  
  
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

    while(1){

    }

  }
  
  if(mode == 2){  //buffered mode

    
    //int n_read=read(fd, buffer, len);
    int n_read=read(fd, buffer, len);
    printf("\n\%d\n", len);
    printf("\n\%d\n", n_read);
    for (int i=0; i<len; i+=2) {
      //fprintf(fd_out,"%d %hhx%hhx\n", frequency_out*(i/2), buffer[i+1], buffer[i]);
      printf("%d %02hhx%02hhx\n", frequency_out*(i/2), buffer[i+1], buffer[i]);
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
