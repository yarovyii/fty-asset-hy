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

#include "fty_asset_accessor.h"

#include <cxxtools/serializationinfo.h>

#include <fty_common.h>
#include <fty_common_messagebus.h>
#include <fty/convert.h>
#include <iostream>

#define RECV_TIMEOUT 5  // messagebus request timeout

namespace fty
{
    static constexpr const char *ASSET_AGENT = "asset-agent-ng";
    static constexpr const char *ASSET_AGENT_QUEUE = "FTY.Q.ASSET.QUERY";
    static constexpr const char *ACCESSOR_NAME = "fty-asset-accessor";
    static constexpr const char *ENDPOINT = "ipc://@/malamute";

    /// static helper to send a MessageBus command
    static messagebus::Message sendCommand(const std::string& command, messagebus::UserData data)
    {
        std::unique_ptr<messagebus::MessageBus> interface(messagebus::MlmMessageBus(ENDPOINT, ACCESSOR_NAME));
        messagebus::Message msg;

        interface->connect();

        msg.metaData().emplace(messagebus::Message::CORRELATION_ID, messagebus::generateUuid());
        msg.metaData().emplace(messagebus::Message::SUBJECT, command);
        msg.metaData().emplace(messagebus::Message::FROM, ACCESSOR_NAME);
        msg.metaData().emplace(messagebus::Message::TO, ASSET_AGENT);
        msg.metaData().emplace(messagebus::Message::REPLY_TO, ACCESSOR_NAME);

        msg.userData() = data;

        return interface->request(ASSET_AGENT_QUEUE, msg, RECV_TIMEOUT);
    }

    /// returns the asset database ID, given the internal name
    fty::Expected<uint32_t> AssetAccessor::assetInameToID(const std::string &iname)
    {
        messagebus::Message ret;

        try
        {
            ret = sendCommand("GET_ID", {iname});
        }
        catch (messagebus::MessageBusException &e)
        {
            return fty::unexpected("MessageBus request failed: {}", e.what());
        }

        if (ret.metaData().at(messagebus::Message::STATUS) != messagebus::STATUS_OK)
        {
            return fty::unexpected("Request of ID from iname failed");
        }

        cxxtools::SerializationInfo si;
        JSON::readFromString(ret.userData().front(), si);

        std::string data;

        si >>= data;

        return fty::convert<uint32_t>(data);
    }

    /// returns the full fty::Asset, given the internal name
    fty::Expected<fty::Asset> AssetAccessor::getAsset(const std::string& iname)
    {
        messagebus::Message ret;

        try
        {
            ret = sendCommand("GET", {iname});
        }
        catch (messagebus::MessageBusException &e)
        {
            return fty::unexpected("MessageBus request failed: {}", e.what());
        }

        if (ret.metaData().at(messagebus::Message::STATUS) != messagebus::STATUS_OK)
        {
            return fty::unexpected("Request of fty::FullAsset from iname failed");
        }

        Asset asset;
        fty::Asset::fromJson(ret.userData().front(), asset);

        return asset;
    }

    /// triggers an update notification. It receives the DTOs of the asset before and after the update
    void AssetAccessor::notifyAssetUpdate(const Asset& oldAsset, const Asset& newAsset)
    {
        messagebus::Message ret;

        cxxtools::SerializationInfo si;

        // before update
        cxxtools::SerializationInfo tmpSi;
        tmpSi <<= oldAsset;

        cxxtools::SerializationInfo& before = si.addMember("");
        before.setCategory(cxxtools::SerializationInfo::Category::Object);
        before = tmpSi;
        before.setName("before");

        // after update
        tmpSi.clear();
        tmpSi <<= newAsset;

        cxxtools::SerializationInfo& after = si.addMember("");
        after.setCategory(cxxtools::SerializationInfo::Category::Object);
        after = tmpSi;
        after.setName("after");

        std::string json = JSON::writeToString(si, false);

        ret = sendCommand("NOTIFY", {json});
    }
} // namespace fty
