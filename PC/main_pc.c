#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

//#define SPEED 19200
#define SPEED 38400
#define PORT "/dev/ttyACM0"
#define STOP 10
#define VREF 2.56
#define BUFFER_MAX 7750
//#define APPROX


int main(int argc, char** argv) {
  if (argc<2) {
    printf("usage: %s output_file", argv[0]);
    return 0;
  }

  char* output_file        = argv[1];
  char* filename           = PORT;
  uint32_t baudrate        = SPEED;
  uint8_t adc_num          = 0;
  uint8_t period        = 0;           //contiene il tempo di campionamento
  uint16_t period_in    = 0;
  uint8_t mode             = 0;
  uint8_t trigger          = 0;
  uint8_t while_var        = 0;
  uint32_t len             = 0;
  uint32_t max_conv        = 0;
  uint32_t n_read          = 0;
  FILE *fd_out;
  uint8_t *buffer, *buffer_var;
  float *buffer_out;

#if SPEED == 19200
  uint32_t t_min_array_approx[8] = {6, 11, 16, 21, 27, 32, 37, 42};       //valori trovati empiricamente. numero di decimi di ms.
  uint32_t t_min_array_no_approx[8] = {11, 21, 32, 42, 53, 63, 74, 84};   //valori trovati empiricamente. numero di decimi di ms.
#elif SPEED == 38400
  uint32_t t_min_array_approx[8] = {5, 6, 8, 11, 14, 16, 19, 21};         //valori trovati empiricamente. numero di decimi di ms.
  uint32_t t_min_array_no_approx[8] = {6, 11, 16, 21, 27, 32, 37, 42};    //valori trovati empiricamente. numero di decimi di ms.
#endif

  
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

  while_var = 0;
  while(! while_var){
    printf("Quanti canali ADC utilizzare?\nSelezionare un numero da 1 a 8:\n");
    scanf("%hhu", &adc_num);
    while ( getchar() != '\n' );
    if(adc_num >= 1 && adc_num <= 8){
      if(adc_num!=1){
        printf("Hai selezionato i canali da A0 a A%hhx!\n\n", adc_num-1);
      } else {
        printf("Hai selezionato il canale A0!\n\n");
      }
      ssize_t res_adc = write(fd, &adc_num, 1);             //invia adc_num
      if(res_adc != 1){
        printf("ERRORE SU INVIO NUMERO CANALI ADC\n");
        return -1;
      }
      while_var = 1;
    }
  }

  fprintf(fd_out,"#Time");                       //Nel file di output inserisco il titolo delle colonne di dati
  for(int i = 0; i<adc_num;i++){
    fprintf(fd_out,"\tChannel %d", i);
  }
  fprintf(fd_out,"\n");

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
        return -2;
      }
      while_var = 1;
    }
  }

  uint32_t t_min = 1;          //tempo minimo (1/frequenza massima)

#ifdef APPROX
  if(mode==1){
    t_min = t_min_array_approx[adc_num - 1];
  } else if(mode == 2){
    t_min = ((uint32_t) STOP * 10000 * adc_num / BUFFER_MAX) + 1;         //devo fare in modo che la grandezza del buffer non superi la SRAM del controllore
  }
#else
  if(mode==1){
    t_min = t_min_array_no_approx[adc_num - 1];
  } else if(mode == 2){
    t_min = ((uint32_t) STOP * 10000 * 2 * adc_num / BUFFER_MAX) + 1;     //devo fare in modo che la grandezza del buffer non superi la SRAM del controllore
  }
#endif

  while_var = 0;
  while(! while_var){
    printf("Ogni quanti DECIMI di ms deve essere effettuato il sampling?\nSelezionare un valore tra %d e 10000:\n", t_min);
    scanf("%hu", &period_in);
    while ( getchar() != '\n' );
    if(period_in >= t_min && period_in <=10000){   
      printf("Hai selezionato %.1f ms\n\n", (double) period_in/10);
      uint8_t period_out[2];
      period_out[0] = (uint8_t) ((period_in>>8) & 0xff);    //prima high
      period_out[1] = (uint8_t) (period_in & 0xff);         //poi low
      ssize_t res_per = write(fd, &period_out, 2);            //invia period 
      if(res_per != 2){
        printf("ERRORE SU INVIO period\n");
        return -3;
      }
      while_var = 1;
    }
  }

  max_conv = (uint32_t) (STOP * 10000) / period_in;
#ifdef APPROX
  len = max_conv * adc_num;
  if(mode == 2){
    buffer_out = (float *) malloc(sizeof(float) * len);
  }
  
#else
  len = max_conv * 2 * adc_num;
  if(mode == 2){
    buffer_out = (float *) malloc(sizeof(float) * (len/2));
  }
#endif

  buffer = (uint8_t *) malloc(sizeof(uint8_t) * len);
  buffer_var = buffer;

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
        return -4;
      }
      while_var = 1;
    }
  }
  
  if(mode == 1){
    //sfrutto il fatto che invia 2*adc_num (o adc_num in modalità APPROX) byte per volta prima di aspettare l'interrupt successivo

#ifdef APPROX
    for(int i=0;i<max_conv;i++){
  
      n_read += read(fd, buffer_var, adc_num);   //devo printare subito i valori che arrivano, dopo averli ricostruiti
      buffer_var += adc_num;
      fprintf(fd_out,"%.1f", (double) period_in*i/10);
      for(int j=0;j<adc_num;j++){
        fprintf(fd_out,"\t%f",  ((uint16_t) buffer[j+i*adc_num]<<2) * VREF / 1024);
      }
      fprintf(fd_out,"\n");
    }
#else
    for(int i=0;i<max_conv;i++){
      
      n_read += read(fd, buffer_var, 2*adc_num);   //devo printare subito i valori che arrivano, dopo averli ricostruiti
      buffer_var += 2*adc_num;
      fprintf(fd_out,"%.1f", (double) period_in*i/10);
      for(int j=0;j<adc_num;j++){
          fprintf(fd_out,"\t%f", ((( (uint16_t) buffer[(2*j)+1+2*i*adc_num]) <<8) | buffer[2*j+2*i*adc_num]) * VREF / 1024);
      }
      fprintf(fd_out,"\n");
    }
#endif
  }
  
  if(mode == 2){  //buffered mode

    for(int i=0;i<((uint32_t) len/64 );i++){       //read legge solo 64 bytes per volta da questa porta
      
      n_read += read(fd, buffer_var, 64);
      buffer_var += 64;
    }

    n_read += read(fd,buffer_var,(len%64));         //restante parte del buffer

#ifdef APPROX
    for(int i = 0; i<len; i++){
        buffer_out[i] = ((uint16_t) buffer[i]<<2) * VREF / 1024;   //trasformo in Volt
    }
#else
    for(int i = 0; i<(len/2); i++){
        buffer_out[i] = ((( (uint16_t) buffer[(2*i)+1]) <<8) | buffer[2*i]) * VREF / 1024;   //unisco i byte high e low dell'adc e trasformo in Volt
    }
#endif

    //inizio scrittura su file di output
    for (int i=0; i<max_conv; i++) {
      fprintf(fd_out,"%.1f", (double) period_in*i/10);
      for(int j=0; j<adc_num; j++){
        fprintf(fd_out,"\t%f", buffer_out[i+j*max_conv]);
      }
      fprintf(fd_out,"\n");
    }
  }

  free(buffer);
  if(mode == 2) free(buffer_out);
  fclose(fd_out);

  return 0;

}
