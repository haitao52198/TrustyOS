/*
 * Copyright (C) 2012-2013 The Android Open Source Project
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

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	/* UUID : {5f902ace-5e5c-4cd8-ae54-87b88c22ddaf} */
	{ 0x5f902ace, 0x5e5c, 0x4cd8,
		{ 0xae, 0x54, 0x87, 0xb8, 0x8c, 0x22, 0xdd, 0xaf } },

	/* optional configuration options here */
	{
		/* openssl need a larger heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(24 * 4096),

		/* openssl need a larger stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(8 * 4096),
	},
};
