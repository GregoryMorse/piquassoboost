#
#  Copyright 2021 Budapest Quantum Computing Group
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import re

import pytest

import piquassoboost as pqb


@pytest.fixture(autouse=True)
def _patch(request):
    regexp = re.compile(f"{re.escape(str(request.config.rootdir))}\/(.+?)\/(.*)")

    result = regexp.search(str(request.fspath))

    if result.group(1) == "piquasso-module":
        # NOTE: Only override the simulators, when the origin Piquasso Python tests are
        # executed. For tests originating in PiquassoBoost, handle everything manually!
        pqb.patch()
