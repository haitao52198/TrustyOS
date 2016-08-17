/*
 * Copyright (C) 2013 The Android Open Source Project
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>
#include <elastos.h>
#include <elapi.h>

int main(void)
{

     AutoPtr<IModuleInfo> moduleInfo;
     ECode ec = _CReflector_AcquireModuleInfo(String("path"), (IModuleInfo**)&moduleInfo);
     if (FAILED(ec)) {
        assert(0);
    }
/*
    Int32 clsCount;
    moduleInfo->GetClassCount(&clsCount);
    BufferOf<IClassInfo*>* buf = BufferOf<IClassInfo*>::Alloc(clsCount);
    moduleInfo->GetAllClassInfos(buf);

    AutoPtr<IClassInfo> clsInfo;
    Int32 i = 0;
    for (; i < clsCount; i++) {
        ClassID id;
        id.pUunm = (char*)malloc(80);
        (*buf)[i]->GetId(&id);
        if (id == clsId) {
            clsInfo = (*buf)[i];
        }
        free(id.pUunm);
    }
*/
	printf("Hello world from Elastos Runtime app\n");

	return 0;
}
