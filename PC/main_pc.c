#include "serial_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define SPEED 19200
#define PORT "/dev/ttyACM0"
#define STOP 10
#define VREF 2.56
#define BUFFER_MAX 7750
#define APPROX

int main(int argc, char** argv) {
  if (argc<2) {
    printf("usage: %s output_file", argv[0]);
    return 0;
  }

  char* output_file        = argv[1];
  char* filename           = PORT;
  int baudrate             = SPEED;
  uint8_t adc_num          = 0;
  uint8_t frequency        = 0;
  uint8_t mode             = 0;
  uint8_t trigger          = 0;
  uint8_t while_var        = 0;
  uint32_t len             = 0;
  uint32_t max_conv        = 0;
  uint32_t n_read          = 0;
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

  uint32_t f_min = 1;          //tempo minimo (1/frequenza massima)

#ifdef APPROX
  if(mode==1){
    f_min = ((uint8_t) adc_num/2) + 1;  //trovato empiricamente che se il tempo è più basso di questo allora ci mette più di STOP secondi in modalità continous.
  } else if(mode == 2){
    f_min = ((uint32_t) STOP * 1000 * adc_num / BUFFER_MAX) + 1;         //devo fare in modo che la grandezza del buffer non superi la SRAM del controllore
  }
#else
  if(mode==1){
    f_min = adc_num + 1;                //trovato empiricamente che se il tempo è più basso di questo allora ci mette più di STOP secondi in modalità continous.
  } else if(mode == 2){
    f_min = ((uint32_t) STOP * 1000 * 2 * adc_num / BUFFER_MAX) + 1;     //devo fare in modo che la grandezza del buffer non superi la SRAM del controllore
  }
#endif

  while_var = 0;
  while(! while_var){
    printf("Ogni quanti ms deve essere effettuato il sampling?\nSelezionare un valore tra %d e 255:\n", f_min);
    scanf("%hhu", &frequency);
    while ( getchar() != '\n' );
    if(frequency >= f_min && frequency <=255){
      printf("Hai selezionato 0x%hhx ms\n\n", frequency);
      ssize_t res_freq = write(fd, &frequency, 1);          //invia frequency low
      if(res_freq != 1){
        printf("ERRORE SU INVIO FREQUENCY\n");
        return -3;
      }
      while_var = 1;
    }
  }

  max_conv = (uint32_t) (STOP * 1000) / frequency;
#ifdef APPROX
  len = max_conv * adc_num;
  buffer_out = (float *) malloc(sizeof(float) * len);
#else
  len = max_conv * 2 * adc_num;
  buffer_out = (float *) malloc(sizeof(float) * (len/2));
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
    fprintf(fd_out,"#");          //la prima conversione va scartata, metto un "#" che gnuplot usa per indicare un commento

#ifdef APPROX
    for(int i=0;i<max_conv;i++){
  
      n_read += read(fd, buffer_var, adc_num);   //devo printare subito i valori che arrivano, dopo averli ricostruiti
      buffer_var += adc_num;
      fprintf(fd_out,"%d", frequency*i);
      for(int j=0;j<adc_num;j++){
        fprintf(fd_out,"\t%f",  buffer[j+i*adc_num] * VREF / 1024);
      }
      fprintf(fd_out,"\n");
    }
#else
    for(int i=0;i<max_conv;i++){
      
      n_read += read(fd, buffer_var, 2*adc_num);   //devo printare subito i valori che arrivano, dopo averli ricostruiti
      buffer_var += 2*adc_num;
      fprintf(fd_out,"%d", frequency*i);
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
        buffer_out[i] = buffer[i] * VREF / 1024;   //trasformo in Volt
    }
#else
    for(int i = 0; i<(len/2); i++){
        buffer_out[i] = ((( (uint16_t) buffer[(2*i)+1]) <<8) | buffer[2*i]) * VREF / 1024;   //unisco i byte high e low dell'adc e trasformo in Volt
    }
#endif

    //inizio scrittura su file di ourput
    for (int i=1; i<max_conv; i++) {             //la prima conversione va scartata
      fprintf(fd_out,"%d", frequency*i);
      for(int j=0; j<adc_num; j++){
        fprintf(fd_out,"\t%f", buffer_out[i+j*max_conv]);
      }
      fprintf(fd_out,"\n");
    }
  }

  free(buffer);
  free(buffer_out);
  fclose(fd_out);

  return 0;

}
