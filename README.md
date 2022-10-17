# Formació IoT - Xarxes de comunicació IoT - Introducció a LoRaWAN al programa d’Industria 4.0 en el marc del Projectes PECT InnoDelta

Aquest repositori recull les presentacions i exemples que es treballaran a la formació sobre IoT / Xarxes de comunicació "IoT / Introducció a LoRaWAN al programa d’Industria 4.0" en el marc del Projectes PECT InnoDelta impulsat per l'ajuntament de Viladecans i realitzada al Cubic entre finals d'Octubre i principis de Novembre de 2022.

## Objectius

L’objectiu de la formació és oferir als assistents un visió global del panorama de l’IoT que inclogui tots els components d’una solució IoT: des de la sensòrica a la explotació de dades, passant per la infraestructura de comunicacions, la gestió de la xarxa o el processat i emmagatzemament de dades.

Per cadascuna de les sessions farem una aproximació teòrica i un exemple pràctic que permeti els assistents entendre quines opcions existeixen i quines decisions tècniques i de negoci s’han de prendre en cadascuna de les fases d’un projecte.

Al final de les 5 sessions els assistents tindran un coneixement pràctic producte d’haver treballat en una solució real end-to-end.

## Temari

### 1. Sessió d’introducció a l’IoT, LoRaWAN i TTN

Teoria
* Introducció a l’IoT
  * Context, casos d’ús, verticals i reptes
  * Comparació de tecnologies sense fils existents (Cel·lular/LPWAN, Sigfox, LoRa(WAN), NB-IoT, Wize, LTE-M...)
  * Tipus de xarxes (privades/publiques, operades...)
* LoRaWAN
  * LoRa i LoRaWAN
  * Característiques tècniques
  * Usos i limitacions
  * Xarxes LoraWAN

Pràctica
* Servidor en el núvol – The Things Network (TTN)
  * Crearem un compte a TTN i analitzarem les opcions que ens proporciona l’interfície web
  * Aprendrem com podem automatitzar tasques amb la CLI
* Opcions on-premise – The Things Stack / Chirpstack
  * Desplegarem el nostre servidor on-premise amb Docker/Balena

### 2. Dispositius

Teoria
* Tipus de dispositius
* Configuració dels dispositius: ADR, cicle de treball, seguretat, activació...
* Xips, mòduls i plaques de desenvolupament
* Dispositius de maquinari obert
* Dispositius comercials
  * Sensors
  * Ponts (ModBUS - LoRaWAN)

Pràctica
* Desenvoluparem un prototipus amb una placa de desenvolupament i un sensor
* El configurarem i connectarem a TTN
* Analitzarem com podem optimitzar l’enviament de dades (freqüència, codificació)
* Descodificarem el missatge al servidor i analitzarem les metadades del mateix

### 3. Anàlisi de dades

Teoria
* Servidor d’Aplicacions
  * Integracions
  * Interfícies d’adquisició de dades
* MQTT en detall
* Serveis al núvol
* Eines de codi lliure existents

Pràctica
* Desplegarem un servidor amb Mosquitto, NodeRED, InfluxDB i Grafana
* El connectarem a TTN per obtenir dades del sensor de la sessió 2
* Configurarem i analitzarem les possibilitats de cada servei
* Notificacions i actuació remota amb Blynk o Telegram

### 4. Comunicacions

Teoria
* Sense fils
  * Conceptes clau de ràdio-freqüència▪ Característiques de les antenes
  * Característiques de la modulació LoRa
  * Regulació
* Amb fils
  * Protocols industrials
  * Característiques de ModBUS
  * Integració ModBUS - LoRaWAN

Pràctica
* Analitzarem diferents tipus d’antena i comunicacions inalàmbriques
* Farem la connexió d’un dispositiu ModBUS a TTN mitjançant una passarel·la ModBUS a LoRaWAN

### 5. Passarel·les

Teoria
* Components d’una passarel·la
  * Host o controlador
  * Concentrador
  * Antena
  * Alimentació, connectors...
* Tipus de passarel·les (comercials, DIY, multi-protocol, mono-canal)
* Quina és la millor passarel·la? Aspectes a tenir en compte
* Protocols de redirecció

Pràctica
* Muntarem una passarel·la basada en Raspberry Pi (en grups)
* La configurarem i la connectarem a TTN

## Llicència

This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License