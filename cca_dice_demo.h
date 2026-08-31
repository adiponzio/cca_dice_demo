#ifndef __CCA_DICE_DEMO_H__
#define __CCA_DICE_DEMO_H__

#define MAX_CERT_LEN 1024

struct cert_chain {
	uint8_t cert_rmm[MAX_CERT_LEN];
	size_t cert_rmm_len;

	uint8_t cert_tfa[MAX_CERT_LEN];
	size_t cert_tfa_len;
};

/*
 * The following lines are the content of the "linux/realm_dice_abi.h" header
 * in the user API include files. However, since this test application is 
 * cross-compiled outside the Realm OS, that header is not available at build
 * time. The workaround consists of reporting the same content of that header
 * here in order to make possible the compilation with buildroot.
 * 
 * As future development a better solution should be found.
 */

#include <linux/ioctl.h>
#include <linux/types.h>

#define RSI_DICE_OP_SIGN_RAK		0
#define RSI_DICE_OP_SIGN_RIK		1
#define RSI_DICE_OP_GET_CERT_RAK        2
#define RSI_DICE_OP_GET_CERT_RIK        3
#define RSI_DICE_OP_GET_CERT_CHAIN      4

struct realm_dice_req {
	union {
		/* Used for REALM_DICE_IOC_SIGN_* */
		struct {
			__u64 payload;      /* User pointer to data to sign */
			__u64 payload_len;
			__u64 signature;    /* User pointer for output */
			__u64 sig_len;      /* In: buffer size, Out: actual size */
		} sign;

		/* Used for REALM_DICE_IOC_GET_CERT_* */
		struct {
			__u64 cert_buf;	  /* User pointer for output */
			__u64 cert_buf_size;
			__u64 cert_len;       /* In: buffer size, Out: actual size */
		} cert;
	};
};


#define REALM_DICE_IOC_SIGN_RAK		_IOWR('D', 0, struct realm_dice_req)
#define REALM_DICE_IOC_SIGN_RIK		_IOWR('D', 1, struct realm_dice_req)
#define REALM_DICE_IOC_GET_CERT_RAK	_IOR('D', 2, struct realm_dice_req)
#define REALM_DICE_IOC_GET_CERT_RIK	_IOR('D', 3, struct realm_dice_req)
#define REALM_DICE_IOC_GET_CERT_CHAIN	_IOR('D', 4, struct realm_dice_req)
#endif /* __CCA_DICE_DEMO_H__ */
