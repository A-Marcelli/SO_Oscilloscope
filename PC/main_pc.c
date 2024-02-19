#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SPEED 19200
#define PORT "/dev/ttyACM0"

int main(int argc, char** argv) {
  if (argc<2) {
    printf("usage: %s output_file", argv[0]);
    return 0;
  }

  char* output_file = argv[1];
  char* filename    = PORT;
  int baudrate      = SPEED;
  
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

  uint8_t adc_num    = 0;
  uint8_t frequency  = 0;
  uint8_t mode       = 0;


  //const int bsize=10;
  //char buf[bsize];
  printf("Quanti canali ADC utilizzare?\nSelezionare un numero da 1 a 8:\n");
  scanf("%hhu", &adc_num);
  printf("Hai selezionato %hhx canali\n\n", adc_num);
  ssize_t res_adc = write(fd, &adc_num, 1);             //invia adc_num
  if(res_adc != 1){
    printf("ERRORE SU INVIO NUMERO CANALI ADC\n");
    return -1;
  }


  printf("Ogni quanti ms deve essere effuttuato il sampling?\nSelezionare un valore tra 1 e 255:\n");
  scanf("%hhu", &frequency);
  printf("Hai selezionato 0x%hhx ms\n\n", frequency);
  ssize_t res_freq = write(fd, &frequency, 1);          //invia frequency
  if(res_freq != 1){
    printf("ERRORE SU INVIO FREQUENCY\n");
    return -2;
  }


  printf("Che modalità si vuole utilizzare?\n1 -> continous sampling\n2 -> buffered mode\n");
  scanf("%hhu", &mode);
  printf("Hai selezionato modalità numero %hhx\n\n", mode);
  ssize_t res_mode = write(fd, &mode, 1);               //invia mode
  if(res_mode != 1){
    printf("ERRORE SU INVIO MODE\n");
    return -3;
  }

  //TRIGGER
  
  while (1) {


    //int n_read=read(fd, buf, bsize);
    //for (int i=0; i<n_read; ++i) {
    //  printf("%c", buf[i]);
    //}
  }
}
