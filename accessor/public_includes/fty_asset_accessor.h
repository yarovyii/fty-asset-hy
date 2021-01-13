/*  =========================================================================
    fty_asset_accessor

    Copyright (C) 2016 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#pragma once

#include <fty/expected.h>
#include <string>

namespace fty
{
    class Asset;

    class AssetAccessor
    {
    public:
        static fty::Expected<uint32_t> assetInameToID(const std::string& iname);
        static fty::Expected<fty::Asset> getAsset(const std::string& iname);
        // static fty::Expected<std::string> assetStatus(const std::string& iname);
        // static fty::Expected<std::string> assetType(const std::string& iname);
        // static fty::Expected<std::string> assetSubtype(const std::string& iname);
        // static fty::Expected<std::string> assetExtName(const std::string& iname);
        // static fty::Expected<std::string> assetParentIname(const std::string& iname);
        // static fty::Expected<std::string> assetPriority(const std::string& iname);
        // static fty::Expected<std::string> assetExtMap(const std::string& iname);
    };

} // namespace fty
