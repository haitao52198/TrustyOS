/*
 * Copyright (C) 2015 The Android Open Source Project
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
#include <stddef.h>
#include <trusty_app_manifest.h>

// TODO: generate my own UUID
trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{

    /* UUID : {38ba0cdc-df0e-11e4-9869-233fb6ae4795} */
    { 0x38ba0cdc, 0xdf0e, 0x11e4,
        { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } },

	/* optional configuration options here */
	{
		/* openssl need a larger heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(2 * 4096),

		/* openssl need a larger stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(1 * 4096),
	},
};
