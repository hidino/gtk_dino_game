rm app
gcc app.c -o app `pkg-config --cflags --libs gtk+-3.0`
./app
