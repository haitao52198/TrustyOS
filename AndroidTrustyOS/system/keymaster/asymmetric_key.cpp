/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "asymmetric_key.h"

#include <new>

#include <openssl/asn1.h>
#include <openssl/stack.h>
#include <openssl/x509.h>

#include "attestation_record.h"
#include "openssl_err.h"
#include "openssl_utils.h"

namespace keymaster {

keymaster_error_t AsymmetricKey::formatted_key_material(keymaster_key_format_t format,
                                                        UniquePtr<uint8_t[]>* material,
                                                        size_t* size) const {
    if (format != KM_KEY_FORMAT_X509)
        return KM_ERROR_UNSUPPORTED_KEY_FORMAT;

    if (material == NULL || size == NULL)
        return KM_ERROR_OUTPUT_PARAMETER_NULL;

    EVP_PKEY_Ptr pkey(EVP_PKEY_new());
    if (!InternalToEvp(pkey.get()))
        return TranslateLastOpenSslError();

    int key_data_length = i2d_PUBKEY(pkey.get(), NULL);
    if (key_data_length <= 0)
        return TranslateLastOpenSslError();

    material->reset(new (std::nothrow) uint8_t[key_data_length]);
    if (material->get() == NULL)
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    uint8_t* tmp = material->get();
    if (i2d_PUBKEY(pkey.get(), &tmp) != key_data_length) {
        material->reset();
        return TranslateLastOpenSslError();
    }

    *size = key_data_length;
    return KM_ERROR_OK;
}

static keymaster_error_t build_attestation_extension(const AuthorizationSet& tee_enforced,
                                                     const AuthorizationSet& sw_enforced,
                                                     X509_EXTENSION_Ptr* extension) {
    ASN1_OBJECT_Ptr oid(
        OBJ_txt2obj(kAttestionRecordOid, 1 /* accept numerical dotted string form only */));
    if (!oid.get())
        return TranslateLastOpenSslError();

    UniquePtr<uint8_t[]> attest_bytes;
    size_t attest_bytes_len;
    keymaster_error_t error =
        build_attestation_record(sw_enforced, tee_enforced, &attest_bytes, &attest_bytes_len);
    if (error != KM_ERROR_OK)
        return error;

    ASN1_OCTET_STRING_Ptr attest_str(ASN1_OCTET_STRING_new());
    if (!attest_str.get() ||
        !ASN1_OCTET_STRING_set(attest_str.get(), attest_bytes.get(), attest_bytes_len))
        return TranslateLastOpenSslError();

    extension->reset(
        X509_EXTENSION_create_by_OBJ(nullptr, oid.get(), 0 /* not critical */, attest_str.get()));
    if (!extension->get())
        return TranslateLastOpenSslError();

    return KM_ERROR_OK;
}

static bool add_public_key(EVP_PKEY* key, X509* certificate, keymaster_error_t* error) {
    if (!X509_set_pubkey(certificate, key)) {
        *error = TranslateLastOpenSslError();
        return false;
    }
    return true;
}

static bool add_attestation_extension(const AuthorizationSet& tee_enforced,
                                      const AuthorizationSet& sw_enforced, X509* certificate,
                                      keymaster_error_t* error) {
    X509_EXTENSION_Ptr attest_extension;
    *error = build_attestation_extension(tee_enforced, sw_enforced, &attest_extension);
    if (*error != KM_ERROR_OK)
        return false;

    if (!X509_add_ext(certificate, attest_extension.get() /* Don't release; copied */,
                      -1 /* insert at end */)) {
        *error = TranslateLastOpenSslError();
        return false;
    }

    return true;
}

static keymaster_error_t get_certificate_blob(X509* certificate, keymaster_blob_t* blob) {
    int len = i2d_X509(certificate, nullptr);
    if (len < 0)
        return TranslateLastOpenSslError();

    uint8_t* data = new uint8_t[len];
    if (!data)
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    uint8_t* p = data;
    i2d_X509(certificate, &p);

    blob->data_length = len;
    blob->data = data;

    return KM_ERROR_OK;
}

static bool allocate_cert_chain(size_t entry_count, keymaster_cert_chain_t* chain,
                                keymaster_error_t* error) {
    if (chain->entries) {
        for (size_t i = 0; i < chain->entry_count; ++i)
            delete[] chain->entries[i].data;
        delete[] chain->entries;
    }

    chain->entry_count = entry_count;
    chain->entries = new keymaster_blob_t[entry_count];
    if (!chain->entries) {
        *error = KM_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    return true;
}

// Copies the intermediate and root certificates into chain, leaving the first slot for the leaf
// certificate.
static bool copy_attestation_chain(const KeymasterContext& context,
                                   keymaster_algorithm_t sign_algorithm,
                                   keymaster_cert_chain_t* chain, keymaster_error_t* error) {

    UniquePtr<keymaster_cert_chain_t, CertificateChainDelete> attest_key_chain(
        context.AttestationChain(sign_algorithm, error));
    if (!attest_key_chain.get())
        return false;

    if (!allocate_cert_chain(attest_key_chain->entry_count + 1, chain, error))
        return false;

    chain->entries[0].data = nullptr;  // Leave empty for the leaf certificate.
    chain->entries[0].data_length = 0;

    for (size_t i = 0; i < attest_key_chain->entry_count; ++i) {
        chain->entries[i + 1] = attest_key_chain->entries[i];
        attest_key_chain->entries[i].data = nullptr;
    }

    return true;
}

keymaster_error_t AsymmetricKey::GenerateAttestation(const KeymasterContext& context,
                                                     const AuthorizationSet& attest_params,
                                                     const AuthorizationSet& tee_enforced,
                                                     const AuthorizationSet& sw_enforced,
                                                     keymaster_cert_chain_t* cert_chain) const {

    keymaster_algorithm_t sign_algorithm;
    if (!attest_params.GetTagValue(TAG_ALGORITHM, &sign_algorithm) ||
        (sign_algorithm != KM_ALGORITHM_RSA && sign_algorithm != KM_ALGORITHM_EC))
        return KM_ERROR_INCOMPATIBLE_ALGORITHM;

    EVP_PKEY_Ptr pkey(EVP_PKEY_new());
    if (!InternalToEvp(pkey.get()))
        return TranslateLastOpenSslError();

    X509_Ptr certificate(X509_new());
    if (!certificate.get())
        return TranslateLastOpenSslError();

    if (!X509_set_version(certificate.get(), 2 /* version 3, but zero-based */))
        return TranslateLastOpenSslError();

    ASN1_INTEGER_Ptr serialNumber(ASN1_INTEGER_new());
    if (!serialNumber.get() ||
        !ASN1_INTEGER_set(
            serialNumber.get(),
            10000 /* TODO(swillden): Figure out what should go in serial number; probably a random
                   * value */) ||
        !X509_set_serialNumber(certificate.get(), serialNumber.get() /* Don't release; copied */))
        return TranslateLastOpenSslError();

    // TODO(swillden): Find useful values (if possible) for issuerName and subjectName.
    X509_NAME_Ptr issuerName(X509_NAME_new());
    if (!issuerName.get() ||
        !X509_set_subject_name(certificate.get(), issuerName.get() /* Don't release; copied  */))
        return TranslateLastOpenSslError();

    X509_NAME_Ptr subjectName(X509_NAME_new());
    if (!subjectName.get() ||
        !X509_set_subject_name(certificate.get(), subjectName.get() /* Don't release; copied */))
        return TranslateLastOpenSslError();

    // TODO(swillden): Use key activity and expiration dates for notBefore and notAfter.
    ASN1_TIME_Ptr notBefore(ASN1_TIME_new());
    if (!notBefore.get() || !ASN1_TIME_set(notBefore.get(), 0) ||
        !X509_set_notBefore(certificate.get(), notBefore.get() /* Don't release; copied */))
        return TranslateLastOpenSslError();

    ASN1_TIME_Ptr notAfter(ASN1_TIME_new());
    if (!notAfter.get() || !ASN1_TIME_set(notAfter.get(), 10000) ||
        !X509_set_notAfter(certificate.get(), notAfter.get() /* Don't release; copied */))
        return TranslateLastOpenSslError();

    keymaster_error_t error = KM_ERROR_OK;
    EVP_PKEY_Ptr sign_key(context.AttestationKey(sign_algorithm, &error));

    if (!sign_key.get() ||  //
        !add_public_key(pkey.get(), certificate.get(), &error) ||
        !add_attestation_extension(tee_enforced, sw_enforced, certificate.get(), &error))
        return error;

    if (!X509_sign(certificate.get(), sign_key.get(), EVP_sha256()))
        return TranslateLastOpenSslError();

    if (!copy_attestation_chain(context, sign_algorithm, cert_chain, &error))
        return error;

    return get_certificate_blob(certificate.get(), &cert_chain->entries[0]);
}

}  // namespace keymaster
