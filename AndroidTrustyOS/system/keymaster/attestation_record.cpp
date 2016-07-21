/*
 * Copyright 2016 The Android Open Source Project
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

#include "attestation_record.h"

#include <assert.h>

#include <openssl/asn1t.h>

#include "openssl_err.h"
#include "openssl_utils.h"

namespace keymaster {

struct stack_st_ASN1_TYPE_Delete {
    void operator()(stack_st_ASN1_TYPE* p) { sk_ASN1_TYPE_free(p); }
};

struct ASN1_STRING_Delete {
    void operator()(ASN1_STRING* p) { ASN1_STRING_free(p); }
};

struct ASN1_TYPE_Delete {
    void operator()(ASN1_TYPE* p) { ASN1_TYPE_free(p); }
};

#define ASN1_INTEGER_SET STACK_OF(ASN1_INTEGER)

typedef struct km_root_of_trust {
    ASN1_OCTET_STRING* verified_boot_key;
    ASN1_INTEGER* os_version;
    ASN1_INTEGER* os_patchlevel;
} KM_ROOT_OF_TRUST;

ASN1_SEQUENCE(KM_ROOT_OF_TRUST) = {
    ASN1_SIMPLE(KM_ROOT_OF_TRUST, verified_boot_key, ASN1_OCTET_STRING),
    ASN1_SIMPLE(KM_ROOT_OF_TRUST, os_version, ASN1_INTEGER),
    ASN1_SIMPLE(KM_ROOT_OF_TRUST, os_patchlevel, ASN1_INTEGER),
} ASN1_SEQUENCE_END(KM_ROOT_OF_TRUST);
IMPLEMENT_ASN1_FUNCTIONS(KM_ROOT_OF_TRUST);

typedef struct km_auth_list {
    ASN1_INTEGER_SET* purpose;
    ASN1_INTEGER* algorithm;
    ASN1_INTEGER* key_size;
    ASN1_INTEGER_SET* block_mode;
    ASN1_INTEGER_SET* digest;
    ASN1_INTEGER_SET* padding;
    ASN1_NULL* caller_nonce;
    ASN1_INTEGER* min_mac_length;
    ASN1_INTEGER_SET* kdf;
    ASN1_INTEGER* ec_curve;
    ASN1_INTEGER* rsa_public_exponent;
    ASN1_NULL* ecies_single_hash_mode;
    ASN1_NULL* include_unique_id;
    ASN1_INTEGER* blob_usage_requirement;
    ASN1_NULL* bootloader_only;
    ASN1_INTEGER* active_date_time;
    ASN1_INTEGER* origination_expire_date_time;
    ASN1_INTEGER* usage_expire_date_time;
    ASN1_INTEGER* min_seconds_between_ops;
    ASN1_INTEGER* max_uses_per_boot;
    ASN1_NULL* no_auth_required;
    ASN1_INTEGER* user_auth_type;
    ASN1_INTEGER* auth_timeout;
    ASN1_NULL* allow_while_on_body;
    ASN1_NULL* all_applications;
    ASN1_OCTET_STRING* application_id;
    ASN1_OCTET_STRING* application_data;
    ASN1_INTEGER* creation_date_time;
    ASN1_INTEGER* origin;
    ASN1_NULL* rollback_resistant;
    KM_ROOT_OF_TRUST* root_of_trust;
    ASN1_INTEGER* os_version;
    ASN1_INTEGER* os_patchlevel;
    ASN1_OCTET_STRING* unique_id;
} KM_AUTH_LIST;

ASN1_SEQUENCE(KM_AUTH_LIST) = {
    ASN1_IMP_SET_OF_OPT(KM_AUTH_LIST, purpose, ASN1_INTEGER, TAG_PURPOSE.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, algorithm, ASN1_INTEGER, TAG_ALGORITHM.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, key_size, ASN1_INTEGER, TAG_KEY_SIZE.masked_tag()),
    ASN1_IMP_SET_OF_OPT(KM_AUTH_LIST, block_mode, ASN1_INTEGER, TAG_BLOCK_MODE.masked_tag()),
    ASN1_IMP_SET_OF_OPT(KM_AUTH_LIST, digest, ASN1_INTEGER, TAG_DIGEST.masked_tag()),
    ASN1_IMP_SET_OF_OPT(KM_AUTH_LIST, padding, ASN1_INTEGER, TAG_PADDING.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, caller_nonce, ASN1_NULL, TAG_CALLER_NONCE.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, min_mac_length, ASN1_INTEGER, TAG_MIN_MAC_LENGTH.masked_tag()),
    ASN1_IMP_SET_OF_OPT(KM_AUTH_LIST, kdf, ASN1_INTEGER, TAG_KDF.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, ec_curve, ASN1_INTEGER, TAG_EC_CURVE.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, rsa_public_exponent, ASN1_INTEGER,
                 TAG_RSA_PUBLIC_EXPONENT.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, ecies_single_hash_mode, ASN1_NULL,
                 TAG_ECIES_SINGLE_HASH_MODE.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, include_unique_id, ASN1_NULL, TAG_INCLUDE_UNIQUE_ID.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, blob_usage_requirement, ASN1_INTEGER,
                 TAG_BLOB_USAGE_REQUIREMENTS.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, bootloader_only, ASN1_NULL, TAG_BOOTLOADER_ONLY.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, active_date_time, ASN1_INTEGER, TAG_ACTIVE_DATETIME.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, origination_expire_date_time, ASN1_INTEGER,
                 TAG_ORIGINATION_EXPIRE_DATETIME.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, usage_expire_date_time, ASN1_INTEGER,
                 TAG_USAGE_EXPIRE_DATETIME.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, min_seconds_between_ops, ASN1_INTEGER,
                 TAG_MIN_SECONDS_BETWEEN_OPS.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, max_uses_per_boot, ASN1_INTEGER, TAG_MAX_USES_PER_BOOT.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, no_auth_required, ASN1_NULL, TAG_NO_AUTH_REQUIRED.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, user_auth_type, ASN1_INTEGER, TAG_USER_AUTH_TYPE.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, auth_timeout, ASN1_INTEGER, TAG_AUTH_TIMEOUT.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, allow_while_on_body, ASN1_NULL,
                 TAG_ALLOW_WHILE_ON_BODY.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, all_applications, ASN1_NULL, TAG_ALL_APPLICATIONS.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, application_id, ASN1_OCTET_STRING, TAG_APPLICATION_ID.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, application_data, ASN1_OCTET_STRING,
                 TAG_APPLICATION_DATA.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, creation_date_time, ASN1_INTEGER,
                 TAG_CREATION_DATETIME.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, origin, ASN1_INTEGER, TAG_ORIGIN.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, rollback_resistant, ASN1_NULL, TAG_ROLLBACK_RESISTANT.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, root_of_trust, KM_ROOT_OF_TRUST, TAG_ROOT_OF_TRUST.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, os_version, ASN1_INTEGER, TAG_OS_VERSION.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, os_patchlevel, ASN1_INTEGER, TAG_OS_PATCHLEVEL.masked_tag()),
    ASN1_IMP_OPT(KM_AUTH_LIST, unique_id, ASN1_NULL, TAG_UNIQUE_ID.masked_tag()),
} ASN1_SEQUENCE_END(KM_AUTH_LIST);
IMPLEMENT_ASN1_FUNCTIONS(KM_AUTH_LIST);

typedef struct km_key_description {
    KM_AUTH_LIST* software_enforced;
    KM_AUTH_LIST* tee_enforced;
} KM_KEY_DESCRIPTION;

ASN1_SEQUENCE(KM_KEY_DESCRIPTION) = {
    ASN1_SIMPLE(KM_KEY_DESCRIPTION, software_enforced, KM_AUTH_LIST),
    ASN1_SIMPLE(KM_KEY_DESCRIPTION, tee_enforced, KM_AUTH_LIST),
} ASN1_SEQUENCE_END(KM_KEY_DESCRIPTION);
IMPLEMENT_ASN1_FUNCTIONS(KM_KEY_DESCRIPTION);

struct KM_AUTH_LIST_Delete {
    void operator()(KM_AUTH_LIST* p) { KM_AUTH_LIST_free(p); }
};

struct KM_KEY_DESCRIPTION_Delete {
    void operator()(KM_KEY_DESCRIPTION* p) { KM_KEY_DESCRIPTION_free(p); }
};

static uint32_t get_uint32_value(const keymaster_key_param_t& param) {
    switch (keymaster_tag_get_type(param.tag)) {
    case KM_ENUM:
    case KM_ENUM_REP:
        return param.enumerated;
    case KM_UINT:
    case KM_UINT_REP:
        return param.integer;
    default:
        assert(false);
        return 0xFFFFFFFF;
    }
}

// Insert value in either the dest_integer or the dest_integer_set, whichever is provided.
static keymaster_error_t insert_integer(ASN1_INTEGER* value, ASN1_INTEGER** dest_integer,
                                        ASN1_INTEGER_SET** dest_integer_set) {
    assert((dest_integer == nullptr) ^ (dest_integer_set == nullptr));
    assert(value);

    if (dest_integer_set) {
        if (!*dest_integer_set)
            *dest_integer_set = sk_ASN1_INTEGER_new_null();
        if (!*dest_integer_set)
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        if (!sk_ASN1_INTEGER_push(*dest_integer_set, value))
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        return KM_ERROR_OK;

    } else if (dest_integer) {
        if (*dest_integer)
            ASN1_INTEGER_free(*dest_integer);
        *dest_integer = value;
        return KM_ERROR_OK;
    }

    assert(false);  // Should never get here.
    return KM_ERROR_OK;
}

// Put the contents of the keymaster AuthorizationSet auth_list in to the ASN.1 record structure,
// record.
static keymaster_error_t build_auth_list(const AuthorizationSet& auth_list, KM_AUTH_LIST* record) {

    assert(record);

    for (auto entry : auth_list) {

        ASN1_INTEGER_SET** integer_set = nullptr;
        ASN1_INTEGER** integer_ptr = nullptr;
        ASN1_OCTET_STRING** string_ptr = nullptr;
        ASN1_NULL** bool_ptr = nullptr;

        switch (entry.tag) {

        /* Ignored tags */
        case KM_TAG_INVALID:
        case KM_TAG_ASSOCIATED_DATA:
        case KM_TAG_NONCE:
        case KM_TAG_AUTH_TOKEN:
        case KM_TAG_MAC_LENGTH:
        case KM_TAG_ALL_USERS:
        case KM_TAG_USER_ID:
        case KM_TAG_USER_SECURE_ID:
        case KM_TAG_EXPORTABLE:
        case KM_TAG_RESET_SINCE_ID_ROTATION:
            continue;

        /* Non-repeating enumerations */
        case KM_TAG_ALGORITHM:
            integer_ptr = &record->algorithm;
            break;
        case KM_TAG_EC_CURVE:
            integer_ptr = &record->ec_curve;
            break;
        case KM_TAG_BLOB_USAGE_REQUIREMENTS:
            integer_ptr = &record->blob_usage_requirement;
            break;
        case KM_TAG_USER_AUTH_TYPE:
            integer_ptr = &record->user_auth_type;
            break;
        case KM_TAG_ORIGIN:
            integer_ptr = &record->origin;
            break;

        /* Repeating enumerations */
        case KM_TAG_PURPOSE:
            integer_set = &record->purpose;
            break;
        case KM_TAG_BLOCK_MODE:
            integer_set = &record->block_mode;
            break;
        case KM_TAG_PADDING:
            integer_set = &record->padding;
            break;
        case KM_TAG_DIGEST:
            integer_set = &record->digest;
            break;
        case KM_TAG_KDF:
            integer_set = &record->kdf;
            break;

        /* Non-repeating unsigned integers */
        case KM_TAG_KEY_SIZE:
            integer_ptr = &record->key_size;
            break;
        case KM_TAG_MIN_MAC_LENGTH:
            integer_ptr = &record->min_mac_length;
            break;
        case KM_TAG_MIN_SECONDS_BETWEEN_OPS:
            integer_ptr = &record->min_seconds_between_ops;
            break;
        case KM_TAG_MAX_USES_PER_BOOT:
            integer_ptr = &record->max_uses_per_boot;
            break;
        case KM_TAG_AUTH_TIMEOUT:
            integer_ptr = &record->auth_timeout;
            break;

        /* Non-repeating long unsigned integers */
        case KM_TAG_RSA_PUBLIC_EXPONENT:
            integer_ptr = &record->rsa_public_exponent;
            break;

        /* Dates */
        case KM_TAG_ACTIVE_DATETIME:
            integer_ptr = &record->active_date_time;
            break;
        case KM_TAG_ORIGINATION_EXPIRE_DATETIME:
            integer_ptr = &record->origination_expire_date_time;
            break;
        case KM_TAG_USAGE_EXPIRE_DATETIME:
            integer_ptr = &record->usage_expire_date_time;
            break;
        case KM_TAG_CREATION_DATETIME:
            integer_ptr = &record->creation_date_time;
            break;

        /* Booleans */
        case KM_TAG_CALLER_NONCE:
            bool_ptr = &record->caller_nonce;
            break;
        case KM_TAG_ECIES_SINGLE_HASH_MODE:
            bool_ptr = &record->ecies_single_hash_mode;
            break;
        case KM_TAG_BOOTLOADER_ONLY:
            bool_ptr = &record->bootloader_only;
            break;
        case KM_TAG_NO_AUTH_REQUIRED:
            bool_ptr = &record->no_auth_required;
            break;
        case KM_TAG_ALL_APPLICATIONS:
            bool_ptr = &record->all_applications;
            break;
        case KM_TAG_ROLLBACK_RESISTANT:
            bool_ptr = &record->rollback_resistant;
            break;
        case KM_TAG_INCLUDE_UNIQUE_ID:
            bool_ptr = &record->include_unique_id;
            break;
        case KM_TAG_ALLOW_WHILE_ON_BODY:
            bool_ptr = &record->allow_while_on_body;
            break;

        /* Byte arrays*/
        case KM_TAG_APPLICATION_ID:
            string_ptr = &record->application_id;
            break;
        case KM_TAG_APPLICATION_DATA:
            string_ptr = &record->application_data;
            break;
        case KM_TAG_UNIQUE_ID:
            string_ptr = &record->unique_id;
            break;

        /* Root of Trust components */
        case KM_TAG_OS_VERSION:
        case KM_TAG_OS_PATCHLEVEL:
        case KM_TAG_ROOT_OF_TRUST:
            if (!record->root_of_trust)
                record->root_of_trust = KM_ROOT_OF_TRUST_new();
            if (!record->root_of_trust)
                return KM_ERROR_MEMORY_ALLOCATION_FAILED;
            switch (entry.tag) {
            case KM_TAG_OS_VERSION:
                integer_ptr = &record->root_of_trust->os_version;
                break;
            case KM_TAG_OS_PATCHLEVEL:
                integer_ptr = &record->root_of_trust->os_patchlevel;
                break;
            case KM_TAG_ROOT_OF_TRUST:
                string_ptr = &record->root_of_trust->verified_boot_key;
                break;
            default:
                assert(false);  // Can't get here.
            }
            break;
        }

        keymaster_tag_type_t type = keymaster_tag_get_type(entry.tag);
        switch (type) {
        case KM_ENUM:
        case KM_ENUM_REP:
        case KM_UINT:
        case KM_UINT_REP: {
            assert((keymaster_tag_repeatable(entry.tag) && integer_set) ||
                   (!keymaster_tag_repeatable(entry.tag) && integer_ptr));

            UniquePtr<ASN1_INTEGER, ASN1_INTEGER_Delete> value(ASN1_INTEGER_new());
            if (!value.get())
                return KM_ERROR_MEMORY_ALLOCATION_FAILED;
            if (!ASN1_INTEGER_set(value.get(), get_uint32_value(entry)))
                return TranslateLastOpenSslError();

            insert_integer(value.release(), integer_ptr, integer_set);
            break;
        }

        case KM_ULONG:
        case KM_ULONG_REP:
        case KM_DATE: {
            assert((keymaster_tag_repeatable(entry.tag) && integer_set) ||
                   (!keymaster_tag_repeatable(entry.tag) && integer_ptr));

            UniquePtr<BIGNUM, BIGNUM_Delete> exponent(BN_new());
            if (!exponent.get())
                return KM_ERROR_MEMORY_ALLOCATION_FAILED;

            if (type == KM_DATE) {
                if (!BN_set_word(exponent.get(), entry.date_time))
                    return TranslateLastOpenSslError();
            } else {
                if (!BN_set_word(exponent.get(), entry.long_integer))
                    return TranslateLastOpenSslError();
            }

            UniquePtr<ASN1_INTEGER, ASN1_INTEGER_Delete> value(
                BN_to_ASN1_INTEGER(exponent.get(), nullptr));
            if (!value.get())
                return KM_ERROR_MEMORY_ALLOCATION_FAILED;

            insert_integer(value.release(), integer_ptr, integer_set);
            break;
        }

        case KM_BOOL:
            assert(bool_ptr);
            if (!*bool_ptr)
                *bool_ptr = ASN1_NULL_new();
            if (!*bool_ptr)
                return KM_ERROR_MEMORY_ALLOCATION_FAILED;
            break;

        /* Byte arrays*/
        case KM_BYTES:
            assert(string_ptr);
            if (!*string_ptr)
                *string_ptr = ASN1_OCTET_STRING_new();
            if (!*string_ptr)
                return KM_ERROR_MEMORY_ALLOCATION_FAILED;
            if (!ASN1_OCTET_STRING_set(*string_ptr, entry.blob.data, entry.blob.data_length))
                return TranslateLastOpenSslError();
            break;

        default:
            return KM_ERROR_UNIMPLEMENTED;
        }
    }

    return KM_ERROR_OK;
}

