#ifndef _CCA_DICE_DEMO_H_
#define _CCA_DICE_DEMO_H_

struct cert_chain {
	uint8_t cert_rmm[400];
	size_t cert_rmm_len;

	uint8_t cert_tfa[400];
	size_t cert_tfa_len;
};

#endif /* _CCA_DICE_DEMO_H_ */
