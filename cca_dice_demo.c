#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/realm_dice.h>

#define DEVICE_PATH "/dev/realm_dice"

int main(void) {
	int fd;
	int ret;
	
	uint8_t hash_to_sign[64];
	uint8_t signature_out[64];
	
	memset(hash_to_sign, 0xAB, sizeof(hash_to_sign));
	memset(signature_out, 0x00, sizeof(signature_out));
	
	printf("[DICE Client] Opening %s...\n", DEVICE_PATH);
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("[DICE Client] Failed to open device");
		return EXIT_FAILURE;
	}
	
	// Struct defined in linux/realm_dice.h
	struct realm_dice_sign_args args = {
		.payload = (uintptr_t)hash_to_sign,
		.payload_len = sizeof(hash_to_sign),
		.signature = (uintptr_t)signature_out,
		.sig_len = sizeof(signature_out)
	};
	
	printf("[DICE Client] Sending signing request ioctl to RMM via kernel...\n");
	ret = ioctl(fd, REALM_DICE_IOC_SIGN, &args);
	close(fd);
	
	if (ret < 0) {
		perror("[DICE Client] IOCTL request failed");
		return EXIT_FAILURE;
	}
	
	printf("[DICE Client] Success! Signature received from RMM:\n");
	for (int i = 0; i < 64; i++) {
		printf("%02x ", signature_out[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	
	return EXIT_SUCCESS;
}
