TARGET = dice-client

all: $(TARGET)

$(TARGET): dice-client.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) dice-client.c

clean:
	rm -f $(TARGET)

install:
	install -D -m 0755 $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)
