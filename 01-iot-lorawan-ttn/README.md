# Recursos pràctics per la sessió 1

## Consola The Things Network

[Consola TTN](https://eu1.cloud.thethings.network)

## Automatització de tasques amb la consola de TTN

### Instal·lar ttn-lw-cli

[Documentació d'instal·lació](https://www.thethingsindustries.com/docs/getting-started/cli/installing-cli/)

Configurar:

```
ttn-lw-cli use eu1.cloud.thethings.network
ttn-lw-cli login
```

### Alta de gateway

```
GATEWAY_ID=v22-gw01
USER_ID=xoseperez-test
FREQUENCY_PLAN=EU_863_870
GATEWAY_EUI=1122334455667788
ttn-lw-cli gateways create $GATEWAY_ID --user-id $USER_ID --frequency-plan-id $FREQUENCY_PLAN --gateway-eui $GATEWAY_EUI --enforce-duty-cycle
```

### Alta d'aplicació

```
APP_ID=v22-test-app01
USER_ID=xoseperez-test
ttn-lw-cli applications create $APP_ID --user-id $USER_ID
ttn-lw-cli application link set $APP_ID --default-formatters.up-formatter FORMATTER_CAYENNELPP
```

### Alta de dispositius

```
APP_ID=v22-test-app01
USER_ID=xoseperez-test
DEVICE_ID=dev01
DEVEUI=72B3D57ED00511AC
APPEUI=0000000000000000
APPKEY=EAA72BB8C812C047BF68B8896636D25E
FREQUENCY_PLAN=EU_863_870
ttn-lw-cli device create $APP_ID $DEVICE_ID --join-eui $APPEUI --dev-eui $DEVEUI --root-keys.app-key.key $APPKEY --frequency-plan-id $FREQUENCY_PLAN --lorawan-version 1.0.3 --lorawan-phy-version 1.0.3-a
```

## Servidor *on-premise*

Opcions de codi obert:

* The Things Stack (software en el que es basa TTN)
    * [Repositori](https://github.com/TheThingsNetwork/lorawan-stack)
    * [Imatge Docker oficial](https://hub.docker.com/r/thethingsnetwork/lorawan-stack/)
    * [Imatge Docker augmentada](https://hub.docker.com/r/xoseperez/the-things-stack)    
    * [Documentació](https://github.com/TheThingsNetwork/lorawan-stack/blob/v3.22/DEVELOPMENT.md)

* Chirpstack
    * [Repositori](https://github.com/brocaar)
    * [Imatge Docker oficial](https://hub.docker.com/u/chirpstack)
    * [Documentació](https://www.chirpstack.io/)

