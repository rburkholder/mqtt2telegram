# mqtt2telegram

Build:
* need to build and install https://github.com/rburkholder/repertory with '-D OU_USE_Telegram=ON'

To Build statically linked application:
```
git clone https://github.com/rburkholder/mqtt2telegram.git
cd mqtt2telegram
mkdir build
cd build
cmake ..
make
cd ..
```

Configuration File template (change usernames, passwords, and addresses):
```
$ cat x64/debug/mqtt2telegram.cfg
mqtt_id = reader
mqtt_host = 127.0.0.1
mqtt_username = admin
mqtt_password = password
mqtt_topic = nut

telegram_token = 64713789....
telegram_chat_id = 10
```

### Telegram Messaging

* [BotFather Token](https://core.telegram.org/bots/tutorial) - create bot, obtain token
* populate the telegram_token in choices.cfg
* start application
* from your regular telegram account, send some text to the bot to register chat id
* use menu SendMessage to send a test message to confirm chat id
* chat id will persist in the state file for next startup
* in strategy file, to send message: m_fTelegram( "message text" );