// Construct an ASN1.1 DER-encoded attestation record containing the values from sw_enforced and
// tee_enforced.
keymaster_error_t build_attestation_record(const AuthorizationSet& sw_enforced,
                                           const AuthorizationSet& tee_enforced,
                                           UniquePtr<uint8_t[]>* asn1_key_desc,
                                           size_t* asn1_key_desc_len) {
    assert(asn1_key_desc && asn1_key_desc_len);

    UniquePtr<KM_KEY_DESCRIPTION, KM_KEY_DESCRIPTION_Delete> key_desc(KM_KEY_DESCRIPTION_new());
    if (!key_desc.get())
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    keymaster_error_t error;

    error = build_auth_list(sw_enforced, key_desc->software_enforced);
    if (error != KM_ERROR_OK)
        return error;
    error = build_auth_list(tee_enforced, key_desc->tee_enforced);
    if (error != KM_ERROR_OK)
        return error;

    int len = i2d_KM_KEY_DESCRIPTION(key_desc.get(), nullptr);
    if (len < 0)
        return TranslateLastOpenSslError();
    *asn1_key_desc_len = len;
    asn1_key_desc->reset(new uint8_t[*asn1_key_desc_len]);
    if (!asn1_key_desc->get())
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    uint8_t* p = asn1_key_desc->get();
    len = i2d_KM_KEY_DESCRIPTION(key_desc.get(), &p);
    if (len < 0)
        return TranslateLastOpenSslError();

    return KM_ERROR_OK;
}

