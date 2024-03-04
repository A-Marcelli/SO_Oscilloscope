# Implementazione di un Oscilloscopio su Atmega2560
In questa repository si trova un implementazione di un oscilloscopio su una board contenente un Atmega2560.
## Come è strutturato
Nella cartella principale ci sono due cartelle, `AVR` e `PC`. In `AVR` sono contenuti tutti i file appartenenti alla parte di progetto dedicata all'Atmega2560, mentre in `PC` sono contenuti i file relativi alla parte PC e lo script GNUPLOT.
Tutte le funzioni create sono dichiarate e definite in file esterni al main come ad esempio `my_adc.c`, ad eccezione della funzione `state_machine` che è contenuta nel file `main_avr.c`.
## Come funziona
Il programma PC comunica tramite porta seriale con la scheda, collegata tramite USB, e gli passa parametri inseriti dall'utente come:
- Numero di canali ADC da utilizzare
- Modalità di esecuzione
- Periodo di campionamento
- Trigger

Un timer scandisce il periodo di campionamento tramite interrupt, l'ADC si occupa delle conversioni e i dati vengono inviati tramite UART o subito o dopo 10 secondi, a seconda della modalità scelta. Il PC si occupa di riceverli, ricostruirli, rielabolarli e stamparli in un file di output il cui nome viene passato dall'utente.
Uno script GNUPLOT si occupa di plottare i risultati in formato `.png`.

L'utente può scegliere di ricevere i risultati con due precisioni differenti:
- Risultato a 10 bit: modalità non approssimata
- Risultato a 8 bit: modalità approssimata

Questa viene decisa in fase di compilazione tramite l'utilizzo della macro `APPROX`, che, se definita, compila il programma in modalità approssimata. Questa si trova rispettivamente nel file `./PC/main\_pc.c` alla riga 13 e nel file `./AVR/avr\_common/my\_adc.h` alla riga 4.

Per maggiori informazioni sul funzionamento del codice e sulla possibilità di personalizzazione di baudrate, approssimazione, durata totale delle conversioni, periodo minimo di campionamento, etc. fare riferimento alla relazione (file pdf) caricata nella directory principale.

## How to run
Il primo passo sarebbe definire le macro, come spiegato nell'ultimo capitolo della relazione, ma se i valori pre-impostati vanno bene per le proprie esigenze si può passare alla compilazione. 
I valori pre-impostati delle macro sono i seguenti.
- PORTA: /dev/ttyACM0
- BAUD: 38400
- modalità non approssimata (APPROX commentata)
- STOP 10 secondi (tempo totale conversioni)

Per la loro posizione e come modificarle fare riferimento alla relazione
### Parte Atmega
Per compilare il progetto e caricarlo sulla scheda è stato usato un makefile, preso dai file utilizzati a lezione, ed è stato modificato per compilare il progetto. Quindi i comandi da utilizzare, direttamente dalla cartella `AVR`, sono:
```
$ make
$ make main_avr.hex
```
Se si cambia il valore della macro BAUD o si cambia la definizione della macro APPROX, prima di chiamare il `$ make` è necessario chiamare il comando `$ make clean`. Quindi se si desidera ricompilare e ricaricare dopo aver modificato le macro i passaggi saranno:
```
$ make clean
$ make
$ make main_avr.hex
```
### Parte PC
Per compilare il progetto facilmente è stato scritto un makefile. Il programma, quando viene eseguito, richiede un argomento che è il nome del file di output dove saranno salvati i dati ricevuti dalla scheda. Spostandosi quindi nella cartella `PC`, i comandi per compilare ed eseguire il programma sono quindi:
```
$ make
$ ./main_pc Risultati.dat
```
Per eliminare i file .o e l'eseguibile è possibile chiamare il comando
```
$ make clean
```
### Parte GNUPLOT
Per plottare i risultati facilmente è stato scritto un piccolo script GNUPLOT, che prende in ingresso 2 argomenti: il numero di canali ADC che sono stati utilizzati e il nome del file che contiene i risultati. Il comando per eseguire lo script, dalla cartella `PC`, è:
```
$ gnuplot -c oscilloscopio.plt NUMERO_CANALI_ADC_USATI NOME_FILE_RISULTATI.dat
```
