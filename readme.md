# Estensione di nDPI con predict_linear di Prometheus
Niccolò Maestripieri <n.maestripieri@studenti.unipi.it> 
Gioele Zerini <g.zerini1@studenti.unipi.it>

## Introduzione
[nDPI](https://www.ntop.org/products/deep-packet-inspection/ndpi/) è una libreria open-source utilizzata per la Deep Packet Inspection. Noi l'abbiamo estesa con la funzione predict_linear descritta nell'API [Prometheus](https://prometheus.io/docs/prometheus/latest/querying/functions/#predict_linear), che permette di predirre il valore futuro di una serie temporale utilizzando la [regressione lineare semplice](https://en.wikipedia.org/wiki/Simple_linear_regression). Per essere conformi alle specifiche di nDPI abbiamo inoltre realizzato una funzione di test da inserire nella libreria. Infine per avere un ulteriore riscontro sul corretto funzionamento del programma abbiamo realizzato due tool che predicono l'utilizzo futuro di memoria e disco.

## Algoritmo
Il prototipo della funzione che abbiamo implementato è il seguente:
``` c
 int ndpi_predict_linear(u_int32_t *values, u_int32_t num_values, u_int32_t predict_period, u_int32_t *predicted_value, float *c, float *m);
```
Possiamo distinguere i parametri tra quelli di input:
- values: array di valori interi
- num_values: numero di elementi dell'array
- predict_period: periodo del punto da predirre, indica la distanza del punto da predirre dall'ultimo punto contenuto nell'array

E quelli di output:
- predicted_value: valore predetto
- c: parametro della retta di regressione che indica l'intersezione della retta con l'asse y
- m: parametro della retta di regressione che indica la pendenza della retta

La funzione restituisce 0 se il calcolo va a buon fine, altrimenti -1.

Per predirre il valore abbiamo usato la regressione lineare semplice, quindi abbiamo calcolato i parametri della retta di regressione relativa alla serie temporale rappresentata dall'array values. Per calcolare questi parametri abbiamo dovuto calcolare le medie dei valori lungo l'asse x (il tempo, rappresentato sagli indici dei valori dell'array) e lungo l'asse y (i valori dell'array values). Per questi valori abbiamo inoltre calcolato la rispettiva deviazione standard e la loro covarianza.

A questo punto si possono calcolare i parametri alfa e beta, che nel protipo della funzione si chiamano rispettivamente c e m. Le formule sono le seguenti:
``` math
 \left( \sum_{k=1}^n a_k bk \right)^2 \leq \left( \sum{k=1}^n ak^2 \right) \left( \sum{k=1}^n b_k^2 \right)
```
Una volta calcolati i parametri si può predirre il valore utilizzando la retta di regressione per calcolare la y del punto che ha come x l'espressione: predict_period + num_values - 1

### Casi limite
I casi in cui l'algoritmo può fallire sono principalmente due:
- parametri di input sbagliati: array non definito o con meno di due elementi, predict_period < 1
- deviazione standard dei dati uguale a 0: questo non permette di calcolare i parametri della retta di regressione, e di conseguenza di fare la predizione. Un esempio che porta a questa situazione è il caso in cui tutti i dati della serie temporale sono uguali fra loro.

## Funzione di test


## Tool
