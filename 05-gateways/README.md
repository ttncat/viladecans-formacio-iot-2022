# Recursos pràctics per la sessió 5

## Sistema operatiu Balena

BalenaOS és un sistema operatiu gestionat de manera remota des del [Balena Dashboard](https://dashboard.balena-cloud.com/) que permet el desplegament de flotes de dispositius.

Per desplegar les passarel·les farem servir una imatge especialment deissenyada per això i que conté:

* UDP Packet Forwarder: senzill protocol de comunicació entre una passarel·la i un LNS
* WiFi Connect: eina que exposa un Punt d'Accés cautiu que permet configurar la connexió WiFi del dispositiu des d'un mòbil

El projecte i les instruccions de desplegament es poden trobar aquí:

https://github.com/ttncat/balena-legacy-gateway