# Estensione di nDPI con predict_linear di Prometheus
Niccolò Maestripieri <n.maestripieri@studenti.unipi.it> 
Gioele Zerini <g.zerini1@studenti.unipi.it>

## Introduzione
[nDPI](https://www.ntop.org/products/deep-packet-inspection/ndpi/) è una libreria open-source utilizzata per la Deep Packet Inspection. Noi l'abbiamo estesa con la funzione predict_linear descritta nell'API [Prometheus](https://prometheus.io/docs/prometheus/latest/querying/functions/#predict_linear), che permette di predirre il valore futuro di una serie temporale utilizzando la [regressione lineare semplice](https://en.wikipedia.org/wiki/Simple_linear_regression). Per essere conformi alle specifiche di nDPI abbiamo inoltre realizzato una funzione di test da inserire nella libreria. Infine per avere un ulteriore riscontro sul corretto funzionamento del programma abbiamo realizzato due tool che predicono l'utilizzo futuro di memoria e disco.
