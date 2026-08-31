#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

//#include <linux/realm_dice_abi.h>
#include "cca_dice_demo.h"

#define DEVICE_PATH "/dev/realm_dice"
#define MAX_BUF_LEN 4 * 1024

void print_usage(const char *prog_name)
{
	fprintf(stdout, "Usage: %s <operation>\n", prog_name);
	fprintf(stdout, "Available operations:\n");
	fprintf(stdout, "  sign-rak    : Sign payload using RAK\n");
	fprintf(stdout, "  sign-rik    : Sign payload using RIK\n");
	fprintf(stdout, "  cert-rak    : Get RAK public key certificate\n");
	fprintf(stdout, "  cert-rik    : Get RIK public key certificate\n");
	fprintf(stdout, "  cert-chain  : Get machine certificate chain\n");
}

void save_cert(uint8_t *data, size_t len, char *filename)
{
	FILE *f = fopen(filename, "wb");
	size_t n = fwrite(data, 1, len, f);
	fclose(f);

	if (n != len)
		printf("Error while saving the certificate.");
}

void save_cert_chain(uint8_t *data)
{
	struct cert_chain *chain = (struct cert_chain*) data;
	size_t n;

	FILE *f = fopen("rmm_cert.der", "wb");
	n = fwrite(chain->cert_rmm, 1, chain->cert_rmm_len, f);
	fclose(f);

	if (n != chain->cert_rmm_len)
		printf("Error while saving RMM certificate.n=%u,len=%u.\n",
		       n, chain->cert_rmm_len);

	f = fopen("tfa_cert.der", "wb");
	n = fwrite(chain->cert_tfa, 1, chain->cert_tfa_len, f);
	fclose(f);

	if (n != chain->cert_tfa_len)
		printf("Error while saving TFA certificate.n=%u,len=%u.\n",
		       n, chain->cert_tfa_len);
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	uint8_t op_code;

	uint8_t hash_to_sign[64];
	uint8_t signature_out[64];
	uint8_t cert_out[MAX_BUF_LEN];

	struct realm_dice_req args;
	uint8_t is_cert_requested = 0;

	if (argc < 2) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	printf("[DICE Client] Opening %s...\n", DEVICE_PATH);
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("[DICE Client] Failed to open device");
		return EXIT_FAILURE;
	}
	/* Multiplex the op value according to argv[1] */
	if (strcmp(argv[1], "sign-rak") == 0) {
		op_code = RSI_DICE_OP_SIGN_RAK;
	} else if (strcmp(argv[1], "sign-rik") == 0) {
		op_code = RSI_DICE_OP_SIGN_RIK;
	} else if (strcmp(argv[1], "cert-rak") == 0) {
		op_code = RSI_DICE_OP_GET_CERT_RAK;
	} else if (strcmp(argv[1], "cert-rik") == 0) {
		op_code = RSI_DICE_OP_GET_CERT_RIK;
	} else if (strcmp(argv[1], "cert-chain") == 0) {
		op_code = RSI_DICE_OP_GET_CERT_CHAIN;
	} else {
		fprintf(stderr, "[Error] Unknown operation: %s\n\n", argv[1]);
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	// Struct defined in linux/realm_dice.h
	switch (op_code) {
	case RSI_DICE_OP_SIGN_RAK:
	case RSI_DICE_OP_SIGN_RIK:
		memset(hash_to_sign, 0xAB, sizeof(hash_to_sign));
		memset(signature_out, 0x00, sizeof(signature_out));
		args.sign.payload = (uintptr_t)hash_to_sign;
		args.sign.payload_len = sizeof(hash_to_sign);
		args.sign.signature = (uintptr_t)signature_out;
		args.sign.sig_len = sizeof(signature_out);
		break;
	case RSI_DICE_OP_GET_CERT_RAK:
	case RSI_DICE_OP_GET_CERT_RIK:
	case RSI_DICE_OP_GET_CERT_CHAIN:
		args.cert.cert_buf = (uintptr_t)cert_out;
		args.cert.cert_buf_size = MAX_BUF_LEN;
		is_cert_requested = 1;
		break;
	default:
		return EXIT_FAILURE;
	}

	//struct realm_dice_sign_args args = {
	//	.payload = (uintptr_t)hash_to_sign,
	//	.payload_len = sizeof(hash_to_sign),
	//	.signature = (uintptr_t)signature_out,
	//	.sig_len = sizeof(signature_out),
	//};
	
	printf("[DICE Client] Sending signing request ioctl to RMM via kernel...\n");
	//ret = ioctl(fd, op_code, &args);
	switch (op_code) {
	case RSI_DICE_OP_SIGN_RAK:
		ret = ioctl(fd, REALM_DICE_IOC_SIGN_RAK, &args);
		break;
	case RSI_DICE_OP_SIGN_RIK:
		ret = ioctl(fd, REALM_DICE_IOC_SIGN_RIK, &args);
		break;
	case RSI_DICE_OP_GET_CERT_RAK:
		ret = ioctl(fd, REALM_DICE_IOC_GET_CERT_RAK, &args);
		break;
	case RSI_DICE_OP_GET_CERT_RIK:
		ret = ioctl(fd, REALM_DICE_IOC_GET_CERT_RIK, &args);
		break;
	case RSI_DICE_OP_GET_CERT_CHAIN:
		ret = ioctl(fd, REALM_DICE_IOC_GET_CERT_CHAIN, &args);
		break;
	default:
		break;
	}

	close(fd);
	
	if (ret < 0) {
		perror("[DICE Client] IOCTL request failed");
		return EXIT_FAILURE;
	}
	
	if (is_cert_requested) {
		/* The kernel should have updated cert_len with the exact bytes written */
		printf("[DICE Client] Success! Certificate received.\n");
		printf("[DICE Client] Exact size: %llu bytes\n", args.cert.cert_len);
		
		switch (op_code) {
		case RSI_DICE_OP_GET_CERT_CHAIN:
			save_cert_chain(cert_out);
			break;
		case RSI_DICE_OP_GET_CERT_RAK:
			save_cert(cert_out, args.cert.cert_len, "rak_cert.der");
			break;
		case RSI_DICE_OP_GET_CERT_RIK:
			save_cert(cert_out, args.cert.cert_len, "rik_cert.der");
			break;
		}

		printf("[DICE Client] Certificate content: ");
		for (int i = 0; i < args.cert.cert_len; i++) {
			printf("%02x ", cert_out[i]);
			if ((i + 1) % 16 == 0) printf("\n");
		}
		printf("\n");
	} else {
		printf("[DICE Client] Success! Signature received (Size: %llu bytes):\n",
		       args.sign.sig_len);
		for (int i = 0; i < args.sign.sig_len; i++) {
			printf("%02x ", signature_out[i]);
			if ((i + 1) % 16 == 0) printf("\n");
		}
		if (args.sign.sig_len % 16 != 0) printf("\n");
	}

	//printf("[DICE Client] Success! Signature received from RMM:\n");
	//for (int i = 0; i < 64; i++) {
	//	printf("%02x ", signature_out[i]);
	//	if ((i + 1) % 16 == 0) printf("\n");
	//}
	
	return EXIT_SUCCESS;
}
