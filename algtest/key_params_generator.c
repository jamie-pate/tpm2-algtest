#include "options.h"
#include "key_params_generator.h"

extern struct tpm_algtest_options options;

bool get_next_rsa_keylen(TPMT_PUBLIC_PARMS* key_params)
{
    while (key_params->parameters.rsaDetail.keyBits <= 4096) {
        key_params->parameters.rsaDetail.keyBits += 32;
        if (keylen_in_options(key_params->parameters.rsaDetail.keyBits)) {
            return true;
        }
    }
    return false;
}

bool get_next_ecc_curve(TPMT_PUBLIC_PARMS* key_params)
{
    while (key_params->parameters.eccDetail.curveID <= 0x0020) {
        ++key_params->parameters.eccDetail.curveID;
        if (curve_in_options(key_params->parameters.eccDetail.curveID)) {
            return true;
        }
    }
    return false;
}

// TODO: at the moment this is sign specific
bool get_next_key_type(TPMT_PUBLIC_PARMS *key_params) {
    switch (key_params->type) {
    case TPM2_ALG_NULL:
        key_params->type = TPM2_ALG_RSA;
        if (type_in_options("rsa")) {
            key_params->parameters.rsaDetail = (TPMS_RSA_PARMS) {
                .symmetric = TPM2_ALG_NULL,
                .scheme = {
                    .scheme = TPM2_ALG_RSASSA,
                    .details = { .rsassa = { .hashAlg = TPM2_ALG_SHA256 } },
                },
                .keyBits = 0,
                .exponent = 0
            };
            return true;
        }
    case TPM2_ALG_RSA:
        key_params->type = TPM2_ALG_ECC;
        if (type_in_options("ecc")) {
            key_params->parameters.eccDetail = (TPMS_ECC_PARMS) {
                .symmetric = TPM2_ALG_NULL,
                .scheme = {
                    .scheme = TPM2_ALG_ECDSA,
                    .details = { .ecdsa = { .hashAlg = TPM2_ALG_SHA256 } },
                },
                .curveID = 0x0000,
                .kdf = TPM2_ALG_NULL
            };
            return true;
        }
    case TPM2_ALG_ECC:
        return false;
    default:
        return false;
    }
}

bool get_next_key_params(TPMT_PUBLIC_PARMS *key_params)
{
    switch (key_params->type) {
    case TPM2_ALG_NULL:
        return get_next_key_type(key_params);
    case TPM2_ALG_RSA:
        if (get_next_rsa_keylen(key_params)) {
            return true;
        } else {
            return get_next_key_type(key_params);
        }
    case TPM2_ALG_ECC:
        if (get_next_ecc_curve(key_params)) {
            return true;
        } else {
            return get_next_key_type(key_params);
        }
    default:
        return false;
    }
}