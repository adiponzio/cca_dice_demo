#ifndef _CCA_DICE_DEMO_H_
#define _CCA_DICE_DEMO_H_

#define MAX_CERT_LEN 1024

struct cert_chain {
	uint8_t cert_rmm[MAX_CERT_LEN];
	size_t cert_rmm_len;

	uint8_t cert_tfa[MAX_CERT_LEN];
	size_t cert_tfa_len;
};

#endif /* _CCA_DICE_DEMO_H_ */
