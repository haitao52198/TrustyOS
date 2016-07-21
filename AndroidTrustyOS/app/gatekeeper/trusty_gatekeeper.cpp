/*
 * Copyright 2015 The Android Open Source Project
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

#include <openssl/hmac.h>
#include <err.h>

extern "C" {
#include <lib/hwkey/hwkey.h>
#include <lib/keymaster/keymaster.h>
#include <lib/rng/trusty_rng.h>
#include <lib/storage/storage.h>

#include <trusty_std.h>
}

#include "trusty_gatekeeper.h"

#define CALLS_BETWEEN_RNG_RESEEDS 32
#define RNG_RESEED_SIZE           64

#define HMAC_SHA_256_KEY_SIZE 32

#define GATEKEEPER_PREFIX "gatekeeper."

#define STORAGE_ID_LENGTH_MAX 64

#define MAX_FAILURE_RECORDS 10

namespace gatekeeper {

static const uint8_t DERIVATION_DATA[HMAC_SHA_256_KEY_SIZE] = "TrustyGateKeeperDerivationData0";

TrustyGateKeeper::TrustyGateKeeper() : GateKeeper() {
    rng_initialized_ = false;
    calls_since_reseed_ = 0;
    num_mem_records_ = 0;

    SeedRngIfNeeded();
}

long TrustyGateKeeper::OpenSession() {
    if (!SeedRngIfNeeded()) {
        return ERR_NOT_READY;
    }

    return DeriveMasterKey();
}

void TrustyGateKeeper::CloseSession() {
    ClearMasterKey();
}

bool TrustyGateKeeper::SeedRngIfNeeded() {
    if (ShouldReseedRng())
        rng_initialized_ = ReseedRng();
    return rng_initialized_;
}

bool TrustyGateKeeper::ShouldReseedRng() {
    if (!rng_initialized_) {
        return true;
    }

    if (++calls_since_reseed_ % CALLS_BETWEEN_RNG_RESEEDS == 0) {
        return true;
    }
    return false;
}

bool TrustyGateKeeper::ReseedRng() {
    UniquePtr<uint8_t[]> rand_seed(new uint8_t[RNG_RESEED_SIZE]);
    memset(rand_seed.get(), 0, RNG_RESEED_SIZE);
    if (trusty_rng_secure_rand(rand_seed.get(), RNG_RESEED_SIZE) != NO_ERROR) {
        return false;
    }

    trusty_rng_add_entropy(rand_seed.get(), RNG_RESEED_SIZE);
    return true;
}

long TrustyGateKeeper::DeriveMasterKey() {
    long rc = hwkey_open();
    if (rc < 0) {
        return rc;
    }

    hwkey_session_t session = (hwkey_session_t) rc;

    master_key_.reset(new uint8_t[HMAC_SHA_256_KEY_SIZE]);

    uint32_t kdf_version = HWKEY_KDF_VERSION_1;
    rc = hwkey_derive(session, &kdf_version, DERIVATION_DATA, master_key_.get(),
            HMAC_SHA_256_KEY_SIZE);

    hwkey_close(session);
    return rc;
}

void TrustyGateKeeper::ClearMasterKey() {
    memset_s(master_key_.get(), 0, HMAC_SHA_256_KEY_SIZE);
    master_key_.reset();
}

bool TrustyGateKeeper::GetAuthTokenKey(const uint8_t **auth_token_key,
        size_t *length) const {
    *length = 0;
    *auth_token_key = NULL;

    long rc = keymaster_open();
    if (rc < 0) {
        return false;
    }

    keymaster_session_t session = (keymaster_session_t) rc;

    uint8_t *key = NULL;
    uint32_t local_length = 0;

    rc = keymaster_get_auth_token_key(session, &key, &local_length);
    keymaster_close(session);

    if (rc == NO_ERROR) {
        *auth_token_key = key;
        *length = local_length;
        return true;
    } else {
        return false;
    }
}

void TrustyGateKeeper::GetPasswordKey(const uint8_t **password_key, size_t *length) {
    *password_key = const_cast<const uint8_t *>(master_key_.get());
    *length = HMAC_SHA_256_KEY_SIZE;
}

void TrustyGateKeeper::ComputePasswordSignature(uint8_t *signature, size_t signature_length,
        const uint8_t *key, size_t key_length, const uint8_t *password,
        size_t password_length, salt_t salt) const {
    // todo: heap allocate
    uint8_t salted_password[password_length + sizeof(salt)];
    memcpy(salted_password, &salt, sizeof(salt));
    memcpy(salted_password + sizeof(salt), password, password_length);
    ComputeSignature(signature, signature_length, key, key_length, salted_password,
            password_length + sizeof(salt));
}

void TrustyGateKeeper::GetRandom(void *random, size_t requested_size) const {
    if (random == NULL) return;
    trusty_rng_secure_rand(reinterpret_cast<uint8_t*>(random), requested_size);
}

void TrustyGateKeeper::ComputeSignature(uint8_t *signature, size_t signature_length,
        const uint8_t *key, size_t key_length, const uint8_t *message,
        const size_t length) const {
    uint8_t buf[HMAC_SHA_256_KEY_SIZE];
    size_t buf_len;

    HMAC(EVP_sha256(), key, key_length, message, length, buf, &buf_len);
    size_t to_write = buf_len;
    if (buf_len > signature_length) to_write = signature_length;
    memset(signature, 0, signature_length);
    memcpy(signature, buf, to_write);
}

uint64_t TrustyGateKeeper::GetMillisecondsSinceBoot() const {
    status_t rc;
    int64_t secure_time_ns = 0;
    rc = gettime(0, 0, &secure_time_ns);
    if (rc != NO_ERROR){
        secure_time_ns = 0;
        TLOGE("%s Error:[0x%x].\n", __func__, rc);
    }
    return secure_time_ns / 1000 / 1000;
}

bool TrustyGateKeeper::GetSecureFailureRecord(uint32_t uid, secure_id_t user_id,
        failure_record_t *record) {

    storage_session_t session;
    int rc = storage_open_session(&session, STORAGE_CLIENT_TD_PORT);
    if (rc < 0) {
        TLOGE("Error: [%d] opening storage session\n", rc);
        return false;
    }

    char id[STORAGE_ID_LENGTH_MAX];
    memset(id, 0, sizeof(id));

    file_handle_t handle;
    snprintf(id, STORAGE_ID_LENGTH_MAX, GATEKEEPER_PREFIX "%u", uid);
    rc = storage_open_file(session, &handle, id, 0, 0);
    if (rc < 0) {
        TLOGE("Error:[%d] opening storage object.\n", rc);
        storage_close_session(session);
        return false;
    }

    failure_record_t owner_record;
    rc = storage_read(handle, 0, &owner_record, sizeof(owner_record));
    storage_close_file(handle);
    storage_close_session(session);

    if (rc < 0) {
        TLOGE("Error:[%d] reading storage object.\n", rc);
        return false;
    }

    if ((size_t) rc < sizeof(owner_record)) {
        TLOGE("Error: invalid object size [%d].\n", rc);
        return false;
    }

    if (owner_record.secure_user_id != user_id) {
        TLOGE("Error:[%llu != %llu] secure storage corrupt.\n",
                owner_record.secure_user_id, user_id);
        return false;
    }

    *record = owner_record;
    return true;
}

bool TrustyGateKeeper::GetFailureRecord(uint32_t uid, secure_id_t user_id, failure_record_t *record,
        bool secure) {
    if (secure) {
        return GetSecureFailureRecord(uid, user_id, record);
    } else {
        return GetMemoryRecord(user_id, record);
    }
}

bool TrustyGateKeeper::ClearFailureRecord(uint32_t uid, secure_id_t user_id, bool secure) {
    failure_record_t record;
    record.secure_user_id = user_id;
    record.last_checked_timestamp = 0;
    record.failure_counter = 0;
    return WriteFailureRecord(uid, &record, secure);
}

bool TrustyGateKeeper::WriteSecureFailureRecord(uint32_t uid, failure_record_t *record) {
    storage_session_t session;
    int rc = storage_open_session(&session, STORAGE_CLIENT_TD_PORT);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage session\n", rc);
        return false;
    }

    char id[STORAGE_ID_LENGTH_MAX];
    memset(id, 0, sizeof(id));
    snprintf(id, STORAGE_ID_LENGTH_MAX, GATEKEEPER_PREFIX "%u", uid);

    file_handle_t handle;
    rc = storage_open_file(session, &handle, id, STORAGE_FILE_OPEN_CREATE, 0);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage object %s\n", rc, id);
        storage_close_session(session);
        return false;
    }

    rc = storage_write(handle, 0, record, sizeof(*record), STORAGE_OP_COMPLETE);
    storage_close_file(handle);
    storage_close_session(session);

    if (rc < 0) {
        TLOGE("Error:[%d] writing storage object.\n", rc);
        return false;
    }

    if ((size_t) rc < sizeof(*record)) {
        TLOGE("Error: invalid object size [%d].\n", rc);
        return false;
    }

    return true;
}

bool TrustyGateKeeper::WriteFailureRecord(uint32_t uid, failure_record_t *record, bool secure) {
    if (secure) {
        return WriteSecureFailureRecord(uid, record);
    } else {
        return WriteMemoryRecord(record);
    }
}

bool TrustyGateKeeper::IsHardwareBacked() const {
    return true;
}

void TrustyGateKeeper::InitMemoryRecords() {
    if (!mem_records_.get()) {
        failure_record_t *mem_recs = new failure_record_t[MAX_FAILURE_RECORDS];
        memset(mem_recs, 0, sizeof(*mem_recs));
        mem_records_.reset(mem_recs);
        num_mem_records_ = 0;
    }
}

bool TrustyGateKeeper::GetMemoryRecord(secure_id_t user_id, failure_record_t *record) {
    InitMemoryRecords();

    for (int i = 0; i < num_mem_records_; i++) {
        if (mem_records_[i].secure_user_id == user_id) {
            *record = mem_records_[i];
            return true;
        }
    }

    record->secure_user_id = user_id;
    record->failure_counter = 0;
    record->last_checked_timestamp = 0;

    return true;
}

bool TrustyGateKeeper::WriteMemoryRecord(failure_record_t *record) {
    InitMemoryRecords();

    int idx = 0;
    int min_idx = 0;
    uint64_t min_timestamp = ~0ULL;
    for (idx = 0; idx < num_mem_records_; idx++) {
        if (mem_records_[idx].secure_user_id == record->secure_user_id) {
            break;
        }

        if (mem_records_[idx].last_checked_timestamp <= min_timestamp) {
            min_timestamp = mem_records_[idx].last_checked_timestamp;
            min_idx = idx;
        }
    }

    if (idx >= MAX_FAILURE_RECORDS) {
        // replace oldest element
        idx = min_idx;
    } else if (idx == num_mem_records_) {
        num_mem_records_++;
    }

    mem_records_[idx] = *record;
    return true;
}

}
