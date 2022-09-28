# TCP-sender

Send hex (writen in files as numbers) to tcp socket.
App make new TCP connection & try to send all packages in one session, sleep 15 seconds after all & send RST.

Usage:
```bash
./tcp_sender-console -c config_file
```

Config Example:
```ini
[DEFAULT]

; after this count do abort
catch_sigint_cnt = 3

address = 10.0.0.16 ; destination address
port = 21400 ; destination port
latency = 100 ; milliseconds before sending every package

data = \
00000006000501650002,\
000000020201 ; and other packages 
```