// Copy all enumerated values with the specified tag from stack to auth_list.
static bool get_repeated_enums(const stack_st_ASN1_INTEGER* stack, keymaster_tag_t tag,
                               AuthorizationSet* auth_list) {
    assert(keymaster_tag_get_type(tag) == KM_ENUM_REP);
    for (size_t i = 0; i < sk_ASN1_INTEGER_num(stack); ++i) {
        if (!auth_list->push_back(
                keymaster_param_enum(tag, ASN1_INTEGER_get(sk_ASN1_INTEGER_value(stack, i)))))
            return false;
    }
    return true;
}

// Add the specified integer tag/value pair to auth_list.
template <keymaster_tag_type_t Type, keymaster_tag_t Tag, typename KeymasterEnum>
static bool get_enum(const ASN1_INTEGER* asn1_int, TypedEnumTag<Type, Tag, KeymasterEnum> tag,
                     AuthorizationSet* auth_list) {
    if (!asn1_int)
        return true;
    return auth_list->push_back(tag, static_cast<KeymasterEnum>(ASN1_INTEGER_get(asn1_int)));
}

// Add the specified ulong tag/value pair to auth_list.
static bool get_ulong(const ASN1_INTEGER* asn1_int, keymaster_tag_t tag,
                      AuthorizationSet* auth_list) {
    if (!asn1_int)
        return true;
    UniquePtr<BIGNUM, BIGNUM_Delete> bn(ASN1_INTEGER_to_BN(asn1_int, nullptr));
    if (!bn.get())
        return false;
    uint64_t ulong = BN_get_word(bn.get());
    return auth_list->push_back(keymaster_param_long(tag, ulong));
}

