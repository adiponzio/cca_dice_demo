TARGET = dice-client

all: $(TARGET)

$(TARGET): cca_dice_demo.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) cca_dice_demo.c

clean:
	rm -f $(TARGET)

install:
	install -D -m 0755 $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)
