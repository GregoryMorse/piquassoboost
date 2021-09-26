/**
 * Copyright 2021 Budapest Quantum Computing Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <PicState.h>


namespace pic {



/** @brief Call to convert a piquassoboost state from PicState_int64 to PicState_int type
 *  @param state_int64 The state to convert
 *  @returns with the converted state of type PicState_int
 */
PicState_int convert_PicState_int64_to_PicState_int(PicState_int64& state_int64){
    size_t size = state_int64.size();

    PicState_int state_int(size);
    int64_t *data_int64 = state_int64.get_data();
    int *data_int = state_int.get_data();
    for (size_t idx = 0; idx < size; idx++){
        data_int[idx] = data_int64[idx];
    }

    // number of photons has to be set manually
    state_int.number_of_photons = state_int64.number_of_photons;

    return state_int;
}

} // PIC

