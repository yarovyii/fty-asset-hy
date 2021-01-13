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

#include "asset/conversion/full-asset.h"
#include "asset/conversion/json.h"

#include <fty_asset_dto.h>
#include <fty_common_asset.h>
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

        return fty::convert<uint32_t>(ret.userData().front());
    }

    fty::Expected<fty::FullAsset> AssetAccessor::getFullAsset(const std::string& iname)
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

        fty::Asset asset;

        fty::conversion::fromJson(ret.userData().front(), asset);
        
        return fty::conversion::toFullAsset(asset);
    }

    // fty::Expected<std::string> AssetAccessor::assetStatus(const std::string& iname)
    // {
    //     messagebus::Message ret;

    //     try
    //     {
    //         ret = sendCommand("GET", {iname});
    //     }
    //     catch (messagebus::MessageBusException &e)
    //     {
    //         return fty::unexpected("MessageBus request failed: {}", e.what());
    //     }

    //     if (ret.metaData().at(messagebus::Message::STATUS) != messagebus::STATUS_OK)
    //     {
    //         return fty::unexpected("Request of ID from iname failed");
    //     }

    //     fty::Asset asset;

    //     fromJson(ret.userData().front(), asset);
        
    //     return asset.getAssetStatus();
    // }

    // fty::Expected<std::string> AssetAccessor::assetType(const std::string& iname)
    // {
        
    // }

    // fty::Expected<std::string> AssetAccessor::assetSubtype(const std::string& iname)
    // {

    // }

    // fty::Expected<std::string> AssetAccessor::assetExtName(const std::string& iname)
    // {

    // }

    // fty::Expected<std::string> AssetAccessor::assetParentIname(const std::string& iname)
    // {

    // }

    // fty::Expected<std::string> AssetAccessor::assetPriority(const std::string& iname)
    // {

    // }

    // fty::Expected<std::string> AssetAccessor::assetExtMap(const std::string& iname)
    // {

    // }

} // namespace fty
