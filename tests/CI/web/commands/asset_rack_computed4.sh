
#
# Copyright (C) 2015 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


#! \file asset_rack_computed4.sh
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* asset_rack_computed4.sh ****************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
test_it "asset_rack_computed4::Prerequisites"
init_script
print_result $?

test_it "asset_rack_computed4"
api_get_json /asset/4 >&5
print_result $?

logmsg_warn "This test can fail with zero vs. null values, see http://jira.mbt.lab.etn.com/browse/BIOS-2283 in this case"

echo
echo "###################################################################################################"
echo "********* asset_rack_computed4.sh ******************* END *****************************************"
echo "###################################################################################################"
echo