#HTTP -MQTT wrapper 
the target of this project is to build a simple HTTP wrapper/web-interface using standard C and CGI for the mosquito MQTT broker client and showcase it using a freeboard variant with paho MQTT as data source  

## deployment notes

### api images

- compile `sendMessage.c` with gcc
   - no special flags or libs are required for this. hence no make file.
   - relies on mosquito server and client installed and deployed in the same server.
- rename and place output in the configured apache api folder.
- rename and copy `listen.sh` into api folder.

### apache api configuration

- enter configuration in `/etc/conf-available/serve-cgi-bin.conf` and `./conf-enabled/serve-cgi-bin.conf`

```xml
<IfDefine ENABLE_USR_LIB_CGI_BIN>
ScriptAlias /api/ /var/www/api/
<Directory "/var/www/api">
AllowOverride None
Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
Require all granted
</Directory>
</IfDefine>
```

### mosquitto server and client 

- in ubuntu , install `mosquitto-clients` and `mosquitto`
- enable ws and otehr config in `/etc/mosquitto/mosquitto.conf` 

```bash
pid_file /var/run/mosquitto.pid

persistence true
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log

include_dir /etc/mosquitto/conf.d

allow_anonymous true 

autosave_interval 1800
persistence_file m2.db
connection_messages true
log_timestamp true


listener 1883 

listener 9001 
protocol websockets

``` 

**Note** This project is under active development. Once i test it out to my satisfaction, I will publish complete deployment instructions and throughput numbers
