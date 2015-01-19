#!/bin/sh

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Michal Hrusecky <MichalHrusecky@eaton.com>
#
# Description: This is library of functions usefull for REST API testing,
#              which can be sourced to interactive shell

[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="@PASSWORD@"
[ -z "$BASE_URL" ] && BASE_URL="http://127.0.0.1:8000/api/v1"

_TOKEN_=""

print_result() {
    _ret=0
    if [ "$1" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        echo " * FAILED"
        _ret=1
        FAILED="$FAILED $NAME"
    fi
    TOTAL="`expr $TOTAL + 1`"
    echo
    return $_ret
}

test_it() {
    if [ "$1" ]; then
        NAME="$1"
    fi
    [ "$NAME" ] || NAME="`basename "$0" .sh`"
    echo "Running test $NAME:"
}

api_get() {
    curl -v --progress-bar "$BASE_URL$1" 2>&1
}

api_get_content() {
    curl "$BASE_URL$1" 2>/dev/null
}

api_get_json() {
    curl -v --progress-bar "$BASE_URL$1" 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
}

api_get_jsonv() {
    api_get_json "$@" | python -c "import sys, json; s=sys.stdin.read(); json.loads(s); print(s)"
}

api_post() {
    curl -v -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

_api_get_token() {
    if [ -z "$_TOKEN_" ]; then
	AUTH_URL="/oauth2/token?username=${BIOS_USER}&password=${BIOS_PASSWD}&grant_type=password"
	_TOKEN_RAW_="`api_get "$AUTH_URL"`"
	_TOKEN_="`echo "$_TOKEN_RAW_" | sed -n 's|.*\"access_token\"[[:blank:]]*:[[:blank:]]*\"\([^\"]*\)\".*|\1|p'`"
    fi
    echo "$_TOKEN_"
}

api_auth_post() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_post_content() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    curl -H "Authorization: Bearer $TOKEN" -d "$2" "$BASE_URL$1" 2>/dev/null
}

api_auth_post_wToken() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    curl -v -d "access_token=$TOKEN&$2" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_post_content_wToken() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    curl -d "access_token=$TOKEN&$2" "$BASE_URL$1" 2>/dev/null
}

api_auth_delete() {
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -X "DELETE" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_put() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	PUT data
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" -X "PUT" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_get() {
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_get_wToken() {
    TOKEN="`_api_get_token`"
    URLSEP='?'
    case "$1" in
        *"?"*) URLSEP='&' ;;
    esac
    curl -v --progress-bar "$BASE_URL$1$URLSEP""access_token=$TOKEN" 2>&1
}

api_auth_get_content() {
    TOKEN="`_api_get_token`"
    curl -H "Authorization: Bearer $TOKEN" "$BASE_URL$1" 2>/dev/null
}

api_auth_get_content_wToken() {
    TOKEN="`_api_get_token`"
    URLSEP='?'
    case "$1" in
        *"?"*) URLSEP='&' ;;
    esac
    curl "$BASE_URL$1$URLSEP""access_token=$TOKEN" 2>/dev/null
}

api_auth_get_json() {
    TOKEN="`_api_get_token`"
    curl -v --progress-bar -H "Authorization: Bearer $TOKEN" "$BASE_URL$1" 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
}

api_auth_get_jsonv() {
    TOKEN="`_api_get_token`"
    api_auth_get_json "$@" | python -c "import sys, json; s=sys.stdin.read(); json.loads(s); print(s)"
}

