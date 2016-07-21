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

#include <fstream>

#include <gtest/gtest.h>

#include "android_keymaster_test_utils.h"
#include "attestation_record.h"

namespace keymaster {
namespace test {

TEST(AttestTest, Simple) {
    AuthorizationSet hw_set(AuthorizationSetBuilder()
                                .RsaSigningKey(512, 3)
                                .Digest(KM_DIGEST_SHA_2_256)
                                .Digest(KM_DIGEST_SHA_2_384)
                                .Authorization(TAG_ROOT_OF_TRUST, "foo", 3)
                                .Authorization(TAG_OS_VERSION, 60000)
                                .Authorization(TAG_OS_PATCHLEVEL, 201512)
                                .Authorization(TAG_APPLICATION_ID, "bar", 3));
    AuthorizationSet sw_set(AuthorizationSetBuilder().Authorization(TAG_ACTIVE_DATETIME, 10));

    UniquePtr<uint8_t[]> asn1;
    size_t asn1_len;
    EXPECT_EQ(KM_ERROR_OK, build_attestation_record(sw_set, hw_set, &asn1, &asn1_len));
    EXPECT_GT(asn1_len, 0U);

    std::ofstream output("attest.der",
                         std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    if (output)
        output.write(reinterpret_cast<const char*>(asn1.get()), asn1_len);
    output.close();

    AuthorizationSet parsed_hw_set;
    AuthorizationSet parsed_sw_set;
    EXPECT_EQ(KM_ERROR_OK,
              parse_attestation_record(asn1.get(), asn1_len, &parsed_sw_set, &parsed_hw_set));

    hw_set.Sort();
    sw_set.Sort();
    parsed_hw_set.Sort();
    parsed_sw_set.Sort();
    EXPECT_EQ(hw_set, parsed_hw_set);
    EXPECT_EQ(sw_set, parsed_sw_set);
}

}  // namespace test
}  // namespace keymaster