// Extract the values from the specified ASN.1 record and place them in auth_list.
static keymaster_error_t extract_auth_list(const KM_AUTH_LIST* record,
                                           AuthorizationSet* auth_list) {
    // Purpose
    if (!get_repeated_enums(record->purpose, TAG_PURPOSE, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Algorithm
    if (!get_enum(record->algorithm, TAG_ALGORITHM, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Key size
    if (record->key_size && !auth_list->push_back(TAG_KEY_SIZE, ASN1_INTEGER_get(record->key_size)))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Block mode
    if (!get_repeated_enums(record->block_mode, TAG_BLOCK_MODE, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Digest
    if (!get_repeated_enums(record->digest, TAG_DIGEST, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Padding
    if (!get_repeated_enums(record->padding, TAG_PADDING, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Caller NONCE
    if (record->caller_nonce && !auth_list->push_back(TAG_CALLER_NONCE))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Min MAC length
    if (record->min_mac_length &&
        !auth_list->push_back(TAG_MIN_MAC_LENGTH, ASN1_INTEGER_get(record->min_mac_length)))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // EC curve
    if (!get_enum(record->ec_curve, TAG_EC_CURVE, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // RSA public exponent
    if (!get_ulong(record->rsa_public_exponent, TAG_RSA_PUBLIC_EXPONENT, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // ECIES single hash mode
    if (record->ecies_single_hash_mode && !auth_list->push_back(TAG_ECIES_SINGLE_HASH_MODE))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Blob usage requirement
    if (!get_enum(record->blob_usage_requirement, TAG_BLOB_USAGE_REQUIREMENTS, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Bootloader only
    if (record->bootloader_only && !auth_list->push_back(TAG_BOOTLOADER_ONLY))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Active date time
    if (!get_ulong(record->active_date_time, TAG_ACTIVE_DATETIME, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Origination expire date time
    if (!get_ulong(record->origination_expire_date_time, TAG_ORIGINATION_EXPIRE_DATETIME,
                   auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Usage Expire date time
    if (!get_ulong(record->usage_expire_date_time, TAG_USAGE_EXPIRE_DATETIME, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Min seconds between ops
    if (record->min_seconds_between_ops &&
        !auth_list->push_back(TAG_MIN_SECONDS_BETWEEN_OPS,
                              ASN1_INTEGER_get(record->min_seconds_between_ops)))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Max uses per boot
    if (record->max_uses_per_boot &&
        !auth_list->push_back(TAG_MAX_USES_PER_BOOT, ASN1_INTEGER_get(record->max_uses_per_boot)))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // No auth required
    if (record->no_auth_required && !auth_list->push_back(TAG_NO_AUTH_REQUIRED))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // User auth type
    if (!get_enum(record->user_auth_type, TAG_USER_AUTH_TYPE, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Auth timeout
    if (record->auth_timeout &&
        !auth_list->push_back(TAG_AUTH_TIMEOUT, ASN1_INTEGER_get(record->auth_timeout)))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // All applications
    if (record->all_applications && !auth_list->push_back(TAG_ALL_APPLICATIONS))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Application ID
    if (record->application_id &&
        !auth_list->push_back(TAG_APPLICATION_ID, record->application_id->data,
                              record->application_id->length))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Application data
    if (record->application_data &&
        !auth_list->push_back(TAG_APPLICATION_DATA, record->application_data->data,
                              record->application_data->length))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Creation date time
    if (!get_ulong(record->creation_date_time, TAG_CREATION_DATETIME, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Origin
    if (!get_enum(record->origin, TAG_ORIGIN, auth_list))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Rollback resistant
    if (record->rollback_resistant && !auth_list->push_back(TAG_ROLLBACK_RESISTANT))
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    // Root of trust
    if (record->root_of_trust) {
        KM_ROOT_OF_TRUST* rot = record->root_of_trust;
        if (!rot->verified_boot_key || !rot->os_version || !rot->os_patchlevel)
            return KM_ERROR_INVALID_KEY_BLOB;

        if (!auth_list->push_back(TAG_OS_VERSION, ASN1_INTEGER_get(rot->os_version)) ||
            !auth_list->push_back(TAG_OS_PATCHLEVEL, ASN1_INTEGER_get(rot->os_patchlevel)) ||
            !auth_list->push_back(TAG_ROOT_OF_TRUST, rot->verified_boot_key->data,
                                  rot->verified_boot_key->length))
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    return KM_ERROR_OK;
}

// Parse the DER-encoded attestation record, placing the results in software_enforced and
// tee_enforced.
keymaster_error_t parse_attestation_record(const uint8_t* asn1_key_desc, size_t asn1_key_desc_len,
                                           AuthorizationSet* software_enforced,
                                           AuthorizationSet* tee_enforced) {
    const uint8_t* p = asn1_key_desc;
    UniquePtr<KM_KEY_DESCRIPTION, KM_KEY_DESCRIPTION_Delete> record(
        d2i_KM_KEY_DESCRIPTION(nullptr, &p, asn1_key_desc_len));
    if (!record.get())
        return TranslateLastOpenSslError();

    keymaster_error_t error = extract_auth_list(record->software_enforced, software_enforced);
    if (error != KM_ERROR_OK)
        return error;

    return extract_auth_list(record->tee_enforced, tee_enforced);
}

}  // namepace keymaster
