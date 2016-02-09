/*  =========================================================================
    agent-asset - generated layer of public API

    Copyright (C) 2014 - 2015 Eaton                                        
                                                                           
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

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Please refer to the README for information about making permanent changes.  #
################################################################################
    =========================================================================
*/

#ifndef AGENT_ASSET_LIBRARY_H_INCLUDED
#define AGENT_ASSET_LIBRARY_H_INCLUDED

//  Set up environment for the application

//  External dependencies
#include <malamute.h>
#include <bios/bios_agent.h>
#include <bios/agents.h>
#include <biosproto.h>

//  AGENT_ASSET version macros for compile-time API detection

#define AGENT_ASSET_VERSION_MAJOR 0
#define AGENT_ASSET_VERSION_MINOR 1
#define AGENT_ASSET_VERSION_PATCH 0

#define AGENT_ASSET_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define AGENT_ASSET_VERSION \
    AGENT_ASSET_MAKE_VERSION(AGENT_ASSET_VERSION_MAJOR, AGENT_ASSET_VERSION_MINOR, AGENT_ASSET_VERSION_PATCH)

#if defined (__WINDOWS__)
#   if defined LIBAGENT_ASSET_STATIC
#       define AGENT_ASSET_EXPORT
#   elif defined LIBAGENT_ASSET_EXPORTS
#       define AGENT_ASSET_EXPORT __declspec(dllexport)
#   else
#       define AGENT_ASSET_EXPORT __declspec(dllimport)
#   endif
#else
#   define AGENT_ASSET_EXPORT
#endif

//  Opaque class structures to allow forward references
//  These classes are stable or legacy and built in all releases


//  Public classes, each with its own header file

#endif
/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Please refer to the README for information about making permanent changes.  #
################################################################################
*/
